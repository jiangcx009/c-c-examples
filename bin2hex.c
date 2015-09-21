/*
 * Author:  wangzb
 * Date:    2015.6.19
 * Func:    Bin2Hex convert
 */
#include <stdio.h>
#include <stdlib.h>

void help(void)
{

    printf(" Bin2Hex tool:  ./Bin2Hex binfile hexfile\n");
}

#define CACHE_LEN 1024
char cache[CACHE_LEN];
char out[CACHE_LEN * 4];

unsigned int bin2hex(char *bin, char *hex, unsigned int len)
{
    unsigned i = 0, off = 0;
    for (i = 0; i < len; i ++) {
        off += sprintf(hex + off, "0x%02x, ", (*(bin + i)) & 0xFF);
        if (i % 8  ==  7) {
            off += sprintf(hex + off, "\r\n");
        }
    }
    return off;
}


int main(int args, void **argv) 
{
    char *bin = NULL, *hex = NULL;
    int i = 0;

    if (args != 3) {
        help();
        exit(0);
    }

    bin = (char *)argv[1];
    hex = (char *)argv[2];

    FILE *binfd = fopen(bin, "rb");
    if (binfd < 0) {
        printf("bin file is not exist\n");
        exit(1);
    }
    FILE *hexfd = fopen(hex, "w+");
    if (hexfd < 0) {
        printf("hex file create failed\n");
        exit(1);
    }

    unsigned int off = ftell(binfd);
    fseek(binfd, 0, SEEK_END);
    unsigned int size = ftell(binfd) - off;
    fseek(binfd, 0, SEEK_SET);

    printf("bin file size:%d\n", size);

    for (i = 0; i < size; i += sizeof(cache)) {
        unsigned int len = (size - i) > sizeof(cache) ? sizeof(cache) : (size - i);
        fread(cache, 1, len, binfd);
        unsigned int hex_len = bin2hex(cache, out, len);
        fwrite(out, 1, hex_len, hexfd);
    }

    printf("bin2hex ok!\n");
    fclose(hexfd);
    fclose(binfd);

}
