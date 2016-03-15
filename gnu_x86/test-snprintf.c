#include <stdio.h>
#include <string.h>



void main() {

    char str[10];
    //const char *p = "123456789";
    const char *p = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    int spl = 3;

    memset(str, 0, 10);

    int len = snprintf(str, spl, "%s", p);

    printf(" p :%s\n str:%s\n splen:%d, retlen:%d, strsize:%ld, plen:%ld\n", p, str, spl, len, sizeof(str), strlen(p));

    pause();
}
