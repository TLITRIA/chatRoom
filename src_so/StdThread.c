#include <stdio.h>
#include <pthread.h>
#include "StdThread.h"
#include <stdlib.h>

/* 线程结构体，只有线程号 */
struct StdThread
{
    pthread_t id;
};

/* 开辟线程 */
Thread *InitThread(void *(funcptr)(void *), void *arg)
{
    Thread *t = (Thread *)malloc(sizeof(Thread));
    if (t == NULL)
    {
        printf("malloc失败");
        return NULL;
    }
    if (pthread_create(&t->id, NULL, funcptr, arg) != 0)
    {
        perror("pthread_create:");
        free(t); // 释放注册空间
        return NULL;
    }
    return t;
}

/* 比较线程与线程结构体的线程是否一致 */
bool IsSameThread(Thread *t, unsigned long ID)
{
    return t->id = ID;
}

/* 回收线程 */
void *ThreadJoin(Thread *t)
{
    void *value;
    pthread_join(t->id, &value);// 线程分离后不需要使用pthread_join
    return value;
}

/* 线程分离 */
void ThreadDetach(Thread *t)
{
    if (pthread_detach(t->id) != 0)
    {
        perror("pthread_detach:");
    }
}

/* 线程取消 */
void ThreadCancel(Thread *t)
{
    if (pthread_cancel(t->id) != 0) // 什么场景使用暂不清楚
    {
        perror("pthread_cancel:");
    }
}

/* 锁结构体， 只有锁 */
struct StdMutex
{
    pthread_mutex_t mutex;
};

/* 初始化锁 */
Mutex *InitMutex()
{
    Mutex *m = (Mutex *)malloc(sizeof(Mutex));
    if (m == NULL)
    {
        printf("初始化锁失败");
        return NULL;
    }
    if (pthread_mutex_init(&m->mutex, NULL) != 0)
    {
        perror("pthread mutex init:");
        free(m);
        return NULL;
    }
    return m;
}

/* 加锁 */
void MutexLock(Mutex *m)
{
    pthread_mutex_lock(&m->mutex);
}
/* 解锁 */
void MutexUnlock(Mutex *m)
{
    pthread_mutex_unlock(&m->mutex);
}
/* 删除锁 */
void ClearMutex(Mutex *m)
{
    pthread_mutex_destroy(&m->mutex);
    free(m);
    m = NULL;
}
/* 条件变量结构体 */
struct StdPthreadCond
{
    pthread_cond_t cond;
};
/* 创建条件变量 */
Cond *InitThreadCond()
{
    Cond *c = (Cond *)malloc(sizeof(Cond));
    if (c == NULL)
    {
        printf("Init malloc error!\n");
        return NULL;
    }
    if (pthread_cond_init(&c->cond, NULL) != 0)
    {
        free(c);
        return NULL;
    }
    return c;
}

/* 条件变量等待 */
void CondWait(Cond *c, Mutex *m)
{
    pthread_cond_wait(&c->cond, &m->mutex);
}
/* 条件变量单播 */
void CondSignal(Cond *c)
{
    pthread_cond_signal(&c->cond);
}
/* 条件变量销毁 */
void ClearCond(Cond *c)
{
    pthread_cond_destroy(&c->cond);
    free(c);
    c == NULL;
}
