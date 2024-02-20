#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

void thread_level1(void *arg)
{
    pthread_detach(pthread_self());
    int info = *(int *)arg;
    

}



int main()
{

    return 0;
}