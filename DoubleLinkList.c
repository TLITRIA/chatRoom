#include "DoubleLinkList.h"

// 创建一个新节点
struct Node *CreateNode(ElementType element)
{
    struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
    if (newNode == NULL)
    {
        printf("CreateNode malloc error!\n");
        return NULL;
    }
    newNode->next = NULL;
    newNode->prev = NULL;
    newNode->value = element;
    return newNode;
}

void InitDLlist(DLlist *list)
{
    list->head = CreateNode(0);
    if (list->head == NULL)
    {
        printf("head Node create error!\n");
        return;
    }
    list->tail = list->head;
    list->len = 0;
}

void InsertDLlistTail(DLlist *list, ElementType element)
{
    struct Node *newNode = CreateNode(element);
    if (newNode == NULL)
    {
        printf("InsertDLlistTail malloc error!\n");
        return;
    }

    list->tail->next = newNode;
    newNode->prev = list->tail;
    // 新节点是新的尾巴
    list->tail = newNode;
    list->len++;
}

void InsertDLlistHead(DLlist *list, ElementType element)
{
    struct Node *newNode = CreateNode(element);
    if (newNode == NULL)
    {
        printf("InsertDLlistHead malloc error!\n");
        return;
    }
    newNode->next = list->head->next;
    newNode->prev = list->head;
    if (list->head->next != NULL)
    {
        list->head->next->prev = newNode;
    }
    else
    {
        // 当链表中数量为空，第一次插入节点，把尾巴给出去，因为为空时，节点插入了，但是tail指针还指向头结点
        list->tail = newNode;
    }
    list->head->next = newNode;
    list->len++;
}

void InsertByIndex(DLlist *list, int index, ElementType element)
{
    if (index < 0 || index >= list->len)
    {
        printf("插入位置非法！\n");
        return;
    }

    struct Node *newNode = CreateNode(element);
    if (newNode == NULL)
    {
        return;
    }
    struct Node *travelPoint = list->head->next;
    for (int i = 0; i < index; i++)
    {
        travelPoint = travelPoint->next;
    }
    newNode->next = travelPoint;
    newNode->prev = travelPoint->prev;
    travelPoint->prev = newNode;
    newNode->prev->next = newNode;
    list->len++;
}

struct Node *FindByIndex(DLlist *list, int index)
{
    if (index < 0 || index >= list->len)
    {
        printf("www非法的查找位置\n");
        return NULL;
    }
    struct Node *travelPoint = list->head->next;
    for (int i = 0; i < index; i++)
    {
        travelPoint = travelPoint->next;
    }
    return travelPoint;
}

void RemoveByIndex(DLlist *list, int index)
{
    struct Node *node = FindByIndex(list, index);
    if (node == NULL)
    {
        return;
    }
    node->prev->next = node->next;
    if (node->next != NULL)
    {
        node->next->prev = node->prev;
    }
    else
    {
        // node是最后一个节点
        list->tail = node->prev;
    }

    free(node);
    list->len--;
}

void RemoveByElement(DLlist *list, ElementType element, bool (*funcPtr)(ElementType, ElementType))
{
    if (funcPtr == NULL)
    {
        return;
    }
    for (int i = 0; i < list->len; i++)
    {
        if (funcPtr(element, FindByIndex(list, i)->value) == true)
        {
            RemoveByIndex(list, i);
            i--;
        }
    }
}

DLlist FindByElement(DLlist *list, ElementType element, bool (*funcPtr)(ElementType, ElementType))
{
    DLlist result;
    InitDLlist(&result);
    if(funcPtr == NULL)
    {
            return result;
    }
    struct Node* travelPoint = list->head->next;
    while(travelPoint != NULL)
    {
        if(funcPtr(element, travelPoint->value) == true)
        {
            InsertDLlistTail(&result, travelPoint->value);
        }
        travelPoint = travelPoint->next;
    }
    return result;
}

void FreeDLlist(DLlist *list, void (*funcPtr)(ElementType))
{
    struct Node *travelPoint = list->head->next;
    while (travelPoint != NULL)
    {
        if (funcPtr != NULL)//如果函数指针传入NULL不操作，如果链表中的值需要释放，则需要自定义释放函数
        {
            funcPtr(travelPoint->value);
        }
        struct Node *next = travelPoint->next;
        free(travelPoint);
        travelPoint = next;
    }
    free(list->head);
    list->head = list->tail = NULL;
    list->len = 0;
}

void Travel(DLlist *list, void (*funcPtr)(ElementType))

{
    if (funcPtr == NULL)
    {
        return;
    }
    printf("list len; %d\n", list->len);
    // 头结点不在输出遍历范围之内
    struct Node *travelPoint = list->head->next; // travelPoint是用于遍历链表的指针
    while (travelPoint != NULL)
    {
        //
        funcPtr(travelPoint->value);
        travelPoint = travelPoint->next;
    }
    printf("\n");
}

void ReTravel(DLlist *list, void (*funcPtr)(ElementType))
{
    if (funcPtr == NULL)
    {
        return;
    }
    printf("list len; %d\n", list->len);

    struct Node *travelPoint = list->tail; // travelPoint是用于遍历链表的指针
    while (travelPoint != list->head)
    {
        //
        funcPtr(travelPoint->value);
        travelPoint = travelPoint->prev;
    }
    printf("\n");
}