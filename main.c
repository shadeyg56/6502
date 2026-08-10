#include <stdio.h>
#include "cpu.h"
#include "instructions.h"

int main() {

    CPU *cpu = init_cpu();

    load_program(cpu->mem, "./6502_functional_test.bin");
    // cpu_reset(cpu);
    cpu->pc = 0x0400;
    printf("program counter: %x\n", cpu->pc);

    while (1) {
        uint16_t prev_pc = cpu->pc;

        uint8_t opcode = cpu_fetch_instruction(cpu);
        //printf("Executing %04x: %02x\n", prev_pc, opcode);
        cpu_execute_instruction(cpu, opcode);

        if (cpu->pc == prev_pc) {
            if (cpu->pc == 0x3469) {
                printf("PASS: all 43 tests\n");
            } else {
                printf("FAIL: trap at $%04X, test #%d\n",
                       cpu->pc, fetch_memory(cpu->mem, 0x0200));
                dump_memory(cpu->mem, "failure-mem-dump.bin");
                printf("CPU Status at failure: 0x%x\n", cpu->flags);
                printf("CPU Accum: %d\n", cpu->accum);
            }
            break;
        }
    }

    free_cpu(cpu);

    return 0;
}