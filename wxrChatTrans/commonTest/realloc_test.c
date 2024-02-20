#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define DEFAULT_BUFFER_SIZE 10

int main()
{
#if 1
    char *strList[] = {"never", "gonna", "give", "you", "up"};
    int lenList = sizeof(strList) / sizeof(strList[0]);
    printf("lenList = %d\n", lenList);

    char *message = (char *)malloc(sizeof(char) * DEFAULT_BUFFER_SIZE);
    memset(message, 0, sizeof(char) * DEFAULT_BUFFER_SIZE);
    int merged = 0;
    int messCap = sizeof(char) * DEFAULT_BUFFER_SIZE;
    for (int idx = 0 ; idx < lenList; idx++)
    {
        printf("===================\n");
        printf("message:%s\tcap:%d\tmerged:%d\n", message, messCap, merged);
        printf("recv = %s\t", strList[idx]);
        printf("recvLen = %d\n", (int)strlen(strList[idx]));

        int recvLen = (int)strlen(strList[idx]);
        if(messCap < merged + recvLen + 1)
        {
            printf("溢出!\n");
            message = (char *)realloc(message, sizeof(char) * (merged + recvLen + 1));
            memset(message + merged, 0, sizeof(char) * (recvLen + 1));
            messCap = merged + recvLen + 1;
        }
        strncat(message, strList[idx], recvLen);

        merged += recvLen;
        printf("message:%s\tcap:%d\tmerged:%d\n", message, messCap, merged);
        
    }
#endif
#if 0
    char * str = (char *)malloc(11);
    strncpy(str, "hello,world", 11);
    memset(str+2, 0, 2);
    for (int idx = 0; idx < 11; idx++)
    {
        printf("str[%d]=%c\n", idx, str[idx]);
    }
#endif


    return 0;
}
