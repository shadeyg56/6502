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


    uint16_t addr_ind = (hi_ind << 8) | lo_ind;
    uint8_t lo_real = fetch_memory(cpu->mem, addr_ind);
    uint8_t hi_real = fetch_memory(cpu->mem, addr_ind+1);
    return (hi_real << 8) | lo_real;
}

uint16_t INDX(CPU *cpu) {
    uint8_t lo_ind = cpu_fetch_instruction(cpu) + cpu->X;
    uint8_t hi_ind = lo_ind+1;

    uint8_t lo_real = fetch_memory(cpu->mem, lo_ind);
    uint8_t hi_real = fetch_memory(cpu->mem, hi_ind);
    
    return (hi_real << 8 | lo_real);
}

uint16_t INDY(CPU *cpu) {
    uint8_t lo_ind = cpu_fetch_instruction(cpu);
    uint8_t hi_ind = lo_ind+1;

    uint8_t lo_real = fetch_memory(cpu->mem, lo_ind);
    uint8_t hi_real = fetch_memory(cpu->mem, hi_ind);
    
    return (hi_real << 8 | lo_real) + cpu->Y;
}

/* CPU Instructions */

void ADC(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, addr);
    uint8_t carry_in = cpu->flags & CPU_STATUS_CARRY;
    uint8_t accum_prev = cpu->accum;
    uint8_t result_bin = accum_prev + value + carry_in;

    if (!(cpu->flags & CPU_STATUS_DECIMAL)) {
        cpu->accum = result_bin;
        set_accum_flags_arith(cpu, value, accum_prev);
    } else {
        uint8_t decimal_carry = carry_in;
        uint16_t lo = (accum_prev & 0x0F) + (value & 0x0F) + carry_in;
        if (lo >= 0x0A) {
            lo = ((lo + 0x06) & 0x0F) + 0x10;
        }
        uint8_t intermediate = (accum_prev & 0xF0) + (value & 0xF0) + lo;

        cpu->accum = bcd_add(accum_prev, value, &decimal_carry);

        set_flag(cpu, CPU_STATUS_CARRY, decimal_carry);
        set_flag(cpu, CPU_STATUS_OVERFLOW,
                 (accum_prev ^ intermediate) & ~(accum_prev ^ value) & (1 << 7));
        set_flag(cpu, CPU_STATUS_NEGATIVE, intermediate & (1 << 7));
        set_flag(cpu, CPU_STATUS_ZERO, result_bin == 0);
    }
}

void AND(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, addr);
    cpu->accum &= value;

    set_zero_negative_flags(cpu, cpu->accum);
}

void ASL(CPU *cpu, uint16_t addr) {
    uint8_t old_value = fetch_memory(cpu->mem, addr);
    uint8_t new_value = old_value * 2;
    write_memory(cpu->mem, addr, new_value);

    set_flag(cpu, CPU_STATUS_CARRY, (old_value & (1 << 7)));
    set_zero_negative_flags(cpu, new_value);
}

void ASL_Accum (CPU *cpu, uint16_t addr) {
    uint8_t old_accum = cpu->accum;
    cpu->accum *= 2;

    set_flag(cpu, CPU_STATUS_CARRY, (old_accum & (1 << 7)));

    set_zero_negative_flags(cpu, cpu->accum); 
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
    set_flag(cpu, CPU_STATUS_OVERFLOW, (value & (1 << 6)));
    set_flag(cpu, CPU_STATUS_NEGATIVE, (value & (1 << 7)));
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
    uint8_t pc_lo = (cpu->pc+1) & 0xFF;
    uint8_t pc_hi = ((cpu->pc+1) >> 8) & 0xFF;
    stack_push(cpu, pc_hi);
    stack_push(cpu, pc_lo);
    stack_push(cpu, cpu->flags | CPU_STATUS_BREAK | CPU_STATUS_UNUSED);

    uint8_t irq_pc_lo = fetch_memory(cpu->mem, IRQ_VECTOR_LO);
    uint8_t irq_pc_hi = fetch_memory(cpu->mem, IRQ_VECTOR_LO+1);
    cpu->pc = (irq_pc_hi << 8) + irq_pc_lo;

    set_flag(cpu, CPU_STATUS_INTERRUPT, 1);
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
   //printf("Comparing %d accum to %d\n", cpu->accum, value);
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
    uint8_t pc_lo = (cpu->pc-1) & 0xFF;
    uint8_t pc_hi = ((cpu->pc-1) >> 8) & 0xFF;
    stack_push(cpu, pc_hi);
    stack_push(cpu, pc_lo);

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
    write_memory(cpu->mem, addr, value_shift);

    set_flag(cpu, CPU_STATUS_CARRY, (value & 1));
    set_zero_negative_flags(cpu, value_shift);
}

