#include <cpu.h>
#include <stdlib.h>

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

void cpu_decode_instruction(CPU *cpu) {
}

void free_cpu(CPU *cpu) {
    free(cpu->mem);
    free(cpu);
}