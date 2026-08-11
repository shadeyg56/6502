#ifndef _CPU_H
#define _CPU_H

#include <stdbool.h>
#include <mem.h>

// CPU Status Flag bitmasks
#define CPU_STATUS_CARRY 1
#define CPU_STATUS_ZERO (1 << 1)
#define CPU_STATUS_INTERRUPT (1 << 2)
#define CPU_STATUS_DECIMAL (1 << 3)
#define CPU_STATUS_BREAK (1 << 4)
#define CPU_STATUS_UNUSED (1 << 5)
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
uint8_t cpu_fetch_instruction(CPU *cpu);
void cpu_execute_instruction(CPU *cpu, uint8_t opcode);
void cpu_reset(CPU *cpu);
void free_cpu(CPU *cpu);
void stack_push(CPU *cpu, uint8_t value);
uint8_t stack_pull(CPU *cpu);
void set_flag(CPU *cpu, uint8_t bitmask, bool cond);
void set_zero_negative_flags(CPU *cpu, uint8_t value);
void set_accum_flags_arith(CPU *cpu, uint8_t operand, uint8_t accum_prev);


#endif