#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>


#include "StdTcp.h"
#include "StdThread.h"
#include "GlobalMessage.h"
#include "DoubleLinkList.h"
#include "StdSqlite.h"
#include "MyString.h"
#include "StdThreadPool.h"
#include "onLine.h"

#define ip "127.0.0.1"
#define port 8080
#define USERNAMESIZE 150
#define FRIENDNAMESIZE 20
#define GROUPNAMESIZE 20
#define SQLSIZE 150

static DLlist ClientList;      // 客户端信息链表
static SQL *g_db;              // 数据库句柄
onLineOutside *PonLine = NULL; // 在线表

// 锁
pthread_mutex_t loginmutex;
pthread_mutex_t groupmutex;

#if 1
/* ======日志调试====== */
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

int obtainFunc(ELEMENTTYPE val)
{
    onLline *num = (onLline *)val;
    return num->sockfd;
}

/* 比较基础数据 */
int compareFunc(ELEMENTTYPE val1, ELEMENTTYPE val2)
{
    int ret = 0;
    onLline *num1 = (onLline *)val1;
    onLline *num2 = (onLline *)val2;
    return strncmp(num1->name, num2->name, strlen(num1->name) < strlen(num2->name) ? strlen(num1->name) : strlen(num2->name));
}

int printFunc(ELEMENTTYPE val)
{
    int ret = 0;
    onLline *num = (onLline *)val;
    printf("在线name:%s  fd:%d\n", num->name, num->sockfd);
    return ret;
}

/* 登录 */
int login(int clientfd, Msg message, char *userName, onLline *info)
{
    int flag = 0;
    char sql[USERNAMESIZE] = {0};
    sprintf(sql, "select * from SignupClient where Username = '%s' and Password = '%s';", message.fromName, message.password);
    int ret = searchIsExist(g_db, sql);
    if (ret == 0) 
    {
        // 填写的用户名或密码不正确
        int flag = 0;
        TcpServerSend(clientfd, &flag, sizeof(flag));
        memset(message.content, 0, sizeof(message.content));
        strcpy(message.content, "你输入的账号或密码不正确, 请重新输入!");
        TcpServerSend(clientfd, &message, sizeof(message));
    }
    else
    {
        strncpy(info->name, message.fromName, sizeof(message.fromName) - 1);
        info->sockfd = clientfd;
        ret = onLineIsContainVal(PonLine, (void *)info);
        if (ret == 0)
        {
            // 用户未登录
            if (onLineInsert(PonLine, (void *)info))
            {
                // 记录客户端姓名
                strcpy(userName, message.fromName);
                onPrintf(PonLine, printFunc);

                flag = 1; // 用户成功登录标志位
                TcpServerSend(clientfd, &flag, sizeof(flag));
            }
        }
        else
        {
            // 用户已经在其他地方登录
            flag = 0;
            TcpServerSend(clientfd, &flag, sizeof(flag));
            memset(message.content, 0, sizeof(message.content));
            strcpy(message.content, "你要登录的账号已经在异地登录,请重新输入");
            TcpServerSend(clientfd, &message, sizeof(message));
        }
    }
}

/* 注册 */
int enroll(int clientfd, Msg message)
{
    int flag = 0;
    char username[USERNAMESIZE] = {0};
    sprintf(username, "select * from SignupClient where Username = '%s';", message.fromName);
    int ret = searchIsExist(g_db, username);
    // ret = findusername(message.fromName,g_db,"select Username from SignupClient;");
    if (ret == 0)
    {
        flag = 1;
        TcpServerSend(clientfd, &flag, sizeof(flag));
        TcpServerRecv(clientfd, &message, sizeof(message));
        ret = searchIsExist(g_db, username);
        if (ret == 0)
        {
            char sql[100] = {0};
            sprintf(sql, "insert into SignupClient values('%s','%s');", message.fromName, message.password);
            if (SqliteExec(g_db, sql) == true)
            {
                strcpy(message.content, "注册成功!");
                // message.cmd = signupsuccess;
                TcpServerSend(clientfd, &message, sizeof(message));

                // 建立一个以用户名为表名的表,用来存储好友名单与群列表
                memset(sql, 0, sizeof(sql));

                sprintf(sql, "create table if not exists %s(name text, flag integer);", message.fromName);
                SqliteExec(g_db, sql);
            }
        }
        else
        {
            flag = 0;
            TcpServerSend(clientfd, &flag, sizeof(flag));
            // sleep(1);
            strcpy(message.content, "注册失败, 由于你输入密码太慢你注册的姓名已被别人抢先!");
            TcpServerSend(clientfd, &message, sizeof(message));
        }
    }
    else
    {
        flag = 0;
        TcpServerSend(clientfd, &flag, sizeof(flag));
        memset(message.content, 0, sizeof(message.content));
        sleep(1);
        strcpy(message.content, "注册失败, 你注册的姓名已被别人抢先或与他人重复!");
        TcpServerSend(clientfd, &message, sizeof(message));
    }
}

