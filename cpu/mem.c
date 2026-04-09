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

uint8_t fetch_memory(Memory *mem, uint16_t addr) {
    if (addr <= 0x100) {
        return mem->zp[addr];
    } else if (addr <= 0x200) {
        return mem->stack[addr-0x100];
    } else if (addr <= RAM_SIZE+0x200) {
        return mem->ram[addr-0x200];
    } else if (addr <= RAM_SIZE+0x200+0x100) {
        return mem->last_six[addr-RAM_SIZE-0x200-0x100];
    }
}

void clear_memory(Memory *mem) {
    memset(mem->zp, 0, sizeof(mem->zp));
    memset(mem->stack, 0, sizeof(mem->stack));
    memset(mem->ram, 0, sizeof(mem->ram));
}