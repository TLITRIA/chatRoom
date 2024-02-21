#ifndef __GLOBALMESSAGE_H_
#define __GLOBALMESSAGE_H_
#include <stdbool.h>
#define DEFAULT_SIZE 20
#define BUFFER_SZIE 1024
enum CMD
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
    NEWFRIENDS, //新朋友
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

struct Message
{
    int cmd;
    char fromName[DEFAULT_SIZE]; // 消息来源对象
    char password[DEFAULT_SIZE]; // 密码
    char toName[DEFAULT_SIZE];   // 消息接收对象
    char content[BUFFER_SZIE];   // 消息主体
};
typedef struct Message Msg;

struct ClientInfo // 用来存放已登录的用户
{
    char Name[DEFAULT_SIZE]; // 客户端用户名
    int sock;      // 登录主机的套接字
};
struct ClientSignup //  用来存放已注册的用户
{
    char name[50];
    char password[DEFAULT_SIZE];
};
struct Root // 用来存放管理者名单
{
    char rootname[50];
};
typedef struct Root RT;
typedef struct ClientSignup CSignup;
typedef struct ClientInfo CInfo;
CInfo *CreateInfo(const char *name, int sock);

CSignup *CreateSignup(const char *name, const char *password); // 自定义
RT *CreateRoot(const char *name);                              // 自定义
void ClearInfo(CInfo *c);
bool IsNameSame(void *ptr1, void *ptr2);

#endif