// 删除好友
int deletefriend(int clientfd, Msg message, const char *userName)
{
    int ret = 0;
    char friendname[FRIENDNAMESIZE] = {0};
    TcpServerRecv(clientfd, friendname, sizeof(friendname));
    char sql2[SQLSIZE];
    memset(sql2, 0, sizeof(sql2));
    // 从在线链表中获取结果集
    char name[FRIENDNAMESIZE] = {0};
    // sprintf(sql2,"select * from LoginClient where Clientfd = %d;", clientfd);
    // GetTableVal(g_db, sql2, name, NULL, 0);
    strncpy(name, userName, strlen(userName));

    memset(sql2, 0, sizeof(sql2));
    sprintf(sql2, "select * from %s where name = '%s';", name, friendname);
    printf("%s\n", sql2);
    // 先查一遍是不是已经是好友， 是则可以删除, 不是则无法删除
    ret = searchIsExist(g_db, sql2);
    if (ret == 0) // 不是好友，删除失败
    {
        message.cmd = DELETEFRIENDFAIL;
        memset(message.content, 0, sizeof(message.content));
        strcpy(message.content, "删除好友好友失败,你要删除的用户并不是你的好友,不可以删除");
        TcpServerSend(clientfd, &message, sizeof(message));
    }
    else if (ret != 0) // 是好友, 可以删除, 查是否在线
    {
        // memset(sql2, 0, sizeof(sql2));
        // sprintf(sql2, "select * from LoginClient where Username = '%s';", friendname);
        // ret = searchIsExist(g_db, sql2);
        onLline info3;
        strncpy(info3.name, friendname, sizeof(friendname) - 1);
        info3.sockfd = -1;
        ret = onLineIsContainVal(PonLine, (void *)&info3);

        if (ret == 0) // 不在线
        {
            message.cmd = DELETEFRIENDFAIL;
            memset(message.content, 0, sizeof(message.content));
            strcpy(message.content, "删除好友失败,你要删除的好友处于不在线状态");
            TcpServerSend(clientfd, &message, sizeof(message));
        }
        else // 在线
        {
            memset(sql2, 0, sizeof(sql2));
            int status = 1; // 表示是好友, 删除
            sprintf(sql2, "delete from %s where name = '%s' and flag = %d;", name, friendname, status);
            if (SqliteExec(g_db, sql2) == true)
            {
                memset(sql2, 0, sizeof(sql2));
                sprintf(sql2, "delete from %s where name = '%s' and flag = %d;", friendname, name, status);
                SqliteExec(g_db, sql2); // 互删好友
                message.cmd = DELETEFRIENDSUCCESS;
                TcpServerSend(clientfd, &message, sizeof(message));
            }
        }
    }
    return ret;
}

