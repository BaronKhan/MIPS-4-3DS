/* mips_cpu_functions.cpp
 * Implements the functions defined in mips_cpu_functions.hpp.
 */

#include "mips_cpu.h"
#include "mips_cpu_functions.h"
#include <stdbool.h>

void mips_cpu_advance_pc(uint32_t* pc, uint32_t* nPC, uint32_t offset) {
    *pc = *nPC;
    *nPC += offset;
}

uint32_t mips_cpu_to_big(const uint8_t* pData) {
    return (((uint32_t)pData[0])<<24) | (((uint32_t)pData[1])<<16) | (((uint32_t)pData[2])<<8) | (((uint32_t)pData[3])<<0);
}

void mips_cpu_to_buffer(const uint32_t data, uint8_t* buffer) {
    buffer[0] = (data>>24)&0xFF;
    buffer[1] = (data>>16)&0xFF;
    buffer[2] = (data>>8)&0xFF;
    buffer[3] = (data>>0)&0xFF;
}

mips_error mips_cpu_f_add(uint32_t* result, uint32_t a, uint32_t b) {
    int32_t a2 = (int32_t)a;
    int32_t b2 = (int32_t)b;
    int32_t sum = a2 + b2;
    if ((a2 < 0 && b2 < 0 && sum >= 0) || (a2 > 0 && b2 > 0 && sum <= 0)) {
        return mips_ExceptionArithmeticOverflow;
    }
    else {
        *result = (uint32_t)sum;
        return mips_Success;
    }
}

uint32_t mips_cpu_f_addu(uint32_t a, uint32_t b) {
    return a+b;
}

mips_error mips_cpu_f_sub(uint32_t* result, uint32_t a, uint32_t b) {
    int32_t a2 = (int32_t)a;
    int32_t b2 = (int32_t)b;
    int32_t diff = a2 - b2;
    if ((a2 > 0 && b2 < 0 && diff < 0) || (a2 < 0 && b2 > 0 && diff > 0)) {
        return mips_ExceptionArithmeticOverflow;
    }
    else {
        *result = (uint32_t)diff;
        return mips_Success;
    }
}

uint32_t mips_cpu_f_subu(uint32_t a, uint32_t b) {
    return a-b;
}

uint32_t mips_cpu_f_and(uint32_t a, uint32_t b) {
    return a & b;
}

uint32_t mips_cpu_f_or(uint32_t a, uint32_t b) {
    return a | b;
}

uint32_t mips_cpu_f_xor(uint32_t a, uint32_t b) {
    return a ^ b;
}

uint32_t mips_cpu_f_slt(uint32_t a, uint32_t b) {
    int32_t a2 = a;
    int32_t b2 = b;
    return (a2 < b2) ? 1 : 0;
}

uint32_t mips_cpu_f_sltu(uint32_t a, uint32_t b) {
    return (a < b) ? 1 : 0;
}

uint32_t mips_cpu_f_sll(uint32_t a, uint32_t shift) {
    return (a << shift);
}

uint32_t mips_cpu_f_srl(uint32_t a, uint32_t shift) {
    return (a >> shift);
}

uint32_t mips_cpu_f_sra(uint32_t a, uint32_t shift) {
    if (((a >> 31) & 1) == 0) {
        return a >> shift;
    }
    else {
        uint32_t result = a >> shift;
        for (uint32_t i=0; i < shift; i++) {
            result = result | (0x80000000 >> i);
        }
        return result;
    }
}

mips_error mips_cpu_f_multu(uint32_t a, uint32_t b, uint32_t* reg_HI, uint32_t* reg_LO) {
    uint64_t result = (uint64_t)a*(uint64_t)b;
    *reg_HI = (result) >> 32;
    *reg_LO = (result & 0x00000000FFFFFFFF);
    return mips_Success;
}

mips_error mips_cpu_f_mult(int32_t a, int32_t b, uint32_t* reg_HI, uint32_t* reg_LO) {
    int64_t result = (int64_t)a*(int64_t)b;
    *reg_HI = (result) >> 32;
    *reg_LO = (result & 0x00000000FFFFFFFF);
    return mips_Success;
}

mips_error mips_cpu_f_divu(uint32_t a, uint32_t b, uint32_t* reg_HI, uint32_t* reg_LO) {
    if (b == 0) {
        *reg_HI = 0;    //undefined
        *reg_LO = 0;    //undefined
        return mips_ExceptionInvalidInstruction;
    }
    *reg_HI = a % b;
    *reg_LO = a / b;
    return mips_Success;
}

mips_error mips_cpu_f_div(uint32_t a, uint32_t b, uint32_t* reg_HI, uint32_t* reg_LO) {
    if (b == 0) {
        *reg_HI = 0;    //undefined
        *reg_LO = 0;    //undefined
    }
    int32_t a2 = a;
    int32_t b2 = b;
    *reg_HI = a2 % b2;
    *reg_LO = a2 / b2;
    return mips_Success;
}

mips_error mips_cpu_f_branch(bool test, uint32_t* pc, uint32_t* nPC, int16_t addr) {
    if (test) {
        mips_cpu_advance_pc(pc, nPC, ((uint32_t)addr) << 2);
    }
    else {
        mips_cpu_advance_pc(pc, nPC, 4);
    }
    return mips_Success;
}

mips_error mips_cpu_f_branchAndLink(bool test, uint32_t* pc, uint32_t* nPC, int16_t addr, uint32_t* retAddr) {
    *retAddr = (*pc)+8;
    if (test) {
        mips_cpu_advance_pc(pc, nPC, ((uint32_t)addr) << 2);
    }
    else {
        mips_cpu_advance_pc(pc, nPC, 4);
    }
    return mips_Success;
}
