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
    fileReady,//准备发文件
    fileagree,//同意
    filerefuse,//拒绝
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

// struct client
// {
//     int cmd;
//     int flag;//功能标志位：-1：退出群聊  0：通知所有在线用户 1：私聊 2：群聊 3：发送文件 4：修改密码 5：管理员操作
//     int root;  //权限标志位： -1：首次进入聊天室 0：普通用户 1：管理员 2：vip用户
//     int forbit; //管理员权限：1：禁言 2:解除禁言 3：踢人
//     char name[50];//账号名
//     char password[20]; //密码
//     char msg[1024]; //聊天信息
//     char toname[50]; //私聊对象
//     struct client *next;
//     struct client *prev;
// };
// typedef struct client Client;
struct ClientInfo // 用来存放已登录的用户
{
    char Name[20]; // 客户端用户名
    int sock;      // 登录主机的套接字
};
struct ClientSignup //  用来存放已注册的用户
{
    char name[50];
    char password[20];
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