/*
 * Author:  wangzb
 * Date:    2015.6.23
 * Func:    reverse file for bin
 */
#include <stdio.h>
#include <stdlib.h>

void help(void)
{
    printf(" reverse4bin tool:  ./reverse4bin src dst\n");
}

#define CACHE_LEN 1024
char cache[CACHE_LEN];

#define REVERSE_FOR_BMP


int main(int args, void **argv)
{
    if (args != 3) {
        help();
        exit(0);
    }

    char *src = (char *)argv[1];
    char *dst = (char *)argv[2];

    FILE *srcfd = fopen(src, "rb");
    if (srcfd < 0) {
        printf("src file is not exist\n");
        exit(1);
    }
    FILE *dstfd = fopen(dst, "w+");
    if (dstfd < 0) {
        printf("dst file create failed\n");
        exit(1);
    }

    unsigned int off = ftell(srcfd);
    fseek(srcfd, 0, SEEK_END);
    unsigned int size = ftell(srcfd) - off;
    fseek(srcfd, 0, SEEK_SET);

#ifdef REVERSE_FOR_BMP
    fread(cache, 1, 54, srcfd);
    fseek(dstfd, 0, SEEK_SET);
    fwrite(cache, 1, 54, dstfd);
    size -= 64;
#endif
    int i = 0; 
    char c = 0;
    for (i = 0; i < size; i ++) {
        fseek(srcfd, -1*i, SEEK_END);
        fread(cache, 1, 1, srcfd);
        //printf("cahce : 0x%02x\n", cache[0] & 0xff);
        fwrite(cache, 1, 1, dstfd);
    }


    printf("reverse bin file ok!\n");
    fclose(srcfd);
    fclose(dstfd);
}
