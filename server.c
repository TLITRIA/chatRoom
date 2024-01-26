#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "StdThread.h"
#include "StdSqlite.h"
#include "MyString.h"
#include <string.h>
#include <pthread.h>
#include "StdThreadPool.h"
#include "DoubleLinkList.h"
#include "GlobalMessage.h"
#include "StdTcp.h"
#include <unistd.h>


#define ip "127.0.0.1"
#define port 8080
#define USERNAMESIZE 150
#define FRIENDNAMESIZE 20
#define GROUPNAMESIZE 20
#define SQLSIZE 150
static DLlist ClientList; // 客户端信息链表
static SQL *d;            // 数据库句柄

//锁
pthread_mutex_t loginmutex;
pthread_mutex_t groupmutex;

//锁
pthread_mutex_t loginmutex;
pthread_mutex_t groupmutex;

//删除好友
int deletefriend(int clientfd, Msg m)
{
  int ret = 0;
  char friendname[FRIENDNAMESIZE] = {0};
  TcpServerRecv(clientfd, friendname, sizeof(friendname));
  char sql2[SQLSIZE];
  memset(sql2, 0, sizeof(sql2));
  //从在线链表中获取结果集
  char name[FRIENDNAMESIZE] = {0};
  sprintf(sql2,"select * from LoginClient where Clientfd = %d;", clientfd);
  GetTableVal(d, sql2, name, NULL, 0);

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

void* clientHandler(void *arg)
{
   int clientfd =* ((int*) arg);
   int tmp = clientfd;
   char Name[DEFAULT_SIZE] = {0};
   while(1)
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
                  if(ret == 0)
                  {
                    //填写的用户名或密码不正确
                    flag = 0;
                    TcpServerSend(clientfd, &flag, sizeof(flag));
                    memset(m.content, 0, sizeof(m.content));
                    strcpy(m.content, "你输入的账号或密码不正确, 请重新输入!");
                    TcpServerSend(clientfd, &m, sizeof(m));
                  } 
                  else
                  {
                    //用户名密码正确，查询是否已经登录
                    memset(sql, 0, sizeof(sql));
                    sprintf(sql, "select * from LoginClient where Username = '%s';", m.fromName);
                    ret = searchIsExist(d, sql);
                    if(ret == 0)
                    {
                      //用户未登录
                      memset(sql, 0, sizeof(sql));
                      sprintf(sql, "insert into LoginClient values('%s', %d);", m.fromName, clientfd);
                      if(SqliteExec(d,sql) == true)
                      {
                         //记录客户端姓名
                         strcpy(Name, m.fromName);

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
                break;
            case SIGNUP:
                
                char username[USERNAMESIZE] = {0};
                sprintf(username, "select * from SignupClient where Username = '%s';", m.fromName);
                ret = searchIsExist(d, username);
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
                break;
            case CHAT:
                //查询在线链表 私聊的对象在不在线 ,在线的情况下看是不是私聊的对象正好处在私聊界面
                //如果处在，则把消息直接发送过去，并保存记录，如果不处在, 则保存聊天记录直到私聊对象处在那个界面
                //再打印消息，如果离线，则保存聊天记录直到上线处在私聊界面
                char sql3[SQLSIZE] = {0};
                sprintf(sql3, "select * from LoginClient where Username = '%s';", m.toName);
                //查询在线链表
                pthread_mutex_lock(&loginmutex);
                int ret = searchIsExist(d, sql3);
                pthread_mutex_unlock(&loginmutex);
                if(ret == 0)    //不在线发不过去
                {
                  m.cmd = CHATFAIL;
                  memset(m.content, 0, sizeof(m.content));
                  strcpy(m.content, "发送失败,你要私聊的对象不在线!");
                  TcpServerSend(clientfd, &m, sizeof(m));
                }
                else //在线， 可以发过去
                {
                  //获取私聊对象的套接字
                  int tonamefd = 0;
                  memset(sql3, 0, sizeof(sql3));
                  sprintf(sql3, "select * from LoginClient where Username = '%s';", m.toName);
                  //这里要不要加锁?
                  GetTableVal(d, sql3, NULL, &tonamefd, 1);
                  printf("%d\n", tonamefd);
                  m.cmd = CHATSUCCESS;
                  TcpServerSend(tonamefd, &m, sizeof(m));
                }

                // l = FindByElement(&ClientList,m.toName,IsNameSame);
                // if(l.len == 0)
                // {
                //     printf("没找到要发送给的用户%s,转发失败！\n",m.toName);
                //     break;
                // }
                // struct Node *n = l.head->next;
                // while(n != NULL)
                // {
                //     CInfo *info  =(CInfo*)n->value;
                //     TcpServerSend(info->sock,&m,sizeof(m));
                //     printf("给%s发消息:%s\n",m.toName,m.content);
                //     n = n->next;
                // }
                break;
                // FreeDLlist(&l,NULL);
            case ALLCHAT:
                  n2 = ClientList.head->next;
                  while(n2 != NULL)   
                  {
                    CInfo *info = (CInfo*)n2->value;
                    if(strcmp(info->Name,m.fromName) != 0)
                                  TcpServerSend(info->sock,&m,sizeof(m));
                    printf("给所有人发消息:%s\n",m.content);
                    n2 = n2->next;
                  } 
                    break;
            case ADDFRIEND:
                char friendname[FRIENDNAMESIZE] = {0};
                TcpServerRecv(clientfd, friendname, sizeof(friendname));
                char sql2[SQLSIZE];
                memset(sql2, 0, sizeof(sql2));
                char name[FRIENDNAMESIZE] = {0};
                sprintf(sql2,"select * from LoginClient where Clientfd = %d;", clientfd);
                GetTableVal(d, sql2, name, NULL, 0);
                memset(sql2, 0, sizeof(sql2));
                sprintf(sql2,"select * from %s where name = '%s';", name, friendname);
                printf("%s\n", sql2);
                //先查一遍是不是已经是好友， 是则不用再添加
                ret = searchIsExist(d, sql2);
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
                  sprintf(sql2,"select * from LoginClient where Username = '%s';", friendname);
                  ret = searchIsExist(d, sql2);
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
                break;
            case DELETEFRIEND:
              
                deletefriend(clientfd, m);
                break;
            case BUILDGROUP:
                // char groupname[GROUPNAMESIZE + BUFFER_SZIE];
                // memset(groupname, 0, sizeof(groupname));
                // strcpy(groupname, m.content);
                // char sql5[SQLSIZE + BUFFER_SZIE] = {0};
                // sprintf(sql5, "select * from Queue where groupname = '%s';", groupname);
                
                // //加锁
                // pthread_mutex_lock(&groupmutex);
                //  ret = searchIsExist(d, sql5);
                // if(ret != 0)  //已经有其他客户端建立了群，不可建立群名一样的群
                // {
                //     printf("1111111222222222\n");
                //     pthread_mutex_unlock(&groupmutex);
                //     m.cmd = BUILDGROUPFAIL;
                //     memset(m.content, 0, sizeof(m.content));
                //     strcpy(m.content, "建群失败,已经有其他客户端建立了群,不可建立群名一样的群");
                //     TcpServerSend(clientfd, &m, sizeof(m));
                    
                // }
                // else
                // {
                //     //可以建群
                //     //m.cmd = BUILDGROUPSUCCESS;
                //     printf("1111111\n");
                //     printf("%s\n", Name);
                //     memset(sql5, 0, sizeof(sql5));
                //     //插入到唯一的群表
                //     sprintf(sql5, "insert into Queue values('%s');", m.content);
                //     if(SqliteExec(d, sql5) == true)
                //     {
                //         pthread_mutex_unlock(&groupmutex);
                //         memset(sql5, 0, sizeof(sql5));
                //         sprintf(sql5, "create table if not exists %s(name text);", m.content);
                //         if(SqliteExec(d, sql5) == true)
                //         {
                //             memset(sql5, 0, sizeof(sql5));
                //             //把群主插入到他所建立的群里面
                //             sprintf(sql5, "insert into %s values('%s');", m.content, Name);
                //             if(SqliteExec(d, sql5) == true)
                //             {
                //                 memset(sql5, 0, sizeof(sql5));
                //                 //在群主的那个客户端表里插入群
                //                 sprintf(sql5, "insert into %s values('%s', 2);", Name, m.content);
                //                 SqliteExec(d, sql5);
                //                 m.cmd = BUILDGROUPSUCCESS;
                //                 TcpServerSend(clientfd, &m, sizeof(m));
                //             }
                //         }
                        
                //     }
                   

                // }
                buildGroup(clientfd, m, Name);
                break;
            case ADDGROUP:
                // char groupname[GROUPNAMESIZE + BUFFER_SZIE];
                // memset(groupname, 0, sizeof(groupname));
                // strcpy(groupname, m.content);
                // char sql5[SQLSIZE + BUFFER_SZIE] = {0};
                // sprintf(sql5, "select * from Queue where groupname = '%s';", groupname);
                
                // //加锁
                // pthread_mutex_lock(&groupmutex);
                // //先查看有没有这个群
                // ret = searchIsExist(d, sql5);
                // if(ret == 0)  //没有这个群, 加群失败
                // {
                 
                //     pthread_mutex_unlock(&groupmutex);
                //     m.cmd = ADDGROUPFAIL;
                //     memset(m.content, 0, sizeof(m.content)); 
                //     strcpy(m.content, "加群失败,群表里不存在你要加的群");
                //     TcpServerSend(clientfd, &m, sizeof(m));
                // }
                // else  //存在这个群， 分两种情况, 用户在与不在这个群， 在则加群失败， 不在则加群成功
                // {
                //     pthread_mutex_unlock(&groupmutex);

                //     memset(sql5, 0, sizeof(sql5));
                //     sprintf(sql5, "select * from %s where name = '%s';", groupname, Name);
                //     ret = searchIsExist(d, sql5);
                //     if(ret != 0)  //用户存在这个群里， 加群失败
                //     {
                //       m.cmd = ADDGROUPFAIL;
                //       memset(m.content, 0, sizeof(m.content));
                //       strcpy(m.content, "你已经处在你要加入的群里, 不可以重复加入!");
                //       TcpServerSend(clientfd, &m, sizeof(m));
                //     }
                //     else     //用户不存在这个群里里， 可以加群
                //     {
                //       memset(sql5, 0, sizeof(sql5));
                //       //把客户导入到所要加的群表里
                //       sprintf(sql5, "insert into %s values('%s');", groupname, Name);
                //       if(SqliteExec(d, sql5) == true)  
                //       {
                //           //更新客户那张表， 增加一个群
                //           memset(sql5, 0, sizeof(sql5));
                //           sprintf(sql5, "insert into %s values('%s', 2);", Name, groupname);
                //           if(SqliteExec(d, sql5) == true)
                //           {
                //             m.cmd = ADDGROUPSUCCESS;
                //             memset(m.content, 0, sizeof(m.content));
                //             TcpServerSend(clientfd, &m, sizeof(m));
                //           }
                    
                //       }
                //     }
                // }
                addGroup(clientfd, m, Name);
                break;
            case QUITGROUP:
                // char groupname[GROUPNAMESIZE + BUFFER_SZIE];
                // memset(groupname, 0, sizeof(groupname));
                // strcpy(groupname, m.content);
                // char sql5[SQLSIZE + BUFFER_SZIE] = {0};
                // sprintf(sql5, "select * from Queue where groupname = '%s';", groupname);
                
                // //加锁
                // pthread_mutex_lock(&groupmutex);
                // //先查看有没有这个群 
                // ret = searchIsExist(d, sql5); 
                // if(ret == 0)  //没有这个群 , 退群失败
                // {
                //       pthread_mutex_unlock(&groupmutex);
                //       m.cmd = QUITGROUPFAIL;
                //       memset(m.content, 0, sizeof(m.content)); 
                //       strcpy(m.content, "退群失败,群表里不存在你要退的群");
                //       TcpServerSend(clientfd, &m, sizeof(m));
                // }
                // else     //存在这个群， 分两种情况, 用户在与不在这个群， 在则退群成功，不在则退群失败
                // {
                //     pthread_mutex_unlock(&groupmutex);

                //     memset(sql5, 0, sizeof(sql5));
                //     sprintf(sql5, "select * from %s where name = '%s';", groupname, Name);
                //     ret = searchIsExist(d, sql5);
                //     if(ret == 0)  //用户不存在存在这个群里， 退群失败
                //     {
                //       m.cmd = QUITGROUPFAIL;
                //       memset(m.content, 0, sizeof(m.content));
                //       strcpy(m.content, "你并不处在你要退出的群里, 不可以退群!");
                //       TcpServerSend(clientfd, &m, sizeof(m));
                //     }
                //     else
                //     {
                //       //用户存在这个群里里， 可以退群
                    
                //       memset(sql5, 0, sizeof(sql5));
                //       //把客户从所要退的群表里删除
                //       sprintf(sql5, "delete from %s where name = '%s';", groupname, Name);
                //       if(SqliteExec(d, sql5) == true)  
                //       {
                //           //更新客户那张表， 删除一个群
                //           memset(sql5, 0, sizeof(sql5));
                //           sprintf(sql5, "delete from %s where name = '%s';", Name, groupname);
                //           if(SqliteExec(d, sql5) == true)
                //           {
                //             m.cmd = QUITGROUPSUCCESS;
                //             memset(m.content, 0, sizeof(m.content));
                //             TcpServerSend(clientfd, &m, sizeof(m));
                //           }
                    
                //       }
                    
                //     }
                // }
                quitGroup(clientfd, m, Name);
                break;
               default:
                   break;
        }

    
  }
   close(clientfd);
}

void InitDB()
{

   d = InitSqlite("server.db");  //本地文件
   SqliteExec(d,"create table if not exists SignupClient(Username text,Password text);");
   SqliteExec(d,"create table if not exists LoginClient(Username text, Clientfd integer);");
   SqliteExec(d, "create table if not exists Log(from_name text, log_data text, to_name text);");
   SqliteExec(d, "create table if not exists Queue(groupname text);");
//    SqliteExec(d,"create table if not exists RootClient(Username text);");
//    SqliteExec(d,"create table if not exists VipClient(Username text);");

}

int main()
{
   TcpS *s = InitTcpServer(ip,port);
   if(s == NULL)
          return -1;
    //初始化数据库
    InitDB();      
   //初始化线程池
    ThreadP *p = InitThreadPool(20,10,10);
   //初始化锁
   pthread_mutex_init(&loginmutex, NULL);
   pthread_mutex_init(&groupmutex, NULL);

    InitDLlist(&ClientList);
    int clientfd = 0;
    printf("启动服务器成功\n");
    while(clientfd = TcpAccept(s))
    { 
        if(clientfd < 0)
                   break;
        
        AddpoolTask(p,clientHandler,&clientfd);
        
    }

  pthread_mutex_destroy(&loginmutex);
  pthread_mutex_destroy(&groupmutex);   
  ClearTcpServer(s);
  ClearThreadPool(p);
  return 0;
}
