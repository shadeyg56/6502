#include <stdio.h>
#include <cpu.h>

int main() {

    CPU *cpu = init_cpu();

    cpu->mem->ram[100] = 'A';

    printf("%c\n", cpu->mem->ram[100]);

    free_cpu(cpu);

    return 0;
}