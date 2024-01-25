#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "StdTcp.h"
#include "StdThread.h"
#include "GlobalMessage.h"
#include "DoubleLinkList.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#define ip "127.0.0.1"
#define port 8080
#define FRIENDNAMESIZE 20

enum STATUS_CODE
{
    SENDFAIL = -1,
};

void printChatroom() // 打印登录成功后的功能页面
{
    printf("\033[0;35m*********************大壮聊天室**********************\033[m\n");
    printf("\033[0;35m|                                                   |\033[m\n");
    printf("\033[0;35m|                                                   |\033[m\n");
    printf("\033[0;35m|                                                   |\033[m\n");
    printf("\033[0;34m|                       3.私聊                      |\033[m\n");
    printf("\033[0;35m|                       4.群聊                      |\033[m\n");
    printf("\033[0;35m|                       5.加好友                    |\033[m\n");
    printf("\033[0;35m|                       6.删好友                    |\033[m\n");
    printf("\033[0;34m|                       7.建群                      |\033[m\n");
    printf("\033[0;35m|                       8.加群                      |\033[m\n");
    printf("\033[0;35m|                       9.退群                      |\033[m\n");
    printf("\033[0;35m|                                                   |\033[m\n");
    printf("\033[0;35m|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|\033[m\n");
}

// 打印进入私聊功能的页面
void printCHAT()
{
    system("clear");
    printf("\033[0;35m*********************私聊聊天室***********************\033[m\n");
    printf("\033[0;35m|                                                   |\033[m\n");
    printf("\033[0;35m|                                                   |\033[m\n");
    printf("\033[0;35m|                                                   |\033[m\n");
    printf("\033[0;34m|                                                   |\033[m\n");
    printf("\033[0;35m|                                输入q! : 退出私聊页面|\033[m\n");
    printf("\033[0;35m|                                输入w! : 传文件     |\033[m\n");
}

int chatroom() // 私聊
{
    int choice = 0;
    while (1)
    {
        printf("\033[0;34m请输入你的选项:\033[m");
        scanf("%d", &choice);
        if (choice == ADDFRIEND)
        {

            printf("请输入你要添加的好友姓名:");
        }
    }
}

int addfriend(TcpC *c)
{
    int ret = 0;
    char friendname[FRIENDNAMESIZE] = {0};
    printf("请输入你想要添加的好友名称:");
    scanf("%s", friendname);
    while (getchar() != '\n')
        ;
    if (TcpClientSend(c, friendname, sizeof(friendname)) == false)
    {
        perror("send");
        return -1;
    }

    return ret;
}

int deletefriend(TcpC *c)
{
    int ret = 0;
    char friendname[FRIENDNAMESIZE] = {0};
    printf("请输入你想要删除的好友名称:");
    scanf("%s", friendname);
    while (getchar() != '\n')
        ;
    if (TcpClientSend(c, friendname, sizeof(friendname)) == false)
    {
        perror("send");
        return -1;
    }

    return ret;
}

//建立群
int buildgroup(TcpC *c, Msg m)
<<<<<<< HEAD
=======
{
    int ret = 0;
    printf("请输入你要建立的群名:");
    scanf("%s", m.content);
    while(getchar() != '\n');
    if(TcpClientSend(c, &m, sizeof(m)) == false)
    {
        perror("send");
        return -1;
    }


    return ret;
}

//加群
int addgroup(TcpC *c, Msg m)
{
    int ret = 0;
    printf("请输入你要加入的群名:");
    scanf("%s", m.content);
    while(getchar() != '\n');
    if(TcpClientSend(c, &m, sizeof(m)) == false)
    {
        perror("send");
        return -1;
    }


    return ret;
}

//退群
int quitgroup(TcpC *c, Msg m)
{
    int ret = 0;
    printf("请输入你要退出的群名:");
    scanf("%s", m.content);
    while(getchar() != '\n');
    if(TcpClientSend(c, &m, sizeof(m)) == false)
    {
        perror("send");
        return -1;
    }


    return ret;
}

