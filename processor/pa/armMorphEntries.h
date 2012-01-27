/*
 * Copyright (c) 2005-2011 Imperas Software Ltd., www.imperas.com
 *
 * YOUR ACCESS TO THE INFORMATION IN THIS MODEL IS CONDITIONAL
 * UPON YOUR ACCEPTANCE THAT YOU WILL NOT USE OR PERMIT OTHERS
 * TO USE THE INFORMATION FOR THE PURPOSES OF DETERMINING WHETHER
 * IMPLEMENTATIONS OF THE ARM ARCHITECTURE INFRINGE ANY THIRD
 * PARTY PATENTS.
 *
 * THE LICENSE BELOW EXTENDS ONLY TO USE OF THE SOFTWARE FOR
 * MODELING PURPOSES AND SHALL NOT BE CONSTRUED AS GRANTING
 * A LICENSE TO CREATE A HARDWARE IMPLEMENTATION OF THE
 * FUNCTIONALITY OF THE SOFTWARE LICENSED HEREUNDER.
 * YOU MAY USE THE SOFTWARE SUBJECT TO THE LICENSE TERMS BELOW
 * PROVIDED THAT YOU ENSURE THAT THIS NOTICE IS REPLICATED UNMODIFIED
 * AND IN ITS ENTIRETY IN ALL DISTRIBUTIONS OF THE SOFTWARE,
 * MODIFIED OR UNMODIFIED, IN SOURCE CODE OR IN BINARY FORM.
 *
 * Licensed under an Imperas Modfied Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.ovpworld.org/licenses/OVP_MODIFIED_1.0_APACHE_OPEN_SOURCE_LICENSE_2.0.pdf
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied.
 *
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef ARM_MORPH_ENTRIES_H
#define ARM_MORPH_ENTRIES_H

// model header files
#include "armVariant.h"


////////////////////////////////////////////////////////////////////////////////
// NORMAL INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Morpher attributes for an instruction with a single variant
//
#define MORPH_SINGLE(_NAME) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_NAME}

//
// Morpher attributes for ARM instructions like ADC
//
#define MORPH_SET_ADC(_NAME, _OP, _FLAGS_RW, _FLAGS_R, _COUT) \
    [ARM_IT_##_NAME##_IMM]         = {morphCB:armEmitBinopI,   interwork:ARM_IW_ARM_V7, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM]          = {morphCB:armEmitBinopR,   interwork:ARM_IW_ARM_V7, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM_SHFT_IMM] = {morphCB:armEmitBinopRSI, interwork:ARM_IW_ARM_V7, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM_SHFT_RS]  = {morphCB:armEmitBinopRSR, interwork:ARM_IW_ARM_V7, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM_RRX]      = {morphCB:armEmitBinopRX,  interwork:ARM_IW_ARM_V7, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_IT]          = {morphCB:armEmitBinopIT,  interwork:ARM_IW_ARM_V7, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RT]          = {morphCB:armEmitBinopRT,  interwork:ARM_IW_ARM_V7, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}

//
// Morpher attributes for ARM instructions like MOV
//
#define MORPH_SET_MOV(_NAME, _OP, _FLAGS_RW, _FLAGS_R, _COUT) \
    [ARM_IT_##_NAME##_IMM]         = {morphCB:armEmitUnopI,    interwork:ARM_IW_ARM_V7, unop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM]          = {morphCB:armEmitUnopReg,  interwork:ARM_IW_ARM_V7, unop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM_SHFT_IMM] = {morphCB:armEmitUnopRSI,  interwork:ARM_IW_ARM_V7, unop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM_SHFT_RS]  = {morphCB:armEmitUnopRSR,  interwork:ARM_IW_ARM_V7, unop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM_RRX]      = {morphCB:armEmitUnopRX,   interwork:ARM_IW_ARM_V7, unop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM_SHFT_RST] = {morphCB:armEmitUnopRSRT, interwork:ARM_IW_ARM_V7, unop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}

//
// Morpher attributes for ARM instructions like MOVT
//
#define MORPH_SET_MOVT(_NAME) \
    [ARM_IT_##_NAME] = {morphCB:armEmitUnopIT}

//
// Morpher attributes for ARM instructions like MLA
//
#define MORPH_SET_MLA(_NAME, _FLAGS_RW) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_NAME, flagsRW:_FLAGS_RW}

//
// Morpher attributes for ARM instructions like SMULL
//
#define MORPH_SET_SMULL(_NAME, _OP1, _OP2, _FLAGS_RW) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_OP1, binop:_OP2, flagsRW:_FLAGS_RW}

//
// Morpher attributes for ARM instructions like CMN
//
#define MORPH_SET_CMN(_NAME, _OP, _FLAGS_RW, _FLAGS_R, _COUT) \
    [ARM_IT_##_NAME##_IMM]         = {morphCB:armEmitCmpopI,   binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM]          = {morphCB:armEmitCmpopR,   binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM_SHFT_IMM] = {morphCB:armEmitCmpopRSI, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM_SHFT_RS]  = {morphCB:armEmitCmpopRSR, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}, \
    [ARM_IT_##_NAME##_RM_RRX]      = {morphCB:armEmitCmpopRX,  binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}

//
// Morpher attributes for ARM instructions like B
//
#define MORPH_SET_B(_NAME, _IS_LINK) \
    [ARM_IT_##_NAME] = {morphCB:armEmitBranchC, condJump:True, isLink:_IS_LINK}

//
// Morpher attributes for ARM instructions like BLX (1)
//
#define MORPH_SET_BLX1(_NAME, _IS_LINK) \
    [ARM_IT_##_NAME] = {morphCB:armEmitBLX, isLink:_IS_LINK}

//
// Morpher attributes for ARM instructions like BLX (2)
//
#define MORPH_SET_BLX2(_NAME, _IS_LINK) \
    [ARM_IT_##_NAME] = {morphCB:armEmitBranchR, isLink:_IS_LINK}

//
// Morpher attributes for MORAC instructions like BAA
//
#define MORPH_SET_BAA(_NAME, _IS_LINK) \
    [ARM_IT_##_NAME] = {morphCB:armEmitBAA, isLink:_IS_LINK}

//
// Morpher attributes for ARM instructions like LDR
//
#define MORPH_SET_LDR(_NAME, _OP) \
    [ARM_IT_##_NAME##_IMM]         = {morphCB:armEmit##_OP##I,   interwork:ARM_IW_L4}, \
    [ARM_IT_##_NAME##_RM]          = {morphCB:armEmit##_OP##R,   interwork:ARM_IW_L4}, \
    [ARM_IT_##_NAME##_RM_SHFT_IMM] = {morphCB:armEmit##_OP##RSI, interwork:ARM_IW_L4}, \
    [ARM_IT_##_NAME##_RM_RRX]      = {morphCB:armEmit##_OP##RX,  interwork:ARM_IW_L4}

//
// Morpher attributes for ARM instructions like LDM (1)
//
#define MORPH_SET_LDM1(_NAME) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_NAME, interwork:ARM_IW_L4}

//
// Morpher attributes for ARM instructions like LDRH
//
#define MORPH_SET_LDRH(_NAME, _OP) \
    [ARM_IT_##_NAME##_IMM] = {morphCB:armEmit##_OP##I}, \
    [ARM_IT_##_NAME##_RM]  = {morphCB:armEmit##_OP##R}

//
// Morpher attributes for ARM instructions like SWP
//
#define MORPH_SET_SWP(_NAME, _OP) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_OP}

//
// Morpher attributes for ARM instructions like LDC
//
#define MORPH_SET_LDC(_NAME, _OP) \
    [ARM_IT_##_NAME##_IMM]       = {morphCB:armEmit##_OP}, \
    [ARM_IT_##_NAME##_UNINDEXED] = {morphCB:armEmit##_OP}

//
// Morpher attributes for ARM instructions like MSR
//
#define MORPH_SET_MSR(_NAME) \
    [ARM_IT_##_NAME##_IMM] = {morphCB:armEmit##_NAME##I}, \
    [ARM_IT_##_NAME##_RM]  = {morphCB:armEmit##_NAME##R}

//
// Morpher attributes for ARM instructions like NOP
//
#define MORPH_SET_NOP(_NAME) \
    [ARM_IT_##_NAME] = {morphCB:armEmitNOP}

//
// Morpher attributes for ARM instructions like SMLA<x><y>
//
#define MORPH_SET_SMLA_XY(_NAME) \
    [ARM_IT_##_NAME##BB] = {morphCB:armEmit##_NAME##BB}, \
    [ARM_IT_##_NAME##BT] = {morphCB:armEmit##_NAME##BT}, \
    [ARM_IT_##_NAME##TB] = {morphCB:armEmit##_NAME##TB}, \
    [ARM_IT_##_NAME##TT] = {morphCB:armEmit##_NAME##TT}

//
// Morpher attributes for ARM instructions like SMLAW<y>
//
#define MORPH_SET_SMLAW_Y(_NAME) \
    [ARM_IT_##_NAME##B] = {morphCB:armEmit##_NAME##B}, \
    [ARM_IT_##_NAME##T] = {morphCB:armEmit##_NAME##T}

//
// Morpher attributes for parallel add/subtract instructions
//
#define MORPH_SET_PAS(_NAME, _OP1, _OP2, _SEXTEND, _SETGE, _HALVE) \
    [ARM_IT_##_NAME##ADD16] = {morphCB:armEmitParallelBinop16, binop:_OP1, binop2:_OP1, exchange:0, sextend:_SEXTEND, setGE:_SETGE, halve:_HALVE}, \
    [ARM_IT_##_NAME##ASX]   = {morphCB:armEmitParallelBinop16, binop:_OP2, binop2:_OP1, exchange:1, sextend:_SEXTEND, setGE:_SETGE, halve:_HALVE}, \
    [ARM_IT_##_NAME##SAX]   = {morphCB:armEmitParallelBinop16, binop:_OP1, binop2:_OP2, exchange:1, sextend:_SEXTEND, setGE:_SETGE, halve:_HALVE}, \
    [ARM_IT_##_NAME##SUB16] = {morphCB:armEmitParallelBinop16, binop:_OP2, binop2:_OP2, exchange:0, sextend:_SEXTEND, setGE:_SETGE, halve:_HALVE}, \
    [ARM_IT_##_NAME##ADD8]  = {morphCB:armEmitParallelBinop8,  binop:_OP1,              exchange:0, sextend:_SEXTEND, setGE:_SETGE, halve:_HALVE}, \
    [ARM_IT_##_NAME##SUB8]  = {morphCB:armEmitParallelBinop8,  binop:_OP2,              exchange:0, sextend:_SEXTEND, setGE:_SETGE, halve:_HALVE}

//
// Morpher attributes for signed multiply instructions with optional argument exchange
//
#define MORPH_SET_MEDIA_X(_NAME, _OP1, _OP2) \
    [ARM_IT_##_NAME]    = {morphCB:armEmit##_OP1, binop:_OP2, exchange:0}, \
    [ARM_IT_##_NAME##X] = {morphCB:armEmit##_OP1, binop:_OP2, exchange:1}

//
// Morpher attributes for signed multiply instructions with optional rounding
//
#define MORPH_SET_MEDIA_R(_NAME, _OP1, _OP2, _ACC) \
    [ARM_IT_##_NAME]    = {morphCB:armEmit##_OP1, binop:_OP2, round:0, accumulate:_ACC}, \
    [ARM_IT_##_NAME##R] = {morphCB:armEmit##_OP1, binop:_OP2, round:1, accumulate:_ACC}

//
// Morpher attributes for ARM instructions like PLD
//
#define MORPH_SET_PLD(_NAME) \
    [ARM_IT_##_NAME##_IMM]         = {morphCB:armEmitNOP}, \
    [ARM_IT_##_NAME##_RM]          = {morphCB:armEmitNOP}, \
    [ARM_IT_##_NAME##_RM_SHFT_IMM] = {morphCB:armEmitNOP}, \
    [ARM_IT_##_NAME##_RM_RRX]      = {morphCB:armEmitNOP}


////////////////////////////////////////////////////////////////////////////////
// THUMB INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Morpher attributes for Thumb instructions like ADD (4)
//
#define MORPH_SET_ADD4(_NAME, _OP, _FLAGS_RW, _FLAGS_R, _COUT) \
    [ARM_IT_##_NAME] = {morphCB:armEmitBinopRT, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}

//
// Morpher attributes for Thumb instructions like ADD (6)
//
#define MORPH_SET_ADD6(_NAME, _OP, _FLAGS_RW, _FLAGS_R, _COUT) \
    [ARM_IT_##_NAME] = {morphCB:armEmitBinopI, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}

//
// Morpher attributes for Thumb instructions like ADD (7)
//
#define MORPH_SET_ADD7(_NAME, _OP, _FLAGS_RW, _FLAGS_R, _COUT) \
    [ARM_IT_##_NAME] = {morphCB:armEmitBinopIT, binop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}

//
// Morpher attributes for Thumb instructions like MOV (3)
//
#define MORPH_SET_MOV3(_NAME, _OP, _FLAGS_RW, _FLAGS_R, _COUT) \
    [ARM_IT_##_NAME] = {morphCB:armEmitUnopReg, unop:_OP, flagsRW:_FLAGS_RW, flagsR:_FLAGS_R, shiftCOut:_COUT}

//
// Morpher attributes for Thumb instructions like ADR
//
#define MORPH_SET_ADR(_NAME, _NEGATE) \
    [ARM_IT_##_NAME] = {morphCB:armEmitBinopADR, binop:vmi_ADD, negate:_NEGATE}

//
// Morpher attributes for Thumb instructions like CBZ
//
#define MORPH_SET_CBZ(_NAME, _JUMP_IF_TRUE) \
    [ARM_IT_##_NAME] = {morphCB:armEmitCBZ, jumpIfTrue:_JUMP_IF_TRUE}

//
// Morpher attributes for Thumb instructions like SDIV
//
#define MORPH_SET_SDIV(_NAME, _OP) \
    [ARM_IT_##_NAME] = {morphCB:armEmitDIV, binop:_OP}

//
// Morpher attributes for ARM instructions like HB
//
#define MORPH_SET_HB(_NAME, _OP, _IS_LINK) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_OP, isLink:_IS_LINK}

//
// Morpher attributes for Single entry SIMD instructions
//
#define MORPH_SET_SINGLE_SIMD(_NAME) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_NAME, iType:ARM_TY_SIMD}

//
// Morpher attributes for Single entry VFP instructions
//
#define MORPH_SET_SINGLE_VFP(_NAME) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_NAME, iType:ARM_TY_VFP}

//
// Morpher attributes for SIMD instructions with Q and D versions
//
#define MORPH_SET_SIMD_QD(_NAME, _FUNC) \
    [ARM_IT_##_NAME##_Q] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, ebytes:4, regs:2, elements:2, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, ebytes:4, regs:1, elements:2, shape:ASDS_NORMAL}

//
// Morpher attributes for SIMD instruction with ebytes B, H or W and Q and D versions
//
#define MORPH_SET_SIMD_QD_BHW(_NAME) \
    [ARM_IT_##_NAME##_W_D] = {morphCB:armEmit##_NAME, iType:ARM_TY_SIMD, ebytes:4, regs:1, elements:2, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_H_D] = {morphCB:armEmit##_NAME, iType:ARM_TY_SIMD, ebytes:2, regs:1, elements:4, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_B_D] = {morphCB:armEmit##_NAME, iType:ARM_TY_SIMD, ebytes:1, regs:1, elements:8, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_W_Q] = {morphCB:armEmit##_NAME, iType:ARM_TY_SIMD, ebytes:4, regs:2, elements:2, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_H_Q] = {morphCB:armEmit##_NAME, iType:ARM_TY_SIMD, ebytes:2, regs:2, elements:4, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_B_Q] = {morphCB:armEmit##_NAME, iType:ARM_TY_SIMD, ebytes:1, regs:2, elements:8, shape:ASDS_NORMAL}

//
// Morpher attributes for SIMD instruction with ebytes B, H, W or D and Q and D versions
//
#define MORPH_SET_SIMD_QD_BHWD(_NAME) \
	MORPH_SET_SIMD_QD_BHW(_NAME), \
    [ARM_IT_##_NAME##_D_D] = {morphCB:armEmit##_NAME, iType:ARM_TY_SIMD, ebytes:8, regs:1, elements:1, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_D_Q] = {morphCB:armEmit##_NAME, iType:ARM_TY_SIMD, ebytes:8, regs:2, elements:1, shape:ASDS_NORMAL}

//
// Morpher attributes for SIMD instruction with ebytes B, H or W
//
#define MORPH_SET_SIMD_BHW(_NAME, _FUNC) \
    [ARM_IT_##_NAME##_W] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, ebytes:4, regs:1, elements:2, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_H] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, ebytes:2, regs:1, elements:4, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_B] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, ebytes:1, regs:1, elements:8, shape:ASDS_NORMAL}

//
// Morpher attributes for SIMD instruction with ebytes B, H, W or D
//
#define MORPH_SET_SIMD_BHWD(_NAME, _FUNC) \
    [ARM_IT_##_NAME##_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, ebytes:8, elements:1, shape:ASDS_NORMAL}, \
    MORPH_SET_SIMD_BHW(_NAME, _FUNC)

//
// Morpher attributes for SIMD instruction with ebytes B only and Q and D versions
//
#define MORPH_SET_SIMD_QD_B(_NAME, _FUNC) \
    [ARM_IT_##_NAME##_Q] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, ebytes:1, regs:2, elements:8, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, ebytes:1, regs:1, elements:8, shape:ASDS_NORMAL}

//
// Morpher attributes for SIMD instruction with ebytes B only
//
#define MORPH_SET_SIMD_B(_NAME, _FUNC) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, ebytes:1, regs:1, elements:8, shape:ASDS_NORMAL}

//
// Morpher attributes for SIMD instructions doing binary op, Q/D versions
// Sign, Shape (normal, long, wide, narrow or highhalf) and satOnNarrow options
//
#define MORPH_SET_SIMD_I_QD(_NAME, _FUNC, _OP, _SEXTEND, _RND, _ACC, _HIGH, _SAT, _USS, _SHAPE) \
    [ARM_IT_##_NAME##_Q] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, binop:_OP, ebytes:8, regs:2, elements:1, sextend:_SEXTEND, round:_RND, accumulate:_ACC, highhalf:_HIGH, satOnNarrow:_SAT, ussat:_USS, shape:_SHAPE}, \
    [ARM_IT_##_NAME##_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, binop:_OP, ebytes:8, regs:1, elements:1, sextend:_SEXTEND, round:_RND, accumulate:_ACC, highhalf:_HIGH, satOnNarrow:_SAT, ussat:_USS, shape:_SHAPE}

//
// Morpher attributes for SIMD instructions doing binary op, H/W/D versions (Narrow)
//
#define MORPH_SET_SIMD_I_HWD(_NAME, _FUNC, _OP, _SEXTEND, _RND, _ACC, _HIGH, _SAT, _USS, _SHAPE) \
    [ARM_IT_##_NAME##_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, binop:_OP, ebytes:4, regs:1, elements:2, sextend:_SEXTEND, round:_RND, accumulate:_ACC, highhalf:_HIGH, satOnNarrow:_SAT, ussat:_USS, shape:_SHAPE}, \
    [ARM_IT_##_NAME##_W] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, binop:_OP, ebytes:2, regs:1, elements:4, sextend:_SEXTEND, round:_RND, accumulate:_ACC, highhalf:_HIGH, satOnNarrow:_SAT, ussat:_USS, shape:_SHAPE}, \
    [ARM_IT_##_NAME##_H] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, binop:_OP, ebytes:1, regs:1, elements:8, sextend:_SEXTEND, round:_RND, accumulate:_ACC, highhalf:_HIGH, satOnNarrow:_SAT, ussat:_USS, shape:_SHAPE}

//
// Morpher attributes for SIMD instructions doing binary op, H/W versions
//
#define MORPH_SET_SIMD_I_HW(_NAME, _FUNC, _OP, _SEXTEND, _RND, _ACC, _HIGH, _SAT, _USS, _SHAPE) \
    [ARM_IT_##_NAME##_W] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, binop:_OP, ebytes:4, regs:1, elements:2, sextend:_SEXTEND, round:_RND, accumulate:_ACC, highhalf:_HIGH, satOnNarrow:_SAT, ussat:_USS, shape:_SHAPE}, \
    [ARM_IT_##_NAME##_H] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, binop:_OP, ebytes:2, regs:1, elements:4, sextend:_SEXTEND, round:_RND, accumulate:_ACC, highhalf:_HIGH, satOnNarrow:_SAT, ussat:_USS, shape:_SHAPE}

//
// Morpher attributes for SIMD instructions doing binary op, B/H/W versions
//
#define MORPH_SET_SIMD_I_BHW(_NAME, _FUNC, _OP, _SEXTEND, _RND, _ACC, _HIGH, _SAT, _USS, _SHAPE) \
	MORPH_SET_SIMD_I_HW(_NAME, _FUNC, _OP, _SEXTEND, _RND, _ACC, _HIGH, _SAT, _USS, _SHAPE), \
    [ARM_IT_##_NAME##_B] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, binop:_OP, ebytes:1, regs:1, elements:8, sextend:_SEXTEND, round:_RND, accumulate:_ACC, highhalf:_HIGH, satOnNarrow:_SAT, ussat:_USS, shape:_SHAPE}

//
// Morpher attributes for SIMD instructions doing binary op, Q/D H/W versions
//
#define MORPH_SET_SIMD_I_QD_HW(_NAME, _FUNC, _OP, _SEXTEND, _RND, _ACC, _HIGH, _SAT, _USS, _SHAPE) \
    [ARM_IT_##_NAME##_W_Q] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, binop:_OP, ebytes:4, regs:2, elements:2, sextend:_SEXTEND, round:_RND, accumulate:_ACC, highhalf:_HIGH, satOnNarrow:_SAT, ussat:_USS, shape:_SHAPE}, \
    [ARM_IT_##_NAME##_H_Q] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, binop:_OP, ebytes:2, regs:2, elements:4, sextend:_SEXTEND, round:_RND, accumulate:_ACC, highhalf:_HIGH, satOnNarrow:_SAT, ussat:_USS, shape:_SHAPE}, \
    [ARM_IT_##_NAME##_W_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, binop:_OP, ebytes:4, regs:1, elements:2, sextend:_SEXTEND, round:_RND, accumulate:_ACC, highhalf:_HIGH, satOnNarrow:_SAT, ussat:_USS, shape:_SHAPE}, \
    [ARM_IT_##_NAME##_H_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, binop:_OP, ebytes:2, regs:1, elements:4, sextend:_SEXTEND, round:_RND, accumulate:_ACC, highhalf:_HIGH, satOnNarrow:_SAT, ussat:_USS, shape:_SHAPE}

//
// Morpher attributes for SIMD instructions doing binary op, Q/D B/H/W versions
//
#define MORPH_SET_SIMD_I_QD_BHW(_NAME, _FUNC, _OP, _SEXTEND, _RND, _ACC, _HIGH, _SAT, _USS, _SHAPE) \
	MORPH_SET_SIMD_I_QD_HW(_NAME, _FUNC, _OP, _SEXTEND, _RND, _ACC, _HIGH, _SAT, _USS, _SHAPE), \
    [ARM_IT_##_NAME##_B_Q] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, binop:_OP, ebytes:1, regs:2, elements:8, sextend:_SEXTEND, round:_RND, accumulate:_ACC, highhalf:_HIGH, satOnNarrow:_SAT, ussat:_USS, shape:_SHAPE}, \
    [ARM_IT_##_NAME##_B_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, binop:_OP, ebytes:1, regs:1, elements:8, sextend:_SEXTEND, round:_RND, accumulate:_ACC, highhalf:_HIGH, satOnNarrow:_SAT, ussat:_USS, shape:_SHAPE}

//
// Morpher attributes for SIMD instructions doing binary op, Q/D B/H/WD versions
// Sign, Shape (normal, long, wide, narrow or highhalf) and satOnNarrow options
//
#define MORPH_SET_SIMD_I_QD_BHWD(_NAME, _FUNC, _OP, _SEXTEND, _RND, _ACC, _HIGH, _SAT, _USS, _SHAPE) \
	MORPH_SET_SIMD_I_QD_BHW(_NAME, _FUNC, _OP, _SEXTEND, _RND, _ACC, _HIGH, _SAT, _USS, _SHAPE), \
    [ARM_IT_##_NAME##_D_Q] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, binop:_OP, ebytes:8, regs:2, elements:1, sextend:_SEXTEND, round:_RND, accumulate:_ACC, highhalf:_HIGH, satOnNarrow:_SAT, ussat:_USS, shape:_SHAPE}, \
    [ARM_IT_##_NAME##_D_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, binop:_OP, ebytes:8, regs:1, elements:1, sextend:_SEXTEND, round:_RND, accumulate:_ACC, highhalf:_HIGH, satOnNarrow:_SAT, ussat:_USS, shape:_SHAPE}

//
// Morpher attributes for SIMD instruction doing binary op, One register and a modified immediate, Q/D versions
//
#define MORPH_SET_SIMD_I_RI_QD(_NAME, _FUNC, _OP, _NEG) \
    [ARM_IT_##_NAME##_Q] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, ebytes:4, regs:2, elements:2, binop:_OP, negate:_NEG, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, ebytes:4, regs:1, elements:2, binop:_OP, negate:_NEG, shape:ASDS_NORMAL}

//
// Morpher attributes for SIMD instruction with Two registers and a unary op -  Q/D versions
//
#define MORPH_SET_SIMD_I_U_QD(_NAME, _FUNC, _OP, _SHAPE) \
    [ARM_IT_##_NAME##_Q] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, ebytes:4, regs:2, elements:2, unop:_OP, shape:_SHAPE}, \
    [ARM_IT_##_NAME##_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, ebytes:4, regs:1, elements:2, unop:_OP, shape:_SHAPE}

//
// Morpher attributes for SIMD instructions doing unary op, Q/D B/H/W versions
//
#define MORPH_SET_SIMD_I_U_QD_BHW(_NAME, _FUNC, _OP, _SHAPE) \
    [ARM_IT_##_NAME##_W_Q] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, unop:_OP, ebytes:4, regs:2, elements:2, shape:_SHAPE}, \
    [ARM_IT_##_NAME##_H_Q] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, unop:_OP, ebytes:2, regs:2, elements:4, shape:_SHAPE}, \
    [ARM_IT_##_NAME##_B_Q] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, unop:_OP, ebytes:1, regs:2, elements:8, shape:_SHAPE}, \
    [ARM_IT_##_NAME##_W_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, unop:_OP, ebytes:4, regs:1, elements:2, shape:_SHAPE}, \
    [ARM_IT_##_NAME##_H_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, unop:_OP, ebytes:2, regs:1, elements:4, shape:_SHAPE}, \
    [ARM_IT_##_NAME##_B_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, unop:_OP, ebytes:1, regs:1, elements:8, shape:_SHAPE}

//
// Morpher attributes for SIMD instructions doing unary op, H/W/D versions (Narrow)
//
#define MORPH_SET_SIMD_I_U_HWD(_NAME, _FUNC, _OP, _SEXTEND, _SAT, _USS, _SHAPE) \
    [ARM_IT_##_NAME##_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, unop:_OP, ebytes:4, regs:1, elements:2, shape:_SHAPE, sextend:_SEXTEND, satOnNarrow:_SAT, ussat:_USS}, \
    [ARM_IT_##_NAME##_W] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, unop:_OP, ebytes:2, regs:1, elements:4, shape:_SHAPE, sextend:_SEXTEND, satOnNarrow:_SAT, ussat:_USS}, \
    [ARM_IT_##_NAME##_H] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, unop:_OP, ebytes:1, regs:1, elements:8, shape:_SHAPE, sextend:_SEXTEND, satOnNarrow:_SAT, ussat:_USS}

//
// Morpher attributes for SIMD VCVT instructions
//
#define MORPH_SET_SIMD_VCVT(_NAME, _FUNC, _SEXTEND) \
    [ARM_IT_##_NAME##_Q] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, ebytes:4, regs:2, elements:2, shape:ASDS_NORMAL, sextend:_SEXTEND}, \
    [ARM_IT_##_NAME##_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, ebytes:4, regs:1, elements:2, shape:ASDS_NORMAL, sextend:_SEXTEND}

//
// Morpher attributes for SIMD VCVT single to half precision instruction
//
#define MORPH_SET_SIMD_VCVT_HF(_NAME, _FUNC) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, ebytes:2, regs:1, elements:4, shape:ASDS_NARROW}

//
// Morpher attributes for SIMD VCVT half to single precision instruction
//
#define MORPH_SET_SIMD_VCVT_FH(_NAME, _FUNC) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, ebytes:2, regs:1, elements:4, shape:ASDS_LONG}

//
// Morpher attributes for SIMD VZIP and VUZP, Q B/H/W, D B/H versions
//
#define MORPH_SET_VZIP(_NAME) \
    [ARM_IT_##_NAME##_W_Q] = {morphCB:armEmit##_NAME, iType:ARM_TY_SIMD, ebytes:4, regs:2, elements:2}, \
    [ARM_IT_##_NAME##_H_Q] = {morphCB:armEmit##_NAME, iType:ARM_TY_SIMD, ebytes:2, regs:2, elements:4}, \
    [ARM_IT_##_NAME##_B_Q] = {morphCB:armEmit##_NAME, iType:ARM_TY_SIMD, ebytes:1, regs:2, elements:8}, \
    [ARM_IT_##_NAME##_H_D] = {morphCB:armEmit##_NAME, iType:ARM_TY_SIMD, ebytes:2, regs:1, elements:4}, \
    [ARM_IT_##_NAME##_B_D] = {morphCB:armEmit##_NAME, iType:ARM_TY_SIMD, ebytes:1, regs:1, elements:8}

//
// Morpher attributes for SIMD instructions doing 32-bit vector binary floating point op, Q and D versions
//
#define MORPH_SET_SIMD_F_QD(_NAME, _FUNC, _OP) \
    [ARM_IT_##_NAME##_Q] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, fbinop:_OP, ebytes:4, regs:2, elements:2, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, fbinop:_OP, ebytes:4, regs:1, elements:2, shape:ASDS_NORMAL}

//
// Morpher attributes for SIMD instructions doing 32-bit vector binary floating point op, Q and D versions
//
#define MORPH_SET_SIMD_F_D(_NAME, _FUNC, _OP) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, fbinop:_OP, ebytes:4, regs:1, elements:2, shape:ASDS_NORMAL}

//
// Morpher attributes for SIMD instructions doing 32-bit vector unary floating point op, Q and D versions
//
#define MORPH_SET_SIMD_F_U_QD(_NAME, _FUNC, _OP) \
    [ARM_IT_##_NAME##_Q] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, funop:_OP, ebytes:4, regs:2, elements:2, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, funop:_OP, ebytes:4, regs:1, elements:2, shape:ASDS_NORMAL}

//
// Morpher attributes for SIMD instructions doing 32-bit vector ternary floating point op, Q and D versions
//
#define MORPH_SET_SIMD_F_T_QD(_NAME, _FUNC, _OP) \
    [ARM_IT_##_NAME##_Q] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, fternop:_OP, ebytes:4, regs:2, elements:2, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, fternop:_OP, ebytes:4, regs:1, elements:2, shape:ASDS_NORMAL}

//
// Morpher attributes for SIMD instructions doing 32-bit vector floating point compare, Q and D versions
//
#define MORPH_SET_VCMP_F_QD(_NAME, _FUNC, _QNAN, _COND) \
    [ARM_IT_##_NAME##_Q] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, allowQNaN:_QNAN, fpRelation:_COND, ebytes:4, regs:2, elements:2, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, allowQNaN:_QNAN, fpRelation:_COND, ebytes:4, regs:1, elements:2, shape:ASDS_NORMAL}

//
// Morpher attributes for SIMD instructions doing 32-bit vector floating point compare, D versions
//
#define MORPH_SET_VCMP_F_D(_NAME, _FUNC, _QNAN, _COND) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, allowQNaN:_QNAN, fpRelation:_COND, ebytes:4, regs:1, elements:2, shape:ASDS_NORMAL}

//
// Morpher attributes for SIMD instructions doing Vector comparisons, Q/D B/H/W versions
//
#define MORPH_SET_VCMP_QD_BHW(_NAME, _FUNC, _COND) \
    [ARM_IT_##_NAME##_W_Q] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, cond:_COND, ebytes:4, regs:2, elements:2}, \
    [ARM_IT_##_NAME##_H_Q] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, cond:_COND, ebytes:2, regs:2, elements:4}, \
    [ARM_IT_##_NAME##_B_Q] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, cond:_COND, ebytes:1, regs:2, elements:8}, \
    [ARM_IT_##_NAME##_W_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, cond:_COND, ebytes:4, regs:1, elements:2}, \
    [ARM_IT_##_NAME##_H_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, cond:_COND, ebytes:2, regs:1, elements:4}, \
    [ARM_IT_##_NAME##_B_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, cond:_COND, ebytes:1, regs:1, elements:8}

//
// Morpher attributes for SIMD instructions doing Vector comparisons, B/H/W versions
//
#define MORPH_SET_VCMP_BHW(_NAME, _FUNC, _COND) \
    [ARM_IT_##_NAME##_W] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, cond:_COND, ebytes:4, regs:1, elements:2}, \
    [ARM_IT_##_NAME##_H] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, cond:_COND, ebytes:2, regs:1, elements:4}, \
    [ARM_IT_##_NAME##_B] = {morphCB:armEmit##_FUNC, iType:ARM_TY_SIMD, cond:_COND, ebytes:1, regs:1, elements:8}

//
// Morpher attributes for SIMD VST1/VLD1 instruction with multiple single elements
//
#define MORPH_SET_SIMD_LDST1(_NAME) \
	MORPH_SET_SIMD_BHWD(_NAME##1_R1, _NAME), \
	MORPH_SET_SIMD_BHWD(_NAME##1_R2, _NAME), \
	MORPH_SET_SIMD_BHWD(_NAME##1_R3, _NAME), \
	MORPH_SET_SIMD_BHWD(_NAME##1_R4, _NAME)

//
// Morpher attributes for SIMD VSTn/VLDn instruction with multiple single elements for n = 2, 3 and 4
//
#define MORPH_SET_SIMD_LDST234(_NAME) \
	MORPH_SET_SIMD_BHW(_NAME##2_R1I1, _NAME), \
	MORPH_SET_SIMD_BHW(_NAME##2_R1I2, _NAME), \
	MORPH_SET_SIMD_BHW(_NAME##2_R2I2, _NAME), \
	MORPH_SET_SIMD_BHW(_NAME##3_R3I1, _NAME), \
	MORPH_SET_SIMD_BHW(_NAME##3_R3I2, _NAME), \
	MORPH_SET_SIMD_BHW(_NAME##4_R4I1, _NAME), \
	MORPH_SET_SIMD_BHW(_NAME##4_R4I2, _NAME)

//
// Morpher attributes for SIMD Vector Shift Left (Register) instructions
//
#define MORPH_SET_SIMD_VSH_R(_NAME, _OP, _OP2, _SEXTEND, _SAT) \
    [ARM_IT_##_NAME##_D_D] = {morphCB:armEmitVShiftReg, iType:ARM_TY_SIMD, binop:_OP, binop2:_OP2, ebytes:8, regs:1, elements:1, sextend:_SEXTEND, satOnNarrow:_SAT, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_W_D] = {morphCB:armEmitVShiftReg, iType:ARM_TY_SIMD, binop:_OP, binop2:_OP2, ebytes:4, regs:1, elements:2, sextend:_SEXTEND, satOnNarrow:_SAT, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_H_D] = {morphCB:armEmitVShiftReg, iType:ARM_TY_SIMD, binop:_OP, binop2:_OP2, ebytes:2, regs:1, elements:4, sextend:_SEXTEND, satOnNarrow:_SAT, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_B_D] = {morphCB:armEmitVShiftReg, iType:ARM_TY_SIMD, binop:_OP, binop2:_OP2, ebytes:1, regs:1, elements:8, sextend:_SEXTEND, satOnNarrow:_SAT, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_D_Q] = {morphCB:armEmitVShiftReg, iType:ARM_TY_SIMD, binop:_OP, binop2:_OP2, ebytes:8, regs:2, elements:1, sextend:_SEXTEND, satOnNarrow:_SAT, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_W_Q] = {morphCB:armEmitVShiftReg, iType:ARM_TY_SIMD, binop:_OP, binop2:_OP2, ebytes:4, regs:2, elements:2, sextend:_SEXTEND, satOnNarrow:_SAT, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_H_Q] = {morphCB:armEmitVShiftReg, iType:ARM_TY_SIMD, binop:_OP, binop2:_OP2, ebytes:2, regs:2, elements:4, sextend:_SEXTEND, satOnNarrow:_SAT, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##_B_Q] = {morphCB:armEmitVShiftReg, iType:ARM_TY_SIMD, binop:_OP, binop2:_OP2, ebytes:1, regs:2, elements:8, sextend:_SEXTEND, satOnNarrow:_SAT, shape:ASDS_NORMAL}

//
// Morpher attributes for SIMD VREV instruction
//
#define MORPH_SET_SIMD_VREV(_NAME) \
    [ARM_IT_##_NAME##16_B_Q] = {morphCB:armEmitVREV, iType:ARM_TY_SIMD, groupSize:2, ebytes:1, regs:2, elements:8, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##16_B_D] = {morphCB:armEmitVREV, iType:ARM_TY_SIMD, groupSize:2, ebytes:1, regs:1, elements:8, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##32_B_Q] = {morphCB:armEmitVREV, iType:ARM_TY_SIMD, groupSize:4, ebytes:1, regs:2, elements:8, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##32_B_D] = {morphCB:armEmitVREV, iType:ARM_TY_SIMD, groupSize:4, ebytes:1, regs:1, elements:8, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##32_H_Q] = {morphCB:armEmitVREV, iType:ARM_TY_SIMD, groupSize:2, ebytes:2, regs:2, elements:4, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##32_H_D] = {morphCB:armEmitVREV, iType:ARM_TY_SIMD, groupSize:2, ebytes:2, regs:1, elements:4, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##64_B_Q] = {morphCB:armEmitVREV, iType:ARM_TY_SIMD, groupSize:8, ebytes:1, regs:2, elements:8, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##64_B_D] = {morphCB:armEmitVREV, iType:ARM_TY_SIMD, groupSize:8, ebytes:1, regs:1, elements:8, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##64_H_Q] = {morphCB:armEmitVREV, iType:ARM_TY_SIMD, groupSize:4, ebytes:2, regs:2, elements:4, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##64_H_D] = {morphCB:armEmitVREV, iType:ARM_TY_SIMD, groupSize:4, ebytes:2, regs:1, elements:4, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##64_W_Q] = {morphCB:armEmitVREV, iType:ARM_TY_SIMD, groupSize:2, ebytes:4, regs:2, elements:2, shape:ASDS_NORMAL}, \
    [ARM_IT_##_NAME##64_W_D] = {morphCB:armEmitVREV, iType:ARM_TY_SIMD, groupSize:2, ebytes:4, regs:1, elements:2, shape:ASDS_NORMAL}

//
// Morpher attributes for VMOV Z, R instruction (W version is not AdvSIMD)
//
#define MORPH_SET_VMOVZR_BHW(_NAME) \
    [ARM_IT_##_NAME##_W] = {morphCB:armEmitVMOVZR, iType:ARM_TY_VFP, ebytes:4, regs:1, elements:2}, \
    [ARM_IT_##_NAME##_H] = {morphCB:armEmitVMOVZR, iType:ARM_TY_SIMD, ebytes:2, regs:1, elements:4}, \
    [ARM_IT_##_NAME##_B] = {morphCB:armEmitVMOVZR, iType:ARM_TY_SIMD, ebytes:1, regs:1, elements:8}

//
// Morpher attributes for VMOV R, Z instruction (Signed and unsigned B and H, no sign for W, W not advSIMD)
//
#define MORPH_SET_VMOVRZ_BHW(_NAME) \
    [ARM_IT_##_NAME##_W]  = {morphCB:armEmitVMOVRZ, iType:ARM_TY_VFP, ebytes:4, regs:1, elements:2, sextend:0}, \
    [ARM_IT_##_NAME##U_H] = {morphCB:armEmitVMOVRZ, iType:ARM_TY_SIMD, ebytes:2, regs:1, elements:4, sextend:0}, \
    [ARM_IT_##_NAME##U_B] = {morphCB:armEmitVMOVRZ, iType:ARM_TY_SIMD, ebytes:1, regs:1, elements:8, sextend:0}, \
    [ARM_IT_##_NAME##S_H] = {morphCB:armEmitVMOVRZ, iType:ARM_TY_SIMD, ebytes:2, regs:1, elements:4, sextend:1}, \
    [ARM_IT_##_NAME##S_B] = {morphCB:armEmitVMOVRZ, iType:ARM_TY_SIMD, ebytes:1, regs:1, elements:8, sextend:1}

//
// Morpher attributes for VFP instructions doing single or double precision binary floating point op
//
#define MORPH_SET_VFP_RRR_F_DS(_NAME, _FUNC, _OP, _NEGATE) \
    [ARM_IT_##_NAME##_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_VFP, fbinop:_OP, ebytes:8, negate:_NEGATE}, \
    [ARM_IT_##_NAME##_S] = {morphCB:armEmit##_FUNC, iType:ARM_TY_VFP, fbinop:_OP, ebytes:4, negate:_NEGATE}

//
// Morpher attributes for VFP instructions doing single or double precision unary floating point op
//
#define MORPH_SET_VFP_RR_F_DS(_NAME, _OP) \
    [ARM_IT_##_NAME##_D] = {morphCB:armEmitVFPUnop, iType:ARM_TY_VFP, funop:_OP, ebytes:8}, \
    [ARM_IT_##_NAME##_S] = {morphCB:armEmitVFPUnop, iType:ARM_TY_VFP, funop:_OP, ebytes:4}

//
// Morpher attributes for VFP instructions with D and S versions
//
#define MORPH_SET_VFP_DS(_NAME, _FUNC) \
    [ARM_IT_##_NAME##_S] = {morphCB:armEmit##_FUNC, iType:ARM_TY_VFP, ebytes:4}, \
    [ARM_IT_##_NAME##_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_VFP, ebytes:8}

//
// Morpher attributes for VFP VCMP instructions with D and S versions
//
#define MORPH_SET_VFP_VCMP(_NAME, _FUNC, _QNAN) \
    [ARM_IT_##_NAME##_D] = {morphCB:armEmit##_FUNC, iType:ARM_TY_VFP, ebytes:8, allowQNaN:_QNAN}, \
    [ARM_IT_##_NAME##_S] = {morphCB:armEmit##_FUNC, iType:ARM_TY_VFP, ebytes:4, allowQNaN:_QNAN}

//
// Morpher attributes for VFP VCVT instructions
//
#define MORPH_SET_VFP_VCVT(_NAME, _FUNC, _DSTBYTES, _SRCBYTES, _SIGN, _RND) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_FUNC, iType:ARM_TY_VFP, ebytes:_DSTBYTES, srcBytes:_SRCBYTES, sextend:_SIGN, roundFPSCR:_RND}

//
// Morpher attributes for VFP VCVT half precision instructions which may specify top or bottom half
//
#define MORPH_SET_VFP_VCVT_H(_NAME, _FUNC, _DSTBYTES, _SRCBYTES, _TOP) \
    [ARM_IT_##_NAME] = {morphCB:armEmit##_FUNC, iType:ARM_TY_VFP, ebytes:_DSTBYTES, srcBytes:_SRCBYTES, highhalf:_TOP}

#endif
