#include <mem.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Memory *init_memory() {
    Memory *mem = malloc(sizeof(Memory));
    if (mem == NULL) {
        printf("Failed to init memory\n");
        return NULL;
    }

    clear_memory(mem);

    return mem;
}

void clear_memory(Memory *mem) {
    memset(mem->zp, 0, sizeof(mem->zp));
    memset(mem->stack, 0, sizeof(mem->stack));
    memset(mem->ram, 0, sizeof(mem->ram));
}