// 建立群聊
int buildGroup(int clientfd, Msg message, const char *userName)
{
    char groupname[GROUPNAMESIZE + BUFFER_SZIE];
    memset(groupname, 0, sizeof(groupname));
    strcpy(groupname, message.content);
    char sql5[SQLSIZE + BUFFER_SZIE] = {0};
    sprintf(sql5, "select * from Queue where groupname = '%s';", groupname);

    // 加锁
    pthread_mutex_lock(&groupmutex);
    int ret = searchIsExist(g_db, sql5);
    if (ret != 0) // 已经有其他客户端建立了群，不可建立群名一样的群
    {
        pthread_mutex_unlock(&groupmutex);
        message.cmd = BUILDGROUPFAIL;
        memset(message.content, 0, sizeof(message.content));
        strcpy(message.content, "建群失败,已经有其他客户端建立了群,不可建立群名一样的群");
        TcpServerSend(clientfd, &message, sizeof(message));
    }
    else
    {
        // 可以建群
        // message.cmd = BUILDGROUPSUCCESS;
        printf("%s\n", userName);
        memset(sql5, 0, sizeof(sql5));
        // 插入到唯一的群表
        sprintf(sql5, "insert into Queue values('%s');", message.content);
        if (SqliteExec(g_db, sql5) == true)
        {
            pthread_mutex_unlock(&groupmutex);
            memset(sql5, 0, sizeof(sql5));
            sprintf(sql5, "create table if not exists %s(name text);", message.content);
            if (SqliteExec(g_db, sql5) == true)
            {
                memset(sql5, 0, sizeof(sql5));
                // 把群主插入到他所建立的群里面
                sprintf(sql5, "insert into %s values('%s');", message.content, userName);
                if (SqliteExec(g_db, sql5) == true)
                {
                    memset(sql5, 0, sizeof(sql5));
                    // 在群主的那个客户端表里插入群
                    sprintf(sql5, "insert into %s values('%s', 2);", userName, message.content);
                    SqliteExec(g_db, sql5);
                    message.cmd = BUILDGROUPSUCCESS;
                    TcpServerSend(clientfd, &message, sizeof(message));
                }
            }
        }
    }
    return ret;
}

// 加入群
int addGroup(int clientfd, Msg message, const char *userName)
{
    char groupname[GROUPNAMESIZE + BUFFER_SZIE];
    memset(groupname, 0, sizeof(groupname));
    strcpy(groupname, message.content);
    char sql5[SQLSIZE + BUFFER_SZIE] = {0};
    sprintf(sql5, "select * from Queue where groupname = '%s';", groupname);

    // 加锁
    pthread_mutex_lock(&groupmutex);
    // 先查看有没有这个群
    int ret = searchIsExist(g_db, sql5);
    if (ret == 0) // 没有这个群, 加群失败
    {

        pthread_mutex_unlock(&groupmutex);
        message.cmd = ADDGROUPFAIL;
        memset(message.content, 0, sizeof(message.content));
        strcpy(message.content, "加群失败,群表里不存在你要加的群");
        TcpServerSend(clientfd, &message, sizeof(message));
    }
    else // 存在这个群， 分两种情况, 用户在与不在这个群， 在则加群失败， 不在则加群成功
    {
        pthread_mutex_unlock(&groupmutex);

        memset(sql5, 0, sizeof(sql5));
        sprintf(sql5, "select * from %s where name = '%s';", groupname, userName);
        ret = searchIsExist(g_db, sql5);
        if (ret != 0) // 用户存在这个群里， 加群失败
        {
            message.cmd = ADDGROUPFAIL;
            memset(message.content, 0, sizeof(message.content));
            strcpy(message.content, "你已经处在你要加入的群里, 不可以重复加入!");
            TcpServerSend(clientfd, &message, sizeof(message));
        }
        else // 用户不存在这个群里里， 可以加群
        {
            memset(sql5, 0, sizeof(sql5));
            // 把客户导入到所要加的群表里
            sprintf(sql5, "insert into %s values('%s');", groupname, userName);
            if (SqliteExec(g_db, sql5) == true)
            {
                // 更新客户那张表， 增加一个群
                memset(sql5, 0, sizeof(sql5));
                sprintf(sql5, "insert into %s values('%s', 2);", userName, groupname);
                if (SqliteExec(g_db, sql5) == true)
                {
                    message.cmd = ADDGROUPSUCCESS;
                    memset(message.content, 0, sizeof(message.content));
                    TcpServerSend(clientfd, &message, sizeof(message));
                }
            }
        }
    }

    return ret;
}