// DLlist friendlist;
void SendMessage(TcpC *c,Msg m)
>>>>>>> DZ
{
    int ret = 0;
    printf("请输入你要建立的群名:");
    scanf("%s", m.content);
    while(getchar() != '\n');
    if(TcpClientSend(c, &m, sizeof(m)) == false)
    {
        perror("send");
        return -1;
    }


    return ret;
}

//加群
int addgroup(TcpC *c, Msg m)
{
    int ret = 0;
    printf("请输入你要加入的群名:");
    scanf("%s", m.content);
    while(getchar() != '\n');
    if(TcpClientSend(c, &m, sizeof(m)) == false)
    {
        perror("send");
        return -1;
    }


    return ret;
}

//退群
int quitgroup(TcpC *c, Msg m)
{
    int ret = 0;
    printf("请输入你要退出的群名:");
    scanf("%s", m.content);
    while(getchar() != '\n');
    if(TcpClientSend(c, &m, sizeof(m)) == false)
    {
        perror("send");
        return -1;
    }


    return ret;
}

// DLlist friendlist;
void SendMessage(TcpC *c, Msg m)
{
    int choice = 0;
    while (1)
    {
        choice = 0;
        printChatroom();
        // 打印好友列表与群列表
        /*
           todo........
        */
        printf("\033[0;34m请输入你的选项:\033[m");
        scanf("%d", &choice);
        while (getchar() != '\n')
            ;
        switch (choice)
        {
        case CHAT:
            
            printf("请输入你要私聊的用户:");
            memset(m.toName, 0, sizeof(m.toName));
            scanf("%s", m.toName);
            while(getchar() != '\n');
            printCHAT();

            while(1)
            {
               m.cmd = CHAT;
               memset(m.content, 0, sizeof(m.content));
               scanf("%s", m.content);
               while(getchar() != '\n'); 
               if(strcmp(m.content, "w!") == 0)
               {
                 /*
                 to do.........
                 */
               }
               else if(strcmp(m.content, "q!") == 0)
               {
                   break;
               }
               else
               {
                    if(TcpClientSend(c,&m,sizeof(m)) == false)
                    {
                        perror("send");
                        return;
                    }
               }
            }

            break;
        case ALLCHAT:
            m.cmd = ALLCHAT;
            memset(m.content, 0, sizeof(m.content));
            printf("请输入你要发送的消息：");
            scanf("%s", m.content);
            while (getchar() != '\n')
                ;
            if (TcpClientSend(c, &m, sizeof(m)) == false)
            {
                perror("send");
                return;
            }
            break;
        case ADDFRIEND:
            m.cmd = ADDFRIEND;
            TcpClientSend(c, &m, sizeof(m));
            addfriend(c);
            break;
        case DELETEFRIEND:
            m.cmd = DELETEFRIEND;
            TcpClientSend(c, &m, sizeof(m));
            deletefriend(c);
            break;
        case BUILDGROUP:
            memset(&m, 0, sizeof(m));
            m.cmd = BUILDGROUP;
            buildgroup(c, m);
            break;
        case ADDGROUP:
            memset(&m, 0, sizeof(m));
            m.cmd = ADDGROUP;
            addgroup(c, m);
            break;
        case QUITGROUP:
            memset(&m, 0, sizeof(m));
            m.cmd = QUITGROUP;
            quitgroup(c, m);
            break;
        default:
            break;
        }
    }
}

void *RecvMessage(void *arg)
{
    TcpC *c = (TcpC *)arg;

    while (1)
    {
        Msg m;
        if (TcpClientRecv(c, &m, sizeof(m)) == false)
        {
            break;
        }
        switch (m.cmd)
        {
            case ADDFRIENDSUCCESS:
                       printf("添加好友成功!\n");
                       break;
            case ADDFRIENDFAIL:
                       printf("%s\n", m.content);
                       break;
            case DELETEFRIENDSUCCESS:
                       printf("删除好友成功!\n");
                       break;
            case DELETEFRIENDFAIL:
                       printf("%s\n", m.content);
                       break;
            case CHATFAIL:
                       printf("%s\n", m.content);
                       break;
            case CHATSUCCESS:
                       printf("%s\n", m.content);
                       break;
            case BUILDGROUPFAIL:
                       printf("%s\n", m.content);
                       break;
            case BUILDGROUPSUCCESS:
                       printf("建立群成功!\n");
                       break;
            case ADDGROUPFAIL:
                       printf("%s\n", m.content);
                       break;
            case ADDGROUPSUCCESS:
                       printf("加群成功!\n");
                       break;
            case QUITGROUPFAIL: 
                       printf("%s\n", m.content);
                       break;
            case QUITGROUPSUCCESS:
                       printf("退群成功!\n");
                       break;
                default:
                        break;
        }
    }
}

