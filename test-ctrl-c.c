
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>


#if 0
/* method 1 is ok, with inputting*/
static void sig_handler(int sig)
{
    printf("signal handler is called!\n");
    exit(0);
}



void main()
{
    signal(SIGINT, sig_handler);

    while(1){
        printf("running \n");
        sleep(1);
    }
}

#else

/* method 2 is ok, with inputting*/
void sig_hander2(int sig, siginfo_t *si, void *arg)
{
    printf("signal hander2 is called!\n");
    exit(0);
}

void main()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = sig_hander2;
    sa.sa_flags = SA_SIGINFO;

    sigaction(SIGINT, &sa, NULL);

    while(1) {
        printf("running \n");
        sleep(1);
    }


}
#endif
