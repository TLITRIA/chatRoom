#include <stdio.h>
#include <termios.h>

/* 关闭scanf回显 */
int closeScanfEcho(void)
{
    struct termios term_setting;
    tcgetattr(0, &term_setting);
    term_setting.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &term_setting);
    return 0;
}

/* 开启scanf回显 */
int openScanfEcho(void)
{
    struct termios term_setting;
    tcgetattr(0, &term_setting);
    term_setting.c_lflag |= ECHO;
    tcsetattr(0, TCSANOW, &term_setting);
    return 0;
}

int main()
{
    char s[128] = {0};
#if 0
    struct termios new_setting, init_setting;
    tcgetattr(0,&init_setting);
    new_setting = init_setting;
    new_setting.c_lflag&=~ECHO;
    tcsetattr(0,TCSANOW,&new_setting);

    printf(">>>");
    scanf("%s", s);
    printf("\n输入的字符是:%s\n", s);
    
    tcsetattr(0,TCSANOW,&init_setting);

    printf(">>>");
    scanf("%s", s);
    printf("输入的字符是:%s\n", s);
#endif

    closeScanfEcho();

    printf(">>>");
    scanf("%s", s);
    printf("输入的字符是:%s\n", s);

    openScanfEcho();

    printf(">>>");
    scanf("%s", s);
    printf("输入的字符是:%s\n", s);

    return 0;
}