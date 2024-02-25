#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <error.h>
#include <pthread.h>
#include "Tcp.h"



/* 宏 */
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define FRIENDNAMESIZE 20 // 


enum STATUS_CODE
{
    SENDFAIL = -1,
};

/* 变量 */
int *g_client = NULL;
pthread_t *g_tid = NULL;


/* ======日志调试====== */
#if 1

#define DEBUG_FLAG "../Log/my_debug.flag" // 日志调试位置自选
#define DEBUG_FILE "../Log/my_debug.log"
/* 调试标记是否存在 */
int g_debug = 0;

/* 文件指针 */
FILE *g_logfp = NULL;

/* asctime最后一个字符改'\0' */
char *del_last(char *string)
{
    char *last_char = strrchr(string, '\0');
    *(last_char - 1) = '\0';
    return string;
}


#define LOGPR(fmt, args...)         \
    do                              \
    {                               \
        if (g_debug)                \
        {                           \
            time_t now;             \
            struct tm *ptm = NULL;  \
            now = time(NULL);       \
            ptm = localtime(&now);  \
            fprintf(g_logfp, "[file:(%s), func(%s), line(%d), time(%s)]\n"fmt"\n",   \
            __FILE__, __FUNCTION__, __LINE__, del_last(asctime(ptm)), ##args);   \
            fflush(g_logfp);                                                     \
        }                                                                        \
    }while(0)

/* 日志 : 就是文件 */
/* 打开日志文件 /*/
void log_init(void)
{   
    time_t now;
    
    /* 避免野指针 */
    struct tm *ptm = NULL;
    /* access函数 成功返回0 */
    if (access(DEBUG_FLAG, F_OK) == 0)
    {
        g_debug = 1;
    }
    
    if (!g_debug)
    {
        return;
    }

    if ((g_logfp = fopen(DEBUG_FILE, "a")) == NULL)
    {
        perror("fopen error");
        return;
    }
    now = time(NULL);
    ptm = localtime(&now);

    LOGPR("=====================log init done.====================\n");
    return; 
}

/* 关闭文件 */
void log_close(void)
{
    if (g_logfp)
    {
        fclose(g_logfp);
        g_logfp = NULL;
    }
}


/* ======日志结束====== */
#endif

/* 函数前置声明 */
/* 打印--登陆成功后的功能页面 */
void printChatroom();
/* 打印--私聊功能页面 */
void printChat();
/* 打印--群聊功能页面 */
void printAllChat();


int chatroom(); // todo 私聊？？

/* 添加好友 */
int addfriend(int clientfd); 
/* 删除好友 */
int deletefriend(int clientfd);
/* 建群 */
int buildgroup(int clientfd, MSture message);
/* 加群 */
int addgroup(int clientfd, MSture message);
/* 退群 */
int quitgroup(int clientfd, MSture message);
/* 发消息 */
void SendMessage(int clientfd, MSture message);
/* 接收消息线程处理函数 */
void *RecvMessage(void *arg);
/* 打印+输入--登陆注册 */
int login_signup(int clientfd, MSture message);
/* 心跳 */
void send_heart(void *arg);
/* 信号处理 */
void sigHandler(int sig);

void printChatroom() // 打印登录成功后的功能页面
{
    printf("\033[0;35m*********************网络聊天室**********************\033[m\n");
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
void printChat()
{
    system("clear");
    printf("\033[0;35m*********************私聊聊天室***********************\033[m\n");
    // printf("\033[0;35m|                                                   |\033[m\n");
    // printf("\033[0;35m|                                                   |\033[m\n");
    // printf("\033[0;35m|                                                   |\033[m\n");
    // printf("\033[0;34m|                                                   |\033[m\n");
    printf("\033[0;35m|                                输入q! : 退出私聊页面|\033[m\n");
    printf("\033[0;35m|                                输入w! : 传文件     |\033[m\n");
}

//打印群聊聊天室
void printAllChat()
{
     system("clear");
    printf("\033[0;35m*********************群聊聊天室***********************\033[m\n");
    printf("\033[0;35m|                                输入q! : 退出群聊页面|\033[m\n");
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


int addfriend(int clientfd)
{
    int ret = 0;
    char friendname[FRIENDNAMESIZE] = {0};
    printf("请输入你想要添加的好友名称:");
    scanf("%s", friendname);
    while (getchar() != '\n')
        ;
    if (TcpClientWrite(clientfd, friendname, sizeof(friendname)) == false)
    {
        perror("send");
        return -1;
    }

    return ret;
}

int deletefriend(int clientfd)
{
    int ret = 0;
    char friendname[FRIENDNAMESIZE] = {0};
    printf("请输入你想要删除的好友名称:");
    scanf("%s", friendname);
    while (getchar() != '\n')
        ;
    if (TcpClientWrite(clientfd, friendname, sizeof(friendname)) == false)
    {
        perror("send");
        return -1;
    }

    return ret;
}

//建立群
int buildgroup(int clientfd, MSture message)
{
    int ret = 0;
    printf("请输入你要建立的群名:");
    scanf("%s", message.content);
    while(getchar() != '\n');
    if(TcpClientWrite(clientfd, &message, sizeof(message)) == false)
    {
        perror("send");
        return -1;
    }


    return ret;
}

//加群
int addgroup(int clientfd, MSture message)
{
    int ret = 0;
    printf("请输入你要加入的群名:");
    scanf("%s", message.content);
    while(getchar() != '\n');
    if(TcpClientWrite(clientfd, &message, sizeof(message)) == false)
    {
        perror("send");
        return -1;
    }


    return ret;
}

//退群
int quitgroup(int clientfd, MSture message)
{
    int ret = 0;
    printf("请输入你要退出的群名:");
    scanf("%s", message.content);
    while(getchar() != '\n');
    if(TcpClientWrite(clientfd, &message, sizeof(message)) == false)
    {
        perror("send");
        return -1;
    }


    return ret;
}



// DLlist friendlist;
void SendMessage(int clientfd, MSture message)
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
            memset(message.toName, 0, sizeof(message.toName));
            scanf("%s", message.toName);
            while(getchar() != '\n');
            printChat();

            while(1)
            {
               message.cmd = CHAT;
               
               memset(message.content, 0, sizeof(message.content));
               scanf("%s", message.content);
               while(getchar() != '\n'); 
               if(strcmp(message.content, "w!") == 0)
               {
                 /*
                 to do.........
                 */
               }
               else if(strcmp(message.content, "q!") == 0)
               {
                   break;
               }
               else
               {
                    if(TcpClientWrite(clientfd,&message,sizeof(message)) == false)
                    {
                        perror("send");
                        return;
                    }
               }
            }

            break;
        case ALLCHAT:
            printf("请输入你要进入的群聊名称:");
            memset(message.toName, 0, sizeof(message.toName));
            scanf("%s", message.toName);
            while(getchar() != '\n');
            printAllChat();
            while(1)
            {
                message.cmd = ALLCHAT;
               
               memset(message.content, 0, sizeof(message.content));
               scanf("%s", message.content);
               while(getchar() != '\n'); 
               if(strcmp(message.content, "w!") == 0)
               {
                 /*
                 to do.........
                 */
               }
               else if(strcmp(message.content, "q!") == 0)
               {
                   break;
               }
               else
               {
                    if(TcpClientWrite(clientfd,&message,sizeof(message)) == false)
                    {
                        perror("send");
                        return;
                    }
               }
            }
            break;
        case ADDFRIEND:
            message.cmd = ADDFRIEND;
           TcpClientWrite(clientfd, &message, sizeof(message));
            addfriend(clientfd);
            break;
        case DELETEFRIEND:
            message.cmd = DELETEFRIEND;
           TcpClientWrite(clientfd, &message, sizeof(message));
            deletefriend(clientfd);
            break;
        case BUILDGROUP:
            memset(&message, 0, sizeof(message));
            message.cmd = BUILDGROUP;
            buildgroup(clientfd, message);
            break;
        case ADDGROUP:
            memset(&message, 0, sizeof(message));
            message.cmd = ADDGROUP;
            addgroup(clientfd, message);
            break;
        case QUITGROUP:
            memset(&message, 0, sizeof(message));
            message.cmd = QUITGROUP;
            quitgroup(clientfd, message);
            break;
        default:
            break;
        }
    }
}

