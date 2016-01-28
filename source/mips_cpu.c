/*  mips_cpu.cpp
 *  Library for a MIPS software simulator, implementing the API of mips.h
 */

#include "mips_cpu.h"
#include "mips_core.h"
#include "mips_cpu_functions.h"

struct mips_cpu_impl {
    uint32_t pc;
    uint32_t nPC;
    uint32_t regs[32];
    uint32_t reg_HI;
    uint32_t reg_LO;
    mips_mem_h mem;
};


mips_cpu_h mips_cpu_create(mips_mem_h mem) {
    //mips_cpu_h cpu = new mips_cpu_impl;
    mips_cpu_h cpu = (struct mips_cpu_impl*)malloc(sizeof(struct mips_cpu_impl));
    cpu->pc = 0;
    cpu->nPC = 0;
    cpu->reg_HI = 0;
    cpu->reg_LO = 0;
    for (int i = 0; i < 32; i++) {
        cpu->regs[i] = 0;
    }
    cpu->mem = mem;
    return cpu;
}


mips_error mips_cpu_reset(mips_cpu_h state) {
    state->pc = 0;
    state->nPC = 0;
    state->reg_HI = 0;
    state->reg_LO = 0;
    for (int i = 0; i < 32; i++) {
        state->regs[i] = 0;
    }
    return mips_Success;
}


mips_error mips_cpu_get_register(
	mips_cpu_h state,	//!< Valid (non-empty) handle to a CPU
	unsigned index,		//!< Index from 0 to 31
	uint32_t *value		//!< Where to write the value to
){
    *value = state->regs[index];
    return mips_Success;
}


mips_error mips_cpu_set_register(
	mips_cpu_h state,	//!< Valid (non-empty) handle to a CPU
	unsigned index,		//!< Index from 0 to 31
	uint32_t value		//!< New value to write into register file
){
    state->regs[index] = value;
    if (state->regs[0] != 0)
        state->regs[0] = 0;
    return mips_Success;
}


mips_error mips_cpu_set_pc(
	mips_cpu_h state,	//!< Valid (non-empty) handle to a CPU
	uint32_t pc			//!< Address of the next instruction to execute.
){
    state->pc = pc;
    state->nPC = pc+4;
    return mips_Success;
}


mips_error mips_cpu_get_pc(
	mips_cpu_h state,	//!< Valid (non-empty) handle to a CPU
	uint32_t *pc		//!< Where to write the byte address too
){
    *pc = state->pc;
    return mips_Success;
}


void mips_cpu_free(
    mips_cpu_h state	//!< Valid (non-empty) handle to a CPU
){
     free(state);
}

mips_error mips_cpu_set_debug_level(mips_cpu_h state, unsigned level, FILE *dest) {
    return mips_Success;
}


