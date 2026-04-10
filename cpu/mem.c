#include <mem.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Memory *init_memory()
{
    Memory *mem = malloc(sizeof(Memory));
    if (mem == NULL) {
        printf("Failed to init memory\n");
        return NULL;
    }

    clear_memory(mem);

    return mem;
}

uint8_t fetch_memory(Memory *mem, uint16_t addr) {
    if (addr <= 0xFF) {
        return mem->zp[addr];
    } else if (addr < 0x200) {
        return mem->stack[addr-0x100];
    } else if (addr < RAM_SIZE+0x200) {
        return mem->ram[addr-0x200];
    } else {
        return mem->last_six[addr-RAM_SIZE-0x200];
    }
}   

int write_memory(Memory *mem, uint16_t addr,  uint8_t data) {
    if (addr <= 0xFF) {
        mem->zp[addr] = data;
    } else if (addr < 0x200) {
        mem->stack[addr-0x100] = data;
    } else if (addr < RAM_SIZE+0x200) {
        mem->ram[addr-0x200] = data;
    } else {
        mem->last_six[addr-RAM_SIZE-0x200] = data;
    }
    return 0;
}

int load_program(Memory *mem, char *filename) {
    FILE *bin = fopen(filename, "rb");

    if (bin == NULL) {
        return -1;
    }

    fseek(bin, 0, SEEK_END);
    long size = ftell(bin);
    fseek(bin, 0, SEEK_SET);

    long unsigned available = (1 << 16);
    
    if (size > available) {
        printf("Program too large: %ld bytes, but only %lu bytes available\n", size, available);
        fclose(bin);
        return -1;
    }

    size_t bytes_read = fread(mem, 1, size, bin);

    if (bytes_read != size) {
        printf("Failed to read program: Got %lu bytes, expected %ld bytes\n", bytes_read, size);
        fclose(bin);
        return -1;
    }
    printf("Read %lu bytes from program\n", bytes_read);

    fclose(bin);

    return 0;
}

void clear_memory(Memory *mem) {
    memset(mem->zp, 0, sizeof(mem->zp));
    memset(mem->stack, 0, sizeof(mem->stack));
    memset(mem->ram, 0, sizeof(mem->ram));
    memset(mem->last_six, 0, sizeof(mem->last_six));
}