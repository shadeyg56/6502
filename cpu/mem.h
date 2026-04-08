#ifndef _MEM_H
#define _MEM_H

#include <stdint.h>

typedef struct {
   uint8_t zp[0x100]; // 256 byte zero page
   uint8_t stack[0x100]; // 256 byte stack
   uint8_t ram[0xFDFB]; 
} Memory;

Memory *init_memory();
void clear_memory(Memory *mem);

#endif