void *RecvMessage(void *arg)
{
    int clientfd = *(int *)arg;

    while (1)
    {
        MSture message;
        if (TcpClientRead(clientfd, &message, sizeof(message)) == false)
        {
            break;
        }
        switch (message.cmd)
        {
            case ADDFRIENDSUCCESS:
                       printf("添加好友成功!\n");
                       break;
            case ADDFRIENDFAIL:
                       printf("%s\n", message.content);
                       break;
            case DELETEFRIENDSUCCESS:
                       printf("删除好友成功!\n");
                       break;
            case DELETEFRIENDFAIL:
                       printf("%s\n", message.content);
                       break;
            case CHATFAIL:
                    //    char ptr[BUFFER_SZIE] = {0};
                    //     while(1)
                    //     {
                    //         memset(ptr, 0, sizeof(ptr));
                    //         if(errno == EAGAIN)
                    //         {
                    //         break;
                    //         }
                    //         TcpClientRead(clientfd, ptr, sizeof(ptr));
                    //         printf("%s\n", ptr);
                    //     }
                       printf("%s\n", message.content);
                       break;
            case CHATSUCCESS:
                    //    char ptr1[BUFFER_SZIE] = {0};
                    //    while(1)
                    //    {
                    //       if(errno == EAGAIN)
                    //       {
                    //         break;
                    //       }
                    //       TcpClientRead(clientfd, ptr1, sizeof(ptr1));
                    //       printf("%s\n", ptr1);
                    //    }
                       printf("%s\n", message.content);
                       break;
            case ALLCHATFAIL:
                       printf("%s\n", message.content);
                       break;
            case ALLCHATSUCCESS:
                       printf("%s\n", message.content);
                       break;
            case BUILDGROUPFAIL:
                       printf("%s\n", message.content);
                       break;
            case BUILDGROUPSUCCESS:
                       printf("建立群成功!\n");
                       break;
            case ADDGROUPFAIL:
                       printf("%s\n", message.content);
                       break;
            case ADDGROUPSUCCESS:
                       printf("加群成功!\n");
                       break;
            case QUITGROUPFAIL: 
                       printf("%s\n", message.content);
                       break;
            case QUITGROUPSUCCESS:
                       printf("退群成功!\n");
                       break;
                default:
                        break;
        }
    }
}

