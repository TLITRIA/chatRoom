#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sqlite3.h>
/* C\S共有的，要不要分？ */
#include "StdTcp.h"
#include "StdThread.h"
#include "GlobalMessage.h"
/* 数据库 */
#include "DoubleLinkList.h"

/* server独有的 */
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

static DLlist ClientList; // 客户端信息链表
static SQL *d;            // 数据库句柄
onLineOutside *PonLine = NULL; //在线表

//锁
pthread_mutex_t loginmutex;
pthread_mutex_t groupmutex;


/* ======日志调试====== */
#define DEBUG_FLAG "../Log/my_debug.flag"
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


// todo
int obtainFunc(ELEMENTTYPE val)
{
    onLline *num = (onLline *)val;
    return num->sockfd;
}

/* 比较基础数据*/
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

/* 登录*/
int login(int clientfd, Msg m, char *Name, onLline *info)
{
    int flag = 0;
    char sql[USERNAMESIZE] = {0};           
    sprintf(sql, "select * from SignupClient where Username = '%s' and Password = '%s';", m.fromName, m.password);
    int ret = searchIsExist(d, sql);
    if(ret == 0)
    {
      //填写的用户名或密码不正确
      int flag = 0;
      TcpServerSend(clientfd, &flag, sizeof(flag));
      memset(m.content, 0, sizeof(m.content));
      strcpy(m.content, "你输入的账号或密码不正确, 请重新输入!");
      TcpServerSend(clientfd, &m, sizeof(m));
    } 
    else
    {
      strncpy(info->name, m.fromName, sizeof(m.fromName) - 1);
      info->sockfd = clientfd;
      ret = onLineIsContainVal(PonLine, (void *)info);
      if(ret == 0)
      {
        //用户未登录
        if(onLineInsert(PonLine, (void *)info))
        {
            //记录客户端姓名
            strcpy(Name, m.fromName);
            onPrintf(PonLine, printFunc);

            flag = 1;  //用户成功登录标志位
            TcpServerSend(clientfd, &flag, sizeof(flag));
        }
        
      }
      else
      {
        //用户已经在其他地方登录
        flag = 0;
        TcpServerSend(clientfd, &flag, sizeof(flag));
        memset(m.content, 0, sizeof(m.content));
        strcpy(m.content, "你要登录的账号已经在异地登录,请重新输入");
        TcpServerSend(clientfd, &m, sizeof(m));
      }
    }
}

/* 注册*/
int enroll(int clientfd, Msg m)
{
    int flag = 0;
    char username[USERNAMESIZE] = {0};
    sprintf(username, "select * from SignupClient where Username = '%s';", m.fromName);
    int ret = searchIsExist(d, username);
    // ret = findusername(m.fromName,d,"select Username from SignupClient;");
    if(ret == 0)
    {
        flag = 1;
        TcpServerSend(clientfd, &flag, sizeof(flag));
        TcpServerRecv(clientfd, &m, sizeof(m));
        ret = searchIsExist(d, username);
        if(ret == 0)
        {
          char sql[100] = {0};
          sprintf(sql,"insert into SignupClient values('%s','%s');",m.fromName,m.password);
          if(SqliteExec(d,sql) == true)
          {
              strcpy(m.content,"注册成功!");
              // m.cmd = signupsuccess;
              TcpServerSend(clientfd,&m,sizeof(m));
              
              //建立一个以用户名为表名的表,用来存储好友名单与群列表
              memset(sql, 0, sizeof(sql));
              
              sprintf(sql,"create table if not exists %s(name text, flag integer);", m.fromName);
              SqliteExec(d, sql);
          }
        }
        else
        {
          flag = 0;
          TcpServerSend(clientfd, &flag, sizeof(flag));
          // sleep(1);
          strcpy(m.content, "注册失败, 由于你输入密码太慢你注册的姓名已被别人抢先!");
          TcpServerSend(clientfd, &m,sizeof(m));
        }
        
    }
    else
    {
      flag = 0;
      TcpServerSend(clientfd, &flag, sizeof(flag));
      memset(m.content, 0, sizeof(m.content));
      sleep(1);
      strcpy(m.content, "注册失败, 你注册的姓名已被别人抢先或与他人重复!");
      TcpServerSend(clientfd, &m, sizeof(m));
    }

}