/* 退群 */
int quitGroup(int clientfd, Msg message, const char *userName)
{
    char groupname[GROUPNAMESIZE + BUFFER_SZIE];
    memset(groupname, 0, sizeof(groupname));
    strcpy(groupname, message.content);
    char sql5[SQLSIZE + BUFFER_SZIE] = {0};
    sprintf(sql5, "select * from Queue where groupname = '%s';", groupname);

    // 加锁
    pthread_mutex_lock(&groupmutex);
    

#if 0
    // 先查看有没有这个群
    int ret = searchIsExist(g_db, sql5);
    if (ret == 0) // 没有这个群 , 退群失败
    {
        pthread_mutex_unlock(&groupmutex);
        message.cmd = QUITGROUPFAIL;
        memset(message.content, 0, sizeof(message.content));
        strcpy(message.content, "退群失败,群表里不存在你要退的群");
        TcpServerSend(clientfd, &message, sizeof(message));
    }
    else // 存在这个群， 分两种情况, 用户在与不在这个群， 在则退群成功，不在则退群失败
    {
        pthread_mutex_unlock(&groupmutex);
        memset(sql5, 0, sizeof(sql5));
        sprintf(sql5, "select * from %s where name = '%s';", groupname, userName);
        ret = searchIsExist(g_db, sql5);
        if (ret == 0) // 用户不存在存在这个群里， 退群失败
        {
            message.cmd = QUITGROUPFAIL;
            memset(message.content, 0, sizeof(message.content));
            strcpy(message.content, "你并不处在你要退出的群里, 不可以退群!");
            TcpServerSend(clientfd, &message, sizeof(message));
        }
        else
        {
            // 用户存在这个群里里， 可以退群
            memset(sql5, 0, sizeof(sql5));
            // 把客户从所要退的群表里删除
            sprintf(sql5, "delete from %s where name = '%s';", groupname, userName);
            if (SqliteExec(g_db, sql5) == true)
            {
                // 更新客户那张表， 删除一个群
                memset(sql5, 0, sizeof(sql5));
                sprintf(sql5, "delete from %s where name = '%s';", userName, groupname);
                if (SqliteExec(g_db, sql5) == true)
                {
                    message.cmd = QUITGROUPSUCCESS;
                    memset(message.content, 0, sizeof(message.content));
                    TcpServerSend(clientfd, &message, sizeof(message));
                }
            }
        }
    }
#else
    // 先查自己是否加入群聊
    memset(sql5, 0, sizeof(sql5));
    sprintf(sql5, "select * from %s where name = '%s';", userName, groupname);
    LOGPR("退群：查找用户%s是否已经加入群聊%s。", userName, groupname);
    int ret = searchIsExist(g_db, sql5);
    if (ret == 0) // 用户未加入这个群， 退群失败
    {
        pthread_mutex_unlock(&groupmutex);
        message.cmd = QUITGROUPFAIL;
        memset(message.content, 0, sizeof(message.content));
        strcpy(message.content, "你未加入你要退出的群, 不可以退群!");
        TcpServerSend(clientfd, &message, sizeof(message));
    }
    else
    {
        pthread_mutex_unlock(&groupmutex);
        // 用户已加入这个群
        memset(sql5, 0, sizeof(sql5));
        // 从群成员表里删除用户
        sprintf(sql5, "delete from %s where name = '%s';", groupname, userName);
        if (SqliteExec(g_db, sql5) == true)
        {
            // 从用户表里删除群
            memset(sql5, 0, sizeof(sql5));
            sprintf(sql5, "delete from %s where name = '%s';", userName, groupname);
            if (SqliteExec(g_db, sql5) == true)
            {
                // 最后返回成功删除的
                message.cmd = QUITGROUPSUCCESS;
                memset(message.content, 0, sizeof(message.content));
                TcpServerSend(clientfd, &message, sizeof(message));
            }
        }

        LOGPR("退群：查找群聊%s是否没有成员。", groupname);
        
        if(judgeGroupEmpty(g_db, sql5) == true)
        {
            LOGPR("退群：删除空的群聊%s", groupname);
            // 删除group中的群聊
            memset(sql5, 0, sizeof(sql5));
            sprintf(sql5, "delete from Queue where groupname = '%s';", groupname);
            SqliteExec(g_db, sql5);

            // 删除群聊的表 "drop table if exists user";
            memset(sql5, 0, sizeof(sql5));
            sprintf(sql5, "drop table if exists %s;", groupname);
            SqliteExec(g_db, sql5);
        }
    }
#endif
    

    return ret;
}

