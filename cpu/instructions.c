#include <instructions.h>
#include <stdio.h>

// uint16_t ACC(CPU *cpu) {
//     return cpu->accum
// }

uint16_t IMM(CPU *cpu) {
    return cpu->pc++;   
}

uint16_t ZP(CPU *cpu) {
    uint8_t addr = cpu_fetch_instruction(cpu);

    return addr;
}

uint16_t ZPX(CPU *cpu) {
    uint8_t addr = cpu_fetch_instruction(cpu) + cpu->X;
    return addr;
}

uint16_t ZPY(CPU *cpu) {
    uint8_t addr = cpu_fetch_instruction(cpu) + cpu->Y;
    return addr;
}

uint16_t ABS(CPU *cpu) {
    uint8_t lo = cpu_fetch_instruction(cpu);
    uint8_t hi = cpu_fetch_instruction(cpu);

    return (hi << 8) | lo;
}

uint16_t ABSX(CPU *cpu) {
    return ABS(cpu) + cpu->X;
}

uint16_t ABSY(CPU *cpu) {
    return ABS(cpu) + cpu->Y;
}

uint16_t REL(CPU *cpu) {
    int8_t addr_rel = (int8_t) cpu_fetch_instruction(cpu);

    return cpu->pc + addr_rel;
}

uint16_t IND(CPU *cpu) {
    uint8_t lo_ind = cpu_fetch_instruction(cpu);
    uint8_t hi_ind = cpu_fetch_instruction(cpu);


    uint16_t addr_ind = (lo_ind << 8) | hi_ind;
    uint8_t lo_real = fetch_memory(cpu->mem, addr_ind);
    uint8_t hi_real = fetch_memory(cpu->mem, addr_ind+1);
    return (hi_real << 8) | lo_real;
}

uint16_t INDX(CPU *cpu) {
    uint8_t lo_ind = cpu_fetch_instruction(cpu) + cpu->X;

    uint8_t lo_real = fetch_memory(cpu->mem, lo_ind);
    
    return lo_real;
}

uint16_t INDY(CPU *cpu) {
    uint8_t lo_ind = cpu_fetch_instruction(cpu) + cpu->X;

    uint8_t lo_real = fetch_memory(cpu->mem, lo_ind);
    
    return lo_real;
}