//删除好友
int deletefriend(int clientfd, Msg m, const char * Name)
{
  int ret = 0;
  char friendname[FRIENDNAMESIZE] = {0};
  TcpServerRecv(clientfd, friendname, sizeof(friendname));
  char sql2[SQLSIZE];
  memset(sql2, 0, sizeof(sql2));
  //从在线链表中获取结果集
  char name[FRIENDNAMESIZE] = {0};
  // sprintf(sql2,"select * from LoginClient where Clientfd = %d;", clientfd);
  // GetTableVal(d, sql2, name, NULL, 0);
  strncpy(name, Name, strlen(Name));

  memset(sql2, 0, sizeof(sql2));
  sprintf(sql2, "select * from %s where name = '%s';", name, friendname);
  printf("%s\n", sql2);
  // 先查一遍是不是已经是好友， 是则可以删除, 不是则无法删除
  ret = searchIsExist(d, sql2);
  if (ret == 0) // 不是好友，删除失败
  {
    m.cmd = DELETEFRIENDFAIL;
    memset(m.content, 0, sizeof(m.content));
    strcpy(m.content, "删除好友好友失败,你要删除的好友并不是你的好友,不可以删除");
    TcpServerSend(clientfd, &m, sizeof(m));
  }
  else if (ret != 0) // 是好友, 可以删除, 查是否在线
  {
    // memset(sql2, 0, sizeof(sql2));
    // sprintf(sql2, "select * from LoginClient where Username = '%s';", friendname);
    // ret = searchIsExist(d, sql2);
    onLline info3;
    strncpy(info3.name, friendname, sizeof(friendname) - 1);
    info3.sockfd = -1;
    ret = onLineIsContainVal(PonLine, (void *)&info3);

    if (ret == 0) // 不在线
    {
      m.cmd = DELETEFRIENDFAIL;
      memset(m.content, 0, sizeof(m.content));
      strcpy(m.content, "删除好友失败,你要删除的好友处于不在线状态");
      TcpServerSend(clientfd, &m, sizeof(m));
    }
    else // 在线
    {
      memset(sql2, 0, sizeof(sql2));
      int status = 1; // 表示是好友, 删除
      sprintf(sql2, "delete from %s where name = '%s' and flag = %d;", name, friendname, status);
      if (SqliteExec(d, sql2) == true)
      {
        memset(sql2, 0, sizeof(sql2));
        sprintf(sql2, "delete from %s where name = '%s' and flag = %d;", friendname, name, status);
        SqliteExec(d, sql2); // 互删好友
        m.cmd = DELETEFRIENDSUCCESS;
        TcpServerSend(clientfd, &m, sizeof(m));
      }
    }
  }
  return ret;
}

