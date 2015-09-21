/*
 * =====================================================================================
 *
 *       Filename:  test_union.c
 *
 *    Description:  联合体的内存使用
 *                  1. union 的成员共享同一块内存,是成员中的最大值
 *                  2. union 的成员寻址都是从同一个地址寻址
 *                  3. union 的成员同一时间只能使用一个
 *
 *        Version:  1.0
 *        Created:  2015年09月18日 12时39分55秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Morning
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>

typedef union test
{
    char    a;
    int     b;
}test_u;

typedef union test_bit
{
    char    a;
    int     b_2:2;
    int     b_3:3;
    int        :3;
    int     b_6:6;
    long    c;
}test_bit_u;

///////////////////////////////function of test_u////////////////////////////
int get_union_size(test_u test_union)
{
    return sizeof(test_union);
}

int get_union_member1_size(test_u test_union)
{
    return sizeof(test_union.a);
}

int get_union_member2_size(test_u test_union)
{
    return sizeof(test_union.b);
}

void* get_union_addr(test_u test_union)
{
    return &test_union;
}

void* get_union_member1_addr(test_u test_union)
{
    return &(test_union.a);
}

void* get_union_member2_addr(test_u test_union)
{
    return &(test_union.b);
}

void print_union_member_value(test_u test_union)
{
    test_union.a = 1;
    test_union.b = 2;
    printf("union member char value:0x%x\n", test_union.a);

    test_union.b = 2;
    test_union.a = 1;
    printf("union member char value:0x%x\n", test_union.a);
}

///////////////////////////////function of test_bit_u////////////////////////////
int get_union_bit_size(test_bit_u test_union)
{
    return sizeof(test_union);
}

int get_union_bit_member1_size(test_bit_u test_union)
{
    return sizeof(test_union.a);
}

int get_union_bit_member2_size(test_bit_u test_union)
{
    //return sizeof(test_union.b_2);
}

int get_union_bit_member5_size(test_bit_u test_union)
{
    return sizeof(test_union.c);
}

void* get_union_bit_addr(test_bit_u test_union)
{
    return &test_union;
}

void* get_union_bit_member1_addr(test_bit_u test_union)
{
    return &(test_union.a);
}

void* get_union_bit_member2_addr(test_bit_u test_union)
{
    //return &(test_union.b_2);
}

void* get_union_bit_member5_addr(test_bit_u test_union)
{
    return &(test_union.c);
}


void main()
{
    test_u      union_test;
    test_bit_u  union_bit_test;

    print_union_member_value(union_test);

    printf("\nunion_u info....\n");
    printf("union addr:0x%x\n", get_union_addr(union_test));
    printf("union member char addr:0x%x\n", get_union_member1_addr(union_test));
    printf("union member int addr:0x%x\n", get_union_member2_addr(union_test));

    printf("total union size:0x%x\n", get_union_size(union_test));
    printf("union char size:0x%x\n", get_union_member1_size(union_test));
    printf("union int size:0x%x\n", get_union_member2_size(union_test));

    printf("\nunion_bit_u info....\n");
    printf("union addr:0x%x\n", get_union_bit_addr(union_bit_test));
    printf("union member char addr:0x%x\n", get_union_bit_member1_addr(union_bit_test));
    printf("union member int_b2 addr:0x%x\n", get_union_bit_member2_addr(union_bit_test));
    printf("union member long_c addr:0x%x\n", get_union_bit_member5_addr(union_bit_test));

    printf("total union size:0x%x\n", get_union_bit_size(union_bit_test));
    printf("union char size:0x%x\n", get_union_bit_member1_size(union_bit_test));
    printf("union int_b2 size:0x%x\n", get_union_bit_member2_size(union_bit_test));
    printf("union long_c size:0x%x\n", get_union_bit_member5_size(union_bit_test));

}
