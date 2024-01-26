#ifndef __STDTCP_H_
#define __STDTCP_H_
#include <stdbool.h>
#include <stddef.h>

// 服务器结构体
struct StdTcpServer;
typedef struct StdTcpServer TcpS;

// 客户端结构体
struct StdTcpClient;
typedef struct StdTcpClient TcpC;

// 服务器端初始化  创建 绑定 监听
TcpS *InitTcpServer(const char *ip, const unsigned short port);

// 建立通信套接字 返回通信要用的套接字
int TcpAccept(TcpS *s);

// 服务器端发送消息
bool TcpServerSend(int clientfd, void *ptr, size_t size);

// 服务器接收消息
bool TcpServerRecv(int clientfd, void *ptr, size_t size);

// 释放服务器申请的堆空间
void ClearTcpServer(TcpS *s);

// 初始化客户端  初始化客户端的套接字，返回的是一个指向结构体的指针  创建套接字，连接服务器
TcpC *InitTcpClient(const char *Serverip, const unsigned short Serverport);

// 客户端发送消息
bool TcpClientSend(TcpC *c, void *ptr, size_t size);

// 客户端接收消息
bool TcpClientRecv(TcpC *c, void *ptr, size_t size);

// 释放客户端申请的堆空间
void ClearTcpClient(TcpC *c);
#endif