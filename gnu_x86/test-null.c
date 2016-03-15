#include <stdio.h>

int main() {
    int a = 0;
    void *p1 = (void *) &a;

    void *p2 = p1;
    void *p3 = p1;

    void *p4 = NULL;
    void *p5 = NULL;
    void *p6 = NULL;

    printf("!p1 + !p2 + !p3 = %d\n", !p1 + !p2 + !p3);
    printf("!p1 + !p2 + !p4 = %d\n", !p1 + !p2 + !p4);
    printf("!p1 + !p5 + !p4 = %d\n", !p1 + !p5 + !p4);
    printf("!p6 + !p5 + !p4 = %d\n", !p6 + !p5 + !p4);

    scanf("%d", &a);
}
