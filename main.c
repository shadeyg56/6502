#include <stdio.h>
#include "cpu.h"
#include "instructions.h"

int main() {

    CPU *cpu = init_cpu();

    load_program(cpu->mem, "./6502_functional_test.bin");
    cpu_reset(cpu);
    printf("program counter: %x\n", cpu->pc);

    while (1) {
        uint8_t opcode = cpu_fetch_instruction(cpu);
        cpu_execute_instruction(cpu, opcode);
    }

    free_cpu(cpu);

    return 0;
}