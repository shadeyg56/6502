#include <cpu.h>
#include <stdbool.h>
#include <stdlib.h>
#include <mem.h>
#include <instructions.h>

CPU *init_cpu() {
    CPU *cpu = malloc(sizeof(CPU));
    cpu->mem = init_memory();

    cpu->pc = 0;
    cpu->sp = 0xFF;
    cpu->accum = 0;
    cpu->X = 0;
    cpu->Y = 0;
    cpu->flags = 0;

    return cpu;
}

/** Resets cpu by fetching start address for program counter
 * from reset address in memory
 */
void cpu_reset(CPU *cpu) {
    uint8_t start_addr_lo = fetch_memory(cpu->mem, POWER_ON_RESET_LO);
    uint8_t start_addr_hi = fetch_memory(cpu->mem, POWER_ON_RESET_LO+1);
    
    uint16_t start_addr = (start_addr_hi << 8) + start_addr_lo;
    cpu->pc = start_addr;
}

uint8_t cpu_fetch_instruction(CPU *cpu) {
    uint8_t data = fetch_memory(cpu->mem, cpu->pc++);
    return data;
}

void cpu_execute_instruction(CPU *cpu, uint8_t opcode) {
    Instruction instruction = INSTRUCTION_TABLE[opcode];
    uint16_t addr = instruction.addr_mode != NULL ? instruction.addr_mode(cpu) : 0;
    instruction.op(cpu, addr);
}

int cpu_run_count(CPU *cpu, uint64_t count) {
    for (int i = 0; i < count; i++) {
        uint8_t opcode = cpu_fetch_instruction(cpu);
        cpu_execute_instruction(cpu, opcode);
    }

    return count;
}

void free_cpu(CPU *cpu) {
    free(cpu->mem);
    free(cpu);
}

void stack_push(CPU *cpu, uint8_t value) {
    write_memory(cpu->mem, STACK_PAGE_START_ADDR + cpu->sp, value);
    cpu->sp--;
}

uint8_t stack_pull(CPU *cpu) {
    cpu->sp++;
    return fetch_memory(cpu->mem, STACK_PAGE_START_ADDR + cpu->sp);
}

void set_flag(CPU *cpu, uint8_t bitmask, bool cond) {
    if (cond) {
        cpu->flags |= bitmask;
    } else {
        cpu->flags &= ~bitmask;
    }
}

/** Sets the zero and negative status flags from a value.
 * Zero is set when the value is 0, negative when bit 7 is set.
 */
void set_zero_negative_flags(CPU *cpu, uint8_t value) {
    set_flag(cpu, CPU_STATUS_ZERO, (value == 0));
    set_flag(cpu, CPU_STATUS_NEGATIVE, (value & (1 << 7)));
}

void set_accum_flags_arith(CPU *cpu, uint8_t operand, uint8_t accum_prev) {
    uint8_t carry_in = cpu->flags & CPU_STATUS_CARRY;
    if (cpu->accum < operand || (carry_in && cpu->accum == operand)) {
            cpu->flags |= CPU_STATUS_CARRY;
    } else {
        cpu->flags &= ~CPU_STATUS_CARRY;
    }

    if (~(accum_prev ^ operand) & (accum_prev ^ cpu->accum) & (1 << 7)) {
        cpu->flags |= CPU_STATUS_OVERFLOW;
    } else {
        cpu->flags &= ~CPU_STATUS_OVERFLOW;
    }

    set_zero_negative_flags(cpu, cpu->accum);

}