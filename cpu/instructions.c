#include <instructions.h>
#include <stdio.h>

// uint16_t ACC(CPU *cpu) {
//     return cpu->accum
// }

uint16_t IMM(CPU *cpu) {
    return cpu->pc++;   
}

uint16_t ZP(CPU *cpu) {
    uint8_t addr = cpu_fetch_instruction(cpu);

    return addr;
}

uint16_t ZPX(CPU *cpu) {
    uint8_t addr = cpu_fetch_instruction(cpu) + cpu->X;
    return addr;
}

uint16_t ZPY(CPU *cpu) {
    uint8_t addr = cpu_fetch_instruction(cpu) + cpu->Y;
    return addr;
}

uint16_t ABS(CPU *cpu) {
    uint8_t lo = cpu_fetch_instruction(cpu);
    uint8_t hi = cpu_fetch_instruction(cpu);

    return (hi << 8) | lo;
}

uint16_t ABSX(CPU *cpu) {
    return ABS(cpu) + cpu->X;
}

uint16_t ABSY(CPU *cpu) {
    return ABS(cpu) + cpu->Y;
}

uint16_t REL(CPU *cpu) {
    int8_t addr_rel = (int8_t) cpu_fetch_instruction(cpu);

    return cpu->pc + addr_rel;
}

uint16_t IND(CPU *cpu) {
    uint8_t lo_ind = cpu_fetch_instruction(cpu);
    uint8_t hi_ind = cpu_fetch_instruction(cpu);


    uint16_t addr_ind = (lo_ind << 8) | hi_ind;
    uint8_t lo_real = fetch_memory(cpu->mem, addr_ind);
    uint8_t hi_real = fetch_memory(cpu->mem, addr_ind+1);
    return (hi_real << 8) | lo_real;
}

uint16_t INDX(CPU *cpu) {
    uint8_t lo_ind = cpu_fetch_instruction(cpu) + cpu->X;

    uint8_t lo_real = fetch_memory(cpu->mem, lo_ind);
    
    return lo_real;
}

uint16_t INDY(CPU *cpu) {
    uint8_t lo_ind = cpu_fetch_instruction(cpu) + cpu->X;

    uint8_t lo_real = fetch_memory(cpu->mem, lo_ind);
    
    return lo_real;
}

/* CPU Instructions */

void ADC(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, addr);
    uint8_t carry_in = cpu->flags & CPU_STATUS_CARRY;
    cpu->accum += value + carry_in;
    
    set_accum_flags_arith(cpu, value);
}

void AND(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, addr);
    cpu->accum &= value;

    set_zero_negative_flags(cpu, cpu->accum);
}

void ASL(CPU *cpu, uint16_t addr) {
    uint8_t old_value = fetch_memory(cpu->mem, addr);
    uint8_t new_value = old_value * 2;

    if (old_value & (1 << 7)) {
        cpu->flags |= CPU_STATUS_CARRY;
    } else {
        cpu->flags ^= CPU_STATUS_CARRY;
    }

    // Fix flags, add accum mode
}

// Branch if Carry Clear
void BCC(CPU *cpu, uint16_t addr) {
    if (!(cpu->flags & CPU_STATUS_CARRY)) {
        cpu->pc = addr;
    }
}

// Branch if Carry Set
void BCS(CPU *cpu, uint16_t addr) {
    if ((cpu->flags & CPU_STATUS_CARRY)) {
        cpu->pc = addr;
    }
}

// Branch if Equal
void BEQ(CPU *cpu, uint16_t addr) {
    if ((cpu->flags & CPU_STATUS_ZERO)) {
        cpu->pc = addr;
    }
}

// Bit Test
void BIT(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, addr);
    uint8_t result = value & cpu->accum;
    set_zero_negative_flags(cpu, result);
    set_flag(cpu, CPU_STATUS_OVERFLOW, (result & (1 << 6)));
}

// Branch if Minus
void BMI(CPU *cpu, uint16_t addr) {
    if ((cpu->flags & CPU_STATUS_NEGATIVE)) {
        cpu->pc = addr;
    }
}

// Branch if Not Equal
void BNE(CPU *cpu, uint16_t addr) {
    if (!(cpu->flags & CPU_STATUS_ZERO)) {
        cpu->pc = addr;
    }
}

// Branch if Positive
void BPL(CPU *cpu, uint16_t addr) {
    if (!(cpu->flags & CPU_STATUS_NEGATIVE)) {
        cpu->pc = addr;
    }
}

// Force Interrupt
void BRK(CPU *cpu, uint16_t addr) {
    uint8_t pc_lo = cpu->pc & 0xFF;
    uint8_t pc_hi = (cpu->pc >> 8) & 0xFF;
    write_memory(cpu->mem, STACK_PAGE_START_ADDR+cpu->sp++, pc_lo);    
    write_memory(cpu->mem, STACK_PAGE_START_ADDR+cpu->sp++, pc_hi);    
    write_memory(cpu->mem, STACK_PAGE_START_ADDR+cpu->sp++, cpu->flags);    

    uint8_t irq_pc_lo = fetch_memory(cpu->mem, IRQ_VECTOR_LO);
    uint8_t irq_pc_hi = fetch_memory(cpu->mem, IRQ_VECTOR_LO+1);
    cpu->pc = (irq_pc_hi << 8) + irq_pc_lo;

    set_flag(cpu, CPU_STATUS_BREAK, 1);
}

