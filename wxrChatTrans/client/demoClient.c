#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <error.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include <json-c/json.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "stdNet.h"
#include "stdJson.h"
#include "stdFile.h"
#include "common.h"

#define SERVER_PORT 8080
#define SERVER_IP "192.168.42.128"

#define SCAN_BUFER_SIZE 128     // 终端输入缓存大小
#define READ_BUFFER_SIZE MAX_JSON_LENGTH   // 接收区缓存大小
#define SEND_BUFFER_SIZE MAX_JSON_LENGTH   // 接收区缓存大小

#define MAX_PAGES 16

enum CLIENT_STATUS
{
    DEFAULT = 0, // 客户端初始页面，登陆或注册
    LOGIN = 1,
    REGISTER,


};

//todo 共享资源

int g_pages = 0;        // 要发送的总共的页数
int g_page_size = 0;    // 每一页的正常大小（区别于最后一页）



int g_client_status = DEFAULT; // 客户端状态，默认初始的状态
char *file_test_path = "";   // 要传输的文件名
int file_test_size = 0;      // 文件大小

int flag_test_send = 0;


/* =====前置声明===== */

/* 信号处理 */
static void clientSigHandler(int sig);
/* 客户端消息解析 */
static int client_parse(char *recvBuffer, char *sendBuffer, char *pageBuffer, int fd, char **file_share_mmap, int *excessBytes);
/* 客户端发送线程 */
static void *thread_send_pages(void *arg);

/* =====前置声明结束===== */





/* ======日志调试====== */
#define DEBUG_FLAG "../../../../log/client_debug.flag"
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




