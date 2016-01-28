/* mips_cpu_functions.hpp
 * Declares the functions for the MIPS CPU.
 */

#ifndef MIPS_CPU_FUNCTIONS_HPP
#define MIPS_CPU_FUNCTIONS_HPP

#include <stdint.h>
#include "mips_cpu.h"
#include <stdbool.h>

void mips_cpu_advance_pc(uint32_t* pc, uint32_t* nPC, uint32_t offset);
uint32_t mips_cpu_to_big(const uint8_t* pData);
void mips_cpu_to_buffer(const uint32_t data, uint8_t* buffer);

mips_error mips_cpu_f_add(uint32_t* result, uint32_t a, uint32_t b);
uint32_t mips_cpu_f_addu(uint32_t a, uint32_t b);
uint32_t mips_cpu_f_and(uint32_t a, uint32_t b);
uint32_t mips_cpu_f_or(uint32_t a, uint32_t b);
mips_error mips_cpu_f_sub(uint32_t* result, uint32_t a, uint32_t b);
uint32_t mips_cpu_f_subu(uint32_t a, uint32_t b);
uint32_t mips_cpu_f_slt(uint32_t a, uint32_t b);
uint32_t mips_cpu_f_sltu(uint32_t a, uint32_t b);
uint32_t mips_cpu_f_xor(uint32_t a, uint32_t b);
uint32_t mips_cpu_f_sll(uint32_t a, uint32_t shift);
uint32_t mips_cpu_f_srl(uint32_t a, uint32_t shift);
uint32_t mips_cpu_f_sra(uint32_t a, uint32_t shift);
mips_error mips_cpu_f_multu(uint32_t a, uint32_t b, uint32_t* reg_HI, uint32_t* reg_LO);
mips_error mips_cpu_f_mult(int32_t a, int32_t b, uint32_t* reg_HI, uint32_t* reg_LO);
mips_error mips_cpu_f_divu(uint32_t a, uint32_t b, uint32_t* reg_HI, uint32_t* reg_LO);
mips_error mips_cpu_f_div(uint32_t a, uint32_t b, uint32_t* reg_HI, uint32_t* reg_LO);
mips_error mips_cpu_f_branch(bool test, uint32_t* pc, uint32_t* nPC, int16_t addr);
mips_error mips_cpu_f_branchAndLink(bool test, uint32_t* pc, uint32_t* nPC, int16_t addr, uint32_t* retAddr);

#endif // MIPS_CPU_FUNCTIONS_HPP
