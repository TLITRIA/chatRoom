#ifndef __DOUNLELINKLIST_H_
#define __DOUNLELINKLIST_H_
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#define ElementType void *

struct Node
{
    ElementType value;
    struct Node *next; // 后驱指针
    struct Node *prev; // 前驱指针
};

struct DoubleLinkList
{
    struct Node *head; // 头指针
    struct Node *tail; // 尾指针
    int len;
};
typedef struct DoubleLinkList DLlist;

void InitDLlist(DLlist *list);
// 尾插
void InsertDLlistTail(DLlist *list, ElementType element);

void InsertDLlistHead(DLlist *list, ElementType element);

void InsertByIndex(DLlist *list, int index, ElementType element);

void RemoveByIndex(DLlist *list, int index);

void RemoveByElement(DLlist *list, ElementType element, bool (*funcPtr)(ElementType, ElementType));

DLlist FindByElement(DLlist *list, ElementType element, bool (*funcPtr)(ElementType, ElementType));
// 当链表中的只需要释放时，传入释放函数的函数指针
void FreeDLlist(DLlist *list, void (*funcPtr)(ElementType));

void Travel(DLlist *list, void (*funcPtr)(ElementType));

void ReTravel(DLlist *list, void (*funcPtr)(ElementType));

#endif