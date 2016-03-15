/*
 * Author:  Zeb
 * Date:    2015.4.28
 * Func:    List all local ip, and find which one can connect to internet
 *
 */

#include <stdio.h>
#include <ifaddrs.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <fcntl.h>


#define IP "192.168.9.64" 
//#define IP "127.0.0.1" 
#define PORT    16501

static char print_cache[128];

const char *print_int_bytes(int a)
{
    int len = 0, i = 0;
    memset(print_cache, 0, sizeof(print_cache));
    for (i = 0; i < sizeof(int); i ++) {
        len += snprintf(print_cache + len, sizeof(print_cache) - len, "%02x ", a & 0xFF);
        a = (a >> 2);
    }
    return print_cache;
}

void test_ip2int()
{
    const char *ip = "192.168.0.1";
    printf("inet_addr(\"192.168.0.1\")\t\t:==>\t%s\n", print_int_bytes(inet_addr(ip)));
    printf("inet_network(\"192.168.0.1\")\t\t:==>\t%s\n", print_int_bytes(inet_network(ip)));
    
    struct in_addr s;
    inet_aton(ip, &s);
    printf("inet_aton(\"192.168.0.1\", &s)\t\t:==>\t%s\n", print_int_bytes(s.s_addr));
    inet_pton(AF_INET, ip, &s);
    printf("inet_pton(AF_INET, \"192.168.0.1\", &s)\t:==>\t%s\n", print_int_bytes(s.s_addr));
}


void test_local_addr()
{
    struct ifaddrs *ias = NULL, *ia = NULL;
    char ip[128];
    void *addr = NULL;
    int socklen = 0;

    if (getifaddrs(&ias) < 0) {
        printf("get local addrs err: %d\n", errno);
        return;
    }

    if (ias != NULL) {
        for (ia = ias ; ia != NULL; ia = ia->ifa_next) {
            int af_fm = ia->ifa_addr->sa_family;
            if (af_fm == AF_INET) {
                addr = &((struct sockaddr_in *)ia->ifa_addr)->sin_addr;
                socklen = INET_ADDRSTRLEN;
            } else if (af_fm == AF_INET6) {
                addr = &((struct sockaddr_in6 *)ia->ifa_addr)->sin6_addr;
                socklen = INET6_ADDRSTRLEN;
            } else {
                continue;
            }
            printf("af name:%s \t af flag:%x family:%x\t", ia->ifa_name, ia->ifa_flags, ia->ifa_addr->sa_family);

            if (ia->ifa_addr) {
                memset(ip, 0, sizeof(ip));
                inet_ntop(af_fm, addr, ip, socklen);
                printf("af addr:%s \t", ip);
            }

            if (ia->ifa_netmask) {
                memset(ip, 0, sizeof(ip));
                inet_ntop(af_fm, addr, ip, socklen);
                printf("af mask:%s \t", ip);
            }

            if ((ia->ifa_flags & IFF_BROADCAST) && ia->ifa_broadaddr) {
                memset(ip, 0, sizeof(ip));
                inet_ntop(af_fm, addr, ip,  socklen);
                printf("af broadcast:%s \t", ip);
            } else if ((ia->ifa_flags & IFF_POINTOPOINT) && ia->ifa_dstaddr) {
                memset(ip, 0, sizeof(ip));
                inet_ntop(af_fm, addr, ip,  socklen);
                printf("af dstaddr:%s \t", ip);
            }
            printf("\n");
        }
    }
}

void test_local_host()
{
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
        printf("open socket err: %d\n", errno);
        return;
    }

    //fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK );

    struct sockaddr_in sa;
    //inet_aton("116.228.149.106", &sa.sin_addr);
    inet_aton(IP, &sa.sin_addr);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);

    if (connect(fd, (struct sockaddr *)&sa, sizeof(struct sockaddr)) < 0) {
        printf("connect to server, err: %d\n", errno);
        return;
    }

    sleep(1);
    
    struct sockaddr local;
    int len = sizeof(struct sockaddr);
    if (getsockname(fd, &local, &len) < 0) {
        printf("get socket name err:%d \n", errno);
        return;
    }

    printf("local ip is %s, port:%d\n", inet_ntoa(((struct sockaddr_in*)&local)->sin_addr),((struct sockaddr_in *)&local)->sin_port);

    close(fd);
    
}

int main() {

    //printf("\n****************************************\n");
    //test_ip2int();

    //printf("\n****************************************\n");
    //test_local_addr();

    printf("\n****************************************\n");
    test_local_host();

    return 0;
}