/* 进入群聊 */
int enterGroup(int clientfd, Msg message, const char *userName)
{
    // todo优化 直接看自己那张表是否有群
    // 先去查有没有这个群
    char sql3[BUFFER_SZIE + USERNAMESIZE] = {0};
    sprintf(sql3, "select * from Queue where groupname = '%s';", message.toName);
    int ret = searchIsExist(g_db, sql3);
    if (ret == 0) // 不存在这个群  进入群聊失败
    {
        message.cmd = ALLCHATFAIL; // 群聊失败
        memset(message.content, 0, sizeof(message.content));
        strcpy(message.content, "进入群聊失败, 你要进入的群聊不存在!");
        TcpServerSend(clientfd, &message, sizeof(message));
    }
    else // 存在这个群，看自己在不在这个群里
    {
        memset(sql3, 0, sizeof(sql3));
        sprintf(sql3, "select * from %s where name = '%s';", message.toName, userName);
        ret = searchIsExist(g_db, sql3);
        if (ret == 0) // 自己不在这个群, 群聊失败
        {
            message.cmd = ALLCHATFAIL; // 群聊失败
            memset(message.content, 0, sizeof(message.content));
            strcpy(message.content, "你并不属于要进入的群聊里,进入群聊最终失败!");
            TcpServerSend(clientfd, &message, sizeof(message));
        }
        else // 在这个群里 群聊成功
        {
            message.cmd = ALLCHATSUCCESS;
            char ptr[DEFAULT_SIZE] = {0};

            // 查看群里在线的用户，每个用户都发一遍 除了自己
            memset(sql3, 0, sizeof(sql3));

            sprintf(sql3, "select name from %s where name <> '%s';", message.toName, userName);
            int row = GetTableVal(g_db, sql3, ptr, NULL, 0, 1);
            pthread_mutex_lock(&loginmutex);

            for (int idx = 1; idx <= row; idx++)
            {
                onLline infofd;
                memset(sql3, 0, sizeof(sql3));
                sprintf(sql3, "select name from %s where name <> '%s';", message.toName, userName);
                // 获取的是群内成员除自己外的每一个名字
                GetTableVal(g_db, sql3, ptr, NULL, 0, idx);
                // memset(sql3, 0, sizeof(sql3));
                // sprintf(sql3, "select Username from LoginClient where Username = '%s';", ptr);

                strncpy(infofd.name, ptr, sizeof(ptr) - 1);
                infofd.sockfd = -6;

                if (onLineIsContainVal(PonLine, (void *)&infofd)) // 群成员在线
                {
                    // 获取套接字
                    //  memset(sql3, 0, sizeof(sql3));
                    //  sprintf(sql3, "select * from LoginClient where Username = '%s';", ptr);
                    int val = 0;
                    onLineObtainValVal(PonLine, (void *)&infofd, &val, obtainFunc);

                    // printf("name:%s val:%d\n", info2.name, val);
                    // GetTableVal(g_db, sql3, NULL, &val, 1, 0);
                    TcpServerSend(val, &message, sizeof(message));
                }
            }
            pthread_mutex_unlock(&loginmutex);
            memset(sql3, 0, sizeof(sql3));
            sprintf(sql3, "insert into Log values('%s', '%s', '%s');", userName, message.content, message.toName);
            SqliteExec(g_db, sql3);
        }
    }
}

