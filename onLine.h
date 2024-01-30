#ifndef __ONLINE_H_
#define __ONLINE_H_
#include "balanceBinarySearchTree.h"

#define BUFFER_LEN 20

/* 在线人员表*/
typedef struct BinarySearchTree onLineOutside;

typedef struct onLine
{
    int sockfd;
    char name[BUFFER_LEN];
}onLline;

/* 在线人员表初始化*/
int onLineInit(onLineOutside **PonLine, int (*compareFunc)(ELEMENTTYPE, ELEMENTTYPE));

/* 人员上线*/
int onLineInsert(onLineOutside *PonLine, ELEMENTTYPE val);

/* 人员下线*/
int onLineRemove(onLineOutside *PonLine, ELEMENTTYPE val);

/* 查看某人是否在线*/
int onLineIsContainVal(onLineOutside *PonLine, ELEMENTTYPE val);

/* 获取套接字*/
int onLineObtainValVal(onLineOutside *PonLine, ELEMENTTYPE val, int *sockfd, int (*obtainFunc)(void *));

/* 销毁表*/
int onLineDestroy(onLineOutside *PonLine);

#endif