//建立群聊
int buildGroup(int clientfd, Msg m, const char *Name)
{
  char groupname[GROUPNAMESIZE + BUFFER_SZIE];
  memset(groupname, 0, sizeof(groupname));
  strcpy(groupname, m.content);
  char sql5[SQLSIZE + BUFFER_SZIE] = {0};
  sprintf(sql5, "select * from Queue where groupname = '%s';", groupname);

  //加锁
  pthread_mutex_lock(&groupmutex);
  int ret = searchIsExist(d, sql5);
  if(ret != 0)  //已经有其他客户端建立了群，不可建立群名一样的群
  {
      pthread_mutex_unlock(&groupmutex);
      m.cmd = BUILDGROUPFAIL;
      memset(m.content, 0, sizeof(m.content));
      strcpy(m.content, "建群失败,已经有其他客户端建立了群,不可建立群名一样的群");
      TcpServerSend(clientfd, &m, sizeof(m));
      
  }
  else
  {
      //可以建群
      //m.cmd = BUILDGROUPSUCCESS;
      printf("%s\n", Name);
      memset(sql5, 0, sizeof(sql5));
      //插入到唯一的群表
      sprintf(sql5, "insert into Queue values('%s');", m.content);
      if(SqliteExec(d, sql5) == true)
      {
          pthread_mutex_unlock(&groupmutex);
          memset(sql5, 0, sizeof(sql5));
          sprintf(sql5, "create table if not exists %s(name text);", m.content);
          if(SqliteExec(d, sql5) == true)
          {
              memset(sql5, 0, sizeof(sql5));
              //把群主插入到他所建立的群里面
              sprintf(sql5, "insert into %s values('%s');", m.content, Name);
              if(SqliteExec(d, sql5) == true)
              {
                  memset(sql5, 0, sizeof(sql5));
                  //在群主的那个客户端表里插入群
                  sprintf(sql5, "insert into %s values('%s', 2);", Name, m.content);
                  SqliteExec(d, sql5);
                  m.cmd = BUILDGROUPSUCCESS;
                  TcpServerSend(clientfd, &m, sizeof(m));
              }
          }
          
      }
      

  }
  return ret;
}

//加入群
int addGroup(int clientfd, Msg m, const char *Name)
{
  char groupname[GROUPNAMESIZE + BUFFER_SZIE];
  memset(groupname, 0, sizeof(groupname));
  strcpy(groupname, m.content);
  char sql5[SQLSIZE + BUFFER_SZIE] = {0};
  sprintf(sql5, "select * from Queue where groupname = '%s';", groupname);
  
  //加锁
  pthread_mutex_lock(&groupmutex);
  //先查看有没有这个群
  int ret = searchIsExist(d, sql5);
  if(ret == 0)  //没有这个群, 加群失败
  {
    
      pthread_mutex_unlock(&groupmutex);
      m.cmd = ADDGROUPFAIL;
      memset(m.content, 0, sizeof(m.content)); 
      strcpy(m.content, "加群失败,群表里不存在你要加的群");
      TcpServerSend(clientfd, &m, sizeof(m));
  }
  else  //存在这个群， 分两种情况, 用户在与不在这个群， 在则加群失败， 不在则加群成功
  {
      pthread_mutex_unlock(&groupmutex);

      memset(sql5, 0, sizeof(sql5));
      sprintf(sql5, "select * from %s where name = '%s';", groupname, Name);
      ret = searchIsExist(d, sql5);
      if(ret != 0)  //用户存在这个群里， 加群失败
      {
        m.cmd = ADDGROUPFAIL;
        memset(m.content, 0, sizeof(m.content));
        strcpy(m.content, "你已经处在你要加入的群里, 不可以重复加入!");
        TcpServerSend(clientfd, &m, sizeof(m));
      }
      else     //用户不存在这个群里里， 可以加群
      {
        memset(sql5, 0, sizeof(sql5));
        //把客户导入到所要加的群表里
        sprintf(sql5, "insert into %s values('%s');", groupname, Name);
        if(SqliteExec(d, sql5) == true)  
        {
            //更新客户那张表， 增加一个群
            memset(sql5, 0, sizeof(sql5));
            sprintf(sql5, "insert into %s values('%s', 2);", Name, groupname);
            if(SqliteExec(d, sql5) == true)
            {
              m.cmd = ADDGROUPSUCCESS;
              memset(m.content, 0, sizeof(m.content));
              TcpServerSend(clientfd, &m, sizeof(m));
            }
      
        }
      }
  }

  return ret;
}

