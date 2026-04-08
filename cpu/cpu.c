#include <cpu.h>
#include <stdlib.h>

CPU *init_cpu() {
    CPU *cpu = malloc(sizeof(CPU));
    cpu->mem = init_memory();

    return cpu;
}

void free_cpu(CPU *cpu) {
    free(cpu->mem);
    free(cpu);
}