// Branch if Overflow Clear
void BVC(CPU *cpu, uint16_t addr) {
    if (!(cpu->flags & CPU_STATUS_OVERFLOW)) {
        cpu->pc = addr;
    }
}

// Branch if Overflow Set
void BVS(CPU *cpu, uint16_t addr) {
    if ((cpu->flags & CPU_STATUS_OVERFLOW)) {
        cpu->pc = addr;
    }
}

// Clear Carry Flag
void CLC(CPU *cpu, uint16_t addr) {
    set_flag(cpu, CPU_STATUS_CARRY, 0);
}

void CLD(CPU *cpu, uint16_t addr) {
    set_flag(cpu, CPU_STATUS_DECIMAL, 0);
}

void CLI(CPU *cpu, uint16_t addr) {
    set_flag(cpu, CPU_STATUS_INTERRUPT, 0);
}

void CLV(CPU *cpu, uint16_t addr) {
    set_flag(cpu, CPU_STATUS_OVERFLOW, 0);
}

// Compare
void CMP(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, addr);
    set_flag(cpu, CPU_STATUS_CARRY, (cpu->accum >= value));
    set_zero_negative_flags(cpu, cpu->accum - value);
}

// Compare X Reg
void CPX(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, addr);
    set_flag(cpu, CPU_STATUS_CARRY, (cpu->X >= value));
    set_zero_negative_flags(cpu, cpu->X - value);
}

// Compare Y Reg
void CPY(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, addr);
    set_flag(cpu, CPU_STATUS_CARRY, (cpu->Y >= value));
    set_zero_negative_flags(cpu, cpu->Y - value);
}

// Decrement Memory
void DEC(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, addr);
    value -= 1;
    write_memory(cpu->mem, addr, value);

    set_zero_negative_flags(cpu, value);
}

// Decrement X Reg
void DEX(CPU *cpu, uint16_t addr) {
    cpu->X -= 1;

    set_zero_negative_flags(cpu, cpu->X);
}

// Decrement Y Reg
void DEY(CPU *cpu, uint16_t addr) {
    cpu->Y -= 1;

    set_zero_negative_flags(cpu, cpu->Y);
}

// Exclusive OR
void EOR(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, addr);
    cpu->accum ^= value;
    set_zero_negative_flags(cpu, cpu->accum);
}

// Increment Memory
void INC(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, addr);
    value += 1;
    write_memory(cpu->mem, addr, value);

    set_zero_negative_flags(cpu, value);
}

// Increment X Reg
void INX(CPU *cpu, uint16_t addr) {
    cpu->X += 1;

    set_zero_negative_flags(cpu, cpu->X);
}

// Increment Y Reg
void INY(CPU *cpu, uint16_t addr) {
    cpu->Y += 1;

    set_zero_negative_flags(cpu, cpu->Y);
}

// Jump
void JMP(CPU *cpu, uint16_t addr) {
    cpu->pc = addr;

    /* The JMP Instruction on the original 6502 contains a hardware bug. 
    Here's an explanation copied from the 6502 Obelisk
        "An original 6502 has does not correctly fetch the target address if the indirect vector falls on a page boundary
        (e.g. $xxFF where xx is any value from $00 to $FF). In this case fetches the LSB from $xxFF as expected but takes the MSB from $xx00. 
        This is fixed in some later chips like the 65SC02 so for 
        compatibility always ensure the indirect vector is not at the end of the page."

    Will decide later if I want to simulate this bug or not. Not going to for now.        
    */
}

// Jump to Subroutine
void JSR(CPU *cpu, uint16_t addr) {
    uint8_t pc_lo = cpu->pc-1 & 0xFF;
    uint8_t pc_hi = (cpu->pc-1 >> 8) & 0xFF;
    write_memory(cpu->mem, STACK_PAGE_START_ADDR+cpu->sp++, pc_lo);    
    write_memory(cpu->mem, STACK_PAGE_START_ADDR+cpu->sp++, pc_hi);

    cpu->pc = addr;

}

// Load Accumulator
void LDA(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, addr);
    cpu->accum = value;

    set_zero_negative_flags(cpu, cpu->accum);
}

// Load X reg
void LDX(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, addr); 
    cpu->X = value;

    set_zero_negative_flags(cpu, cpu->X);
}

// Load Y reg
void LDY(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, addr); 
    cpu->Y = value;

    set_zero_negative_flags(cpu, cpu->Y);
}

// Logical Shift Right
void LSR(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, addr);
    uint8_t value_shift = value >> 1;

    set_flag(cpu, CPU_STATUS_CARRY, (value & 1));
    set_zero_negative_flags(cpu, value_shift);
}

