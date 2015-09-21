/*
 * Author:Bob Wang
 * Date: 2014.4.14
 * Func: Test when event loop exits, if event base contains a event/timer,
 * after calling event_base_loopexit/break;
 */

#include <stdio.h>
#include "event.h"
#include <pthread.h>
#include <string.h>
#include <time.h>

char times[32];

char *time_str()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct tm *t;
    t = localtime(&tv.tv_sec);

    memset(times, 0, sizeof(times));
    int len = strftime(times, sizeof(times), "%H:%M:%S", t);
    snprintf(times + len, sizeof(times) - len, "%03ld", tv.tv_usec/1000);

    return times;
}


void *loop(void *arg)
{
    struct event_base *base = (struct event_base *)arg;

    printf("[%s] : event base loop is started!\n", time_str());

    event_base_loop(base, 0);

    printf("[%s] : event base loop is stopped!\n", time_str());

    return NULL;
}


void ev_handle(int fd, short which, void *arg)
{
    printf("[%s] : event is actived\n", time_str());
}

void timer_handle(int fd, short which, void *arg)
{
    printf("[%s] : timer is actived\n", time_str());
}

int main()
{
    struct event_base *base = event_base_new();

    struct timeval tv = {5, 0};
    struct event *timer = evtimer_new(base, timer_handle, NULL);
    if (timer == NULL) {
        printf("xxxxxxxx!!!\n");
        return;
    }
    pthread_t tid;

/*************** timer + loopexit ************************************/
    /*start loop for timer*/
    pthread_create(&tid, NULL, loop, base);

    printf("[%s] : timer is added\n", time_str());
    evtimer_add(timer , &tv);

    /*check if timer exit immediately after call event_base_loopexit*/
    printf("[%s] : loop exit is called\n", time_str());
    event_base_loopexit(base, NULL);
    printf("[%s] : thread starts join\n", time_str());
    pthread_join(tid, NULL);
    printf("[%s] : thread join ok\n", time_str());

    printf("\n\n============================\n\n");
/*************** timer + loopbreak ***********************************/

    /*start loop for timer*/
    pthread_create(&tid, NULL, loop, base);

    printf("[%s] : timer is added\n", time_str());
    evtimer_add(timer, &tv);

    /*check if timer exit immediately after call event_base_loopexit*/
    printf("[%s] : loop break is called\n", time_str());
    event_base_loopbreak(base);
    printf("[%s] : thread starts join\n", time_str());
    pthread_join(tid, NULL);
    printf("[%s] : thread join ok\n", time_str());


}
