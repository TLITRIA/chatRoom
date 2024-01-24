#ifndef __LINKQUEUE_H_
#define __LINKQUEUE_H_
#include "DoubleLinkList.h"

struct LinkQueue
{
    DLlist list;
};

typedef struct LinkQueue LQueue;

void InitLinkQueue(LQueue* q);

void QPush(LQueue* q, ElementType element);

void QPop(LQueue* q);

ElementType* GetFront(LQueue* q);

void FreeQueue(LQueue *q);
bool IsQueueEmpty(LQueue* q);

int GetQueueLen(LQueue* q);

#endif