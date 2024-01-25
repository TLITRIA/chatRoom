#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "StdThread.h"
#include "StdSqlite.h"
#include "MyString.h"
#include <string.h>
#include "StdThreadPool.h"
#include "DoubleLinkList.h"
#include "GlobalMessage.h"
#include "StdTcp.h"
#include <unistd.h>
#define ip "172.25.41.88"
#define port 8080
#define USERNAMESIZE 150
#define FRIENDNAMESIZE 20
#define SQLSIZE 150
static DLlist ClientList; // 客户端信息链表
static SQL *d;            // 数据库句柄

// 删除好友
int deletefriend(int clientfd, Msg m)
{
  int ret = 0;
  char friendname[FRIENDNAMESIZE] = {0};
  TcpServerRecv(clientfd, friendname, sizeof(friendname));
  char sql2[SQLSIZE];
  memset(sql2, 0, sizeof(sql2));
  char name[FRIENDNAMESIZE] = {0};
  sprintf(sql2, "select * from LoginClient where Clientfd = %d;", clientfd);
  GetTableVal(d, sql2, name, 0);
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
    memset(sql2, 0, sizeof(sql2));
    sprintf(sql2, "select * from LoginClient where Username = '%s';", friendname);
    ret = searchIsExist(d, sql2);
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

void *clientHandler(void *arg)
{
  int clientfd = *((int *)arg);
  int tmp = clientfd;
  while (1)
  {

    Msg m;
    if (TcpServerRecv(clientfd, &m, sizeof(m)) == false)
    {
      char sql[SQLSIZE] = {0};
      sprintf(sql, "delete from LoginClient where Clientfd = %d;", tmp);
      SqliteExec(d, sql);
      break;
    }
    CInfo *c;
    DLlist l;
    struct Node *n2;
    int ret = 0;
    int flag = 0;
    c = CreateInfo(m.fromName, clientfd);
    InsertDLlistTail(&ClientList, c);
    switch (m.cmd)
    {
    case LOGIN:
      char sql[USERNAMESIZE] = {0};
      sprintf(sql, "select * from SignupClient where Username = '%s' and Password = '%s';", m.fromName, m.password);
      ret = searchIsExist(d, sql);
      if (ret == 0)
      {
        // 填写的用户名或密码不正确
        flag = 0;
        TcpServerSend(clientfd, &flag, sizeof(flag));
        memset(m.content, 0, sizeof(m.content));
        strcpy(m.content, "你输入的账号或密码不正确, 请重新输入!");
        TcpServerSend(clientfd, &m, sizeof(m));
      }
      else
      {
        // 用户名密码正确，查询是否已经登录
        memset(sql, 0, sizeof(sql));
        sprintf(sql, "select * from LoginClient where Username = '%s';", m.fromName);
        ret = searchIsExist(d, sql);
        if (ret == 0)
        {
          // 用户未登录
          memset(sql, 0, sizeof(sql));
          sprintf(sql, "insert into LoginClient values('%s', %d);", m.fromName, clientfd);
          if (SqliteExec(d, sql) == true)
          {
            flag = 1; // 用户成功登录标志位
            TcpServerSend(clientfd, &flag, sizeof(flag));
          }
        }
        else
        {
          // 用户已经在其他地方登录
          flag = 0;
          TcpServerSend(clientfd, &flag, sizeof(flag));
          memset(m.content, 0, sizeof(m.content));
          strcpy(m.content, "你要登录的账号已经在异地登录,请重新输入");
          TcpServerSend(clientfd, &m, sizeof(m));
        }
      }
      break;
    case SIGNUP:

      char username[USERNAMESIZE] = {0};
      sprintf(username, "select * from SignupClient where Username = '%s';", m.fromName);
      ret = searchIsExist(d, username);
      // ret = findusername(m.fromName,d,"select Username from SignupClient;");
      if (ret == 0)
      {
        flag = 1;
        TcpServerSend(clientfd, &flag, sizeof(flag));
        TcpServerRecv(clientfd, &m, sizeof(m));
        ret = searchIsExist(d, username);
        if (ret == 0)
        {
          char sql[100] = {0};
          sprintf(sql, "insert into SignupClient values('%s','%s');", m.fromName, m.password);
          if (SqliteExec(d, sql) == true)
          {
            strcpy(m.content, "注册成功!");
            // m.cmd = signupsuccess;
            TcpServerSend(clientfd, &m, sizeof(m));

            // 建立一个以用户名为表名的表,用来存储好友名单与群列表
            memset(sql, 0, sizeof(sql));

            sprintf(sql, "create table if not exists %s(name text, flag integer);", m.fromName);
            SqliteExec(d, sql);
          }
        }
        else
        {
          flag = 0;
          TcpServerSend(clientfd, &flag, sizeof(flag));
          // sleep(1);
          strcpy(m.content, "注册失败, 由于你输入密码太慢你注册的姓名已被别人抢先!");
          TcpServerSend(clientfd, &m, sizeof(m));
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
      break;
    case CHAT:
      l = FindByElement(&ClientList, m.toName, IsNameSame);
      if (l.len == 0)
      {
        printf("没找到要发送给的用户%s,转发失败！\n", m.toName);
        break;
      }
      struct Node *n = l.head->next;
      while (n != NULL)
      {
        CInfo *info = (CInfo *)n->value;
        TcpServerSend(info->sock, &m, sizeof(m));
        printf("给%s发消息:%s\n", m.toName, m.content);
        n = n->next;
      }
      break;
      // FreeDLlist(&l,NULL);
    case ALLCHAT:
      n2 = ClientList.head->next;
      while (n2 != NULL)
      {
        CInfo *info = (CInfo *)n2->value;
        if (strcmp(info->Name, m.fromName) != 0)
          TcpServerSend(info->sock, &m, sizeof(m));
        printf("给所有人发消息:%s\n", m.content);
        n2 = n2->next;
      }
      break;
    case ADDFRIEND:
      char friendname[FRIENDNAMESIZE] = {0};
      TcpServerRecv(clientfd, friendname, sizeof(friendname));
      char sql2[SQLSIZE];
      memset(sql2, 0, sizeof(sql2));
      char name[FRIENDNAMESIZE] = {0};
      sprintf(sql2, "select * from LoginClient where Clientfd = %d;", clientfd);
      GetTableVal(d, sql2, name, 0);
      memset(sql2, 0, sizeof(sql2));
      sprintf(sql2, "select * from %s where name = '%s';", name, friendname);
      printf("%s\n", sql2);
      // 先查一遍是不是已经是好友， 是则不用再添加
      ret = searchIsExist(d, sql2);
      if (ret != 0) // 已经是好友，添加失败
      {
        m.cmd = ADDFRIENDFAIL;
        memset(m.content, 0, sizeof(m.content));
        strcpy(m.content, "加好友失败,你要添加的好友已经是你的好友,不可以重复添加");
        TcpServerSend(clientfd, &m, sizeof(m));
      }
      else if (ret == 0) // 不是好友，查询是否在线
      {
        memset(sql2, 0, sizeof(sql2));
        sprintf(sql2, "select * from LoginClient where Username = '%s';", friendname);
        ret = searchIsExist(d, sql2);
        if (ret == 0) // 不在线
        {
          m.cmd = ADDFRIENDFAIL;
          memset(m.content, 0, sizeof(m.content));
          strcpy(m.content, "加好友失败,你要添加的好友处于不在线状态");
          TcpServerSend(clientfd, &m, sizeof(m));
        }
        else // 在线
        {
          memset(sql2, 0, sizeof(sql2));
          int status = 1; // 表示是好友
          sprintf(sql2, "insert into %s values('%s', %d);", name, friendname, status);
          if (SqliteExec(d, sql2) == true)
          {
            memset(sql2, 0, sizeof(sql2));
            sprintf(sql2, "insert into %s values('%s', %d);", friendname, name, status);
            SqliteExec(d, sql2); // 互加好友
            m.cmd = ADDFRIENDSUCCESS;
            TcpServerSend(clientfd, &m, sizeof(m));
          }
        }
      }
      break;
    case DELETEFRIEND:
      // char friendname[FRIENDNAMESIZE] = {0};
      // TcpServerRecv(clientfd, friendname, sizeof(friendname));
      // char sql2[SQLSIZE];
      // memset(sql2, 0, sizeof(sql2));
      // char name[FRIENDNAMESIZE] = {0};
      // sprintf(sql2,"select * from LoginClient where Clientfd = %d;", clientfd);
      // GetTableVal(d, sql2, name, 0);
      // memset(sql2, 0, sizeof(sql2));
      // sprintf(sql2,"select * from %s where name = '%s';", name, friendname);
      // printf("%s\n", sql2);
      // //先查一遍是不是已经是好友， 是则可以删除, 不是则无法删除
      // ret = searchIsExist(d, sql2);
      // if(ret == 0)  //不是好友，删除失败
      // {
      //   m.cmd = DELETEFRIENDFAIL;
      //   memset(m.content, 0, sizeof(m.content));
      //   strcpy(m.content, "删除好友好友失败,你要删除的好友并不是你的好友,不可以删除");
      //   TcpServerSend(clientfd, &m, sizeof(m));
      // }
      // else if(ret != 0) //是好友, 可以删除, 查是否在线
      // {
      //   memset(sql2, 0 , sizeof(sql2));
      //   sprintf(sql2,"select * from LoginClient where Username = '%s';", friendname);
      //   ret = searchIsExist(d, sql2);
      //   if(ret == 0)  //不在线
      //   {
      //     m.cmd = DELETEFRIENDFAIL;
      //     memset(m.content, 0, sizeof(m.content));
      //     strcpy(m.content, "删除好友失败,你要删除的好友处于不在线状态");
      //     TcpServerSend(clientfd, &m, sizeof(m));
      //   }
      //   else  //在线
      //   {
      //     memset(sql2, 0, sizeof(sql2));
      //     int status = 1;  //表示是好友, 删除
      //     sprintf(sql2,"delete from %s where name = '%s' and flag = %d;", name, friendname, status);
      //     if(SqliteExec(d,sql2) == true)
      //     {
      //         memset(sql2, 0, sizeof(sql2));
      //         sprintf(sql2,"delete from %s where name = '%s' and flag = %d;", friendname, name, status);
      //         SqliteExec(d,sql2);  //互删好友
      //         m.cmd = ADDFRIENDSUCCESS;
      //         TcpServerSend(clientfd, &m, sizeof(m));
      //     }
      //   }
      // }
      deletefriend(clientfd, m);
      break;
    default:
      break;
    }
  }
  close(clientfd);
}

void InitDB()
{
  d = InitSqlite("server.db"); // 本地文件
  SqliteExec(d, "create table if not exists SignupClient(Username text,Password text);");
  SqliteExec(d, "create table if not exists LoginClient(Username text, Clientfd integer);");
  //    SqliteExec(d,"create table if not exists RootClient(Username text);");
  //    SqliteExec(d,"create table if not exists VipClient(Username text);");
}

int main()
{
  TcpS *s = InitTcpServer(ip, port);
  if (s == NULL)
    return -1;
  // 初始化数据库
  InitDB();
  // 初始化线程池
  ThreadP *p = InitThreadPool(20, 10, 10);
  InitDLlist(&ClientList);
  int clientfd = 0;
  printf("启动服务器成功\n");
  while (clientfd = TcpAccept(s))
  {
    if (clientfd < 0)
      break;

    AddpoolTask(p, clientHandler, &clientfd);
  }
  ClearTcpServer(s);
  ClearThreadPool(p);
  return 0;
}