/* 退群*/
int quitGroup(int clientfd, Msg m, const char *Name)
{
  char groupname[GROUPNAMESIZE + BUFFER_SZIE];
  memset(groupname, 0, sizeof(groupname));
  strcpy(groupname, m.content);
  char sql5[SQLSIZE + BUFFER_SZIE] = {0};
  sprintf(sql5, "select * from Queue where groupname = '%s';", groupname);
  
  //加锁
  pthread_mutex_lock(&groupmutex);
  //先查看有没有这个群 
  int ret = searchIsExist(d, sql5); 
  if(ret == 0)  //没有这个群 , 退群失败
  {
        pthread_mutex_unlock(&groupmutex);
        m.cmd = QUITGROUPFAIL;
        memset(m.content, 0, sizeof(m.content)); 
        strcpy(m.content, "退群失败,群表里不存在你要退的群");
        TcpServerSend(clientfd, &m, sizeof(m));
  }
  else     //存在这个群， 分两种情况, 用户在与不在这个群， 在则退群成功，不在则退群失败
  {
      pthread_mutex_unlock(&groupmutex);

      memset(sql5, 0, sizeof(sql5));
      sprintf(sql5, "select * from %s where name = '%s';", groupname, Name);
      ret = searchIsExist(d, sql5);
      if(ret == 0)  //用户不存在存在这个群里， 退群失败
      {
        m.cmd = QUITGROUPFAIL;
        memset(m.content, 0, sizeof(m.content));
        strcpy(m.content, "你并不处在你要退出的群里, 不可以退群!");
        TcpServerSend(clientfd, &m, sizeof(m));
      }
      else
      {
        //用户存在这个群里里， 可以退群
      
        memset(sql5, 0, sizeof(sql5));
        //把客户从所要退的群表里删除
        sprintf(sql5, "delete from %s where name = '%s';", groupname, Name);
        if(SqliteExec(d, sql5) == true)  
        {
            //更新客户那张表， 删除一个群
            memset(sql5, 0, sizeof(sql5));
            sprintf(sql5, "delete from %s where name = '%s';", Name, groupname);
            if(SqliteExec(d, sql5) == true)
            {
              m.cmd = QUITGROUPSUCCESS;
              memset(m.content, 0, sizeof(m.content));
              TcpServerSend(clientfd, &m, sizeof(m));
            }
      
        }
      
      }
  }

  return ret;
}

/* 进入群聊*/
int enterGroup(int clientfd, Msg m, const char *Name)
{
    //先去查有没有这个群
    char sql3[BUFFER_SZIE + USERNAMESIZE] = {0};
    sprintf(sql3, "select * from Queue where groupname = '%s';", m.toName);
    int ret = searchIsExist(d, sql3);
    if(ret == 0) //不存在这个群  进入群聊失败
    {
      m.cmd = ALLCHATFAIL; //群聊失败
      memset(m.content, 0, sizeof(m.content));
      strcpy(m.content, "进入群聊失败, 你要进入的群聊不存在!");
      TcpServerSend(clientfd, &m, sizeof(m));
    }
    else  //存在这个群，看自己在不在这个群里
    {
        memset(sql3, 0, sizeof(sql3));
        sprintf(sql3, "select * from %s where name = '%s';", m.toName, Name);
        ret = searchIsExist(d, sql3);
        if(ret == 0)  //自己不在这个群, 群聊失败
        {
          m.cmd = ALLCHATFAIL;  //群聊失败
          memset(m.content, 0, sizeof(m.content));
          strcpy(m.content, "你并不属于要进入的群聊里,进入群聊最终失败!");
          TcpServerSend(clientfd, &m, sizeof(m));
        }
        else  //在这个群里 群聊成功
        {
          m.cmd = ALLCHATSUCCESS; 
          char ptr[DEFAULT_SIZE] = {0};

          //查看群里在线的用户，每个用户都发一遍 除了自己
          memset(sql3, 0, sizeof(sql3));
          
          sprintf(sql3, "select name from %s where name <> '%s';", m.toName, Name);
          int row = GetTableVal(d, sql3, ptr, NULL, 0, 1);
          pthread_mutex_lock(&loginmutex);

          for(int idx = 1; idx <=row; idx++)
          {
            onLline infofd;
            memset(sql3, 0, sizeof(sql3));
            sprintf(sql3, "select name from %s where name <> '%s';", m.toName, Name);
            //获取的是群内成员除自己外的每一个名字
            GetTableVal(d, sql3, ptr, NULL, 0, idx);
            // memset(sql3, 0, sizeof(sql3));
            // sprintf(sql3, "select Username from LoginClient where Username = '%s';", ptr);
          
            strncpy(infofd.name, ptr, sizeof(ptr) - 1);
            infofd.sockfd = -6;                   

            if(onLineIsContainVal(PonLine, (void *)&infofd)) //群成员在线
            {
              //获取套接字  
              // memset(sql3, 0, sizeof(sql3));
              // sprintf(sql3, "select * from LoginClient where Username = '%s';", ptr);
              int val = 0;
              onLineObtainValVal(PonLine, (void *)&infofd, &val, obtainFunc);

              // printf("name:%s val:%d\n", info2.name, val);
              // GetTableVal(d, sql3, NULL, &val, 1, 0);
              TcpServerSend(val, &m, sizeof(m));
            }
          }
          pthread_mutex_unlock(&loginmutex);
          memset(sql3, 0, sizeof(sql3));
          sprintf(sql3, "insert into Log values('%s', '%s', '%s');", Name, m.content, m.toName);
          SqliteExec(d, sql3);
        }
    }

}

