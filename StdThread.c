#include <stdio.h>
#include <pthread.h>
#include "StdThread.h"
#include <stdlib.h>
struct StdThread
{
    pthread_t id;
};

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
        free(t);
        return NULL;
    }
    return t;
}

bool IsSameThread(Thread *t, unsigned long ID)
{
    return t->id = ID;
}

void *ThreadJoin(Thread *t)
{
    void *value;
    pthread_join(t->id, &value);
    return value;
}

void ThreadDetach(Thread *t)
{
    if (pthread_detach(t->id) != 0)
    {
        perror("pthread_detach:");
    }
}

void ThreadCancel(Thread *t)
{
    if (pthread_cancel(t->id) != 0)
    {
        perror("pthread_cancel:");
    }
}

struct StdMutex
{
    pthread_mutex_t mutex;
};

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
void MutexLock(Mutex *m)

{
    pthread_mutex_lock(&m->mutex);
}
void MutexUnlock(Mutex *m)
{
    pthread_mutex_unlock(&m->mutex);
}
void ClearMutex(Mutex *m)
{
    pthread_mutex_destroy(&m->mutex);
    free(m);
    m = NULL;
}

struct StdPthreadCond
{
    pthread_cond_t cond;
};

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

void CondWait(Cond *c, Mutex *m)
{
    pthread_cond_wait(&c->cond, &m->mutex);
}

void CondSignal(Cond *c)
{
    pthread_cond_signal(&c->cond);
}

void ClearCond(Cond *c)
{
    pthread_cond_destroy(&c->cond);
    free(c);
    c == NULL;
}