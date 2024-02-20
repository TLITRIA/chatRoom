#include <stdio.h>
#include "onLine.h"
#include "balanceBinarySearchTree.h"



/* 在线表初始化*/
int onLineInit(onLineOutside **PonLine, int (*compareFunc)(ELEMENTTYPE, ELEMENTTYPE))
{
    int ret = balanceBinarySearchTreeInit(PonLine, compareFunc);
    return ret;
}

/* 人员上线*/
int onLineInsert(onLineOutside *PonLine, ELEMENTTYPE val)
{
    int ret = balanceBinarySearchTreeInsert(PonLine, val);
    if (ret)
    {
        printf("客户上线\n");
        return ret;
    }
    return 0;
}

/* 人员下线*/
int onLineRemove(onLineOutside *PonLine, ELEMENTTYPE val)
{
    int ret = balanceBinarySearchTreeRemove(PonLine, val);
    if (ret)
    {
        printf("客户下线\n");
        return ret;
    }
    return 0;
}

/* 查看某人是否在线*/
int onLineIsContainVal(onLineOutside *PonLine, ELEMENTTYPE val)
{
    int ret = balanceBinarySearchTreeIsContainVal(PonLine, val);
    if (ret)
    {
        printf("在线\n");
    }
    return ret;
}

/* 获取套接字*/
int onLineObtainValVal(onLineOutside *PonLine, ELEMENTTYPE val, int *sockfd, int (*obtainFunc)(void *))
{
    int ret = balanceBinarySearchTreeIsContainValVal(PonLine, val, sockfd, obtainFunc);
    return ret;
}

/* 遍历*/
int onPrintf(onLineOutside *PonLine, int (*printFunc)(void *))
{
    balanceBinarySearchTreeInOrderTravel(PonLine, printFunc);
    return 1;
}

/* 销毁表*/
int onLineDestroy(onLineOutside *PonLine)
{
    int ret = balanceBinarySearchTreeDestroy(PonLine);
    return ret;
}