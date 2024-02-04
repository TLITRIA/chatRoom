#ifndef __COMMON_H_
#define __COMMON_H_

#define ELEMENTTYPE void*

/* 链表结点取别名*/
typedef struct DoubleLinkNode
{
    ELEMENTTYPE data;
    /* 指向前一个节点的指针*/
    struct DoubleLinkNode *prev;
    /* 指向下一个节点的指针*/
    struct DoubleLinkNode *next;/* 指针类型*/
}DoubleLinkNode;

/* 链表*/
typedef struct DoubleLinkList
{
    DoubleLinkNode * head;/* 链表的虚拟头结点*/
    DoubleLinkNode * tail;/* 尾指针不需要分配空间*/
    int len;/* 链表长度*/
}DoubleLinkList;



/* ====函数返回码==== */
enum RETURN_CODE
{
    NOT_FIND = -5,
    INVILID_ACCESS,
    NULL_PTR,
    MALLOC_ERROR,
    DEFAULT_ERROR = -1,
    ON_SUCCESS,
};

/* ====宏函数==== */







#endif