// 登录注册
int login_signup(TcpC *c, Msg m)
{
    printf("\033[0;35m*********************大壮聊天室**********************\033[m\n");
    printf("\033[0;35m|                                                   |\033[m\n");
    printf("\033[0;35m|                                                   |\033[m\n");
    printf("\033[0;35m|                                                   |\033[m\n");
    printf("\033[0;34m|                       1.登录                      |\033[m\n");
    printf("\033[0;35m|                                                   |\033[m\n");
    printf("\033[0;35m|                                                   |\033[m\n");
    printf("\033[0;35m|                                                   |\033[m\n");
    printf("\033[0;34m|                       2.注册                      |\033[m\n");
    printf("\033[0;35m|                                                   |\033[m\n");
    printf("\033[0;35m|                                                   |\033[m\n");
    printf("\033[0;35m|                                                   |\033[m\n");
    printf("\033[0;35m|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|\033[m\n");
    int choice = 0;
    int ret = 0;
    while (1)
    {
        printf("\033[0;34m请输入你的选项:\033[m");
        scanf("%d", &choice);
        while (getchar() != '\n')
            ;
        if (choice == LOGIN)
        {
            m.cmd = LOGIN;
            int flag = 0;
            printf("请输入你要登录的姓名:");
            memset(m.fromName, 0, sizeof(m.fromName));
            scanf("%s", m.fromName);
            while (getchar() != '\n')
                ;
            printf("请输入你要登录的密码:");
            memset(m.password, 0, sizeof(m.password));
            scanf("%s", m.password);
            while (getchar() != '\n')
                ;
            if (TcpClientSend(c, &m, sizeof(m)) == false)
            {
                ClearTcpClient(c);
                return SENDFAIL;
            }
            TcpClientRecv(c, &flag, sizeof(flag));
            if (flag == 1)
            {
                printf("登录成功!\n");
                return ret;
            }
            else
            {
                TcpClientRecv(c, &m, sizeof(m));
                printf("%s\n", m.content);
            }
        }
        if (choice == SIGNUP)
        {
            m.cmd = SIGNUP;
            printf("请输入你要注册的姓名:");
            memset(m.fromName, 0, sizeof(m.fromName));
            scanf("%s", m.fromName);
            while (getchar() != '\n')
                ;
            TcpClientSend(c, &m, sizeof(m));
            int flag = 0; // 判断注册的用户有没有重名的标志位
            TcpClientRecv(c, &flag, sizeof(flag));
            if (flag == 1)
            {
                printf("请输入你要注册的密码:");
                memset(m.password, 0, sizeof(m.password));
                scanf("%s", m.password);
                while (getchar() != '\n')
                    ;
                TcpClientSend(c, &m, sizeof(m));
                TcpClientRecv(c, &m, sizeof(m));
                printf("%s\n", m.content);
            }
            else
            {
                sleep(2);
                memset(m.content, 0, sizeof(m.content));
                TcpClientRecv(c, &m, sizeof(m));
                printf("%s\n", m.content);
            }
        }
    }
}

int main()
{
    TcpC *c = InitTcpClient(ip, port);
    // InitDLlist(&friendlist);
    if (c == NULL)
        return -1;
    printf("客户端连接成功\n");

    Msg m;
    login_signup(c, m);

    Thread *t = InitThread(RecvMessage, c);
    ThreadDetach(t);
    SendMessage(c, m);

    while (1)
        ;
    free(t);
    ClearTcpClient(c);
    return 0;
}