#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>


#include "common.h"
#include "stdNet.h"

#define MAX_LISTEN  128
#define LOCAL_IPADDRESS "127.0.0.1"
#define BUFFER_SIZE 128



/* 设置句柄非阻塞 */
int setNonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

/* 将socket添加到红黑树[返回值] */
int epollAddEventET(int epfd, int fd)
{
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;     
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
    return ret;
}



/* epoll */
/* 创建epoll */
int epollCreate(void)
{
    int epfd = epoll_create(1);
    if (epfd == -1)
    {
        perror("epoll error");
        exit(-1); // 操作系统出大问题
    }
    return epfd;
}



/* 服务端 */
/* 服务端创建套接字 */
int serverInit(void)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket error");
        exit(-1);
    }
    printf("套接字创建成功, fd=%d\n", sockfd);
    return sockfd;
}

/* 设置端口复用 */
int serverSetsockopt(int sockfd)
{
    int enableOpt = 1;
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&enableOpt, sizeof(enableOpt));
    if (ret == -1)
    {
        perror("setsockopt error");
        exit(-1);
    }
    printf("套接字设置端口复用成功\n");
    return ret;
}

/* 服务端绑定socket */
int serverBindLocal(int sockfd, unsigned short port)
{
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));/* 清除脏数据 */
    
    serverAddress.sin_family = AF_INET;                  /* 地址族 */
    serverAddress.sin_port = htons(port);               /* 端口需要转成大端 */
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);   /* ip地址需要转成大端 */

    int ret = bind(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (ret == -1)
    {
        perror("bind error");
        exit(-1);
    }
    printf("套接字绑定成功, ip: %s, port: %d\n",
        inet_ntoa(serverAddress.sin_addr), port);
    return ret;
}
/* 服务端监听 */
int serverSetListen(int sockfd, int max_listen)
{
    int ret = listen(sockfd, max_listen);
    if (ret == -1)
    {
        perror("listen error");
        exit(-1);
    }
    printf("服务端监听成功...\n");
    return ret;
}

/* 阻塞并等待客户端,[可选传出参数] */
// ??这跟accept()好像没封装多少
#if 0
int serverAcceptConn(int listenfd, struct sockaddr_in *addr)
{
    int acceptfd = -1;
    if (addr == NULL) { acceptfd = accept(listenfd, NULL, NULL); }
    else { int addrlen = sizeof(struct sockaddr_in);
        acceptfd = accept(listenfd, (struct sockaddr*)addr, &addrlen); }
    if (acceptfd == -1) {perror("accept error"); return -1;}
    return acceptfd;
}
#endif

/* ===================客户端================== */
/* 客户端设置端口复用 */
int clientInit(void)
{
    return serverInit();
}


/* 客户端连接服务器 */
int clientConnect(int sockfd, const char* server_ip, unsigned short port)
{
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, (void *)&(serverAddress.sin_addr.s_addr));
    
    int ret = connect(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (ret == -1)
    {
        perror("connect error");
        return ret;
    }

    printf("连接服务器成功...\n");
    return ret;
}


