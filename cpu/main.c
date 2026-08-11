#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "instructions.h"

int run_functional_test(CPU *cpu) {

    load_program(cpu->mem, "./6502_functional_test.bin");
    cpu_reset(cpu);
    cpu->pc = 0x0400; // starting addr for test
    while (1) {
        uint16_t prev_pc = cpu->pc;

        uint8_t opcode = cpu_fetch_instruction(cpu);
        cpu_execute_instruction(cpu, opcode);

        if (cpu->pc == prev_pc) {
            if (cpu->pc == 0x3469) {
                printf("PASS: all 43 tests\n");
                return 0;
            } else {
                printf("FAIL: trap at $%04X, test #%d\n",
                    cpu->pc, fetch_memory(cpu->mem, 0x0200));
                dump_memory(cpu->mem, "failure-mem-dump.bin");
                printf("CPU Status at failure: 0x%x\n", cpu->flags);
                return -1;
            }
        }
    }

    return -1;
}

int run_program(CPU *cpu, char *program_path) {

    int err = load_program(cpu->mem, program_path);
    if (err) {
        printf("Failed to load program %s into memory\n", program_path);
        return -1;
    }
    cpu_reset(cpu);

    while (1) {
        uint8_t opcode = cpu_fetch_instruction(cpu);
        cpu_execute_instruction(cpu, opcode); 
    }

    return 0;

}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: %s <path_to_program_file>\n", argv[0]);
        return -1;
    }

    CPU *cpu = init_cpu();

    if (strcmp(argv[1], "test") == 0) {
        run_functional_test(cpu);
    } else {
        run_program(cpu, argv[1]);
    }

    free_cpu(cpu);

    return 0;
}