/* 添加好友 */
int addFriend(int clientfd, Msg message, const char *userName)
{
    // 先打印当前在线人员
    
    /**
     * PonLine
     * onPrintf
    */
//   onPrintf(PonLine, printFunc);


    char friendname[FRIENDNAMESIZE] = {0};
    TcpServerRecv(clientfd, friendname, sizeof(friendname));
    char sql2[SQLSIZE];
    memset(sql2, 0, sizeof(sql2));
    char name[FRIENDNAMESIZE] = {0};
    // sprintf(sql2,"select * from LoginClient where Clientfd = %d;", clientfd);
    // GetTableVal(g_db, sql2, name, NULL, 0);
    strncpy(name, userName, strlen(userName));
    memset(sql2, 0, sizeof(sql2));
    sprintf(sql2, "select * from %s where name = '%s';", name, friendname);
    printf("name=%s\n", name);
    printf("%s\n", sql2);
    
    // 查一下是否是自己
    if (strncmp(name, friendname, strlen(name)) == 0)
    {
        message.cmd = ADDFRIENDFAIL;
        memset(message.content, 0, sizeof(message.content));
        strcpy(message.content, "加好友失败,不可以添加自己为好友");
        TcpServerSend(clientfd, &message, sizeof(message));
        return 0;
    }

    // 查一遍是不是已经是好友， 是则不用再添加
    int ret = searchIsExist(g_db, sql2);
    if (ret != 0) // 已经是好友，添加失败
    {
        message.cmd = ADDFRIENDFAIL;
        memset(message.content, 0, sizeof(message.content));
        strcpy(message.content, "加好友失败,你要添加的好友已经是你的好友,不可以重复添加");
        TcpServerSend(clientfd, &message, sizeof(message));
    }
    else if (ret == 0) // 不是好友，查询是否在线
    {
        memset(sql2, 0, sizeof(sql2));
        // sprintf(sql2,"select * from LoginClient where Username = '%s';", friendname);
        // ret = searchIsExist(g_db, sql2);
        onLline info2;
        strncpy(info2.name, friendname, sizeof(friendname) - 1);
        info2.sockfd = -1;
        ret = onLineIsContainVal(PonLine, (void *)&info2);

        if (ret == 0) // 不在线
        {
            message.cmd = ADDFRIENDFAIL;
            memset(message.content, 0, sizeof(message.content));
            strcpy(message.content, "加好友失败,你要添加的好友处于不在线状态");
            TcpServerSend(clientfd, &message, sizeof(message));
        }
        else // 在线
        {
            memset(sql2, 0, sizeof(sql2));
            int status = 1; // 表示是好友
            sprintf(sql2, "insert into %s values('%s', %d);", name, friendname, status);
            if (SqliteExec(g_db, sql2) == true)
            {
                memset(sql2, 0, sizeof(sql2));
                sprintf(sql2, "insert into %s values('%s', %d);", friendname, name, status);
                SqliteExec(g_db, sql2); // 互加好友
                message.cmd = ADDFRIENDSUCCESS;
                TcpServerSend(clientfd, &message, sizeof(message));
            }
        }
    }
}

