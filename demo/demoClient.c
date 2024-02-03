#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <json-c/json.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <error.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>

#include "stdNet.h"
#include "stdJson.h"
#include "stdFile.h"
#include "common.h"

#define SERVER_PORT 8080
#define SERVER_IP "192.168.42.128"

#define SCAN_BUFER_SIZE 128     // 终端输入缓存大小
#define READ_BUFFER_SIZE MAX_NORMAL_MESSAGE    // 接收区缓存大小
#define MAX_PAGES 16

int tmp_if_send_test_message = 0;

/* pages结构体 */
typedef struct PAGES
{
    char *message;
    int sockfd;
} PAGES;

/* 文件传输, 在合适位置置0或者其他操作 */
PAGES args[MAX_PAGES];
int pages = 0; // 要发送的总共的页数



//todo 共享资源
int client_status = 0; // 客户端状态
char *file_test_path = "";   // 要传输的文件名
int file_test_size = 0;      // 文件大小

/* 前置声明 */

/* 信号处理 */
static void clientSigHandler(int sig);
/* 客户端消息解析 */
static int client_parse(char *message, int fd, int client_status, char *file_share_mmap);
/* 客户端发送线程 */
static void *thread_send_pages(void *arg);
/* 前置声明结束 */


/* 客户端接收线程 */
void *thread_client_read(void *arg)
{
    /* 线程分离 */
    pthread_detach(pthread_self());
    /* 初始化信息 */
    int fd = *(int *)arg;

    char recvBuffer[READ_BUFFER_SIZE];
    
    int readBytes = 0;

    char *file_share_mmap;


    while (1)
    {
        memset(recvBuffer, 0, sizeof(recvBuffer));
        readBytes = read(fd, recvBuffer, sizeof(recvBuffer) - 1);
        
        
        if (readBytes < 0)
        {
            perror("read error");
            exit(-1);
        }
        else if (readBytes == 0)
        {
            printf("readBytes:0\n");
            printf("服务端下线\n"); 
            exit(-1);// todo处理
        }
        else
        {
            client_parse(recvBuffer, fd, client_status, file_share_mmap);
        }
    }
    exit(-1);
}

/* 信号处理 */
static void clientSigHandler(int sig)
{// todo ctrl-C ctrl-Z等等信号处理
    printf("客户端中断处理\tsig:%d\n", sig);
}

/* 消息解析 */
static int client_parse(char *message, int fd, int client_status, char *file_share_mmap)
{
    printf("recv: %s\n", message);
    usleep(500);
    /* 字符转JSON */
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

    int command = json_object_get_int(commandObj);// cmdCode = 0时报错处理
    printf("command = %d\n", command);
    
    switch (command)    /* 只有客户端能收到的命令 */
    {

    
    case FILE_PRE: // 
    {
        printf("识别FILE_PRE操作\n");
        break;
    }

    case FILE_PRE_SENDBACK: // 判断对方是否同意接收
    {
        printf("判断对方是否同意接收\n");
        int if_ready = json_parse_int(optionObj, "if_ready");
        if (if_ready == -1)
        {
            printf("接收方未准备好, 发送终止\n");
            return -1;
        }
        
        pages = json_parse_int(optionObj, "pages");
        int excessBytes = json_parse_int(optionObj, "lastPageBytes");
        printf("对方确认接收, 要分%d页, 最后一页读取%d字节\n", pages, excessBytes);

        
        
        /* 共享存储映射 */
        file_share_mmap = file_mmap_share(file_test_path, file_test_size);
        

        //file_share_mmap[0]='A'; 确认了共享存储映射
        //printf("shared:%s\n",file_share_mmap);
        
        char *tmp_str = (char *)malloc(MAX_PAGES_SIZE + 1);
        if (tmp_str == NULL)
        {
            printf("malloc error\n");
            break;
        }
        memset(tmp_str, 0, MAX_PAGES_SIZE + 1);
        
        char *tmpBuffer = (char*)malloc(sizeof(char) * MAX_PAGES_SIZE);
        if (tmpBuffer == NULL)
        {
            printf("malloc error\n");
            break;
        }

        // todo 发送前直接开辟消耗资源,结构体不是必须的
        memset(args, 0, sizeof(args));
        for (int idx = 0; idx < pages; idx++)
        {
            int length = (idx != pages - 1) ? MAX_PAGES_SIZE : excessBytes;

            memset(tmpBuffer, 0, sizeof(char) * MAX_PAGES_SIZE);

            strncpy(tmpBuffer, file_share_mmap + idx * MAX_PAGES_SIZE, length);

            args[idx].sockfd = fd;
            args[idx].message = json_send_file(idx + 1, tmpBuffer);
            //printf("send:%d, message:%s\n\n", (int)strlen(args[idx].message), args[idx].message);
        }


        /* 发送第一页 */
        write(args[0].sockfd, args[0].message, strlen(args[0].message));

        break;
    }

    case FILE_RECV_STATUS:
    {
        /* 判断还有没有剩余数据，有就继续发剩下的数据，没有就发结束 */
        /* 对面没收到就再发一次或直接报错退出 */
        /* 最后一页接收完毕也就代表整个流程结束了，释放一些数据，一些变量返回初始状态 */
        
        int recv_status = json_parse_int(optionObj, "recv_status");
        int page = json_parse_int(optionObj, "page");
        printf("第%d页的消息发送结果是%d\n", page, recv_status);

        if (recv_status == -1)
        {
            /* 发送失败的处理，比如累计连续发送失败就终止发送 */
        }
        else
        {
            /* 发送下一页信息或者没有消息发完 */
            if (page == pages)
            {
                /* 文件发完了，发送一条确认结束的消息 */
                // todo
                pages = 0;
            }
            else
            {
                write(args[page].sockfd, args[page].message, strlen(args[page].message));
            }
            
        }

        


        break;
    }

    default:
    {
        printf("无法识别命令\n");
        break;
    }

    }
    return ON_SUCCESS;
}








