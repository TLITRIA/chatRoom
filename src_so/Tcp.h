#ifndef __TCP_H_
#define __TCP_H_
#include <stdbool.h>
#define DEFAULT_SIZE 20
#define BUFFER_SZIE 1024
enum STATUSCODE
{
    LOGIN = 1, //登录
    SIGNUP, //注册
    CHAT ,    //私聊发消息
    ALLCHAT,//群聊
    ADDFRIEND, //加好友
    DELETEFRIEND, //删好友
    BUILDGROUP, //建群
    ADDGROUP,  //加群
    QUITGROUP, //退群
    NEWFRIENDS,//新朋友
    ADDFRIENDFAIL, //加好友失败
    ADDFRIENDSUCCESS, //加好友成功
    DELETEFRIENDFAIL, //删除好友失败
    DELETEFRIENDSUCCESS, //删除好友成功
    BUILDGROUPFAIL, //建立群失败
    BUILDGROUPSUCCESS, //建立群成功
    ADDGROUPFAIL, //加群失败
    ADDGROUPSUCCESS, //加群成功
    QUITGROUPFAIL, //退群失败
    QUITGROUPSUCCESS, //退群成功
    CHATFAIL, //私聊失败
    CHATSUCCESS, //私聊成功
    ALLCHATFAIL, //群聊失败
    ALLCHATSUCCESS, //群聊成功
    fileReady,//准备发文件
    HEART, 
    FILEAGREE,//同意
    FILEREFUSE,//拒绝
    fileStart,  //开始发文件
    filesending,//在发文件中
    fileend//文件发送结束
};

typedef struct MessageStructure
{
    int cmd;
    char fromName[DEFAULT_SIZE]; // 消息来源对象
    char password[DEFAULT_SIZE]; // 密码
    char toName[DEFAULT_SIZE];   // 消息接收对象
    char content[BUFFER_SZIE];   // 消息主体
}MSture;

// 服务器端初始化  创建 绑定 监听
int TcpServerInit();

// 建立通信套接字 返回通信要用的套接字
int TcpAccept(int sfd);

// 服务器端发送消息
bool TcpServerWrite(int clientfd, void *ptr, int size);

// 服务器接收消息
bool TcpServerRead(int clientfd, void *ptr, int size);


// 初始化客户端  初始化客户端的套接字，返回的是一个指向结构体的指针  创建套接字，连接服务器
int TcpClientInit();

// 客户端发送消息
bool TcpClientWrite(int c, void *ptr, int size);

// 客户端接收消息
bool TcpClientRead(int c, void *ptr, int size);

#endif