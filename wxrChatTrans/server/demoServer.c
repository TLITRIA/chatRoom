#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <pthread.h>
#include <json-c/json.h>
#include <sys/mman.h>

#include "common.h"
#include "stdNet.h"
#include "stdJson.h"
#include "stdFile.h"

#define SERVER_PORT 8080
#define LOCAL_IPADDRESS "127.0.0.1"

#define MAX_LISTEN  128

#define EVENT_SIZE 64    // event数
#define READ_BUFFER_SIZE MAX_JSON_LENGTH    // 接收区缓存大小
#define MERGE_BUFFER_SIZE 1024   // 合并区缓存大小

char *default_message = "默认返回消息\n";



/* 静态函数 */
static int server_parse(char *message, server_recv arg, int pages, char **file_recv_mmap);


/* ======日志调试====== */
#define DEBUG_FLAG "../../../../log/server_debug.flag"
#define DEBUG_FILE "../../../../log/myChatRoom.log"

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
            fprintf(g_logfp, "[time(%s), file:(%s), func(%s), line(%d) ]\n"fmt"\n",   \
            del_last(asctime(ptm)), __FILE__, __FUNCTION__, __LINE__, ##args);   \
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
    //LOGPR("\033[1;32m服务端日志调试启动\033[0m\n");
    //LOGPR("=====================log init done.====================\n");
    //LOGPR("=================%s\n", del_last(asctime(ptm)));
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

/* 服务端接收线程 */
void *thread_server_read(void *arg)
{
    /* 线程分离 */
    pthread_detach(pthread_self());
    /* 初始化信息 */
    server_recv info = *(server_recv *)arg;
    
    int pages = 0; // 统计页数 

    char recvBuffer[READ_BUFFER_SIZE] = {0};

    int server_status = 0;          // 服务器状态
    
    int readBytes = 0;

    char *file_recv_mmap;

    /* 接受缓冲区合并为消息 */
    char *message = (char *)malloc(sizeof(char) * MERGE_BUFFER_SIZE);
    memset(message, 0, sizeof(char) * MERGE_BUFFER_SIZE);
    int merged = 0;
    int messCap = sizeof(char) * MERGE_BUFFER_SIZE;


    while (1)
    {
        memset(recvBuffer, 0, sizeof(recvBuffer));
        readBytes = read(info.fd, recvBuffer, sizeof(recvBuffer) - 1);
        if (readBytes < 0)
        {
            if (errno == EAGAIN)
            {
                printf("read end...\n\n");
                //printf("info%s\n", file_recv_mmap);
                server_parse(message, info, pages, &file_recv_mmap);
            }
            else    /* 将文件句柄从红黑树上删除 */
            {
                perror("read error");
                epoll_ctl(info.epfd, EPOLL_CTL_DEL, info.fd, NULL);
                close(info.fd); // todo
            }
            break;
        }
        else if (readBytes == 0)        /* 将该文件句柄从红黑树踢出 */
        {
            printf("客户端:%d下线......\n", info.fd); // todo下线处理
            epoll_ctl(info.epfd, EPOLL_CTL_DEL, info.fd, NULL);
            close(info.fd);
            break;
        }
        else
        {
            if(messCap < merged + readBytes + 1)
            {
                LOGPR("消息溢出，接收：%s\n", message);
                //exit(-1);
                memset(message, 0, messCap);// todo
            }
            strncat(message, recvBuffer, readBytes);
            merged += readBytes;

            //usleep(300);
        }
    }
}

/* 信号处理函数 */

/* 服务端解析函数 */
static int server_parse(char *message, server_recv arg, int pages, char **file_recv_mmap)
{
    LOGPR("\033[1;32m server recv \033[0m%s\n", message);

    /* 初始化信息 */
    usleep(500);
    /* 字符串转JSON */
    struct json_object * messageObj = json_tokener_parse((const char *)message);
    if (messageObj == NULL)
    {
        printf("JSON解释失败\n");
        return -1;
    }
    /* 提取command */
    struct json_object * commandObj = json_object_object_get(messageObj, "command");
    if (commandObj == NULL)
    {
        printf("未找到键:\"command\"\n");
        return -1;
    }
    /* 提取option */
    struct json_object * optionObj = json_object_object_get(messageObj, "option");
    if (optionObj == NULL)
    {
        printf("未找到键:\"option\"\n");
        return -1;
    }

    int cmdCode = json_object_get_int(commandObj);// cmdCode = 0时报错处理
    
    switch (cmdCode)   /* 只有服务端能收到的命令 */
    {
    case FILE_PRE_SEND: // 接收到了   传输前的文件信息
    {
        printf("接收到了头消息\n");

        /* 提取文件长度 */
        int file_length = json_parse_int(optionObj, "length");
        /* 提取文件名称 */
        char *file_name = json_parse_str(optionObj, "name");
        /* 记录页的大小 */
        int page_size = MAX_PAGES_SIZE;

        /* 最后一页读取的字符数 */
        int lastPageBytes = 0; // 这个变量传给了发送方
        /* 页数 */
        int file_pages = file_length / MAX_PAGES_SIZE;
        if (file_length % MAX_PAGES_SIZE != 0)
        {
            lastPageBytes = file_length - file_pages * MAX_PAGES_SIZE;
            file_pages++;
        }
        else
        {
            lastPageBytes = MAX_PAGES_SIZE;
        }

        pages = file_pages; // 同步一下要接收的页数
        
        //printf("pages:%d, max_page_size:%d, excess:%d\n", file_pages, MAX_PAGES_SIZE, lastPageBytes);
        /* 接收准备 */
        /* 创建接收文件file_name */
        char * new_path = (char *)malloc(sizeof(char) * (strlen(file_name) + strlen("recv_"))); 
        if (new_path == NULL)
        {
            return -1;
        }
        memset(new_path, 0, strlen(file_name) + strlen("recv_"));
        strncpy(new_path, "recv_", strlen("recv_"));
        strncat(new_path, file_name, strlen(file_name));

        /* 共享存储映射 */
        int file_size = (file_pages - 1) * MAX_PAGES_SIZE + lastPageBytes;

        int ret = access(new_path, F_OK);  // todo
        if (ret == -1)
        {
            ret = open(new_path, O_RDWR | O_CREAT, 0777);
            if (ret == -1)
            {
                printf("create error\n");
                return -1;
            }
            close(ret);
        }

        int new_fd = open(new_path, O_RDWR);
        if (new_fd == -1)
        {
            perror("open error");
            return -1;
        }

        ret = ftruncate(new_fd, file_size);
        if (ret == -1)
        {
            perror("ftruncate error");
            return -1;
        }

        char * file_mmap = (char *)mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, new_fd, 0);
        if (file_mmap == NULL)
        {
            perror("mmap error");
            return -1;
        }

        close(new_fd);

        *file_recv_mmap = file_mmap;

        int if_ready = 1; // 默认选择同意接收
        /* 接收完毕发送确认消息 */
        char *file_pre_check = json_pre_check(if_ready, file_pages, page_size, lastPageBytes);
        write(arg.fd, file_pre_check, strlen(file_pre_check));

        printf("接收完毕发送确认消息\n");
        break;
    }

    case FILE_MAIN_SEND:    // 接收消息 文件主体
    {
        int page = json_parse_int(optionObj, "page");
        char * context = json_parse_str(optionObj, "context");
        printf("接收页码:%d, 内容:%s\n", page, context);

        /* 写入文件 */
        strncpy(*file_recv_mmap + (page-1) * MAX_PAGES_SIZE, context, strlen(context));

        int recv_status = 1; // 表示消息完整收到并且已经写入文件
        char * string = json_send_check(recv_status, page);

        write(arg.fd, string, strlen(string));

        if (pages == page)
        {
            /* 接收完全部页数，等待发送方的“确认结束传输”的命令 */
            printf("消息接收完毕\n");
            if (*file_recv_mmap != NULL)
            {
                munmap(*file_recv_mmap, (page-1) * MAX_PAGES_SIZE + strlen(context));
                *file_recv_mmap == NULL;
            }
            
            pages = 0;
        }
        

        break;
    }

    default:
    {
        printf("无法识别命令\n");
        break;
    }
    
    }
    return -1;
}