/* 私聊 */
int chatfriend(int clientfd, Msg message, const char *userName)
{
    int ret = 0;
    char sql3[SQLSIZE + BUFFER_SZIE] = {0};
    // 检查是否是自己
    

    // 先查一遍是不是自己的好友,再看是不是在线
    sprintf(sql3, "select * from %s where name = '%s' and flag = %d;", userName, message.toName, 1);
    ret = searchIsExist(g_db, sql3);
    if (ret == 0) // 不是自己的好友， 私聊失败
    {
        message.cmd = CHATFAIL;
        memset(message.content, 0, sizeof(message.content));
        strcpy(message.content, "你要私聊的对象不是你的好友, 私聊失败!");
        TcpServerSend(clientfd, &message, sizeof(message));
    }
    else // 是好友，查看是否在线
    {
        // 查询在线链表
        pthread_mutex_lock(&loginmutex);
        // int ret = searchIsExist(g_db, sql3);
        onLline info2;
        strncpy(info2.name, message.toName, sizeof(message.toName) - 1);
        info2.sockfd = -1;
        ret = onLineIsContainVal(PonLine, (void *)&info2);
        pthread_mutex_unlock(&loginmutex);
        if (ret == 0) // 不在线发不过去
        {
            message.cmd = CHATFAIL;
            memset(message.content, 0, sizeof(message.content));
            strcpy(message.content, "发送失败,你要私聊的好友不在线!");
            TcpServerSend(clientfd, &message, sizeof(message));
        }
        else // 在线， 可以发过去
        {
            // 获取私聊对象的套接字
            int tonamefd = 0;
            // 保存聊天记录到表里
            memset(sql3, 0, sizeof(sql3));
            sprintf(sql3, "insert into Log values('%s', '%s', '%s');", userName, message.content, message.toName);
            SqliteExec(g_db, sql3);

            onLineObtainValVal(PonLine, (void *)&info2, &tonamefd, obtainFunc);
            message.cmd = CHATSUCCESS;
            TcpServerSend(tonamefd, &message, sizeof(message));
        }
    }

    return ret;
}
/* 处理线程 */
void *clientHandler(void *arg)
{

    int clientfd = *((int *)arg);
    int tmp = clientfd;
    char userName[DEFAULT_SIZE] = {0};
    onLline info;
    while (1)
    {

        Msg message;

        if (TcpServerRecv(clientfd, &message, sizeof(message)) == false)
        {
            onLineRemove(PonLine, (void *)&info);
            onPrintf(PonLine, printFunc);
            break;
        }
        switch (message.cmd)
        {
        case LOGIN: // 登陆
            login(clientfd, message, userName, &info);
            break;
        case SIGNUP: // 注册
            enroll(clientfd, message);
            break;
        case CHAT: // 私聊
            chatfriend(clientfd, message, userName);
            break;
            // FreeDLlist(&l,NULL);
        case ALLCHAT: // 群聊
            enterGroup(clientfd, message, userName);
            break;
        case ADDFRIEND: // 添加朋友
            addFriend(clientfd, message, userName);
            break;
        case DELETEFRIEND: // 删除好友
            deletefriend(clientfd, message, userName);
            break;
        case HEART: // todo 心跳检测
            break;
        case BUILDGROUP: // 建立群聊
            buildGroup(clientfd, message, userName);
            break;
        case ADDGROUP: //
            addGroup(clientfd, message, userName);
            break;
        case QUITGROUP: //
            quitGroup(clientfd, message, userName);
            break;
        default:
            break;
        }
    }
    close(clientfd);
    pthread_exit(NULL);
}

/* 初始化数据库 */
void InitDB()
{
    g_db = InitSqlite("server.db"); // 本地文件
    SqliteExec(g_db, "create table if not exists SignupClient(Username text,Password text);");
    SqliteExec(g_db, "create table if not exists LoginClient(Username text, Clientfd integer);");
    SqliteExec(g_db, "create table if not exists Log(from_name text, log_data text, to_name text);");
    SqliteExec(g_db, "create table if not exists Queue(groupname text);");
    //    SqliteExec(g_db,"create table if not exists RootClient(Username text);");
    //    SqliteExec(g_db,"create table if not exists VipClient(Username text);");
}

int main()
{
    log_init();
    /* 服务器端初始化  创建 绑定 监听 */
    TcpS *server = TcpServerInit(ip, port);

    if (server == NULL)
    {
        log_close();
        return -1;
    }
    // 初始化数据库
    InitDB();

    // 初始化线程池
    ThreadP *pid = InitThreadPool(20, 10, 10);
    // 初始化锁
    pthread_mutex_init(&loginmutex, NULL);
    pthread_mutex_init(&groupmutex, NULL);
    /* 初始化在线表 */
    onLineInit(&PonLine, compareFunc); // todo
                                       /* 在线客户端链表初始化 */
    InitDLlist(&ClientList);
    int clientfd = 0;
    printf("启动服务器成功\n");
    while (clientfd = TcpAccept(server))
    {
        if (clientfd < 0)
            break;
        AddpoolTask(pid, clientHandler, &clientfd);
        // AddpoolTask(pid, clientHander2, &clientfd);
    }

    pthread_mutex_destroy(&loginmutex);
    pthread_mutex_destroy(&groupmutex);
    ClearTcpServer(server);
    ClearThreadPool(pid);
    log_close();
    return 0;
}
