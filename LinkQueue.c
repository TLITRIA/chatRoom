#include "LinkQueue.h"

void InitLinkQueue(LQueue *q)
{
    InitDLlist(&q->list);
}

void QPush(LQueue *q, ElementType element)
{
    InsertDLlistTail(&q->list, element);
}



void QPop(LQueue *q)
{
    RemoveByIndex(&q->list,0);
}

ElementType* GetFront(LQueue* q)
{
    if(q->list.len == 0)
                   return NULL;
    return &q->list.head->next->value;
}

bool IsQueueEmpty(LQueue *q)
{
    return q->list.len == 0;
}

int GetQueueLen(LQueue *q)
{
    return q->list.len;
}

void FreeQueue(LQueue *q)
{
    FreeDLlist(&q->list,NULL);
}