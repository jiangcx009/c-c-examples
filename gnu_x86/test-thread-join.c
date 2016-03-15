#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


int kill = 0;
void *loop(void *arg)
{
    pthread_t pid = pthread_self();
    printf("sub thread is created! its pid is %d\n", (unsigned int)pid);

    /*Sleep 1s*/
    while (!kill) {
        printf("sub thread try to join itself\n");
        int res = pthread_join(pid, NULL);
        if (res < 0) {
            printf("sub thread join itself failed\n");
        } else {
            printf("sub thread join itself ok\n");
        }
        sleep(1);
    }

    printf("sub thread exit!\n");
}


void main() {

    pthread_t pid;

    int res = pthread_create(&pid, NULL, loop, NULL);
    if (res != 0) {
        printf("create thread failed!\n");
    } else {
        printf("create thread ok!, pid = %d\n", (unsigned int)pid);
    }

    /* Sleep 10s*/
    printf("main thread sleep 2s\n");
    sleep(2);

    printf("main thread wait kill sub thread\n");
    kill = 1;
    res = pthread_join(pid, NULL);
    if (res < 0) {
        printf("main thread join sub thread failed\n");
    } else {
        printf("main thread join sub thread ok\n");
    }

    printf("main thread exit!\n");

}
