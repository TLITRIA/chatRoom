#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>


void sigExitHandler(int sig)
{
    printf("sig:%d\n", sig);
}


int main()
{
    /* 注册信号 */

    // 1.默认动作
    // 2.忽略
    // 3.自定义处理函数
    signal(SIGINT, sigExitHandler);
    //signal(SIGQUIT, sigExitHandler);
    //signal(SIGTSTP, sigExitHandler); ^Z

    while (1)
    {
        sleep(2);
        printf("hello world\n");
        
    }
    /**
     * 要学会用killall杀死进程！！
     * 先不要用全部的退出信号
     * pidof
     * ps
     * ps -aux | grep signal_test.out | grep -v grep
    */

    return 0;
}