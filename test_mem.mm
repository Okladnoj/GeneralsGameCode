#include <stdlib.h>
#include <stdio.h>

int main() {
    void* p = malloc(1024);
    free(p);
    printf("Malloc works!\n");
    return 0;
}