/* 添加好友*/
int addFriend(int clientfd, Msg m, const char *Name)
{
    char friendname[FRIENDNAMESIZE] = {0};
    TcpServerRecv(clientfd, friendname, sizeof(friendname));
    char sql2[SQLSIZE];
    memset(sql2, 0, sizeof(sql2));
    char name[FRIENDNAMESIZE] = {0};
    // sprintf(sql2,"select * from LoginClient where Clientfd = %d;", clientfd);
    // GetTableVal(d, sql2, name, NULL, 0);
    strncpy(name, Name, strlen(Name));
    memset(sql2, 0, sizeof(sql2));
    sprintf(sql2,"select * from %s where name = '%s';", name, friendname);
    printf("name=%s\n", name);
    printf("%s\n", sql2);
    //先查一遍是不是已经是好友， 是则不用再添加
    int ret = searchIsExist(d, sql2);
    if(ret != 0)  //已经是好友，添加失败
    {
      m.cmd = ADDFRIENDFAIL;
      memset(m.content, 0, sizeof(m.content));
      strcpy(m.content, "加好友失败,你要添加的好友已经是你的好友,不可以重复添加");
      TcpServerSend(clientfd, &m, sizeof(m));
    }
    else if(ret == 0) //不是好友，查询是否在线
    {
      memset(sql2, 0 , sizeof(sql2));
      // sprintf(sql2,"select * from LoginClient where Username = '%s';", friendname);
      // ret = searchIsExist(d, sql2);
      onLline info2;
      strncpy(info2.name, friendname, sizeof(friendname) - 1);
      info2.sockfd = -1;
      ret = onLineIsContainVal(PonLine, (void *)&info2);

      if(ret == 0)  //不在线
      {
        m.cmd = ADDFRIENDFAIL;
        memset(m.content, 0, sizeof(m.content));
        strcpy(m.content, "加好友失败,你要添加的好友处于不在线状态");
        TcpServerSend(clientfd, &m, sizeof(m));
      }
      else  //在线
      {
        memset(sql2, 0, sizeof(sql2));
        int status = 1;  //表示是好友
        sprintf(sql2,"insert into %s values('%s', %d);", name, friendname, status);
        if(SqliteExec(d,sql2) == true)
        {
            memset(sql2, 0, sizeof(sql2));
            sprintf(sql2,"insert into %s values('%s', %d);", friendname, name, status);
            SqliteExec(d,sql2);  //互加好友
            m.cmd = ADDFRIENDSUCCESS;
            TcpServerSend(clientfd, &m, sizeof(m));
        }
      }
    }


}

