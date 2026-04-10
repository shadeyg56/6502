#include <stdio.h>
#include "cpu.h"
#include "instructions.h"

int main() {

    CPU *cpu = init_cpu();

    cpu->mem->zp[0] = 0x01;
    cpu->mem->zp[1] = 0x20;

    cpu->mem->stack[0x20] = 0xFC;
    cpu->mem->stack[0x21] = 0xBA;

    uint16_t addr = IND(cpu);
    printf("%x\n", addr);

    uint8_t x = fetch_memory(cpu->mem, 0xFFFC);
    uint8_t y = fetch_memory(cpu->mem, 0xFFFD);
    printf("%x %x\n", x, y);

    load_program(cpu->mem, "./6502_functional_test.bin");
    cpu_reset(cpu);
    printf("program counter: %x\n", cpu->pc);

    free_cpu(cpu);

    return 0;
}