#include <cpu.h>
#include <stdlib.h>

CPU *init_cpu() {
    CPU *cpu = malloc(sizeof(CPU));
    cpu->mem = init_memory();

    return cpu;
}

uint8_t cpu_fetch_instruction(CPU *cpu) {
    uint8_t data = fetch_memory(cpu->mem, cpu->pc++);
}

void cpu_decode_instruction(CPU *cpu) {
}

void free_cpu(CPU *cpu) {
    free(cpu->mem);
    free(cpu);
}