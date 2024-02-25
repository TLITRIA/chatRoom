#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
#include "threadPool.h"
#include "Tcp.h"

#define MAX_LISTEN 128
#define SERVER_PORT 8080
#define BUFFER_SIZE 128
#define LOCAL_IPADDRESS "192.168.204.128"

#define SERVER_IP "127.0.0.1"

#define MINTHREADS 5
#define MAXTHREADS 10
#define MAXQUEUE 50


// 服务器端初始化  创建 绑定 监听
int TcpServerInit()
{
    /* 创建socket套接字*/
    int S_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (S_sockfd == -1)
    {
        perror("socket error");
        exit(-1);
    }
    /* 设置端口复用*/
    int enableOpt = 1;
    int ret = setsockopt(S_sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&enableOpt, sizeof(enableOpt));
    if (ret == -1)
    {
        perror("setsockopt error");
        exit(-1);
    }

    /* 绑定*/
    struct sockaddr_in localAddress;
    /* 清除脏数据*/
    memset(&localAddress, 0, sizeof(localAddress));

    /* 地址族*/
    localAddress.sin_family = AF_INET;
    /* 端口需要转成大端*/
    localAddress.sin_port = htons(SERVER_PORT);
    #if 1
    /* INADDR_ANY = 0x00000000 */
    localAddress.sin_addr.s_addr = INADDR_ANY;
    #else/* 不是全0就需要转换*/
    /* ip地址需要转换为大端*/
    inet_pton(AF_INET, LOCAL_IPADDRESS, &(localAddress.sin_addr.s_addr));
    #endif
    int localAddresslen = sizeof(localAddress);
    ret = bind(S_sockfd, (struct sockaddr *)&localAddress, localAddresslen);
    if (ret == -1)
    {
        perror("bind error");
        exit(-1);
    }

    /* 监听*/
    ret = listen(S_sockfd, MAX_LISTEN);
    if (ret == -1)
    {
        perror("listen error");
        exit(-1);
    }
    printf("hh\n");
    return S_sockfd;
}

// 建立通信套接字 //返回通信要用的套接字
int TcpAccept(int sfd)
{
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    int acceptfd = accept(sfd, (struct sockaddr *)&clientaddr, &len);
    if (acceptfd >= 0)
    {
        printf("连接到客户端ip:%s, 端口：%d,sockfd:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), acceptfd);
    }
    else
    {
        perror("accept error");
    }
    return acceptfd;
}

// 服务器端发送消息
bool TcpServerWrite(int clientfd, void *ptr, int size)
{
    if (write(clientfd, ptr, size) <= 0)
    {
        perror("write error");
        return false;
    }
    return true;
}

// 服务器端接收消息
bool TcpServerRead(int clientfd, void *ptr, int size)
{
    int ret = read(clientfd, ptr, size);
    if (ret < 0)
    {
        perror("read error");
        return false;
    }
    else if (ret == 0)
    {
        printf("客户端下线\n");
        return false;
    }
    return true;
}


// 初始化客户端的套接字，返回的是一个指向结构体的指针  创建套接字，连接服务器
int TcpClientInit()
{
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd == -1)
    {
        perror("socket error");
        exit(-1);
    }
    
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);

    int ret = inet_pton(AF_INET, SERVER_IP, (void *)&(serverAddress.sin_addr.s_addr));
    if (ret == -1)
    {
        perror("inet_pton error");
        exit(-1);
    }
    
    ret = connect(cfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (ret == -1)
    {
        perror("connect error");
        exit(-1);
    }
    return cfd;
}

// 客户端发送消息
bool TcpClientWrite(int c, void *ptr, int size)
{
    if (write(c, ptr, size) <= 0)
    {
        perror("send error");
        return false;
    }
    return true;
}

// 客户端接收消息
bool TcpClientRead(int c, void *ptr, int size)
{
    int ret = read(c, ptr, size);
    if (ret < 0)
    {
        perror("recv error");
        return false;
    }
    if (ret == 0)
    {
        printf("服务端下线\n");
        return false;
    }
    return true;
}
