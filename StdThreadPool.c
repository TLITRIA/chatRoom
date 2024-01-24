#include "StdThreadPool.h"
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <pthread.h>
#include "LinkQueue.h"
#include "DoubleLinkList.h"
#include "StdThread.h"
#include <stdbool.h>
#define Sleep_time 10

struct PoolTask
{
      void* (*func)(void *);//线程任务函数
      void * arg;//函数参数

};

typedef struct PoolTask task;

task * CreateTask(void* (*func)(void *),void *arg)
{
    task *t =(task*)malloc(sizeof(task));
    if(t== NULL)
    {
        printf("task malloc error\n");
        return NULL;
    }
    t->func = func;
    t->arg = arg;
    return t;
}

void FreeTask(task *t)
{
    free(t);
}
struct StdThreadPool
{
    DLlist threads;
    LQueue taskQueue;
    int max_thrd_num;
    int min_thrd_num;
    int max_queue_size;

    int bsy_thrd_num;//正在忙的线程
    int shutdown;//线程池标志位
    int kill_thrd_num; //自杀线程数量
    Mutex* pool_mutex;
    Mutex * bsy_num_mutex;//控制忙线程数量的锁
    Cond * queue_not_empty;//任务队列不为空条件变量
    Cond* queue_not_full;//任务队列不为满的条件变量

     ThreadP *manager;//管理者线程
};

bool  CheckThreadSame(void *ptr1,void *ptr2)
{
    pthread_t pthreadID = (pthread_t) ptr1;
    Thread *t = (Thread *)ptr2;
    return IsSameThread(t,pthreadID);
}
void * thread_handler(void *arg)
{
    ThreadP *p = (ThreadP *)arg;
    while(1)
    {
            MutexLock(p->pool_mutex);
            while(IsQueueEmpty(&p->taskQueue) == true && p->shutdown == false)
            {
                CondWait(p->queue_not_empty,p->pool_mutex);
                
                //线程自杀
                 if(p->kill_thrd_num > 0)
                 {
                    RemoveByElement(&p->threads,(void *)pthread_self(),CheckThreadSame);
                    p->kill_thrd_num--;
                    MutexUnlock(p->pool_mutex);
                    //线程退出        
                    pthread_exit(NULL);
                 }
            }
            //线程池关闭，退出线程，不干活了
            if(p->shutdown == true)
            {
                   MutexUnlock(p->pool_mutex);
                   pthread_exit(NULL);
            }
            task *t = (task*)*GetFront(&p->taskQueue);
            QPop(&p->taskQueue);
            
            CondSignal(p->queue_not_full);
            
            MutexUnlock(p->pool_mutex);
           
           MutexLock(p->bsy_num_mutex);
            p->bsy_thrd_num++;
            MutexUnlock(p->bsy_num_mutex);
            
             t->func(t->arg);//执行线程任务
            FreeTask(t);
        
            MutexLock(p->bsy_num_mutex);
            p->bsy_thrd_num--;
            MutexUnlock(p->bsy_num_mutex);

    }
}

void * thread_manager(void *arg)
{
    ThreadP *p = (ThreadP*)arg;
    while(p->shutdown != true)
    {
         sleep(Sleep_time);
         MutexLock(p->pool_mutex);

         //线程少了，创建新的线程
        int queueLen = GetQueueLen(&p->taskQueue);
        //条件：忙线程数量小于任务数量，且线程数量没有超过最大值
        if(p->bsy_thrd_num < GetQueueLen(&p->taskQueue) && p->threads.len < p->max_thrd_num)
        {
            int addnum = (queueLen - p->bsy_thrd_num)/2;
            for(int i = 0; i< addnum;i++)
            {
                Thread *t = InitThread(thread_handler,p);
                InsertDLlistTail(&p->threads,t);
            }
        }
        //线程多了
        if(p->bsy_thrd_num *2 < p->threads.len && p->threads.len >p->min_thrd_num)
        {
            int minusNum = (p->threads.len-p->bsy_thrd_num)/2;
           p->kill_thrd_num = minusNum;
            for(int i = 0; i < minusNum;i++)
            {
                CondSignal(p->queue_not_empty);
            }
        }
        MutexUnlock(p->pool_mutex);
    }
}
ThreadP * InitThreadPool(int max_thrd_num,int min_thrd_num,int max_queue_size)
{
      ThreadP *pool = (ThreadP *)malloc(sizeof(ThreadP));
      if(pool == NULL)
      {
        printf("init malloc error!\n");
        return NULL;
      }
      pool->max_thrd_num = max_thrd_num;
      pool->min_thrd_num = min_thrd_num;
      pool->max_queue_size = max_queue_size;
      pool->shutdown = false;
      pool->bsy_thrd_num = 0;
      pool->kill_thrd_num = 0;
      
      //初始化列表
      InitDLlist(&pool->threads);
       //初始化任务队列
      InitLinkQueue(&pool->taskQueue);
      //初始化线程池锁
      pool->pool_mutex = InitMutex();
      //初始化忙线程数量锁
      pool->bsy_num_mutex = InitMutex();
      //初始化队列不为空的条件变量
      pool->queue_not_empty = InitThreadCond();
     //初始化队列不为满的条件变量
       pool->queue_not_full = InitThreadCond();
      for(int i = 0; i< max_thrd_num;i++)
      {
        Thread *t = InitThread(thread_handler,pool);
        //将创建好的线程放入线程列表中
        InsertDLlistTail(&pool->threads,t);
      }
      
      //初始化管理者线程
      pool->manager = (ThreadP*)InitThread(thread_manager,pool);
      return pool;
}

void AddpoolTask(ThreadP *pool,void *(funcPtr)(void *),void *arg)
{
    MutexLock(pool->pool_mutex);
    //当任务队列为满
    while(GetQueueLen(&pool->taskQueue) >= pool->max_queue_size &&pool->shutdown == false)
    {
        CondWait(pool->queue_not_full,pool->pool_mutex);
    }
    if(pool->shutdown == true)
    {
        MutexUnlock(pool->pool_mutex);
        return ;
    }
    QPush(&pool->taskQueue,CreateTask(funcPtr,arg));
    CondSignal(pool->queue_not_empty);
    
    MutexUnlock(pool->pool_mutex);
}
 

 void JoinThreadCallBack(void *arg)
 {
    Thread * t  = (Thread*)arg;
    ThreadJoin(t);
 }
void ClearThreadPool(ThreadP *p)
{
    if(p == NULL)
           return;
    p->shutdown = true;

    //管理者线程先结束
    ThreadJoin((Thread*)p->manager);
    free(p->manager);
    //要杀死的线程数量置为0
    p->kill_thrd_num = 0;
    //唤醒休眠线程结束掉
    int sleepThread = p->threads.len - p->bsy_thrd_num;
    for(int i  = 0; i < sleepThread; i++)
    {
        CondSignal(p->queue_not_full);
    }
    //依次等待线程结束进行回收
    Travel(&p->threads,JoinThreadCallBack);
      //清理线程列表
    FreeDLlist(&p->threads,NULL);
     //清理任务队列
    while(IsQueueEmpty(&p->taskQueue) == false)
    {
          void *task = GetFront(&p->taskQueue);
          
          QPop(&p->taskQueue);
    }

    FreeQueue(&p->taskQueue);

    //清理锁和条件变量
    ClearMutex(p->bsy_num_mutex);
    ClearMutex(p->pool_mutex);
    ClearCond(p->queue_not_empty);
    ClearCond(p->queue_not_full);
}