static void serverSigHandler(int sig)
{// todo ctrl-C ctrl-Z等等信号处理
    printf("服务端端中断处理\tsig:%d\n", sig);
}


int main()
{
/* 启动日志调试 */
    log_init();
    LOGPR("\033[1;32m服务端日志调试启动\033[0m\n"); // 绿色

/* 信号注册 */
    //signal(SIGINT, serverSigHandler);

    
/* 创建socket套接字 */
    int serverfd = serverInit();

/* 设置端口复用 */
    serverSetsockopt(serverfd);

/* 绑定 */
    serverBindLocal(serverfd, SERVER_PORT);

/* 监听 */
    serverSetListen(serverfd, MAX_LISTEN);


/* 创建epoll红黑树实例 */
    int epfd = epollCreate();
    

/* 将sockfd添加到红黑池 */   
    int ret = epollAddEventET(epfd, serverfd);
    if (ret == -1)
    {
        perror("epoll ctl error");
        exit(-1); // 操作系统出大问题
    }

    struct epoll_event events[EVENT_SIZE];
    memset(events, 0, sizeof(events));
    int maxEventSize = sizeof(events) / sizeof(events[0]);
    
    /* data列表 */
    while (1)
    {
        /* 等待 */ /* nums代表等待到的事件数 */
        int nums = epoll_wait(epfd, events, maxEventSize, -1);
        if (nums == -1)
        {
            perror("epoll wait error");
            exit(-1);
        }

        /* 到这里有两种情况：1.超时(上面未设置超时) 2.有监听缓存区的数据出现变化 */
        for (int idx = 0; idx < nums; idx++)  /* 判断是哪个fd */
        {
            int fd = events[idx].data.fd;

            if (fd == serverfd) /* 如果有响应的是服务器本身fd */
            {
                /* 意味着有新的客户接入 */
                // todo NULL替换以接收客户端地址信息 ，扒一个二叉树代码过来，非必要。
                /* 客户的信息 */
                struct sockaddr_in clientAddress;
                memset(&clientAddress, 0, sizeof(clientAddress));
                socklen_t clientAddressLen = 0;
                int acceptfd = accept(serverfd, (struct sockaddr *)&clientAddress, &clientAddressLen); // 获取客户端套接字fd
                if (acceptfd == -1)
                {
                    perror("acceptfd error");
                    exit(-1);
                }

                //printf("accept client:%d, port:%d\n", clientAddress.sin_addr.s_addr, clientAddress.sin_port);

                /* 将通信句柄fd 设置成非阻塞模式 */
                setNonblocking(acceptfd);

                /* 将通信的句柄添加到树结点 */
                ret = epollAddEventET(epfd, acceptfd);
                if (ret == -1)
                {
                    perror("epoll ctl error");
                    continue;
                }
                
            }
            else  /* 如果有响应的是客户端 */
            {   
                /* 意味着客户端有数据传输 */
                /* 创建接收线程 */
                
                pthread_t tid;
                server_recv *recv = (server_recv *)malloc(sizeof(server_recv));
                memset(recv, 0, sizeof(server_recv));
                recv->fd = fd;
                recv->epfd = epfd;
                ret = pthread_create(&tid, NULL, thread_server_read, recv);
                if (ret == -1)
                {
                    perror("pthread_create error");
                    exit(-1);
                }
            }
        }
    }
    
    /* 关闭文件描述符 */
    close(serverfd);

    /* 关闭日志调试 */
    LOGPR("\033[1;32m服务端日志调试关闭\033[0m\n");
    log_close();
    return 0;
}