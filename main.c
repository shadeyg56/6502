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

    free_cpu(cpu);

    return 0;
}