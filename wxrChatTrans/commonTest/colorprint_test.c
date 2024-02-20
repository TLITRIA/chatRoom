#include <stdio.h>


int main()
{
    printf("\033[1;38mhello\n\033[0m"); // 灰
    printf("\033[1;37mhello\n\033[0m"); // 白
    printf("\033[1;36mhello\n\033[0m"); // 淡蓝
    printf("\033[1;35mhello\n\033[0m"); // 紫
    printf("\033[1;34mhello\n\033[0m"); // 蓝
    printf("\033[1;33mhello\n\033[0m"); // 黄
    printf("\033[1;32mhello\n\033[0m"); // 绿
    printf("\033[1;31mhello\n\033[0m"); // 红
    printf("\033[1;30mhello\n\033[0m"); // 灰
    printf("\033[1;29mhello\n\033[0m"); // 白
    
    //printf("\033[1;2mhello\n\033[0m"); // 
    //printf("\033[1;10mhello\n\033[0m"); // 
    //printf("\033[1;mhello\n\033[0m"); // 
    return 0;
}