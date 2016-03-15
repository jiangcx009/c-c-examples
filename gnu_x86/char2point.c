/*
 * =====================================================================================
 *
 *       Filename:  char2point.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2015年08月12日 19时32分43秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>

void main(void)
{
    char *p_ch = (char *)(0x2000);
    char *p_ma = (char *)malloc(1);
    //*(char *)(0x2000) = 0xbeef;
    //*p_ch = 0xbeef;
    //printf("p_ch: 0x%x, *p_ch: 0x%x\n", p_ch, *p_ch);
    printf("p_ma: 0x%x, *p_ma: 0x%x\n", p_ma, *p_ma);
}


