/*
 * Author:  Zeb
 * Date:    2015.4.27
 * Func:    Listening one port, waiting other test to connect it.
 *
 */

#include <stdio.h>
#include <arpa/inet.h>

#define IP "192.168.9.64" 
//#define IP "127.0.0.1" 
#define PORT    16501

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in sai;
    inet_aton(IP, &sai.sin_addr);
    sai.sin_family = AF_INET;
    sai.sin_port = htons(PORT);

    bind(fd, (struct sockaddr *)&sai, sizeof(struct sockaddr));

    listen(fd, 1000);

    struct sockaddr peer;
    int len =  sizeof(struct sockaddr);

    while(1) {
        char c;
        printf("Listening %s:%d\n",IP,PORT);
        int peerfd = accept(fd, &peer, &len);
        printf("Accept a new connection, sleep 3 seconds\n");
        //printf("Close it?\n");
        //scanf("%c", &c);
        //if (c == 'y' || c == 'Y')
        close(peerfd);
    }
}