int main()
{
/* 信号注册 */
    signal(SIGINT, clientSigHandler);

/* 创建套接字 */
    int sockfd = clientInit();

/* 连接服务器 */
    clientConnect(sockfd, SERVER_IP, SERVER_PORT);


/* 收发数据 */

    /* 写缓冲区 */
    char writeBuffer[SCAN_BUFER_SIZE];
    memset(writeBuffer, 0, sizeof(writeBuffer) * sizeof(char));

    /* 创建线程接收数据 */
    pthread_t tid;
    int ret = pthread_create(&tid, NULL, thread_client_read, (void *)&sockfd);
    if (ret == -1)
    {
        perror("pthread_create error");
        exit(-1);
    }
    
    while (1)
    {
        while (tmp_if_send_test_message != 0)
        {
            usleep(500);
            continue;
        }
        usleep(500);
        

        /* 只进行文件传输 */
        #if 1
        printf("\n输入文件名:");
        scanf("%127s", writeBuffer); 
        #else
        // 
        strncpy(writeBuffer, "randomfile.bin", strlen("randomfile.bin"));
        tmp_if_send_test_message = 1;
        #endif
        /* 文件路径读取 */
        file_test_path = (char *)malloc(sizeof(char) * strlen(writeBuffer));
        memset(file_test_path, 0, sizeof(char) * strlen(writeBuffer));
        strncpy(file_test_path, writeBuffer, strlen(writeBuffer));
        printf("file_test_path:%s\n", file_test_path);
        
        /* 初始化测试文件 */
        //file_init(file_test_path);

        /* 确认文件存在 */
        int ret = access(file_test_path, F_OK);
        if (ret != 0)
        {
            perror("access error");
            continue;
        }

        /* 文件信息 */
        file_test_size = file_get_size(file_test_path);
        printf("file exists, size:%d\n", file_test_size);

        printf("\n");
        char * messagePre = json_pre_file(file_test_size, writeBuffer);
        write(sockfd, messagePre, strlen(messagePre));

        


        //char *message = json_sendback(writeBuffer);

        //char *headmessage = json_length(PRE_MESSAGE, (int)strlen(message));

        //// 发送头消息
        //write(sockfd, headmessage, strlen(headmessage));
        //usleep(200);
        //// 发送消息主体
        //write(sockfd, message, strlen(message));
    }
    
    
    /* 休息5S */
    sleep(5);
    close(sockfd);


    return 0;
}