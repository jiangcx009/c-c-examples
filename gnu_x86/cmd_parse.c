/*
 * xxxx
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define ADDR_CHAR_NUM   8
#define VAL_CHAR_NUM    8

int cmd_parse(char * buf, int size)
{
    char *paddr = NULL, *pval = NULL;
    int addr_len = 0, val_len = 0;
    char in_type = 0; //0:addr, 1:val
    size_t i = 0;
    int ret = 0;
    unsigned int addr, val;
    char addr_s[ADDR_CHAR_NUM + 1], val_s[VAL_CHAR_NUM + 1];

    memset(addr_s, 0, sizeof(addr_s));
    memset(val_s, 0, sizeof(val_s));

    while(i < size) {
        char c = *(buf + i);
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {

            if (in_type == 0 ) {
                if (paddr == NULL) {
                    paddr = buf + i;
                }
                addr_s[addr_len] = *(buf + i);
                addr_len ++;
                if (addr_len > ADDR_CHAR_NUM) {
                    ret = 1;
                    break;
                }
            } else if (in_type == 1) {
                if (pval == NULL) {
                    pval = buf + i;
                }
                val_s[val_len] = *(buf + i);
                val_len ++;
                if (val_len > VAL_CHAR_NUM) {
                    ret = 2;
                    break;
                }
            }
        } else {
            if (paddr != NULL && pval == NULL) {
                in_type = 1;
            } else if (pval != NULL) {
                break;
            }
        }
        i ++;
    }

    if (ret == 0 && addr_len == ADDR_CHAR_NUM) {
        sscanf(addr_s, "%x",&addr );
        printf("addr:%s  ==> 0x%08x\n",addr_s, addr);

        printf("pval:%s\n", val_s);
        sscanf(val_s, "%x", &val);
        printf("val:%s  ==> 0x%08x\n",val_s, val);
    }
    return 0;
}


void main(int args, void *argv[]) 
{
    char buf[] = "fe003000 cf";
    cmd_parse(buf, 11);
}
