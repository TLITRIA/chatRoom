#include <stdio.h>
#include <string.h>
#include <stdlib.h>



int main()
{
    char *str1 = "nevergonnagiveyouup";
    char str2[5] = {0};
    char *str3 = "\033[1;32m\033[0m\n";

    strncpy(str2, str1 + 1, 4);
    printf("str2:%s\n", str2);

    int len = (int)strlen(str1) + strlen("\033[1;32m\033[0m\n");
    char *newPtr = (char *)malloc(sizeof(char) * len);
    if (newPtr == NULL)
    {
        perror("malloc error");
        return 0;
    }
    memset(newPtr, 0, sizeof(char) * len);

    sprintf(newPtr, "\033[1;32m%s\033[0m\n", str1);
    printf("newPtr:%snewPtr[len-2]:%c\n", newPtr, newPtr[len - 2]); // 输出m

    return 0;
}