mips_error mips_cpu_step(
	mips_cpu_h state	//! Valid (non-empty) handle to a CPU
){
    ///Need to fetch, decode and execute instruction here.
    ///FETCH
    uint8_t buffer[4];
    uint32_t instr, opcode;
    uint32_t addrOffset;
    mips_error err = mips_mem_read(state->mem, state->pc, 4, buffer);

    if (err != mips_Success) {
        return err;
    }

    instr = mips_cpu_to_big(buffer);

    ///DECODE
    char instr_type;
    opcode = instr >> 26;

    if (opcode == 0)
        instr_type = 'r';
    else if (opcode == 2 || opcode == 3)
        instr_type = 'j';
    else
        instr_type = 'i';

    if (instr_type == 'r') {
        ///decode registers to be used
        int s1, s2, dest, shift;
        s1 = ((instr >> 21) & 0x1F);
        s2 = ((instr >> 16) & 0x1F);
        dest = ((instr >> 11) & 0x1F);
        shift = ((instr >> 6) & 0x1F);
        //decode last 6 bits for function
        uint32_t func = (instr & 0x3F);

        //check if func is not a shift (sll, sra, srl) and shift is not 0, then return error
        if ((func != 0/*sll*/) && (func != 2/*srl*/) && (func != 3/*sra*/) && (shift != 0))
            return mips_ExceptionInvalidInstruction;

        switch (func) {
        case 0x20:  //add
            err = mips_cpu_f_add(&(state->regs[dest]), state->regs[s1], state->regs[s2]);
            if (err != mips_Success)
                return err;
            break;
        case 0x21:	//addu
            state->regs[dest] = mips_cpu_f_addu(state->regs[s1], state->regs[s2]);
            break;
        case 0x24:   //and
            state->regs[dest] = mips_cpu_f_and(state->regs[s1], state->regs[s2]);
            break;
        case 0x8:	//jr
            if (s2 != 0 || dest != 0)
                return mips_ExceptionInvalidInstruction;
            state->pc = state->nPC;
            state->nPC = state->regs[s1];
            return mips_Success;
            break;
        case 0x9:   //jalr
            if (s2 != 0)
                return mips_ExceptionInvalidInstruction;
            if (dest != 0)
                state->regs[dest] = (state->pc)+8;
            state->pc = state->nPC;
            state->nPC = state->regs[s1];
            return mips_Success;
            break;
        case 0x25:  //or
            state->regs[dest] = mips_cpu_f_or(state->regs[s1], state->regs[s2]);
            break;
        case 0x26:  //xor
            state->regs[dest] = mips_cpu_f_xor(state->regs[s1], state->regs[s2]);
            break;
        case 0x22:  //sub
            err = mips_cpu_f_sub(&(state->regs[dest]), state->regs[s1], state->regs[s2]);
            if (err != mips_Success)
                return err;
            break;
        case 0x23:  //subu
            state->regs[dest] = mips_cpu_f_subu(state->regs[s1], state->regs[s2]);
            break;
        case 0x2B:  //sltu
            state->regs[dest] = mips_cpu_f_sltu(state->regs[s1], state->regs[s2]);
            break;
        case 0x2A:  //slt
            state->regs[dest] = mips_cpu_f_slt(state->regs[s1], state->regs[s2]);
            break;
        case 0:  //sll
            if (s1 != 0)
                return mips_ExceptionInvalidInstruction;
            state->regs[dest] = mips_cpu_f_sll(state->regs[s2], shift);
            break;
        case 0x4:  //sllv
            state->regs[dest] = mips_cpu_f_sll(state->regs[s2], state->regs[s1]);
            break;
        case 0x2:  //srl
            if (s1 != 0)
                return mips_ExceptionInvalidInstruction;
            state->regs[dest] = mips_cpu_f_srl(state->regs[s2], shift);
            break;
        case 0x6:  //srlv
            state->regs[dest] = mips_cpu_f_srl(state->regs[s2], state->regs[s1]);
            break;
        case 0x3:  //sra
            if (s1 != 0)
                return mips_ExceptionInvalidInstruction;
            state->regs[dest] = mips_cpu_f_sra(state->regs[s2], shift);
            break;
        case 0x7:  //srav
                if (state->regs[s1] >= 32)
                        return mips_ExceptionInvalidInstruction;
            state->regs[dest] = mips_cpu_f_sra(state->regs[s2], state->regs[s1]);
            break;
        case 0x19:  //multu
            if (dest != 0)
                return mips_ExceptionInvalidInstruction;
            err = mips_cpu_f_multu(state->regs[s1], state->regs[s2], &(state->reg_HI), &(state->reg_LO));
            if (err != mips_Success)
                return err;
            break;
        case 0x18:  //mult
            if (dest != 0)
                return mips_ExceptionInvalidInstruction;
            err = mips_cpu_f_mult((int32_t)state->regs[s1], (int32_t)state->regs[s2], &(state->reg_HI), &(state->reg_LO));
            if (err != mips_Success)
                return err;
            break;
        case 0x10:  //mfhi
            if (s1 != 0 || s2 != 0)
                return mips_ExceptionInvalidInstruction;
            state->regs[dest] = state->reg_HI;
            break;
        case 0x12:  //mflo
            if (s1 != 0 || s2 != 0)
                return mips_ExceptionInvalidInstruction;
            state->regs[dest] = state->reg_LO;
            break;
        case 0x11:  //mthi
            if (s2 != 0 || dest != 0)
                return mips_ExceptionInvalidInstruction;
            state->reg_HI = state->regs[s1];
            break;
        case 0x13:  //mtlo
            if (s2 != 0 || dest != 0)
                return mips_ExceptionInvalidInstruction;
            state->reg_LO = state->regs[s1];
            break;
        case 0x1B:  //divu
            if (dest != 0)
                return mips_ExceptionInvalidInstruction;
            err = mips_cpu_f_divu(state->regs[s1], state->regs[s2], &(state->reg_HI), &(state->reg_LO));
            if (err != mips_Success)
                return err;
            break;
        case 0x1A:  //div
            if (dest != 0)
                return mips_ExceptionInvalidInstruction;
            err = mips_cpu_f_div(state->regs[s1], state->regs[s2], &(state->reg_HI), &(state->reg_LO));
            if (err != mips_Success)
                return err;
            break;
        default:
            return mips_ExceptionInvalidInstruction;
            break;
        }
        if (state->regs[0] != 0)
            state->regs[0] = 0;
        mips_cpu_advance_pc(&(state->pc), &(state->nPC), 4);
        return mips_Success;
    }
    else if (instr_type == 'j') {
        uint32_t addr = (instr & 0x3FFFFFF);
        switch (opcode) {
        case 2:   //j
            state->pc = state->nPC;
            state->nPC = ((state->nPC) & 0xF0000000) | (addr << 2);
            return mips_Success;
            break;
        case 3:   //jal
            state->regs[31] = state->pc+8;
            state->pc = state->nPC;
            state->nPC = ((state->nPC) & 0xF0000000) | (addr << 2);
            return mips_Success;
            break;
        default:
            return mips_ExceptionInvalidInstruction;
            break;
        }
        mips_cpu_advance_pc(&(state->pc),&(state->nPC), 4);
        return mips_Success;
    }
    else if (instr_type == 'i') {
        int16_t data = (instr & 0xFFFF);
        int s1, s2;
        s1 = ((instr >> 21) & 0x1F);
        s2 = ((instr >> 16) & 0x1F);
        switch (opcode) {
        case 0x9:   //addiu
            state->regs[s2] = mips_cpu_f_addu(state->regs[s1], (int32_t) data);
            break;
        case 0xC:   //andi
            state->regs[s2] = mips_cpu_f_and(state->regs[s1], (uint16_t) data);
            break;
        case 0xD:   //ori
            state->regs[s2] = mips_cpu_f_or(state->regs[s1], (uint16_t) data);
            break;
        case 0xE:   //xori
            state->regs[s2] = mips_cpu_f_xor(state->regs[s1], (uint16_t) data);
            break;
        case 0xB:   //sltiu
            state->regs[s2] = mips_cpu_f_sltu(state->regs[s1], (int32_t) data);
            break;
        case 0xA:   //slti
            state->regs[s2] = mips_cpu_f_slt(state->regs[s1], (int32_t) data);
            break;
        case 0x8:   //addi
            err = mips_cpu_f_add(&(state->regs[s2]), state->regs[s1], (int32_t) data);
            if (err != mips_Success)
                return err;
            break;
        case 0x23:  //lw
            err = mips_mem_read(state->mem, state->regs[s1] + data, 4, buffer);
            if (err != mips_Success)
                return err;
            state->regs[s2] = mips_cpu_to_big(buffer);
        case 0x2B:  //sw
            mips_cpu_to_buffer(state->regs[s2], buffer);
            err = mips_mem_write(state->mem, state->regs[s1] + data, 4, buffer);
            if (err != mips_Success)
                return err;
            break;
        case 0xF:   //lui
            state->regs[s2] = (((uint32_t)data) << 16);
            break;

        case 0x24:  //lbu
            addrOffset = (state->regs[s1] + data) % 4;
            err = mips_mem_read(state->mem, state->regs[s1] + data - addrOffset, 4, buffer);
            if (err != mips_Success)
                return err;
            switch (addrOffset) {
            case 0:
                state->regs[s2] = ((mips_cpu_to_big(buffer)>>24) & 0xFF);
                break;
            case 1:
                state->regs[s2] = ((mips_cpu_to_big(buffer)>>16) & 0xFF);
                break;
            case 2:
                state->regs[s2] = ((mips_cpu_to_big(buffer)>>8) & 0xFF);
                break;
            case 3:
                state->regs[s2] = ((mips_cpu_to_big(buffer)>>0) & 0xFF);
                break;
            }
            break;

        case 0x20:  //lb
            addrOffset = (state->regs[s1] + data) % 4;
            err = mips_mem_read(state->mem, state->regs[s1] + data - addrOffset, 4, buffer);
            if (err != mips_Success)
                return err;
            switch (addrOffset) {
            case 0:
                state->regs[s2] = ((mips_cpu_to_big(buffer)>>24) & 0xFF);
                break;
            case 1:
                state->regs[s2] = ((mips_cpu_to_big(buffer)>>16) & 0xFF);
                break;
            case 2:
                state->regs[s2] = ((mips_cpu_to_big(buffer)>>8) & 0xFF);
                break;
            case 3:
                state->regs[s2] = ((mips_cpu_to_big(buffer)>>0) & 0xFF);
                break;
            }
            break;

        case 0x25:  //lhu
            addrOffset = (state->regs[s1] + data) % 4;
            if (addrOffset == 0 || addrOffset == 2) {
                err = mips_mem_read(state->mem, state->regs[s1] + data - addrOffset, 4, buffer);
                if (err != mips_Success)
                    return err;
                if (addrOffset == 0) {
                    state->regs[s2] = ((mips_cpu_to_big(buffer)>>16) & 0xFFFF);
                }
                else {
                    state->regs[s2] = ((mips_cpu_to_big(buffer)) & 0xFFFF);
                }
            }
            else
                return mips_ExceptionInvalidAlignment;
            break;

        case 0x21:  //lh
            addrOffset = (state->regs[s1] + data) % 4;
            if (addrOffset == 0 || addrOffset == 2) {
                err = mips_mem_read(state->mem, state->regs[s1] + data - addrOffset, 4, buffer);
                if (err != mips_Success)
                    return err;
                if (addrOffset == 0) {
                    state->regs[s2] = (int16_t)(((mips_cpu_to_big(buffer)>>16) & 0xFFFF));
                }
                else {
                    state->regs[s2] = (int16_t)(((mips_cpu_to_big(buffer)) & 0xFFFF));
                }
            }
            else
                return mips_ExceptionInvalidAlignment;
            break;

        case 0x28:  //sb
            addrOffset = (state->regs[s1] + data) % 4;
            err = mips_mem_read(state->mem, state->regs[s1] + data - addrOffset, 4, buffer);
            if (err != mips_Success)
                return err;
            switch (addrOffset) {
            case 0:
                state->regs[s2] = ((state->regs[s2] & 0xFF)<<24) | (mips_cpu_to_big(buffer) & (0xFFFFFF));
                mips_cpu_to_buffer(state->regs[s2], buffer);
                err = mips_mem_write(state->mem, state->regs[s1] + data - addrOffset, 4, buffer);
                if (err != mips_Success)
                    return err;
                break;
            case 1:
                state->regs[s2] = ((state->regs[s2] & 0xFF)<<16) | (mips_cpu_to_big(buffer) & (0xFF00FFFF));
                mips_cpu_to_buffer(state->regs[s2], buffer);
                err = mips_mem_write(state->mem, state->regs[s1] + data - addrOffset, 4, buffer);
                if (err != mips_Success)
                    return err;
                break;
            case 2:
                state->regs[s2] = ((state->regs[s2] & 0xFF)<<8) | (mips_cpu_to_big(buffer) & (0xFFFF00FF));
                mips_cpu_to_buffer(state->regs[s2], buffer);
                err = mips_mem_write(state->mem, state->regs[s1] + data - addrOffset, 4, buffer);
                if (err != mips_Success)
                    return err;
                break;
            case 3:
                state->regs[s2] = ((state->regs[s2] & 0xFF)<<0) | (mips_cpu_to_big(buffer) & (0xFFFFFF00));
                mips_cpu_to_buffer(state->regs[s2], buffer);
                err = mips_mem_write(state->mem, state->regs[s1] + data - addrOffset, 4, buffer);
                if (err != mips_Success)
                    return err;
                break;
            }

            break;

        case 0x29:  //sh
            addrOffset = (state->regs[s1] + data) % 4;
            if (addrOffset == 0 || addrOffset == 2) {
                err = mips_mem_read(state->mem, state->regs[s1] + data - addrOffset, 4, buffer);
                if (err != mips_Success)
                        return err;
                if (addrOffset == 0) {
                    state->regs[s2] = ((state->regs[s2] & 0xFFFF)<<16) | (mips_cpu_to_big(buffer) & (0x00FFFF));
                    mips_cpu_to_buffer(state->regs[s2], buffer);
                    err = mips_mem_write(state->mem, state->regs[s1] + data - addrOffset, 4, buffer);
                    if (err != mips_Success)
                        return err;
                }
                else {
                    state->regs[s2] = ((state->regs[s2] & 0xFFFF)<<0) | (mips_cpu_to_big(buffer) & (0xFFFF0000));
                    mips_cpu_to_buffer(state->regs[s2], buffer);
                    err = mips_mem_write(state->mem, state->regs[s1] + data - addrOffset, 4, buffer);
                    if (err != mips_Success)
                        return err;
                }
            }
            else
                return mips_ExceptionInvalidAlignment;
            break;


        case 0x4:   //beq
            err=mips_cpu_f_branch(((int32_t)(state->regs[s1]) == (int32_t)(state->regs[s2])), &(state->pc), &(state->nPC), data);
            return err;
            break;
        case 0x5:   //bne
            err=mips_cpu_f_branch(((int32_t)(state->regs[s1]) != (int32_t)(state->regs[s2])), &(state->pc), &(state->nPC), data);
            return err;
            break;

        case 0x1:   //bgez, bgezal, bltz and bltzal
            if (s2 == 1) {  //bgez
                err=mips_cpu_f_branch((((int32_t)(state->regs[s1])) >= 0), &(state->pc), &(state->nPC), data);
                return err;
            }
            else if (s2 == 17) {    //bgezal
                err=mips_cpu_f_branchAndLink((((int32_t)(state->regs[s1])) >= 0), &(state->pc), &(state->nPC), data, &(state->regs[31]));
                return err;
            }
            else if (s2 == 0) {  //bltz
                err=mips_cpu_f_branch((((int32_t)(state->regs[s1])) < 0), &(state->pc), &(state->nPC), data);
                return err;
            }
            else if (s2 == 16) {    //bltzal
                err=mips_cpu_f_branchAndLink((((int32_t)(state->regs[s1])) < 0), &(state->pc), &(state->nPC), data, &(state->regs[31]));
                return err;
            }
            else {
                return mips_ExceptionInvalidInstruction;
            }
            break;

        case 0x7:   //bgtz
            if (s2 == 0) {
                err=mips_cpu_f_branch((((int32_t)(state->regs[s1])) > 0), &(state->pc), &(state->nPC), data);
                return err;
            }
            else {
                return mips_ExceptionInvalidInstruction;
            }
            break;
        case 0x6:   //blez
            if (s2 == 0) {
                err=mips_cpu_f_branch((((int32_t)(state->regs[s1])) <=0), &(state->pc), &(state->nPC), data);
                return err;
            }
            else {
                return mips_ExceptionInvalidInstruction;
            }
            break;

        case 0x22:  //lwl
            addrOffset = (state->regs[s1] + data) % 4;
            switch (addrOffset) {
            case 0:
                //load normal word
                err = mips_mem_read(state->mem, state->regs[s1] + data, 4, buffer);
                if (err != mips_Success)
                    return err;
                state->regs[s2] = mips_cpu_to_big(buffer);
            break;
            case 1:
                //load the last 3 of the current address and store in most significant part of reg
                err = mips_mem_read(state->mem, state->regs[s1] + data - addrOffset, 4, buffer);
                if (err != mips_Success)
                    return err;
                state->regs[s2] = (state->regs[s2] & 0xFF) | (((mips_cpu_to_big(buffer)>>8) & 0xFFFFFF)<<8);
                break;
            case 2:
                //load the last 2 of the current address and store in most significant part of reg
                err = mips_mem_read(state->mem, state->regs[s1] + data - addrOffset, 4, buffer);
                if (err != mips_Success)
                    return err;
                state->regs[s2] = (state->regs[s2] & 0xFFFF) | (((mips_cpu_to_big(buffer)) & 0xFFFF)<<16);
                break;
            case 3:
                //load the last byte of the current address and store in most significant part of reg
                err = mips_mem_read(state->mem, state->regs[s1] + data - addrOffset, 4, buffer);
                if (err != mips_Success)
                    return err;
                state->regs[s2] = (state->regs[s2] & 0xFFFFFF) | (((mips_cpu_to_big(buffer)) & 0xFF)<<24);
                break;
            }
            break;

        case 0x26:  //lwr
            addrOffset = (state->regs[s1] + data) % 4;
            switch (addrOffset) {
            case 0:
                //do nothing
            break;
            case 1:
                //load the first byte of the next address and store in least significant part of reg
                err = mips_mem_read(state->mem, state->regs[s1] + data - addrOffset + 4, 4, buffer);
                if (err != mips_Success)
                    return err;
                state->regs[s2] = (state->regs[s2] & 0xFFFFFF00) | ((mips_cpu_to_big(buffer)>>24) & 0xFF);
                break;
            case 2:
                //load the first 2 of the next address and store in least significant part of reg
                err = mips_mem_read(state->mem, state->regs[s1] + data - addrOffset + 4, 4, buffer);
                if (err != mips_Success)
                    return err;
                state->regs[s2] = (state->regs[s2] & 0xFFFF0000) | ((mips_cpu_to_big(buffer)>>16) & 0xFFFF);
                break;
            case 3:
                //load the first 3 bytes of the next address and store in least significant part of reg
                err = mips_mem_read(state->mem, state->regs[s1] + data - addrOffset + 4, 4, buffer);
                if (err != mips_Success)
                    return err;
                state->regs[s2] = (state->regs[s2] & 0xFF000000) | ((mips_cpu_to_big(buffer)>>8) & 0xFFFFFF);
                break;
            }
            break;

        default:
            return mips_ExceptionInvalidInstruction;
            break;
        }
        if (state->regs[0] != 0)
            state->regs[0] = 0;
        mips_cpu_advance_pc(&(state->pc), &(state->nPC), 4);
        return mips_Success;
    }

    return mips_ExceptionInvalidInstruction;
}
