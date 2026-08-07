#ifndef _MEM_H
#define _MEM_H

#include <stdint.h>

#define RAM_SIZE ((1 << 16) - 0x206)
#define POWER_ON_RESET_LO 0xFFFC
#define IRQ_VECTOR_LO 0xFFFE

#define STACK_PAGE_START_ADDR 0x100

typedef struct {
   uint8_t zp[0x100]; // 256 byte zero page
   uint8_t stack[0x100]; // 256 byte stack
   uint8_t ram[RAM_SIZE]; 
   uint8_t last_six[0x06];
} Memory;

Memory *init_memory();
int load_program(Memory *mem, char *filename);
uint8_t fetch_memory(Memory *mem, uint16_t addr);
int write_memory(Memory *mem, uint16_t addr,  uint8_t data);
void clear_memory(Memory *mem);

#endif