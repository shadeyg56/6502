#ifndef _INSTRUCTIONS_H
#define _INSTRUCTIONS_H

#include <stdint.h>
#include <cpu.h>
#include "util.h"

typedef struct {
    uint8_t opcode;
    void (*op)(CPU *, uint16_t);
    uint16_t (*addr_mode)(CPU *);
} Instruction;

extern const Instruction INSTRUCTION_TABLE[256];

// CPU Addressing Modes
uint16_t IMM(CPU *cpu);
uint16_t ZP(CPU *cpu);
uint16_t ZPX(CPU *cpu);
uint16_t ZPY(CPU *cpu);
uint16_t ABS(CPU *cpu);
uint16_t ABSX(CPU *cpu);
uint16_t ABSY(CPU *cpu);
uint16_t REL(CPU *cpu);
uint16_t IND(CPU *cpu);
uint16_t INDX(CPU *cpu);
uint16_t INDY(CPU *cpu);


#endif