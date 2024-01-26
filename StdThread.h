#ifndef __STDTHREAD_H_
#define __STDTHREAD_H_
#include <stdbool.h>
struct StdThread;
typedef struct StdThread Thread;
Thread *InitThread(void *(funcptr)(void *), void *arg);
bool IsSameThread(Thread *t, unsigned long ID);
void *ThreadJoin(Thread *t);
void ThreadDetach(Thread *t);
void ThreadCancel(Thread *t);

struct StdMutex;
typedef struct StdMutex Mutex;
void MutexLock(Mutex *m);
Mutex *InitMutex();
void MutexUnlock(Mutex *m);
void ClearMutex(Mutex *m);

struct StdPthreadCond;
typedef struct StdPthreadCond Cond;
Cond *InitThreadCond();
void CondWait(Cond *c, Mutex *m);

void CondSignal(Cond *c);

void ClearCond(Cond *c);
#endif