/* 客户端接收线程 */
void *thread_client_read(void *arg)
{
    /* 线程分离 */
    pthread_detach(pthread_self());
    /* 初始化信息 */

    int fd = *(int *)arg;

    char recvBuffer[READ_BUFFER_SIZE];  // todo 每个接收线程都有独立的接收缓存区，结束符
    char sendBuffer[SEND_BUFFER_SIZE];  // 每个接收线程都有独立的发送缓存区
    char *pageBuffer;                   // 独立的分页缓冲区
    int readBytes = 0;                  // 
    int excessBytes = 0;                // 最后一页读取的字符数

    char *file_share_mmap;              // 共享存储映射



    pageBuffer = (char *)malloc(sizeof(char) * g_page_size);
    memset(pageBuffer, 0, sizeof(char) * g_page_size);

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
            client_parse(recvBuffer, 
                        sendBuffer, 
                        pageBuffer, 
                        fd, 
                        &file_share_mmap, 
                        &excessBytes);
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
static int client_parse(char *recvBuffer, 
                        char *sendBuffer, 
                        char *pageBuffer, 
                        int fd, 
                        char **file_share_mmap, 
                        int *excessBytes)
{
    
    LOGPR("\033[1;32m client recv \033[0m%s\n", recvBuffer);
    
    usleep(500);
    /* 字符转JSON */
    struct json_object * messageObj = json_tokener_parse((const char *)recvBuffer);
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
    case FILE_PRE_CHECK: // 接收到了 对文件信息的回应
    {   
        int if_ready = json_parse_int(optionObj, "if_ready");
        if (if_ready == -1)
        {
            printf("接收方未准备好, 发送终止\n");
            return -1;
        }
        /* 读取分页页数 */
        g_pages = json_parse_int(optionObj, "pages"); // todo 
        g_page_size = json_parse_int(optionObj, "page_size");
        if (g_page_size == 0)
        {
            LOGPR("g_page_size == 0\n");
        }
        *excessBytes = json_parse_int(optionObj, "lastPageBytes");
        LOGPR("对方确认接收, 要分%d页, 每页读%d字节，最后一页读取%d字节\n", g_pages, g_page_size, *excessBytes);

        /* 共享存储映射 */
        char * file_mmap = file_mmap_share(file_test_path, file_test_size);
        if (file_mmap == NULL)
        {
            LOGPR("共享存储映射失败，没有错误处理！");
        }

        *file_share_mmap = file_mmap;
        int page = 1; // 当前发送的页数是第一页
        

        /* 接收页缓存 */ // todo函数参数修改：传入指针和消息的长度
        pageBuffer = (char *)malloc(sizeof(char) * g_page_size);
        memset(pageBuffer, 0, sizeof(char) * g_page_size);
        strncpy(pageBuffer, *file_share_mmap, sizeof(char) * g_page_size);
        

        memset(sendBuffer, 0, sizeof(char) * SEND_BUFFER_SIZE);
        
        json_send_page(1, pageBuffer, &sendBuffer); // 发送第一页的数据
        LOGPR("准备传输第一页：%s\n", sendBuffer);
        
        /* 发送第一页 */
        write(fd, sendBuffer, strlen(sendBuffer));
        break;
    }

    case FILE_MAIN_CHECK: // 接收到了 对文件主体的回应
    {
        /* 判断还有没有剩余数据，有就继续发剩下的数据，没有就发结束 */
        /* 对面没收到就再发一次或直接报错退出 */
        /* 最后一页接收完毕也就代表整个流程结束了，释放一些数据，一些变量返回初始状态 */
        
        int recv_status = json_parse_int(optionObj, "recv_status");
        int page = json_parse_int(optionObj, "page"); // 当前发送的页数
        printf("第%d页的消息发送结果是%d\n", page, recv_status);

        if (recv_status == -1)
        {
            /* 发送失败的处理，比如累计连续发送失败就终止发送 */
        }
        else
        {
            /* 发送下一页信息或者没有消息发完 */
            if (page == g_pages)
            {
                /* 文件发完了，发送一条确认结束的消息 */
                // todo
                g_pages = 0;
            }
            else
            {
                page++;
                /* 接收页缓存 */ 
                memset(pageBuffer, 0, sizeof(char) * g_page_size);
                strncpy(pageBuffer, *file_share_mmap + (page - 2) * g_page_size, (page == g_pages && *excessBytes != 0) ? *excessBytes : g_page_size);

                memset(sendBuffer, 0, sizeof(char) * SEND_BUFFER_SIZE);
                json_send_page(page, pageBuffer, &sendBuffer); // 发送后页的数据
                LOGPR("准备传输第%d页：%s\n", page, sendBuffer);
                
                /* 发送第一页 */
                write(fd, sendBuffer, strlen(sendBuffer));
                sleep(1); // 发送延迟
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

/* 文件调试 */
    log_init();
    LOGPR("\033[1;32m客户端日志调试启动\033[0m\n"); // 绿色

/* 信号注册 */
    //signal(SIGINT, clientSigHandler);


    int sockfd = 0;// 客户端套接字



    do
    {
        /* 创建套接字 */
        sockfd = clientInit();

        /* 创建客户端输入缓冲区 */
        char writeBuffer[SCAN_BUFER_SIZE];
        memset(writeBuffer, 0, sizeof(writeBuffer) * sizeof(char));

        while (1) // 客户端输入的循环
        {
            switch (g_client_status)
            {
            case DEFAULT:
            {
                LOGPR("\033[1;32m客户端初始状态\033[0m\n"); 

                /* 连接服务器 */
                int ret = clientConnect(sockfd, SERVER_IP, SERVER_PORT);
                if (ret == -1)
                {
                    LOGPR("\033[1;32m客户端连接被拒绝\033[0m\n");
                    break;
                }

                /* 创建接收数据线程 */
                pthread_t tid;
                ret = pthread_create(&tid, NULL, thread_client_read, (void *)&sockfd);
                if (ret == -1)
                {
                    perror("pthread_create error");
                    break;
                }


                break;
            }

            default:
                LOGPR("\033[1;32m无法识别客户端状态\033[0m\n"); 
                break;
            }



    #if 1
            /* 文件传输测试 */
            while (flag_test_send != 0)
            {
                sleep(1);
                continue;
            }
            usleep(500); 
            strncpy(writeBuffer, "text.text", strlen("text.text"));
            flag_test_send = 1; // 标志位，代表已经发过一次了
            

            /* 文件路径读取 */
            file_test_path = (char *)malloc(sizeof(char) * strlen(writeBuffer));
            memset(file_test_path, 0, sizeof(char) * strlen(writeBuffer) + 1);
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
    #else

    #endif
        }
        
        
        
        sleep(5);  // todo 可不可以输入加锁，在提示信息输出完毕前禁止输入
        LOGPR("\033[1;32m客户端一轮输入结束\033[0m\n");
    } while (0);

// 错误处理
    /* 关闭套接字 */
    if (sockfd != 0)
    {
        close(sockfd);
    }
    
    



    /* 关闭日志调试 */
    LOGPR("\033[1;32m客户端日志调试关闭\033[0m\n");
    log_close();
    return 0;
}