int newFriends(int clientfd, Msg m, const char *Name)
{
  char sql[SQLSIZE];
  memset(sql, 0, sizeof(sql));
  char **result = NULL;
  int row = 0;
  int column = 0;
  char * errormsg = NULL;
  
  sprintf(sql,"select * from newFriends where to_name = '%s';", Name);
  int ret = sqlite3_get_table(d, sql, &result, &row, &column, &errormsg);
  if (ret != SQLITE_OK)
  {
        printf("sqlite3_get_table error:%s\n", errormsg);
        exit(-1);
  }
  for (int idx = 0; idx < row; idx++)
  {
    printf("%s\n", result[idx][0]);
    memset(m.content, 0, sizeof(m.fromName));
    strcpy(m.content, result[idx][0]);
    TcpServerSend(clientfd, &m, sizeof(m));
  }

  return 0;
}

int agreeWith(Msg m, const char *Name)
{
  char sql[SQLSIZE];
  memset(sql, 0, sizeof(sql));

  int status = 1;  //表示是好友
  sprintf(sql,"insert into %s values('%s', %d);", Name, m.toName, status);
  if(SqliteExec(d,sql) == true)
  {
      memset(sql, 0, sizeof(sql));
      sprintf(sql,"insert into %s values('%s', %d);", m.toName, Name, status);
      SqliteExec(d, sql);  //互加好友
  }
  onLline infofd;
  strcpy(infofd.name, m.toName);
  infofd.sockfd = 0;
  if(onLineIsContainVal(PonLine, (void *)&infofd)) //群成员在线
  {
    //获取套接字  
    // memset(sql3, 0, sizeof(sql3));
    // sprintf(sql3, "select * from LoginClient where Username = '%s';", ptr);
    int val = 0;
    onLineObtainValVal(PonLine, (void *)&infofd, &val, obtainFunc);
    memset(&m, 0, sizeof(m));
    m.cmd = ADDFRIENDFAIL;
    strcpy(m.fromName, Name);
    strcpy(m.content, "同意加好友");
    TcpServerSend(val, &m, sizeof(m));
  }
  return 0;
}

int refuse(Msg m, const char *Name)
{
  onLline infofd;
  strcpy(infofd.name, m.toName);
  infofd.sockfd = 0;
  if(onLineIsContainVal(PonLine, (void *)&infofd)) //群成员在线
  {
    //获取套接字  
    // memset(sql3, 0, sizeof(sql3));
    // sprintf(sql3, "select * from LoginClient where Username = '%s';", ptr);
    int val = 0;
    onLineObtainValVal(PonLine, (void *)&infofd, &val, obtainFunc);
    memset(&m, 0, sizeof(m));
    m.cmd = ADDFRIENDFAIL;
    strcpy(m.fromName, Name);
    strcpy(m.content, "拒绝加好友");
    TcpServerSend(val, &m, sizeof(m));
  }
  return 0;
}


