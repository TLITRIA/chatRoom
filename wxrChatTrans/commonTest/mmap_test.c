#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>


char * file_path = "mmap_test.text";
#define FILE_SET_SIZE 128

int main()
{
    int ret = access(file_path, O_RDWR);
    if (ret == -1)
    {
        perror("access error");
        return -1;
    }

    struct stat bufStat;
    stat(file_path, &bufStat);
    int file_size = (int)bufStat.st_size;
    printf("file exists, size:%d\n", file_size);

    int fd = open(file_path, O_RDWR);
    if (fd == -1)
    {
        perror("open error");
        return -1;
    }

    ret = ftruncate(fd, FILE_SET_SIZE);
    if (ret == -1)
    {
        perror("ftruncate error");
        return -1;
    }


    char *mapped_memory = (char *)mmap(NULL, FILE_SET_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_memory == NULL)
    {
        perror("mmap error");
        return -1;
    }

    for (int idx = 0; idx < FILE_SET_SIZE; idx++)
    {
        mapped_memory[idx] = '0' + idx % 10;
    }
    
    
    printf("%s\n", mapped_memory);


    /* 回收 */
    munmap(mapped_memory, FILE_SET_SIZE);
    close(fd);

    return 0;
}