// No Operation
void NOP(CPU *cpu, uint16_t addr) {

}

// Logical Inclusive OR
void ORA(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, addr); 
    cpu->accum |= value;

    set_zero_negative_flags(cpu, cpu->accum);
}

// Push Accumulator
void PHA(CPU *cpu, uint16_t addr) {
    write_memory(cpu->mem, STACK_PAGE_START_ADDR+cpu->sp++, cpu->accum);
}

// Push Processor State
void PHP(CPU *cpu, uint16_t addr) {
    write_memory(cpu->mem, STACK_PAGE_START_ADDR+cpu->sp++, cpu->flags);
}

// Pull Accumulator
void PLA(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, STACK_PAGE_START_ADDR+cpu->sp--);
    cpu->accum = value;
    set_zero_negative_flags(cpu, cpu->accum);
}

// Pull Processor State
void PLP(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, STACK_PAGE_START_ADDR+cpu->sp--);
    cpu->flags = value;
}

// Rotate Left
void ROL(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, addr);
    uint8_t new_value = value << 1;

    new_value |= (cpu->flags & CPU_STATUS_CARRY);
    write_memory(cpu->mem, addr, new_value);

    set_flag(cpu, CPU_STATUS_CARRY, (value & (1 << 7)));
    set_zero_negative_flags(cpu, new_value);
}

// Rotate right
void ROR(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, addr);
    uint8_t new_value = value >> 1;

    new_value |= ((cpu->flags & CPU_STATUS_CARRY)) << 7;
    write_memory(cpu->mem, addr, new_value);

    set_flag(cpu, CPU_STATUS_CARRY, (value & 1));
    set_zero_negative_flags(cpu, new_value);
}

// Return from Interrupt
void RTI(CPU *cpu, uint16_t addr) {
   uint8_t flags = fetch_memory(cpu->mem, STACK_PAGE_START_ADDR+cpu->sp--); 
   uint8_t pc_hi = fetch_memory(cpu->mem, STACK_PAGE_START_ADDR+cpu->sp--);
   uint8_t pc_lo = fetch_memory(cpu->mem, STACK_PAGE_START_ADDR+cpu->sp--);

   cpu->flags = flags;
   cpu->pc = (pc_hi << 8) + pc_lo;
}

// Return from Subroutine
void RTS(CPU *cpu, uint16_t addr) {
   uint8_t pc_hi = fetch_memory(cpu->mem, STACK_PAGE_START_ADDR+cpu->sp--);
   uint8_t pc_lo = fetch_memory(cpu->mem, STACK_PAGE_START_ADDR+cpu->sp--);

   cpu->pc = (pc_hi << 8) + pc_lo; 
}

// Subtract with Carry
void SBC(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, addr);
    uint8_t carry_in = cpu->flags & CPU_STATUS_CARRY;

    cpu->accum -= value - (1-carry_in);

    set_accum_flags_arith(cpu, value);
}

// Set Carry Flag
void SEC(CPU *cpu, uint16_t addr) {
    set_flag(cpu, CPU_STATUS_CARRY, 1);
}

// Set Decimal Flag
void SED(CPU *cpu, uint16_t addr) {
    set_flag(cpu, CPU_STATUS_DECIMAL, 1);
}

// Set Interrupt Disable
void SEI(CPU *cpu, uint16_t addr) {
    set_flag(cpu, CPU_STATUS_INTERRUPT, 1);
}

// Store Accumulator
void STA(CPU *cpu, uint16_t addr) {
    write_memory(cpu->mem, addr, cpu->accum);
}

// Store X Reg
void STX(CPU *cpu, uint16_t addr) {
    write_memory(cpu->mem, addr, cpu->X);
}

// Store Y Reg
void STY(CPU *cpu, uint16_t addr) {
    write_memory(cpu->mem, addr, cpu->Y);
}

// Transfer Accumulator to X
void TAX(CPU *cpu, uint16_t addr) {
    cpu->X = cpu->accum;
    set_zero_negative_flags(cpu, cpu->X);
}

// Transfer Accumulator to Y
void TAY(CPU *cpu, uint16_t addr) {
    cpu->Y = cpu->accum;
    set_zero_negative_flags(cpu, cpu->Y);
}

// Transfer SP to X
void TSX(CPU *cpu, uint16_t addr) {
    cpu->X = fetch_memory(cpu->mem, STACK_PAGE_START_ADDR-cpu->sp--);
    set_zero_negative_flags(cpu, cpu->X);
}

// Transfer X to Accumulator
void TXA(CPU *cpu, uint16_t addr) {
    cpu->accum = cpu->X;
    set_zero_negative_flags(cpu, cpu->accum);
}

// Transfer X to SP
void TXS(CPU *cpu, uint16_t addr) {
    write_memory(cpu->mem, STACK_PAGE_START_ADDR+cpu->sp++, cpu->X);
}

// Transfer Y to Accumulator
void TYA(CPU *cpu, uint16_t addr) {
    cpu->accum = cpu->Y;
    set_zero_negative_flags(cpu, cpu->accum);
}