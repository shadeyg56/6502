#ifndef _CPU_H
#define _CPU_H

#include <mem.h>

// CPU Status Flag bitmasks
#define CPU_STATUS_CARRY 1
#define CPU_STATUS_ZERO (1 << 1)
#define CPU_STATUS_INTERRUPT (1 << 2)
#define CPU_STATUS_DECIMAL (1 << 3)
#define CPU_STATUS_BREAK (1 << 4)
#define CPU_STATUS_OVERFLOW (1 << 6)
#define CPU_STATUS_NEGATIVE (1 << 7)

typedef struct {
    Memory *mem;
    // registers
    uint16_t pc; // program counter
    uint8_t sp; // stack pointer
    uint8_t accum; // accumulator
    uint8_t X; // index X
    uint8_t Y; //index y
    uint8_t flags; // cpu status flags

} CPU; 

CPU *init_cpu();
void free_cpu(CPU *cpu);


#endif