#ifndef _MEM_H
#define _MEM_H

#include <stdint.h>

#define RAM_SIZE (1 << 16) - 0x206

typedef struct {
   uint8_t zp[0x100]; // 256 byte zero page
   uint8_t stack[0x100]; // 256 byte stack
   uint8_t ram[RAM_SIZE]; 
   uint8_t last_six[0x06];
} Memory;

Memory *init_memory();
uint8_t fetch_memory(Memory *mem, uint16_t addr);
void clear_memory(Memory *mem);

#endif