/* 打印+输入--登陆注册 */
int login_signup(int clientfd, MSture message)
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
            message.cmd = LOGIN;
            int flag = 0;
            printf("请输入你要登录的姓名:");
            memset(message.fromName, 0, sizeof(message.fromName));
            scanf("%s", message.fromName);
            while (getchar() != '\n')
                ;
            printf("请输入你要登录的密码:");
            memset(message.password, 0, sizeof(message.password));
            scanf("%s", message.password);
            while (getchar() != '\n')
                ;
            if (TcpClientWrite(clientfd, &message, sizeof(message)) == false)
            {
                // Clearintlient(clientfd);
                // return SENDFAIL;
            }
            TcpClientRead(clientfd, &flag, sizeof(flag));
            if (flag == 1)
            {
                printf("登录成功!\n");
                return ret;
            }
            else
            {
                TcpClientRead(clientfd, &message, sizeof(message));
                printf("%s\n", message.content);
            }
        }
        if (choice == SIGNUP)
        {
            message.cmd = SIGNUP;
            printf("请输入你要注册的姓名:");
            memset(message.fromName, 0, sizeof(message.fromName));
            scanf("%s", message.fromName);
            while (getchar() != '\n')
                ;
           TcpClientWrite(clientfd, &message, sizeof(message));
            int flag = 0; // 判断注册的用户有没有重名的标志位
            TcpClientRead(clientfd, &flag, sizeof(flag));
            if (flag == 1)
            {
                printf("请输入你要注册的密码:");
                memset(message.password, 0, sizeof(message.password));
                scanf("%s", message.password);
                while (getchar() != '\n')
                    ;
               TcpClientWrite(clientfd, &message, sizeof(message));
                TcpClientRead(clientfd, &message, sizeof(message));
                printf("%s\n", message.content);
            }
            else
            {
                sleep(2);
                memset(message.content, 0, sizeof(message.content));
                TcpClientRead(clientfd, &message, sizeof(message));
                printf("%s\n", message.content);
            }
        }
    }
}

void send_heart(void *arg)
{
    int clientfd = *(int *)arg;

    //心跳检测
    while(1)
    {
        char* buf = "I am alive";
       TcpClientWrite(clientfd, buf, sizeof(buf));
        sleep(3);
    }
}

/* 捕捉信号*/
void sigHandler(int sig)
{
    if (g_tid != NULL)
    {
        free(g_tid);
        // Clearintlient(g_client);
        printf("成功回收资源\n");
        exit(-1);
    }

}


int main()
{
    /* 打开文件调试 */
    log_init();

    
    /* 注册退出信号 */
    signal(SIGINT, sigHandler);
    signal(SIGTSTP, sigHandler);
    signal(SIGQUIT, sigHandler);

    /* 初始化客户端到连接服务器，返回结构体中是通讯套接字 */
    int clientfd = TcpClientInit();
    // InitDLlist(&friendlist);
    if (clientfd < 0) /* 判空 */
        return -1;
    printf("客户端连接成功\n");

    /* 初始化消息 */
    MSture message;
    login_signup(clientfd, message);

    /* 开辟接收处理线程，并分离 */
    pthread_t tid;
    int ret = pthread_create(&tid, NULL, RecvMessage, (void *)&clientfd);
    if (ret != 0)
    {
        char *err = strerror(ret);
        printf("err:%s\n", err);
        _exit(-1);
    }

    pthread_detach(tid);

    // Thread *t1 = InitThread(send_heart,clientfd);
    // ThreadDetach(t1);

    /* 捕捉信号 */
    // g_client = clientfd;
    // g_tid = tid;// ？？


    /* 循环发送消息 */
    SendMessage(clientfd, message);

    while(1); // todo 删除确认
    // free(tid);
    // Clearintlient(clientfd);
    /* 关闭文件调试 */
    log_init();

    return 0;
}