/**封装网络练接的接口
 * 
 * 
 * 
*/
#ifndef __STD_NET_H_
#define __STD_NET_H_


/* 服务端接收线程参数结构体 */
typedef struct server_recv
{
    int fd;
    int epfd;
} server_recv;


/* 设置句柄非阻塞 */
int setNonblocking(int fd);


/* 创建epoll */
int epollCreate(void);

/* 将socket添加到红黑树[返回值] */
int epollAddEventET(int epfd, int fd);

/* ==========服务端========== */
/* 服务端创建套接字 */
int serverInit(void);

/* 设置端口复用 */
int serverSetsockopt(int sockfd);

/* 服务端绑定本地IP */
int serverBindLocal(int sockfd, unsigned short port);

/* 服务端监听 */
int serverSetListen(int sockfd, int max_listen);

#if 0
/* 阻塞并等待客户端,[可选传出参数] */
int serverAcceptConn(int listenfd, struct sockaddr_in *addr);
#endif


/* ==========客户端========== */
/* 客户端设置端口复用 */
int clientInit(void);

/* 客户端连接服务器 */
int clientConnect(int sockfd, const char* server_ip, unsigned short port);





#endif // __STD_NET_H_