/* 私聊*/
int chatfriend(int clientfd, Msg m, const char *Name)
{
  int ret = 0;
  char sql3[SQLSIZE + BUFFER_SZIE] = {0};
  
  //先查一遍是不是自己的好友,再看是不是在线
  sprintf(sql3, "select * from %s where name = '%s' and flag = %d;", Name, m.toName, 1);
  ret = searchIsExist(d, sql3);
  if(ret == 0) //不是自己的好友， 私聊失败
  {
      
      m.cmd = CHATFAIL;
      memset(m.content, 0, sizeof(m.content));
      strcpy(m.content, "你要私聊的对象不是你的好友, 私聊失败!");
      TcpServerSend(clientfd, &m, sizeof(m));
  } 
  else    //是好友，查看是否在线
  {
      // //查询在线链表
      pthread_mutex_lock(&loginmutex);
      // int ret = searchIsExist(d, sql3);
      onLline info2;
      strncpy(info2.name, m.toName, sizeof(m.toName) - 1);
      info2.sockfd = -1;
      ret = onLineIsContainVal(PonLine, (void *)&info2);
      pthread_mutex_unlock(&loginmutex);
      if(ret == 0)    //不在线发不过去
      {
        m.cmd = CHATFAIL;
        memset(m.content, 0, sizeof(m.content));
        strcpy(m.content, "发送失败,你要私聊的好友不在线!");
        TcpServerSend(clientfd, &m, sizeof(m));
      }
      else //在线， 可以发过去
      {
        //获取私聊对象的套接字
        int tonamefd = 0;
        //保存聊天记录到表里
        memset(sql3, 0 , sizeof(sql3));
        sprintf(sql3, "insert into Log values('%s', '%s', '%s');", Name, m.content, m.toName);
        SqliteExec(d, sql3);

        onLineObtainValVal(PonLine,(void *)&info2, &tonamefd, obtainFunc);
        m.cmd = CHATSUCCESS;
        TcpServerSend(tonamefd, &m, sizeof(m));
      }
  }

  return ret;
}
/* 处理线程 */
void* clientHandler(void *arg)
{

   int clientfd =* ((int*) arg);
   int tmp = clientfd;
   char Name[DEFAULT_SIZE] = {0};
   onLline info;
   while(1)
  {

    Msg m;
    
    if (TcpServerRecv(clientfd, &m, sizeof(m)) == false)
    {   
      onLineRemove(PonLine, (void *)&info);
      onPrintf(PonLine, printFunc);
      break;
    }
    switch (m.cmd)
    {
            case LOGIN:
                login(clientfd, m, Name, &info);
                break;
            case SIGNUP:
                enroll(clientfd, m);
                break;
            case CHAT:
                chatfriend(clientfd, m, Name);
                break;
                // FreeDLlist(&l,NULL);
            case ALLCHAT:
                enterGroup(clientfd, m, Name);
                break;
            case ADDFRIEND:
                addFriend(clientfd, m, Name);
                break;
            case DELETEFRIEND:
                deletefriend(clientfd, m, Name);
                break;
            case HEART:
                break;
            case BUILDGROUP:
                buildGroup(clientfd, m, Name);
                break;
            case ADDGROUP:              
                addGroup(clientfd, m, Name);
                break;
            case QUITGROUP:
                quitGroup(clientfd, m, Name);
                break;
            case NEWFRIENDS:
                newFriends(clientfd, m, Name);
                break;
            case FILEAGREE:

                break;
            case FILEREFUSE:
              
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
   d = InitSqlite("server.db");  //本地文件
   SqliteExec(d,"create table if not exists SignupClient(Username text,Password text);");
   SqliteExec(d,"create table if not exists LoginClient(Username text, Clientfd integer);");
   SqliteExec(d, "create table if not exists Log(from_name text, log_data text, to_name text);");
   SqliteExec(d, "create table if not exists Queue(groupname text);");
   SqliteExec(d,"create table if not exists newFriends(Username text, to_name text);");
//    SqliteExec(d,"create table if not exists RootClient(Username text);");
//    SqliteExec(d,"create table if not exists VipClient(Username text);");
}



int main()
{
  log_init();
   TcpS *server = TcpServerInit(ip, port);
   if(server == NULL)
          return -1;
    //初始化数据库
    InitDB();      
   //初始化线程池
    ThreadP *p = InitThreadPool(20,10,10);
   //初始化锁
   pthread_mutex_init(&loginmutex, NULL);
   pthread_mutex_init(&groupmutex, NULL);
   /* 初始化在线表*/
   onLineInit(&PonLine, compareFunc);

    InitDLlist(&ClientList);
    int clientfd = 0;
    printf("启动服务器成功\n");
    while(clientfd = TcpAccept(server))
    { 
        if(clientfd < 0)
                   break;
        
        AddpoolTask(p,clientHandler,&clientfd);
        // AddpoolTask(p, clientHander2, &clientfd);
        
    }

  pthread_mutex_destroy(&loginmutex);
  pthread_mutex_destroy(&groupmutex);   
  ClearTcpServer(server);
  ClearThreadPool(p);
  log_close();
  return 0;
}
