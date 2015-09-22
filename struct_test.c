/*
 * =====================================================================================
 *
 *       Filename:  struct_test.c
 *
 *    Description:  结构提的内存使用
 *
 *        Version:  1.0
 *        Created:  2015年09月22日 09时38分58秒
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

enum 
{
    TEST_S0,
    TEST_S1,
    TEST_S2
};

typedef struct test
{
    char    a;
    int     b;
    char    c;
    float   d;
    char    e;
    char    f[4];
}test_s;

typedef struct test_bit
{
    char    a;
    int     b_2:2;
    int     b_3:3;
    char    c;
}test_bit_s;

typedef struct test_union
{
    int     a;
    char    b;
    union   {
        int     c;
        char    d;
    }
}test_union_s;


int get_struct_size ( int selection )
{
    switch(selection) {
        case TEST_S0 : return sizeof(test_s); break;
        case TEST_S1 : return sizeof(test_bit_s); break;
        case TEST_S2 : return sizeof(test_union_s); break;
    }
}

void* get_struct_addr ( void *test )
{
    return test;
}

void* get_struct_member_addr ( int selection, int member, void *test )
{
    switch(selection) {
        case TEST_S0 : 
            switch(member) {
                case 0 : return &((test_s *)test)->a; break;
                case 1 : return &((test_s *)test)->b; break;
                case 2 : return &((test_s *)test)->c; break;
                case 3 : return &((test_s *)test)->d; break;
                case 4 : return &((test_s *)test)->e; break;
                case 5 : return &((test_s *)test)->f; break;
            }
            break;
        case TEST_S1:
            switch(member) {
                case 0 : return &((test_bit_s *)test)->a; break;
                //case 1 : return &((test_bit_s *)test)->b_2; break;
                //case 2 : return &((test_bit_s *)test)->b_3; break;
                case 1 : return &((test_bit_s *)test)->c; break;
            }
            break;
        case TEST_S2:
            switch(member) {
                case 0 : return &((test_union_s *)test)->a; break;
                case 1 : return &((test_union_s *)test)->b; break;
                case 2 : return &((test_union_s *)test)->c; break;
                case 3 : return &((test_union_s *)test)->d; break;
            }
            break;
    } 
}

 
int get_struct_member_size ( int selection, int member, void *test )
{
    switch(selection) {
        case TEST_S0 : 
            switch(member) {
                case 0 : return sizeof(((test_s *)test)->a); break;
                case 1 : return sizeof(((test_s *)test)->b); break;
                case 2 : return sizeof(((test_s *)test)->c); break;
                case 3 : return sizeof(((test_s *)test)->d); break;
                case 4 : return sizeof(((test_s *)test)->e); break;
                case 5 : return sizeof(((test_s *)test)->f); break;
            }
            break;
        case TEST_S1 :
            switch(member) {
                case 0 : return sizeof(((test_bit_s *)test)->a); break;
                //case 1 : return sizeof(((test_bit_s *)test)->b_2); break;
                //case 2 : return sizeof(((test_bit_s *)test)->b_3); break;
                case 1 : return sizeof(((test_bit_s *)test)->c); break;
            }
            break;
        case TEST_S2 : 
            switch(member) {
                case 0 : return sizeof(((test_union_s *)test)->a); break;
                case 1 : return sizeof(((test_union_s *)test)->b); break;
                case 2 : return sizeof(((test_union_s *)test)->c); break;
                case 3 : return sizeof(((test_union_s *)test)->d); break;
            }
    }
} 

void main()
{
    int             i = 0;
    test_s          struct_test;
    test_bit_s      struct_bit_test;
    test_union_s    struct_union_test;

    printf("\nstruct_test info...\n");
    printf("total struct size:0x%x\n", get_struct_size(TEST_S0));
    printf("total struct addr:0x%x\n", get_struct_addr((void *)&struct_test));

    for ( i = 0; i < 6; i++ ) {
        printf("struct member %d addr:0x%x size:0x%x\n", i, \
                get_struct_member_addr(TEST_S0, i, (void *)&struct_test), \
                get_struct_member_size(TEST_S0, i, (void *)&struct_test));
    }

    printf("\nstruct_bit_test info...\n");
    printf("total struct size:0x%x\n", get_struct_size(TEST_S1));
    printf("total struct addr:0x%x\n", get_struct_addr((void *)&struct_bit_test));

    for ( i = 0; i < 2; i++ ) {
        printf("struct member %d addr:0x%x size:0x%x\n", i, \
                get_struct_member_addr(TEST_S1, i, (void *)&struct_bit_test), \
                get_struct_member_size(TEST_S1, i, (void *)&struct_bit_test));
    }

    printf("\nstruct_union_test info...\n");
    printf("total struct size:0x%x\n", get_struct_size(TEST_S2));
    printf("total struct addr:0x%x\n", get_struct_addr((void *)&struct_union_test));

    for ( i = 0; i < 4; i++ ) {
        printf("struct member %d addr:0x%x size:0x%x\n", i, \
                get_struct_member_addr(TEST_S2, i, (void *)&struct_union_test), \
                get_struct_member_size(TEST_S2, i, (void *)&struct_union_test));
    }
}
