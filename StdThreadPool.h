#ifndef __STDTHREADPOOL_H_
#define __STDTHREADPOOL_H_
struct StdThreadPool;
typedef struct StdThreadPool ThreadP;

ThreadP * InitThreadPool(int max_thrd_num,int min_thrd_num,int max_queue_size);
void AddpoolTask(ThreadP *pool,void *(funcPtr)(void *),void *arg);
void ClearThreadPool(ThreadP *p);
#endif