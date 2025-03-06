/**
 * Apple and Linux have different behavior when reallocating a memory block.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char *p = malloc(2);
    printf("p: %p\n", p);

    p = realloc(p, 3000);
    strcpy(p, "hello");
    printf("p: %p\n", p);

    p = realloc(p, 2);
    printf("p: %p\n", p);
    
    printf("===============================\n");
    printf("p: %p %s\n", p, p);

    return 0;
}