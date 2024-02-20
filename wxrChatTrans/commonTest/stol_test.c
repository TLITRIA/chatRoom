#include <stdio.h>
#include <stdlib.h>



int main()
{
    const char *ptr = "12abc12";
    char *endptr = NULL;
    int value = strtol(ptr, &endptr, 0);
    printf("value:%d,\tvalue(八进制):%o\tendptr:%s\n", value, value, endptr);

}