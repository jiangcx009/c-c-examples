/*
 * Author:  wangzb
 * Date:    2015.6.19
 * Func:    Bin2Hex convert
 */
#include <stdio.h>
#include <stdlib.h>



/* verify gcsoc_logo && bin2hex */
extern char gcsoc_logo[];

#define CACHE_LEN 1024

int main()
{
    char cache[CACHE_LEN];

    FILE *src = fopen("gcsoc.bmp", "rb");
    FILE *dst = fopen("gcsoc3.bmp", "+wb");

    fseek(src, 0, SEEK_END);
    fseek(dst, 0, SEEK_END);


    fread(cache, 1, 64, srcfd);
    fwrite(cache, 1, 64, dstfd);

    fclose(srcfd);

    fwrite(gcsoc_logo,1,320*240*3,dstfd);
    fclose(dstfd)


    return 0;
}