void LSR_Accum(CPU *cpu, uint16_t addr) {
    uint16_t accum_prev = cpu->accum;
    cpu->accum >>= 1;

    set_flag(cpu, CPU_STATUS_CARRY, (accum_prev & 1));
    set_zero_negative_flags(cpu, cpu->accum);
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
    stack_push(cpu, cpu->accum);
}

// Push Processor State
void PHP(CPU *cpu, uint16_t addr) {
    // Functional test expects break and reserved flags to be set
    // Might reassemble test at some point to disable this
    stack_push(cpu, cpu->flags | CPU_STATUS_BREAK | CPU_STATUS_UNUSED);
}

// Pull Accumulator
void PLA(CPU *cpu, uint16_t addr) {
    uint8_t value = stack_pull(cpu);
    cpu->accum = value;
    set_zero_negative_flags(cpu, cpu->accum);
}

// Pull Processor State
void PLP(CPU *cpu, uint16_t addr) {
    uint8_t value = stack_pull(cpu);
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

void ROL_Accum(CPU *cpu, uint16_t addr) {
    uint8_t prev_accum = cpu->accum;
    cpu->accum <<= 1;

    cpu->accum |= (cpu->flags & CPU_STATUS_CARRY);

    set_flag(cpu, CPU_STATUS_CARRY, (prev_accum & (1 << 7)));
    set_zero_negative_flags(cpu, cpu->accum);
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


void ROR_Accum(CPU *cpu, uint16_t addr) {
    uint8_t prev_accum = cpu->accum;
    cpu->accum >>= 1;

    cpu->accum |= (cpu->flags & CPU_STATUS_CARRY) << 7;

    set_flag(cpu, CPU_STATUS_CARRY, (prev_accum & 1));
    set_zero_negative_flags(cpu, cpu->accum);
}

// Return from Interrupt
void RTI(CPU *cpu, uint16_t addr) {
   uint8_t flags = stack_pull(cpu);
   uint8_t pc_lo = stack_pull(cpu);
   uint8_t pc_hi = stack_pull(cpu);

   cpu->flags = flags;
   cpu->pc = (pc_hi << 8) + pc_lo;
}

// Return from Subroutine
void RTS(CPU *cpu, uint16_t addr) {
   uint8_t pc_lo = stack_pull(cpu);
   uint8_t pc_hi = stack_pull(cpu);

   /* JSR pushed the address of its last operand byte, so step past it. */
   cpu->pc = ((pc_hi << 8) + pc_lo) + 1;
}

// Subtract with Carry
void SBC(CPU *cpu, uint16_t addr) {
    uint8_t value = fetch_memory(cpu->mem, addr);
    uint8_t carry_in = cpu->flags & CPU_STATUS_CARRY;
    uint8_t accum_prev = cpu->accum;
    uint8_t borrow = 1 - carry_in;
    uint8_t result_bin = accum_prev - value - borrow;

    if (!(cpu->flags & CPU_STATUS_DECIMAL)) {
        cpu->accum = result_bin;
    } else {
        uint8_t bcd_borrow = borrow;
        cpu->accum = bcd_subtract(accum_prev, value, &bcd_borrow);
    }

    if (borrow ? (accum_prev > value) : (accum_prev >= value)) {
        cpu->flags |= CPU_STATUS_CARRY;
    } else {
        cpu->flags &= ~CPU_STATUS_CARRY;
    }

    if ((result_bin ^ ~value) & (accum_prev ^ result_bin) & (1 << 7)) {
        cpu->flags |= CPU_STATUS_OVERFLOW;
    } else {
        cpu->flags &= ~CPU_STATUS_OVERFLOW;
    }

    set_zero_negative_flags(cpu, result_bin);

    //printf("Flags: %02x\n", cpu->flags);

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
    cpu->X = cpu->sp;
    set_zero_negative_flags(cpu, cpu->X);
}

// Transfer X to Accumulator
void TXA(CPU *cpu, uint16_t addr) {
    cpu->accum = cpu->X;
    set_zero_negative_flags(cpu, cpu->accum);
}

// Transfer X to SP
void TXS(CPU *cpu, uint16_t addr) {
    // TXS is the only transfer that does not touch the status flags
    cpu->sp = cpu->X;
}

// Transfer Y to Accumulator
void TYA(CPU *cpu, uint16_t addr) {
    cpu->accum = cpu->Y;
    set_zero_negative_flags(cpu, cpu->accum);
}

/* Opcode lookup table. */  
const Instruction INSTRUCTION_TABLE[256] = {
    { 0x00, BRK, NULL },
    { 0x01, ORA, INDX },
    { 0x02, NOP, NULL },
    { 0x03, NOP, NULL },
    { 0x04, NOP, NULL },
    { 0x05, ORA, ZP },
    { 0x06, ASL, ZP },
    { 0x07, NOP, NULL },
    { 0x08, PHP, NULL },
    { 0x09, ORA, IMM },
    { 0x0A, ASL_Accum, NULL }, /* Accumulator op */
    { 0x0B, NOP, NULL },
    { 0x0C, NOP, NULL },
    { 0x0D, ORA, ABS },
    { 0x0E, ASL, ABS },
    { 0x0F, NOP, NULL },
    { 0x10, BPL, REL },
    { 0x11, ORA, INDY },
    { 0x12, NOP, NULL },
    { 0x13, NOP, NULL },
    { 0x14, NOP, NULL },
    { 0x15, ORA, ZPX },
    { 0x16, ASL, ZPX },
    { 0x17, NOP, NULL },
    { 0x18, CLC, NULL },
    { 0x19, ORA, ABSY },
    { 0x1A, NOP, NULL },
    { 0x1B, NOP, NULL },
    { 0x1C, NOP, NULL },
    { 0x1D, ORA, ABSX },
    { 0x1E, ASL, ABSX },
    { 0x1F, NOP, NULL },
    { 0x20, JSR, ABS },
    { 0x21, AND, INDX },
    { 0x22, NOP, NULL },
    { 0x23, NOP, NULL },
    { 0x24, BIT, ZP },
    { 0x25, AND, ZP },
    { 0x26, ROL, ZP },
    { 0x27, NOP, NULL },
    { 0x28, PLP, NULL },
    { 0x29, AND, IMM },
    { 0x2A, ROL_Accum, NULL }, /* Accumulator op */
    { 0x2B, NOP, NULL },
    { 0x2C, BIT, ABS },
    { 0x2D, AND, ABS },
    { 0x2E, ROL, ABS },
    { 0x2F, NOP, NULL },
    { 0x30, BMI, REL },
    { 0x31, AND, INDY },
    { 0x32, NOP, NULL },
    { 0x33, NOP, NULL },
    { 0x34, NOP, NULL },
    { 0x35, AND, ZPX },
    { 0x36, ROL, ZPX },
    { 0x37, NOP, NULL },
    { 0x38, SEC, NULL },
    { 0x39, AND, ABSY },
    { 0x3A, NOP, NULL },
    { 0x3B, NOP, NULL },
    { 0x3C, NOP, NULL },
    { 0x3D, AND, ABSX },
    { 0x3E, ROL, ABSX },
    { 0x3F, NOP, NULL },
    { 0x40, RTI, NULL },
    { 0x41, EOR, INDX },
    { 0x42, NOP, NULL },
    { 0x43, NOP, NULL },
    { 0x44, NOP, NULL },
    { 0x45, EOR, ZP },
    { 0x46, LSR, ZP },
    { 0x47, NOP, NULL },
    { 0x48, PHA, NULL },
    { 0x49, EOR, IMM },
    { 0x4A, LSR_Accum, NULL }, /* Accumulator op */
    { 0x4B, NOP, NULL },
    { 0x4C, JMP, ABS },
    { 0x4D, EOR, ABS },
    { 0x4E, LSR, ABS },
    { 0x4F, NOP, NULL },
    { 0x50, BVC, REL },
    { 0x51, EOR, INDY },
    { 0x52, NOP, NULL },
    { 0x53, NOP, NULL },
    { 0x54, NOP, NULL },
    { 0x55, EOR, ZPX },
    { 0x56, LSR, ZPX },
    { 0x57, NOP, NULL },
    { 0x58, CLI, NULL },
    { 0x59, EOR, ABSY },
    { 0x5A, NOP, NULL },
    { 0x5B, NOP, NULL },
    { 0x5C, NOP, NULL },
    { 0x5D, EOR, ABSX },
    { 0x5E, LSR, ABSX },
    { 0x5F, NOP, NULL },
    { 0x60, RTS, NULL },
    { 0x61, ADC, INDX },
    { 0x62, NOP, NULL },
    { 0x63, NOP, NULL },
    { 0x64, NOP, NULL },
    { 0x65, ADC, ZP },
    { 0x66, ROR, ZP },
    { 0x67, NOP, NULL },
    { 0x68, PLA, NULL },
    { 0x69, ADC, IMM },
    { 0x6A, ROR_Accum, NULL }, /* Accumulator op */
    { 0x6B, NOP, NULL },
    { 0x6C, JMP, IND },
    { 0x6D, ADC, ABS },
    { 0x6E, ROR, ABS },
    { 0x6F, NOP, NULL },
    { 0x70, BVS, REL },
    { 0x71, ADC, INDY },
    { 0x72, NOP, NULL },
    { 0x73, NOP, NULL },
    { 0x74, NOP, NULL },
    { 0x75, ADC, ZPX },
    { 0x76, ROR, ZPX },
    { 0x77, NOP, NULL },
    { 0x78, SEI, NULL },
    { 0x79, ADC, ABSY },
    { 0x7A, NOP, NULL },
    { 0x7B, NOP, NULL },
    { 0x7C, NOP, NULL },
    { 0x7D, ADC, ABSX },
    { 0x7E, ROR, ABSX },
    { 0x7F, NOP, NULL },
    { 0x80, NOP, NULL },
    { 0x81, STA, INDX },
    { 0x82, NOP, NULL },
    { 0x83, NOP, NULL },
    { 0x84, STY, ZP },
    { 0x85, STA, ZP },
    { 0x86, STX, ZP },
    { 0x87, NOP, NULL },
    { 0x88, DEY, NULL },
    { 0x89, NOP, NULL },
    { 0x8A, TXA, NULL },
    { 0x8B, NOP, NULL },
    { 0x8C, STY, ABS },
    { 0x8D, STA, ABS },
    { 0x8E, STX, ABS },
    { 0x8F, NOP, NULL },
    { 0x90, BCC, REL },
    { 0x91, STA, INDY },
    { 0x92, NOP, NULL },
    { 0x93, NOP, NULL },
    { 0x94, STY, ZPX },
    { 0x95, STA, ZPX },
    { 0x96, STX, ZPY },
    { 0x97, NOP, NULL },
    { 0x98, TYA, NULL },
    { 0x99, STA, ABSY },
    { 0x9A, TXS, NULL },
    { 0x9B, NOP, NULL },
    { 0x9C, NOP, NULL },
    { 0x9D, STA, ABSX },
    { 0x9E, NOP, NULL },
    { 0x9F, NOP, NULL },
    { 0xA0, LDY, IMM },
    { 0xA1, LDA, INDX },
    { 0xA2, LDX, IMM },
    { 0xA3, NOP, NULL },
    { 0xA4, LDY, ZP },
    { 0xA5, LDA, ZP },
    { 0xA6, LDX, ZP },
    { 0xA7, NOP, NULL },
    { 0xA8, TAY, NULL },
    { 0xA9, LDA, IMM },
    { 0xAA, TAX, NULL },
    { 0xAB, NOP, NULL },
    { 0xAC, LDY, ABS },
    { 0xAD, LDA, ABS },
    { 0xAE, LDX, ABS },
    { 0xAF, NOP, NULL },
    { 0xB0, BCS, REL },
    { 0xB1, LDA, INDY },
    { 0xB2, NOP, NULL },
    { 0xB3, NOP, NULL },
    { 0xB4, LDY, ZPX },
    { 0xB5, LDA, ZPX },
    { 0xB6, LDX, ZPY },
    { 0xB7, NOP, NULL },
    { 0xB8, CLV, NULL },
    { 0xB9, LDA, ABSY },
    { 0xBA, TSX, NULL },
    { 0xBB, NOP, NULL },
    { 0xBC, LDY, ABSX },
    { 0xBD, LDA, ABSX },
    { 0xBE, LDX, ABSY },
    { 0xBF, NOP, NULL },
    { 0xC0, CPY, IMM },
    { 0xC1, CMP, INDX },
    { 0xC2, NOP, NULL },
    { 0xC3, NOP, NULL },
    { 0xC4, CPY, ZP },
    { 0xC5, CMP, ZP },
    { 0xC6, DEC, ZP },
    { 0xC7, NOP, NULL },
    { 0xC8, INY, NULL },
    { 0xC9, CMP, IMM },
    { 0xCA, DEX, NULL },
    { 0xCB, NOP, NULL },
    { 0xCC, CPY, ABS },
    { 0xCD, CMP, ABS },
    { 0xCE, DEC, ABS },
    { 0xCF, NOP, NULL },
    { 0xD0, BNE, REL },
    { 0xD1, CMP, INDY },
    { 0xD2, NOP, NULL },
    { 0xD3, NOP, NULL },
    { 0xD4, NOP, NULL },
    { 0xD5, CMP, ZPX },
    { 0xD6, DEC, ZPX },
    { 0xD7, NOP, NULL },
    { 0xD8, CLD, NULL },
    { 0xD9, CMP, ABSY },
    { 0xDA, NOP, NULL },
    { 0xDB, NOP, NULL },
    { 0xDC, NOP, NULL },
    { 0xDD, CMP, ABSX },
    { 0xDE, DEC, ABSX },
    { 0xDF, NOP, NULL },
    { 0xE0, CPX, IMM },
    { 0xE1, SBC, INDX },
    { 0xE2, NOP, NULL },
    { 0xE3, NOP, NULL },
    { 0xE4, CPX, ZP },
    { 0xE5, SBC, ZP },
    { 0xE6, INC, ZP },
    { 0xE7, NOP, NULL },
    { 0xE8, INX, NULL },
    { 0xE9, SBC, IMM },
    { 0xEA, NOP, NULL },
    { 0xEB, NOP, NULL },
    { 0xEC, CPX, ABS },
    { 0xED, SBC, ABS },
    { 0xEE, INC, ABS },
    { 0xEF, NOP, NULL },
    { 0xF0, BEQ, REL },
    { 0xF1, SBC, INDY },
    { 0xF2, NOP, NULL },
    { 0xF3, NOP, NULL },
    { 0xF4, NOP, NULL },
    { 0xF5, SBC, ZPX },
    { 0xF6, INC, ZPX },
    { 0xF7, NOP, NULL },
    { 0xF8, SED, NULL },
    { 0xF9, SBC, ABSY },
    { 0xFA, NOP, NULL },
    { 0xFB, NOP, NULL },
    { 0xFC, NOP, NULL },
    { 0xFD, SBC, ABSX },
    { 0xFE, INC, ABSX },
    { 0xFF, NOP, NULL },
};
