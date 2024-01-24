#include "StdTcp.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_LISTEN 10
//服务器端套接字
struct StdTcpServer
{
    int sock;
};

//客户端套接字
struct StdTcpClient
{
    int sock;
};

//服务器端初始化  创建 绑定 监听
TcpS *InitTcpServer(const char *ip, const unsigned short port)
{
    TcpS *s = (TcpS *)malloc(sizeof(TcpS));
    if (s == NULL)
    {
        perror("malloc");
        return NULL;
    }
    s->sock = socket(AF_INET, SOCK_STREAM, 0);
    if (s->sock < 0)
    {
        perror("socket");
        free(s);
        return NULL;
    }
   //1代表启用端口复用，0代表禁用端口复用
    int opt = 1;
    setsockopt(s->sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(int));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    if (bind(s->sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(s->sock);
        free(s);
        return NULL;
    }
    if (listen(s->sock, MAX_LISTEN) < 0)
    {
        perror("listen");
        close(s->sock);
        free(s);
        return NULL;
    }
    return s;
}

//建立通信套接字 //返回通信要用的套接字
int TcpAccept(TcpS *s)
{
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    int acceptfd = accept(s->sock, (struct sockaddr *)&clientaddr, &len);
    if (acceptfd >= 0)
    {
        printf("连接到客户端ip:%s, 端口：%d,sockfd:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), acceptfd);
    }
    else
    {
        perror("accept");
    }
    return acceptfd;
}

//服务器端发送消息
bool TcpServerSend(int clientfd, void *ptr, size_t size)
{
    if (send(clientfd, ptr, size, 0) <= 0)
    {
        perror("send");
        return false;
    }
    return true;
}

//服务器端接收消息
bool TcpServerRecv(int clientfd, void *ptr, size_t size)
{
    int ret = recv(clientfd, ptr, size, 0);
    if (ret < 0)
    {
        perror("recv");
        return false;
    }
    else if (ret == 0)
    {
        printf("客户端下线\n");
        return false;
    }
    return true;
}

//释放服务器申请的堆空间
void ClearTcpServer(TcpS *s)
{
    close(s->sock);
    free(s);
}

//初始化客户端的套接字，返回的是一个指向结构体的指针  创建套接字，连接服务器
TcpC *InitTcpClient(const char *Serverip, const unsigned short Serverport)
{
    TcpC *c = (TcpC *)malloc(sizeof(TcpC));
    if(c  == NULL)
    {
        perror("malloc");
        return NULL;
    }
     c->sock = socket(AF_INET,SOCK_STREAM,0);
    if(c->sock < 0)
    {
        perror("socket");
        free(c);
        return NULL;

    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(Serverport);
    addr.sin_addr.s_addr = inet_addr(Serverip);
    if(connect(c->sock,(struct sockaddr*)&addr,sizeof(addr)) < 0)
    {
        perror("connect");
        close(c->sock);
        free(c);
        return NULL;
    }
    return c;
}

//客户端发送消息
bool TcpClientSend(TcpC *c,void *ptr,size_t size)
{
    if(send(c->sock,ptr,size,0) <= 0)
    {
        perror("send");
        return false;
    }
    return true;

}

//客户端接收消息
bool TcpClientRecv(TcpC *c,void *ptr,size_t size)
{
    int ret = recv(c->sock, ptr, size, 0);
    if (ret < 0)
    {
        perror("recv");
        return false;
    }
    if (ret == 0)
    {
        printf("服务端下线\n");
        return false;
    }
    return true;
}

//释放客户端申请的堆空间
void ClearTcpClient(TcpC *c)
{
    close(c->sock);
    free(c);
}