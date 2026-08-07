#include <cpu.h>
#include <stdbool.h>
#include <stdlib.h>
#include <mem.h>

CPU *init_cpu() {
    CPU *cpu = malloc(sizeof(CPU));
    cpu->mem = init_memory();

    return cpu;
}

/** Resets cpu by fetching start address for program counter
 * from reset address in memory
 */
void cpu_reset(CPU *cpu) {
    uint8_t start_addr_lo = fetch_memory(cpu->mem, POWER_ON_RESET_LO);
    uint8_t start_addr_hi = fetch_memory(cpu->mem, POWER_ON_RESET_LO+1);
    
    uint16_t start_addr = start_addr_lo | (start_addr_hi << 8);
    cpu->pc = start_addr;
}

uint8_t cpu_fetch_instruction(CPU *cpu) {
    uint8_t data = fetch_memory(cpu->mem, cpu->pc++);
    return data;
}

void cpu_decode_instruction(CPU *cpu, uint8_t opcode) {
    
}

void free_cpu(CPU *cpu) {
    free(cpu->mem);
    free(cpu);
}

// void stack_push(CPU *cpu, uint8_t value) {
//     uint16_t stack_start_addr = 0x100;

// }

void set_flag(CPU *cpu, uint8_t bitmask, bool cond) {
    if (cond) {
        cpu->flags |= bitmask;
    } else {
        cpu->flags ^= bitmask;
    }
}

/** Sets the zero and negative status flags from a value.
 * Zero is set when the value is 0, negative when bit 7 is set.
 */
void set_zero_negative_flags(CPU *cpu, uint8_t value) {
    set_flag(cpu, CPU_STATUS_ZERO, (value == 0));
    set_flag(cpu, CPU_STATUS_NEGATIVE, (value & (1 << 7)));
}

void set_accum_flags_arith(CPU *cpu, uint8_t operand) {
    uint8_t carry_in = cpu->flags & CPU_STATUS_CARRY;
    if (cpu->accum < operand || (carry_in && cpu->accum == operand)) {
            cpu->flags |= CPU_STATUS_CARRY;
    } else {
        cpu->flags ^= CPU_STATUS_CARRY;
    }

    set_zero_negative_flags(cpu, cpu->accum);

    // TODO: Overflow flag
}