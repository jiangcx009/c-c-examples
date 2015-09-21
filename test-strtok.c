#include <stdio.h>
#include <string.h>

void main() {
    char str[] = "Server:client:01:A";
    char *p = strtok(str, ":");

    printf("string out while p is %s\n",p);
    while(p != NULL) {
        p = strtok(NULL, ":");
        printf("string in while p is %s\n",p);
    }
    pause();
}
