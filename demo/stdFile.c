#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>


#include "common.h"

#define FILE_SET_SIZE 500


/* 初始化一个例程文件 */
int file_init(char *file_path)
{
    int ret = access(file_path, F_OK);
    if (ret == -1)
    {
        int ret = open(file_path, O_RDWR | O_CREAT, 0644);
        if (ret == -1)
        {
            perror("create error");
            return DEFAULT_ERROR;
        }
    }

    int fd = open(file_path, O_RDWR);
    if (fd == -1)
    {
        perror("open error");
        return DEFAULT_ERROR;
    }

    ret = ftruncate(fd, FILE_SET_SIZE);
    if (ret == -1)
    {
        perror("ftruncate error");
        return DEFAULT_ERROR;
    }

    char *mapped_memory = (char *)mmap(NULL, FILE_SET_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_memory == NULL)
    {
        perror("mmap error");
        return DEFAULT_ERROR;
    }
   
    for (int idx = 0; idx < FILE_SET_SIZE; idx++)
    {
        mapped_memory[idx] = '0' + idx % 10;
    }
    /* 回收 */
    munmap(mapped_memory, FILE_SET_SIZE);
    close(fd);

    return ON_SUCCESS;
}

/* 创建接收文件, 并共享存储映射 */
char *file_new_recv(char *file_path, int length)
{
    int ret = access(file_path, F_OK);
    if (ret == -1)
    {
        int fd = open(file_path, O_RDWR | O_CREAT, 0644);
        if (fd == -1)
        {
            printf("create error\n");
            return NULL;
        }
        close(fd);
    }

    int fd = open(file_path, O_RDWR | O_TRUNC); // 清空数据
    if (fd == -1)
    {
        printf("open error\n");
        return NULL;
    }

    ret = ftruncate(fd, length);
    if (ret == -1)
    {
        printf("ftruncate error\n");
        return NULL;
    }

    char *file_path_out = (char *)mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (file_path_out == NULL)
    {
        printf("file_path_out error\n");
        return NULL;
    }
    /* 回收 */
    close(fd);
    printf("file_path_out if null:%d\n", (int)(file_path_out == NULL));
    return file_path_out;
}

/* 获取文件大小 */
int file_get_size(char *file_path)
{
    struct stat bufStat;
    int ret = stat(file_path, &bufStat);
    if (ret == -1)
    {
        printf("stat error:%d\n", errno);
        return DEFAULT_ERROR;
    }
    
    return (int)bufStat.st_size;
}

/* 共享存储映射 */
char *file_mmap_share(char *file_path, int file_share_size)
{
    int ret = access(file_path, F_OK);
    if (ret == -1)
    {
        perror("access error");
        return NULL;
    }

    int fd = open(file_path, O_RDWR);
    if (fd == -1)
    {
        perror("open error");
        return NULL;
    }

    char *mapped_memory = (char *)mmap(NULL, file_share_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_memory == NULL)
    {
        perror("mmap error");
        return NULL;
    }

    close(fd);
    return mapped_memory;
}