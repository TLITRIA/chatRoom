#include "common.h"

#include <termios.h>

/* ====宏定义==== */
#define ELEMENTTYPE void*
#define FALSE 0
#define TRUE 1





/* 全局变量 */


/* ====静态函数前置声明==== */


/* ====静态函数声明结束==== */

/* 关闭scanf回显 */
int closeScanfEcho(void)
{
    struct termios term_setting;
    tcgetattr(0, &term_setting);
    term_setting.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &term_setting);
    return ON_SUCCESS;
}

/* 开启scanf回显 */
int openScanfEcho(void)
{
    struct termios term_setting;
    tcgetattr(0, &term_setting);
    term_setting.c_lflag |= ECHO;
    tcsetattr(0, TCSANOW, &term_setting);
    return ON_SUCCESS;
}
