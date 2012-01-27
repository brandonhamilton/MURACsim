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

#ifndef ARM_DECODE_ENTRIES_THUMB32_H
#define ARM_DECODE_ENTRIES_THUMB32_H

// VMI header files
#include "vmi/vmiDecode.h"

//
// This macro adds a decode table entry for a 32-bit Thumb instruction class
//
#define DECODE_TT32(_PRIORITY, _NAME, _PATTERN) \
    {type:TT32_##_NAME, priority:_PRIORITY, name:#_NAME"_T", pattern:_PATTERN}

//
// This macro adds an undefined instruction decode table entry for a 32-bit Thumb instruction class
//
#define DECODE_LAST(_PRIORITY, _PATTERN) \
    {type:TT_LAST, priority:_PRIORITY, name:"LAST_T", pattern:_PATTERN}

//
// Decode entries for 32-bit Thumb instructions like AND
//
#define DECODE_SET_32_AND(_NAME, _OP) \
    DECODE_TT32(0, _NAME##_IMM,         "|111|10.0|" _OP "|.|....|0...|....|........"), \
    DECODE_TT32(0, _NAME##_RM_SHFT_IMM, "|111|0101|" _OP "|.|....|....|....|........"), \
    DECODE_TT32(1, _NAME##_RM_RRX,      "|111|0101|" _OP "|.|....|.000|....|0011....")

//
// Decode entries for 32-bit Thumb instructions like TST
//
#define DECODE_SET_32_TST(_NAME, _OP) \
    DECODE_TT32(2, _NAME##_IMM,         "|111|10.0|" _OP "|1|....|0...|1111|........"), \
    DECODE_TT32(2, _NAME##_RM_SHFT_IMM, "|111|0101|" _OP "|1|....|....|1111|........"), \
    DECODE_TT32(3, _NAME##_RM_RRX,      "|111|0101|" _OP "|1|....|.000|1111|0011....")

//
// Decode entries for 32-bit Thumb instructions like MOV
//
#define DECODE_SET_32_MOV(_NAME, _OP) \
    DECODE_TT32(2, _NAME##_IMM,         "|111|10.0|" _OP "|.|1111|0...|....|........"), \
    DECODE_TT32(2, _NAME##_RM_SHFT_IMM, "|111|0101|" _OP "|.|1111|....|....|........"), \
    DECODE_TT32(3, _NAME##_RM_RRX,      "|111|0101|" _OP "|.|1111|.000|....|0011....")

//
// Decode entries for 32-bit Thumb instructions like PKHBT
//
#define DECODE_SET_32_PKHBT(_NAME, _OP) \
    DECODE_TT32(0, _NAME, "|111|01|01|0110|0|....|....|....|.." _OP "0|....")

//
// Decode entries for 32-bit Thumb instructions like ADD (plain binary immediate)
//
#define DECODE_SET_32_ADD_PI(_NAME, _OP) \
    DECODE_TT32(0, _NAME, "|111|10|.1|" _OP "|....|0|...|....|........")

//
// Decode entries for 32-bit Thumb instructions like ADR (plain binary immediate)
//
#define DECODE_SET_32_ADR_PI(_NAME, _OP) \
    DECODE_TT32(1, _NAME, "|111|10|.1|" _OP "|1111|0|...|....|........")

//
// Decode entries for 32-bit Thumb instructions like SSAT16
//
#define DECODE_SET_32_SSAT16(_NAME, _OP) \
    DECODE_TT32(1, _NAME, "|111|10|.1|" _OP "|....|0|000|....|00......")

//
// Decode entries for 32-bit Thumb instructions like LSL
//
#define DECODE_SET_32_LSL(_NAME, _OP1, _OP2, _OP3) \
    DECODE_TT32(0, _NAME, "|111|1101|0|" _OP1 "|" _OP3 "|1111|....|" _OP2 "|....")

//
// Decode entries for 32-bit Thumb instructions like SXTH
//
#define DECODE_SET_32_SXTH(_NAME, _OP1, _OP2, _OP3) \
    DECODE_TT32(1, _NAME, "|111|1101|0|" _OP1 "|" _OP3 "|1111|....|" _OP2 "|....")

//
// Decode entries for parallel add/subtract instructions
//
#define DECODE_SET_32_PAS(_NAME, _OP) \
    DECODE_TT32(0, S##_NAME,  "111|1101|01|" _OP "|....|1111|....|0000|....|"), \
    DECODE_TT32(0, Q##_NAME,  "111|1101|01|" _OP "|....|1111|....|0001|....|"), \
    DECODE_TT32(0, SH##_NAME, "111|1101|01|" _OP "|....|1111|....|0010|....|"), \
    DECODE_TT32(0, U##_NAME,  "111|1101|01|" _OP "|....|1111|....|0100|....|"), \
    DECODE_TT32(0, UQ##_NAME, "111|1101|01|" _OP "|....|1111|....|0101|....|"), \
    DECODE_TT32(0, UH##_NAME, "111|1101|01|" _OP "|....|1111|....|0110|....|")

//
// Decode entries for 32-bit Thumb instructions like MLA
//
#define DECODE_SET_32_MLA(_NAME, _OP1, _OP2) \
    DECODE_TT32(0, _NAME, "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "|....")

//
// Decode entries for 32-bit Thumb instructions like MUL
//
#define DECODE_SET_32_MUL(_NAME, _OP1, _OP2) \
    DECODE_TT32(1, _NAME, "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "|....")

//
// Decode entries for 32-bit Thumb DSP instructions like SMLA<x><y>
//
#define DECODE_SET_32_SMLA_XY(_NAME, _OP1, _OP2) \
    DECODE_TT32(0, _NAME##BB, "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "00|...."), \
    DECODE_TT32(0, _NAME##BT, "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "01|...."), \
    DECODE_TT32(0, _NAME##TB, "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "10|...."), \
    DECODE_TT32(0, _NAME##TT, "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "11|....")

//
// Decode entries for 32-bit Thumb DSP instructions like SMUL<x><y>
//
#define DECODE_SET_32_SMUL_XY(_NAME, _OP1, _OP2) \
    DECODE_TT32(1, _NAME##BB, "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "00|...."), \
    DECODE_TT32(1, _NAME##BT, "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "01|...."), \
    DECODE_TT32(1, _NAME##TB, "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "10|...."), \
    DECODE_TT32(1, _NAME##TT, "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "11|....")

//
// Decode entries for 32-bit Thumb DSP instructions like SMLAD<x>
//
#define DECODE_SET_32_SMLAD(_NAME, _OP1, _OP2) \
    DECODE_TT32(0, _NAME,    "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "0|...."), \
    DECODE_TT32(0, _NAME##X, "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "1|....")

//
// Decode entries for 32-bit Thumb DSP instructions like SMUAD<x>
//
#define DECODE_SET_32_SMUAD(_NAME, _OP1, _OP2) \
    DECODE_TT32(1, _NAME,    "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "0|...."), \
    DECODE_TT32(1, _NAME##X, "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "1|....")

//
// Decode entries for 32-bit Thumb DSP instructions like SMLAW<y>
//
#define DECODE_SET_32_SMLAW(_NAME, _OP1, _OP2) \
    DECODE_TT32(0, _NAME##B, "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "0|...."), \
    DECODE_TT32(0, _NAME##T, "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "1|....")

//
// Decode entries for 32-bit Thumb DSP instructions like SMULW<y>
//
#define DECODE_SET_32_SMULW(_NAME, _OP1, _OP2) \
    DECODE_TT32(1, _NAME##B, "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "0|...."), \
    DECODE_TT32(1, _NAME##T, "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "1|....")

//
// Decode entries for 32-bit Thumb DSP instructions like SMMLA
//
#define DECODE_SET_32_SMMLA(_NAME, _OP1, _OP2) \
    DECODE_TT32(0, _NAME,    "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "0|...."), \
    DECODE_TT32(0, _NAME##R, "|111|1101|1|" _OP1 "|....|....|....|" _OP2 "1|....")

//
// Decode entries for 32-bit Thumb DSP instructions like SMMUL
//
#define DECODE_SET_32_SMMUL(_NAME, _OP1, _OP2) \
    DECODE_TT32(1, _NAME,    "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "0|...."), \
    DECODE_TT32(1, _NAME##R, "|111|1101|1|" _OP1 "|....|1111|....|" _OP2 "1|....")

//
// Decode entries for 32-bit Thumb instructions like BFC
//
#define DECODE_SET_32_BFC(_NAME, _OP) \
    DECODE_TT32(1, _NAME, "|111|10|.1|" _OP "|1111|0|...|....|........")

//
// Decode entries for 32-bit Thumb instructions like B (1)
//
#define DECODE_SET_32_B1(_NAME, _OP1, _OP2) \
    DECODE_TT32(0, _NAME, "|111|10|...........|1|" _OP1 "|..........." _OP2)

//
// Decode entries for 32-bit Thumb undefined instructions
//
#define DECODE_SET_32_UNDEF(_NAME, _OP1, _OP2) \
    DECODE_TT32(1, _NAME, "|111|10|" _OP2 "|....|1|" _OP1 "|....|........")

//
// Decode entries for 32-bit Thumb instructions like MSR
//
#define DECODE_SET_32_MSR(_NAME, _OP1, _OP2) \
    DECODE_TT32(2, _NAME, "|111|10|" _OP2 "|....|1|" _OP1 "|....|........")

//
// Decode entries for 32-bit Thumb hint instructions like NOP
//
#define DECODE_SET_32_HINT1(_NAME, _OP1, _OP2, _OP3) \
    DECODE_TT32(3, _NAME, "|111|10|" _OP2 "|....|1|" _OP1 "|.000|" _OP3)

//
// Decode entries for 32-bit Thumb hint instructions like YIELD
//
#define DECODE_SET_32_HINT2(_NAME, _OP1, _OP2, _OP3) \
    DECODE_TT32(4, _NAME, "|111|10|" _OP2 "|....|1|" _OP1 "|.000|" _OP3)

//
// Decode entries for 32-bit Thumb instructions like CLREX
//
#define DECODE_SET_32_CLREX(_NAME, _OP) \
    DECODE_TT32(2, _NAME, "|111|10|0111011|....|10.0|....|" _OP "|....")

//
// Decode entries for 32-bit Thumb instructions like SRS
//
#define DECODE_SET_32_SRS(_NAME, _OP1, _OP2) \
    DECODE_TT32(1, _NAME, "|111|0100|" _OP1 "|0|" _OP2 "|................")

//
// Decode entries for 32-bit Thumb instructions like POPM
//
#define DECODE_SET_32_POPM(_NAME, _OP1, _OP2) \
    DECODE_TT32(2, _NAME, "|111|0100|" _OP1 "|0|" _OP2 "|................")

//
// Decode entries for 32-bit Thumb instructions like LDRD_IMM
//
#define DECODE_SET_32_LDRD_IMM(_NAME, _OP1, _OP2, _OP3) \
    DECODE_TT32(0, _NAME, "|111|0100|" _OP1 "|1|" _OP2 "|....|........" _OP3 "....")

//
// Decode entries for 32-bit Thumb instructions like LDREX
//
#define DECODE_SET_32_LDREX(_NAME, _OP1, _OP2, _OP3) \
    DECODE_TT32(1, _NAME, "|111|0100|" _OP1 "|1|" _OP2 "|....|........" _OP3 "....")

//
// Decode entries for 32-bit Thumb instructions like LDR
//
#define DECODE_SET_32_LDR(_NAME, _SIGN, _SZ) \
    DECODE_TT32(0, _NAME##_IMM1,        "|111|1100|" _SIGN "1|" _SZ "|1|....|....|......|......"), \
    DECODE_TT32(1, _NAME##_IMM2,        "|111|1100|" _SIGN "0|" _SZ "|1|....|....|1..1..|......"), \
    DECODE_TT32(1, _NAME##_IMM2,        "|111|1100|" _SIGN "0|" _SZ "|1|....|....|1100..|......"), \
    DECODE_TT32(3, _NAME##_IMM3,        "|111|1100|" _SIGN "0|" _SZ "|1|1111|....|......|......"), \
    DECODE_TT32(2, _NAME##_RM,          "|111|1100|" _SIGN "0|" _SZ "|1|....|....|000000|00...."), \
    DECODE_TT32(1, _NAME##_RM_SHFT_IMM, "|111|1100|" _SIGN "0|" _SZ "|1|....|....|000000|......"), \
    DECODE_TT32(1, _NAME##T_IMM,        "|111|1100|" _SIGN "0|" _SZ "|1|....|....|1110..|......")

//
// Decode entries for 32-bit Thumb instructions like STR
//
#define DECODE_SET_32_STR(_NAME, _SIGN, _SZ) \
    DECODE_TT32(0, _NAME##_IMM1,        "|111|1100|" _SIGN "1|" _SZ "|0|....|....|......|......"), \
    DECODE_TT32(1, _NAME##_IMM2,        "|111|1100|" _SIGN "0|" _SZ "|0|....|....|1..1..|......"), \
    DECODE_TT32(1, _NAME##_IMM2,        "|111|1100|" _SIGN "0|" _SZ "|0|....|....|1100..|......"), \
    DECODE_TT32(3, _NAME##_IMM3,        "|111|1100|" _SIGN "0|" _SZ "|0|1111|....|......|......"), \
    DECODE_TT32(2, _NAME##_RM,          "|111|1100|" _SIGN "0|" _SZ "|0|....|....|000000|00...."), \
    DECODE_TT32(1, _NAME##_RM_SHFT_IMM, "|111|1100|" _SIGN "0|" _SZ "|0|....|....|000000|......"), \
    DECODE_TT32(1, _NAME##T_IMM,        "|111|1100|" _SIGN "0|" _SZ "|0|....|....|1110..|......")

//
// Decode entries for 32-bit Thumb instructions like PLD
//
#define DECODE_SET_32_PLD(_NAME, _SIGN, _SZ) \
    DECODE_TT32(5, _NAME##_IMM1,        "|111|1100|" _SIGN "1|" _SZ "|1|....|1111|......|......"), \
    DECODE_TT32(6, _NAME##_IMM2,        "|111|1100|" _SIGN "0|" _SZ "|1|....|1111|1100..|......"), \
    DECODE_TT32(8, _NAME##_IMM3,        "|111|1100|" _SIGN "0|" _SZ "|1|1111|1111|......|......"), \
    DECODE_TT32(7, _NAME##_RM,          "|111|1100|" _SIGN "0|" _SZ "|1|....|1111|000000|00...."), \
    DECODE_TT32(6, _NAME##_RM_SHFT_IMM, "|111|1100|" _SIGN "0|" _SZ "|1|....|1111|000000|......")

//
// Decode entries for 32-bit Thumb instructions like UHINT
//
#define DECODE_SET_32_UHINT(_NAME, _SZ) \
    DECODE_TT32(4, _NAME, "|111|1100|..|" _SZ "|1|....|1111|......|......")

//
// Decode entries for 32-bit Thumb instructions like CDP
//
#define DECODE_SET_32_CDP(_NAME) \
    DECODE_TT32(0, _NAME, "....|1110|....|....|....|....|...|0|....")

//
// Decode entries for 32-bit Thumb instructions like CDP2
//
#define DECODE_SET_32_CDP2(_NAME) \
    DECODE_TT32(1, _NAME, "1111|1110|....|....|....|....|...|0|....")

//
// Decode entries for 32-bit Thumb instructions like LDC
//
#define DECODE_SET_32_LDC(_NAME, _OP) \
    DECODE_TT32(0, _NAME##_IMM,       "....|110|...." _OP "|....|....|....|........"), \
    DECODE_TT32(1, _NAME##_UNINDEXED, "....|110|0..0" _OP "|....|....|....|........")

//
// Decode entries for 32-bit Thumb instructions like LDC2
//
#define DECODE_SET_32_LDC2(_NAME, _OP) \
    DECODE_TT32(2, _NAME##_IMM,       "1111|110|...." _OP "|....|....|....|........"), \
    DECODE_TT32(3, _NAME##_UNINDEXED, "1111|110|0..0" _OP "|....|....|....|........")

//
// Decode entries for 32-bit Thumb instructions like MCR
//
#define DECODE_SET_32_MCR(_NAME, _OP) \
    DECODE_TT32(0, _NAME, "....|1110|...|" _OP "|....|....|....|...|1|....")

//
// Decode entries for 32-bit Thumb instructions like MCR2
//
#define DECODE_SET_32_MCR2(_NAME, _OP) \
    DECODE_TT32(1, _NAME, "1111|1110|...|" _OP "|....|....|....|...|1|....")

//
// Decode entries for 32-bit Thumb DSP instructions like MCRR
//
#define DECODE_SET_32_MCRR(_NAME, _OP) \
    DECODE_TT32(4, _NAME, "....|1100010" _OP "|....|....|....|....|....")

//
// Decode entries for 32-bit Thumb DSP instructions like MCRR2
//
#define DECODE_SET_32_MCRR2(_NAME, _OP) \
    DECODE_TT32(5, _NAME, "1111|1100010" _OP "|....|....|....|....|....")

//
// Decode entries for 32-bit Thumb DSP instructions like ENTERX
//
#define DECODE_SET_32_ENTERX(_NAME, _OP) \
    DECODE_TT32(2, _NAME, "1111|0011|1011|....|10.0|....|000" _OP "....")

//
// Decode entries for 32-bit Thumb SIMD instruction VEXT: 3 regs same length and Q/D versions
//  Undefined Instruction when Q==1 && (Qm<0>==1 || Qn<0>==1 || Qd<0>==1)
//  Undefined instruction when Q==0 && <bit 11>==1
//
#define DECODE_SET_32_VEXT(_NAME) \
    DECODE_LAST(3,            "1110|1111|1.11|....|....|....|...0|...."), \
    DECODE_TT32(4, _NAME##_Q, "1110|1111|1.11|...0|...0|....|.1.0|...0"), \
    DECODE_TT32(4, _NAME##_D, "1110|1111|1.11|....|....|....|.0.0|...."), \
    DECODE_LAST(5,            "1110|1111|1.11|....|....|1...|.0.0|....")

//
// Decode entries for 32-bit Thumb SIMD instruction VTBL and VTBX: 3 regs same length and D version only
//
#define DECODE_SET_32_VTBL(_NAME, _OP) \
    DECODE_TT32(3, _NAME, "1111|1111|1.11|....|....|10..|." _OP ".0|....")

//
// Decode entries for 32-bit Thumb SIMD instruction VDUP: 1 Reg and a scalar: Q,Z or D,Z   8, 16 and 32 bit versions
//  Undefined Instruction when Q==1 && (Qd<0>==1)
//  Undefined instruction when 18:16=000
//
#define DECODE_SET_32_VDUPZ(_NAME) \
    DECODE_LAST(3,              "1111|1111|1.11|....|....|1100|0..0|...."), \
    DECODE_TT32(4, _NAME##_W_Q, "1111|1111|1.11|.100|...0|1100|01.0|...."), \
    DECODE_TT32(4, _NAME##_W_D, "1111|1111|1.11|.100|....|1100|00.0|...."), \
    DECODE_TT32(4, _NAME##_H_Q, "1111|1111|1.11|..10|...0|1100|01.0|...."), \
    DECODE_TT32(4, _NAME##_H_D, "1111|1111|1.11|..10|....|1100|00.0|...."), \
    DECODE_TT32(4, _NAME##_B_Q, "1111|1111|1.11|...1|...0|1100|01.0|...."), \
    DECODE_TT32(4, _NAME##_B_D, "1111|1111|1.11|...1|....|1100|00.0|...."), \
    DECODE_LAST(5,              "1111|1111|1.11|.000|....|1100|0..0|....")

//
// Decode entries for 32-bit Thumb SIMD instructions with 3 regs same length, D version only
//
#define DECODE_SET_32_SIMD_RRR(_NAME, _OPA, _OPB, _OPU, _OPC) \
    DECODE_LAST(2,        "111" _OPU "|1111|0." _OPC "....|...." _OPA "..." _OPB "...."), \
    DECODE_TT32(3, _NAME, "111" _OPU "|1111|0." _OPC "....|...." _OPA ".0." _OPB "....")

//
// Decode entries for 32-bit Thumb SIMD instructions with 3 regs same length and Q/D versions
//
#define DECODE_SET_32_SIMD_RRR_QD(_NAME, _OPA, _OPB, _OPU, _OPC) \
    DECODE_LAST(2,            "111" _OPU "|1111|0." _OPC "....|...." _OPA "..." _OPB "...."), \
    DECODE_TT32(3, _NAME##_Q, "111" _OPU "|1111|0." _OPC "...0|...0" _OPA ".1." _OPB "...0"), \
    DECODE_TT32(3, _NAME##_D, "111" _OPU "|1111|0." _OPC "....|...." _OPA ".0." _OPB "....")

//
// Decode entries for 32-bit Thumb SIMD instructions with 3 regs same length, D version only and esize 8, 16, 32 in bits 21:20
//  Undefined Instruction when size==b11 || Q==1
//
#define DECODE_SET_32_SIMD_RRR_D_BHW(_NAME, _OPA, _OPB, _OPU) \
    DECODE_LAST(2,            "111" _OPU "|1111|0...|....|...." _OPA "..." _OPB "...."), \
    DECODE_TT32(3, _NAME##_W, "111" _OPU "|1111|0.10|....|...." _OPA ".0." _OPB "...."), \
    DECODE_TT32(3, _NAME##_H, "111" _OPU "|1111|0.01|....|...." _OPA ".0." _OPB "...."), \
    DECODE_TT32(3, _NAME##_B, "111" _OPU "|1111|0.00|....|...." _OPA ".0." _OPB "....")

//
// Decode entries for 32-bit Thumb SIMD instructions with 3 regs same length, Q/D versions and esize 8, 16, 32 in bits 21:20
//  Undefined Instruction when size==b11 || (Q==1 && (Qn<0>=1 || Qm<0>==1 || Qd<0>==1))
//
#define DECODE_SET_32_SIMD_RRR_QD_BHW(_NAME, _OPA, _OPB, _OPU) \
    DECODE_LAST(2,              "111" _OPU "|1111|0...|....|...." _OPA "..." _OPB "...."), \
    DECODE_TT32(3, _NAME##_W_Q, "111" _OPU "|1111|0.10|...0|...0" _OPA ".1." _OPB "...0"), \
    DECODE_TT32(3, _NAME##_H_Q, "111" _OPU "|1111|0.01|...0|...0" _OPA ".1." _OPB "...0"), \
    DECODE_TT32(3, _NAME##_B_Q, "111" _OPU "|1111|0.00|...0|...0" _OPA ".1." _OPB "...0"), \
    DECODE_TT32(3, _NAME##_W_D, "111" _OPU "|1111|0.10|....|...." _OPA ".0." _OPB "...."), \
    DECODE_TT32(3, _NAME##_H_D, "111" _OPU "|1111|0.01|....|...." _OPA ".0." _OPB "...."), \
    DECODE_TT32(3, _NAME##_B_D, "111" _OPU "|1111|0.00|....|...." _OPA ".0." _OPB "....")

//
// Decode entries for 32-bit Thumb SIMD instructions with 3 regs same length, Q/D versions and esize 16 or 32 in bits 21:20
//  Undefined Instruction when size==b11 || size==b00 || (Q==1 && (Qn<0>=1 || Qm<0>==1 || Qd<0>==1))
//
#define DECODE_SET_32_SIMD_RRR_QD_HW(_NAME, _OPA, _OPB, _OPU) \
    DECODE_LAST(2,              "111" _OPU "|1111|0...|....|...." _OPA "..." _OPB "...."), \
    DECODE_TT32(3, _NAME##_W_Q, "111" _OPU "|1111|0.10|...0|...0" _OPA ".1." _OPB "...0"), \
    DECODE_TT32(3, _NAME##_H_Q, "111" _OPU "|1111|0.01|...0|...0" _OPA ".1." _OPB "...0"), \
    DECODE_TT32(3, _NAME##_W_D, "111" _OPU "|1111|0.10|....|...." _OPA ".0." _OPB "...."), \
    DECODE_TT32(3, _NAME##_H_D, "111" _OPU "|1111|0.01|....|...." _OPA ".0." _OPB "....")

//
// Polynomial type instruction:
// Decode entries for 32-bit Thumb SIMD instructions with 3 regs same length, Q/D versions and esize 8 in bits 21:20
//  Undefined Instruction when size!=b00 || (Q==1 && (Qn<0>=1 || Qm<0>==1 || Qd<0>==1))
//
#define DECODE_SET_32_SIMD_RRR_QD_P(_NAME, _OPA, _OPB, _OPU) \
    DECODE_LAST(2,            "111" _OPU "|1111|0...|....|...." _OPA "..." _OPB "...."), \
    DECODE_TT32(3, _NAME##_Q, "111" _OPU "|1111|0.00|...0|...0" _OPA ".1." _OPB "...0"), \
    DECODE_TT32(3, _NAME##_D, "111" _OPU "|1111|0.00|....|...." _OPA ".0." _OPB "....")

//
// Decode entries for 32-bit Thumb SIMD instructions with 2 regs + imm, Q/D versions and esize 8, 16, 32 or 64 in part of imm
//  Undefined Instruction when Q==1 && (Qn<0>==1 || Qm<0>==1 || Qd<0>==1)
//
#define DECODE_SET_32_SIMD_RRR_QD_BHWD(_NAME, _OPA, _OPB, _OPU)  \
    DECODE_LAST(2,              "111" _OPU "|1111|0...|....|...." _OPA "..." _OPB "...."), \
    DECODE_TT32(3, _NAME##_D_Q, "111" _OPU "|1111|0.11|...0|...0" _OPA ".1." _OPB "...0"), \
    DECODE_TT32(3, _NAME##_W_Q, "111" _OPU "|1111|0.10|...0|...0" _OPA ".1." _OPB "...0"), \
    DECODE_TT32(3, _NAME##_H_Q, "111" _OPU "|1111|0.01|...0|...0" _OPA ".1." _OPB "...0"), \
    DECODE_TT32(3, _NAME##_B_Q, "111" _OPU "|1111|0.00|...0|...0" _OPA ".1." _OPB "...0"), \
    DECODE_TT32(3, _NAME##_D_D, "111" _OPU "|1111|0.11|....|...." _OPA ".0." _OPB "...."), \
    DECODE_TT32(3, _NAME##_W_D, "111" _OPU "|1111|0.10|....|...." _OPA ".0." _OPB "...."), \
    DECODE_TT32(3, _NAME##_H_D, "111" _OPU "|1111|0.01|....|...." _OPA ".0." _OPB "...."), \
    DECODE_TT32(3, _NAME##_B_D, "111" _OPU "|1111|0.00|....|...." _OPA ".0." _OPB "....")

////////////////////////////////////////////////////////////////////////


//
// Decode entries for SIMD instructions with 3 regs different lengths, Long=QDD, and esize 8, 16 or 32 in bits 21:20
//  Undefined instruction if Vd<0>==1
//  (NOTE: esize=11 is used in other encodings, which must have priority > 2)
//
#define DECODE_SET_32_SIMD_L_BHW(_NAME, _OPA, _OPU) \
    DECODE_LAST(2,            "111" _OPU "|1111|1...|....|...." _OPA ".0.0|...."), \
    DECODE_TT32(3, _NAME##_W, "111" _OPU "|1111|1.10|....|...0" _OPA ".0.0|...."), \
    DECODE_TT32(3, _NAME##_H, "111" _OPU "|1111|1.01|....|...0" _OPA ".0.0|...."), \
    DECODE_TT32(3, _NAME##_B, "111" _OPU "|1111|1.00|....|...0" _OPA ".0.0|....")

//
// Decode entries for SIMD instructions with 3 regs different lengths, Long=QDD, and esize 16 or 32 in bits 21:20
//  Undefined instruction if Vd<0>==1 || esize==0
//  (NOTE: esize=11 is used in other encodings, which must have priority > 2)
//
#define DECODE_SET_32_SIMD_L_HW(_NAME, _OPA, _OPU) \
    DECODE_LAST(2,            "111" _OPU "|1111|1...|....|...." _OPA ".0.0|...."), \
    DECODE_TT32(3, _NAME##_W, "111" _OPU "|1111|1.10|....|...0" _OPA ".0.0|...."), \
    DECODE_TT32(3, _NAME##_H, "111" _OPU "|1111|1.01|....|...0" _OPA ".0.0|....")

//
// Polynomial type instruction:
// Decode entries for SIMD instructions with 3 regs different lengths, Long=QDD, and esize 8 in bits 21:20
//  Undefined instruction if Vd<0>==1 || esize!=0 || U != 0
//  (NOTE: esize=11 is used in other encodings, which must have priority > 2)
//
#define DECODE_SET_32_SIMD_L_P(_NAME, _OPA) \
    DECODE_LAST(2,         "111.|1111|1...|....|...." _OPA ".0.0|...."), \
    DECODE_TT32(3, _NAME,  "1110|1111|1.00|....|...0" _OPA ".0.0|....")

//
// Decode entries for SIMD instructions with 3 regs different lengths, Wide=QQD, and esize 8, 16 or 32 in bits 21:20
//  Undefined instruction if Vd<0>==1 or Vn<0>==1
//  (NOTE: esize=11 is used in other encodings, which must have priority > 2)
//
#define DECODE_SET_32_SIMD_W_BHW(_NAME, _OPA, _OPU) \
    DECODE_LAST(2,            "111" _OPU "|1111|1...|....|...." _OPA ".0.0|...."), \
    DECODE_TT32(3, _NAME##_W, "111" _OPU "|1111|1.10|...0|...0" _OPA ".0.0|...."), \
    DECODE_TT32(3, _NAME##_H, "111" _OPU "|1111|1.01|...0|...0" _OPA ".0.0|...."), \
    DECODE_TT32(3, _NAME##_B, "111" _OPU "|1111|1.00|...0|...0" _OPA ".0.0|....")

//
// Decode entries for SIMD instructions with 3 regs different lengths, Narrow=DQQ, and esize 16, 32 or 64 in bits 21:20
//  Undefined instruction if Vm<0>==1 or Vn<0>==1
//  (NOTE: esize=11 is used in other encodings, which must have priority > 2)
//
#define DECODE_SET_32_SIMD_N_HWD(_NAME, _OPA, _OPU) \
    DECODE_LAST(2,            "111" _OPU "|1111|1...|....|...." _OPA ".0.0|...."), \
    DECODE_TT32(3, _NAME##_D, "111" _OPU "|1111|1.10|...0|...." _OPA ".0.0|...0"), \
    DECODE_TT32(3, _NAME##_W, "111" _OPU "|1111|1.01|...0|...." _OPA ".0.0|...0"), \
    DECODE_TT32(3, _NAME##_H, "111" _OPU "|1111|1.00|...0|...." _OPA ".0.0|...0")

//
// Decode entries for SIMD instructions with regs RRZ, Q/D versions and esize 16 or 32 in bits 21:20
//  Undefined Instruction when esize==b00 || (Q && (Vd<0>=1 || Vn<0>=1))
//  (NOTE: esize=11 is used in other encodings, which must have priority > 2)
//
#define DECODE_SET_32_SIMD_RRZ_QD_HW(_NAME, _OPA) \
    DECODE_LAST(2,              "111.|1111|1...|....|...." _OPA ".1.0|...."), \
    DECODE_TT32(3, _NAME##_W_D, "1110|1111|1.10|....|...." _OPA ".1.0|...."), \
    DECODE_TT32(3, _NAME##_H_D, "1110|1111|1.01|....|...." _OPA ".1.0|...."), \
    DECODE_TT32(3, _NAME##_W_Q, "1111|1111|1.10|...0|...0" _OPA ".1.0|...."), \
    DECODE_TT32(3, _NAME##_H_Q, "1111|1111|1.01|...0|...0" _OPA ".1.0|....")

//
// Decode entries for SIMD instructions with regs RRZ, Q/D versions and esize 32 in bits 21:20
//  Undefined Instruction when esize!=b10 || (Q && (Vd<0>=1 || Vn<0>=1))
//  (NOTE: esize=11 is used in other encodings, which must have priority > 2)
//
#define DECODE_SET_32_SIMD_RRZ_QD(_NAME, _OPA) \
    DECODE_LAST(2,            "111.|1111|1...|....|...." _OPA ".1.0|...."), \
    DECODE_TT32(3, _NAME##_D, "1110|1111|1.10|....|...." _OPA ".1.0|...."), \
    DECODE_TT32(3, _NAME##_Q, "1111|1111|1.10|...0|...0" _OPA ".1.0|....")

//
// Decode entries for SIMD instructions with regs Long Scalar=QDZ and esize 16 or 32 in bits 21:20
//  Undefined Instruction when esize==b00 || (Q && (Vd<0>=1)
//  (NOTE: esize=11 is used in other encodings, which must have priority > 2)
//
#define DECODE_SET_32_SIMD_LZ_HW(_NAME, _OPA, _OPU) \
    DECODE_LAST(2,            "111" _OPU "|1111|1...|....|...." _OPA ".1.0|...."), \
    DECODE_TT32(3, _NAME##_W, "111" _OPU "|1111|1.10|....|...0" _OPA ".1.0|...."), \
    DECODE_TT32(3, _NAME##_H, "111" _OPU "|1111|1.01|....|...0" _OPA ".1.0|....")

//
// Decode entries for SIMD instructions with  regs same length and shift amount, Q/D versions and esize 8, 16, 32 or 64
//  Undefined Instruction when Q==1 && (Qm<0>==1 || Qd<0>==1)
//  (NOTE: 7,21:19=0000 is used in other encodings, which must have priority > 3)
//
#define DECODE_SET_32_SIMD_RRI_QD_BHWD(_NAME, _OPA, _OPU)  \
    DECODE_LAST(2,              "111" _OPU "|1111|1...|....|...." _OPA "...1|...."), \
    DECODE_TT32(3, _NAME##_D_D, "111" _OPU "|1111|1...|....|...." _OPA "10.1|...."), \
    DECODE_TT32(3, _NAME##_D_Q, "111" _OPU "|1111|1...|....|...0" _OPA "11.1|...0"), \
    DECODE_TT32(3, _NAME##_W_D, "111" _OPU "|1111|1.1.|....|...." _OPA "00.1|...."), \
    DECODE_TT32(3, _NAME##_W_Q, "111" _OPU "|1111|1.1.|....|...0" _OPA "01.1|...0"), \
    DECODE_TT32(3, _NAME##_H_D, "111" _OPU "|1111|1.01|....|...." _OPA "00.1|...."), \
    DECODE_TT32(3, _NAME##_H_Q, "111" _OPU "|1111|1.01|....|...0" _OPA "01.1|...0"), \
    DECODE_TT32(3, _NAME##_B_D, "111" _OPU "|1111|1.00|1...|...." _OPA "00.1|...."), \
    DECODE_TT32(3, _NAME##_B_Q, "111" _OPU "|1111|1.00|1...|...0" _OPA "01.1|...0")

//
// Decode entries for SIMD instruction VQSHLUS:
//   Same as  DECODE_SET_32_SIMD_RRI_QD_BHWD but with undefined instruction when U=0
//
#define DECODE_SET_32_SIMD_VQSHLUS(_NAME, _OPA, _OPU)  \
    DECODE_LAST(2,       "1110|1111|1...|....|....|0110|...1|...."), \
    DECODE_SET_32_SIMD_RRI_QD_BHWD(_NAME, _OPA, _OPU)

//
// Decode entries for SIMD instructions with 2 regs same length and immediate, Q/D versions and esize 32 only
//  Undefined Instruction when Q==1 && (Qm<0>==1 || Qd<0>==1)
//  Undefined Instruction when bit 21 (imm6) == 0)
//  (NOTE: 21:19 = 000 is used in other encodings, which must have priority > 2)
//
#define DECODE_SET_32_SIMD_RRI_QD_W(_NAME, _OPA, _OPU)  \
    DECODE_LAST(2,            "111" _OPU "|1111|1...|....|...." _OPA "0..1|...."), \
    DECODE_TT32(3, _NAME##_D, "111" _OPU "|1111|1.1.|....|...." _OPA "00.1|...."), \
    DECODE_TT32(3, _NAME##_Q, "111" _OPU "|1111|1.1.|....|...0" _OPA "01.1|...0")

//
// Decode entries for SIMD instructions with 2 regs different length and shift amount, Long = QDI and esize 8, 16, or 32
//  Undefined Instruction when Qd<0>==1
//  (NOTE: 21:19=000 is used in other encodings, which must have priority > 2)
//  (NOTE: 18:16=000 is VMOVL, which must have priority > 2)
//
#define DECODE_SET_32_SIMD_LI_BHW(_NAME, _OPA, _OPU)  \
    DECODE_LAST(2,            "111" _OPU "|1111|1...|....|...." _OPA "00.1|...."), \
    DECODE_TT32(3, _NAME##_W, "111" _OPU "|1111|1.1.|....|...0" _OPA "00.1|...."), \
    DECODE_TT32(3, _NAME##_H, "111" _OPU "|1111|1.01|....|...0" _OPA "00.1|...."), \
    DECODE_TT32(3, _NAME##_B, "111" _OPU "|1111|1.00|1...|...0" _OPA "00.1|....")

//
// Decode entries for SIMD instruction VMOVL - special case of VSHLL with shift amount = 0
//
#define DECODE_SET_32_SIMD_VMOVL(_NAME, _OPA, _OPU)  \
    DECODE_TT32(4, _NAME##_W, "111" _OPU "|1111|1.10|0000|...0" _OPA "00.1|...."), \
    DECODE_TT32(4, _NAME##_H, "111" _OPU "|1111|1.01|0000|...0" _OPA "00.1|...."), \
    DECODE_TT32(4, _NAME##_B, "111" _OPU "|1111|1.00|1000|...0" _OPA "00.1|....")

//
// Decode entries for SIMD instructions with 2 regs different length (narrow) and shift amount and esize 16, 32 or 64
//  Undefined Instruction when Qm<0>==1
//  (NOTE: 21:19=000 is used in other encodings, which must have priority > 2)
//
#define DECODE_SET_32_SIMD_NI_HWD(_NAME, _OPA, _OPU, _OPB)  \
    DECODE_LAST(2,            "111" _OPU "|1111|1...|....|...." _OPA "0" _OPB ".1|...."), \
    DECODE_TT32(3, _NAME##_D, "111" _OPU "|1111|1.1.|....|...." _OPA "0" _OPB ".1|...0"), \
    DECODE_TT32(3, _NAME##_W, "111" _OPU "|1111|1.01|....|...." _OPA "0" _OPB ".1|...0"), \
    DECODE_TT32(3, _NAME##_H, "111" _OPU "|1111|1.00|1...|...." _OPA "0" _OPB ".1|...0")

//
// Decode entries for SIMD instructions with 2 regs same length, Q/D versions and esize 8, 16, 32 in bits 19:18
//  Undefined Instruction when size==b11 || (Q==1 && (Qm<0>==1 || Qd<0>==1))
//
#define DECODE_SET_32_SIMD_RR_QD_BHW(_NAME, _OPA, _OPB) \
    DECODE_LAST(3,              "1111|1111|1.11|.." _OPA "|....|0" _OPB "..0|...."), \
    DECODE_TT32(4, _NAME##_W_Q, "1111|1111|1.11|10" _OPA "|...0|0" _OPB "1.0|...0"), \
    DECODE_TT32(4, _NAME##_H_Q, "1111|1111|1.11|01" _OPA "|...0|0" _OPB "1.0|...0"), \
    DECODE_TT32(4, _NAME##_B_Q, "1111|1111|1.11|00" _OPA "|...0|0" _OPB "1.0|...0"), \
    DECODE_TT32(4, _NAME##_W_D, "1111|1111|1.11|10" _OPA "|....|0" _OPB "0.0|...."), \
    DECODE_TT32(4, _NAME##_H_D, "1111|1111|1.11|01" _OPA "|....|0" _OPB "0.0|...."), \
    DECODE_TT32(4, _NAME##_B_D, "1111|1111|1.11|00" _OPA "|....|0" _OPB "0.0|....")

//
// Decode entries for SIMD instructions with 2 regs same length,
//  Q versions for esize 8, 16, 32, D versions for size 8 and 16, esize in bits 19:18
//  Undefined Instruction when size==b11 || || (Q=0 && esize=b10) || (Q==1 && (Qm<0>==1 || Qd<0>==1))
//
#define DECODE_SET_32_SIMD_RR_QBHW_DBH(_NAME, _OPA, _OPB) \
    DECODE_LAST(3,              "1111|1111|1.11|.." _OPA "|....|0" _OPB "..0|...."), \
    DECODE_TT32(4, _NAME##_W_Q, "1111|1111|1.11|10" _OPA "|...0|0" _OPB "1.0|...0"), \
    DECODE_TT32(4, _NAME##_H_Q, "1111|1111|1.11|01" _OPA "|...0|0" _OPB "1.0|...0"), \
    DECODE_TT32(4, _NAME##_B_Q, "1111|1111|1.11|00" _OPA "|...0|0" _OPB "1.0|...0"), \
    DECODE_TT32(4, _NAME##_H_D, "1111|1111|1.11|01" _OPA "|....|0" _OPB "0.0|...."), \
    DECODE_TT32(4, _NAME##_B_D, "1111|1111|1.11|00" _OPA "|....|0" _OPB "0.0|....")

//
// Decode entries for SIMD instructions with 2 regs same length, Q/D versions and esize 8 in bits 19:18
//  Undefined Instruction when size!=b00 || (Q==1 && (Qm<0>==1 || Qd<0>==1))
//
#define DECODE_SET_32_SIMD_RR_QD_B(_NAME, _OPA, _OPB) \
    DECODE_LAST(3,            "1111|1111|1.11|.." _OPA "|....|0" _OPB "..0|...."), \
    DECODE_TT32(4, _NAME##_Q, "1111|1111|1.11|00" _OPA "|...0|0" _OPB "1.0|...0"), \
    DECODE_TT32(4, _NAME##_D, "1111|1111|1.11|00" _OPA "|....|0" _OPB "0.0|....")

//
// Decode entries for SIMD instructions with 2 regs same length, Q/D versions and esize32 in bits 19:18
//  Undefined Instruction when size!=b10 || (Q==1 && (Qm<0>==1 || Qd<0>==1))
//
#define DECODE_SET_32_SIMD_RR_QD_W(_NAME, _OPA, _OPB) \
    DECODE_LAST(3,            "1111|1111|1.11|.." _OPA "|....|0" _OPB "..0|...."), \
    DECODE_TT32(4, _NAME##_Q, "1111|1111|1.11|10" _OPA "|...0|0" _OPB "1.0|...0"), \
    DECODE_TT32(4, _NAME##_D, "1111|1111|1.11|10" _OPA "|....|0" _OPB "0.0|....")

//
// Decode entries for SIMD instructions with 1 reg and a mod imm val, Q/D versions
//  Undefined Instruction when Q==1 &&  Qd<0>==1
//
#define DECODE_SET_32_SIMD_RI_QD(_NAME, _OP, _CMODE) \
    DECODE_LAST(3,            "111.|1111|1.00|0...|....|" _CMODE "|0." _OP "1|...."), \
    DECODE_TT32(4, _NAME##_Q, "111.|1111|1.00|0...|...0|" _CMODE "|01" _OP "1|...."), \
    DECODE_TT32(4, _NAME##_D, "111.|1111|1.00|0...|....|" _CMODE "|00" _OP "1|....")

//
// Decode entries for SIMD instructions with 2 regs different lengths, Long=QD, and esize 8, 16 or 32 in bits 19:18
//  Undefined instruction if Vd<0>==1 || esize==b11
//
#define DECODE_SET_32_SIMD_L2_BHW(_NAME, _OPA, _OPB) \
    DECODE_LAST(3,            "1111|1111|1.11|.." _OPA "|....|0" _OPB ".0|...."), \
    DECODE_TT32(4, _NAME##_W, "1111|1111|1.11|10" _OPA "|...0|0" _OPB ".0|...."), \
    DECODE_TT32(4, _NAME##_H, "1111|1111|1.11|01" _OPA "|...0|0" _OPB ".0|...."), \
    DECODE_TT32(4, _NAME##_B, "1111|1111|1.11|00" _OPA "|...0|0" _OPB ".0|....")

//
// Decode entries for SIMD instructions with 2 regs different lengths, Narrow=DQ, and esize 16, 32 or 64 in bits 19:18
//  Undefined instruction if Vm<0>==1 || esize==b11
//
#define DECODE_SET_32_SIMD_N2_HWD(_NAME, _OPA, _OPB) \
    DECODE_LAST(3,            "1111|1111|1.11|.." _OPA "|....|0" _OPB ".0|...."), \
    DECODE_TT32(4, _NAME##_D, "1111|1111|1.11|10" _OPA "|....|0" _OPB ".0|...0"), \
    DECODE_TT32(4, _NAME##_W, "1111|1111|1.11|01" _OPA "|....|0" _OPB ".0|...0"), \
    DECODE_TT32(4, _NAME##_H, "1111|1111|1.11|00" _OPA "|....|0" _OPB ".0|...0")

//
// Decode entries for SIMD instructions with 2 regs different lengths, Long=QD, and esize 16 in bits 19:18
//  Undefined instruction if Vd<0>==1 || esize!=b01
//
#define DECODE_SET_32_SIMD_L2_H(_NAME, _OPA, _OPB) \
    DECODE_LAST(3,        "1111|1111|1.11|.." _OPA "|....|0" _OPB ".0|...."), \
    DECODE_TT32(4, _NAME, "1111|1111|1.11|01" _OPA "|...0|0" _OPB ".0|....")

//
// Decode entries for SIMD instructions with 2 regs different lengths, Narrow=DQ, and esize 16 in bits 19:18
//  Undefined instruction if Vm<0>==1 || esize!=b01
//
#define DECODE_SET_32_SIMD_N2_H(_NAME, _OPA, _OPB) \
    DECODE_LAST(3,        "1111|1111|1.11|.." _OPA "|....|0" _OPB ".0|...."), \
    DECODE_TT32(4, _NAME, "1111|1111|1.11|01" _OPA "|....|0" _OPB ".0|...0")

//
// Decode entries for VFP/SIMD instructions like VMRS
//
#define DECODE_SET_32_VMRS(_NAME, _OPL, _OPC, _OPA, _OPB) \
    DECODE_TT32(2, _NAME, "1110|1110" _OPA _OPL "|....|....|101" _OPC "|." _OPB "1|...."), \
    DECODE_LAST(2,        "1111|1110" _OPA _OPL "|....|....|101" _OPC "|." _OPB "1|....")

//
// Decode entries for VFP/SIMD instructions like VMOVRRD
//
#define DECODE_SET_32_VMOVRRD(_NAME, _OPL, _OPC, _OP) \
    DECODE_TT32(6, _NAME, "1110|1100|010" _OPL "........|101" _OPC _OP "|...."), \
    DECODE_LAST(7,        "1111|1100|010" _OPL "........|101" _OPC _OP "|....")

//
// Decode entries for VFP/SIMD instruction VDUP (ARM core register)
//
#define DECODE_SET_32_VDUPR(_NAME) \
    DECODE_LAST(2,              "1110|1110|1..0|....|....|1011|.0.1|...."), \
    DECODE_TT32(3, _NAME##_W_D, "1110|1110|1000|....|....|1011|.001|...."), \
    DECODE_TT32(3, _NAME##_H_D, "1110|1110|1000|....|....|1011|.011|...."), \
    DECODE_TT32(3, _NAME##_B_D, "1110|1110|1100|....|....|1011|.001|...."), \
    DECODE_TT32(3, _NAME##_W_Q, "1110|1110|1010|...0|....|1011|.001|...."), \
    DECODE_TT32(3, _NAME##_H_Q, "1110|1110|1010|...0|....|1011|.011|...."), \
    DECODE_TT32(3, _NAME##_B_Q, "1110|1110|1110|...0|....|1011|.001|...."), \
    DECODE_LAST(4,              "1111|1110|1..0|....|....|1011|.0.1|....")

//
// Decode entries for VFP/SIMD instruction VMOV (ARM core register to scalar)
//
#define DECODE_SET_32_VMOVZR(_NAME) \
    DECODE_TT32(2, _NAME##_W, "1110|1110|00.0|....|....|1011|.001|...."), \
    DECODE_TT32(2, _NAME##_H, "1110|1110|00.0|....|....|1011|..11|...."), \
    DECODE_TT32(2, _NAME##_B, "1110|1110|01.0|....|....|1011|...1|...."), \
    DECODE_LAST(3,            "1111|1110|0..0|....|....|1011|...1|....")

//
// Decode entries for VFP/SIMD instruction VMOV (scalar to ARM core register)
//
#define DECODE_SET_32_VMOVRZ(_NAME) \
    DECODE_TT32(2, _NAME##_W,  "1110|1110|00.1|....|....|1011|.001|...."), \
    DECODE_TT32(2, _NAME##S_H, "1110|1110|00.1|....|....|1011|..11|...."), \
    DECODE_TT32(2, _NAME##U_H, "1110|1110|10.1|....|....|1011|..11|...."), \
    DECODE_TT32(2, _NAME##S_B, "1110|1110|01.1|....|....|1011|...1|...."), \
    DECODE_TT32(2, _NAME##U_B, "1110|1110|11.1|....|....|1011|...1|...."), \
    DECODE_LAST(3,             "1111|1110|...1|....|....|1011|...1|....")

//
// Decode entries for SIMD instructions vrev16, vrev32 and vrev64
//  Undefined Instruction when Q==1 && (Qm<0>==1 || Qd<0>==1)
//  Undefined Instruction when size (19:18) + op (8:7) >= 3
//
#define DECODE_SET_32_SIMD_VREV(_NAME)  \
    DECODE_LAST(3,                "1111|1111|1.11|..00|....|000.|...0|...."), \
    DECODE_TT32(4, _NAME##16_B_D, "1111|1111|1.11|0000|....|0001|00.0|...."), \
    DECODE_TT32(4, _NAME##16_B_Q, "1111|1111|1.11|0000|...0|0001|01.0|...0"), \
    DECODE_TT32(4, _NAME##32_B_D, "1111|1111|1.11|0000|....|0000|10.0|...."), \
    DECODE_TT32(4, _NAME##32_B_Q, "1111|1111|1.11|0000|...0|0000|11.0|...0"), \
    DECODE_TT32(4, _NAME##32_H_D, "1111|1111|1.11|0100|....|0000|10.0|...."), \
    DECODE_TT32(4, _NAME##32_H_Q, "1111|1111|1.11|0100|...0|0000|11.0|...0"), \
    DECODE_TT32(4, _NAME##64_B_D, "1111|1111|1.11|0000|....|0000|00.0|...."), \
    DECODE_TT32(4, _NAME##64_B_Q, "1111|1111|1.11|0000|...0|0000|01.0|...0"), \
    DECODE_TT32(4, _NAME##64_H_D, "1111|1111|1.11|0100|....|0000|00.0|...."), \
    DECODE_TT32(4, _NAME##64_H_Q, "1111|1111|1.11|0100|...0|0000|01.0|...0"), \
    DECODE_TT32(4, _NAME##64_W_D, "1111|1111|1.11|1000|....|0000|00.0|...."), \
    DECODE_TT32(4, _NAME##64_W_Q, "1111|1111|1.11|1000|...0|0000|01.0|...0")

//
// Decode entries for VFP instructions with S/D versions
//
#define DECODE_SET_32_VFP_DS(_NAME, _OP1, _OP2, _OP3) \
    DECODE_TT32(2, _NAME##_D, "1110|1110|" _OP1 _OP2 "|....|1011|" _OP3 ".0|...."), \
    DECODE_TT32(2, _NAME##_S, "1110|1110|" _OP1 _OP2 "|....|1010|" _OP3 ".0|...."), \
    DECODE_LAST(3,            "1111|1110|" _OP1 _OP2 "|....|101.|" _OP3 ".0|....")

//
// Decode entries for VFP instructions S version only
//
#define DECODE_SET_32_VFP_S(_NAME, _OP1, _OP2, _OP3) \
    DECODE_TT32(2, _NAME, "1110|1110|" _OP1 _OP2 "|....|1010|" _OP3 ".0|...."), \
    DECODE_LAST(3,        "1111|1110|" _OP1 _OP2 "|....|1010|" _OP3 ".0|....")

//
// Decode entries for SIMD/VFP load/store instructions
//
#define DECODE_SET_32_SDFP_LDST(_NAME, _OP) \
    DECODE_TT32(4, _NAME##_D, "1110|110" _OP "|....|....|1011|....|...."), \
    DECODE_TT32(4, _NAME##_S, "1110|110" _OP "|....|....|1010|....|...."), \
    DECODE_LAST(6,            "1111|110" _OP "|....|....|101.|....|....")

//
// Decode entries for SIMD/VFP PUSH/POP instructions
//
#define DECODE_SET_32_SDFP_PUSH_POP(_NAME, _OP) \
    DECODE_TT32(5, _NAME##_D, "1110|110" _OP "|1101|....|1011|....|...."), \
    DECODE_TT32(5, _NAME##_S, "1110|110" _OP "|1101|....|1010|....|....")

//
// Decode entries for SIMD VSTn/VLDn multiple n element structs  Any alignment allowed
//
#define DECODE_SET_32_SIMD_LDSTN_BHWD(_NAME, _OPA, _OPL, _OPB) \
	DECODE_TT32(4, _NAME##_D,"1111|1001|" _OPA "." _OPL "0|....|....|" _OPB "|11..|...."), \
	DECODE_TT32(4, _NAME##_W,"1111|1001|" _OPA "." _OPL "0|....|....|" _OPB "|10..|...."), \
	DECODE_TT32(4, _NAME##_H,"1111|1001|" _OPA "." _OPL "0|....|....|" _OPB "|01..|...."), \
	DECODE_TT32(4, _NAME##_B,"1111|1001|" _OPA "." _OPL "0|....|....|" _OPB "|00..|....")

//
// Decode entries for SIMD VSTn/VLDn multiple n element structs  Byte, half, word or double, alignment 0 or 1 only
//
#define DECODE_SET_32_SIMD_LDSTN_BHWD_A01(_NAME, _OPA, _OPL, _OPB) \
	DECODE_SET_32_SIMD_LDSTN_BHWD(_NAME, _OPA, _OPL, _OPB), \
	DECODE_LAST(5,           "1111|1001|" _OPA "." _OPL "0|....|....|" _OPB "|..1.|....")

//
// Decode entries for SIMD VSTn/VLDn multiple n element structs  Byte, half, word or double, alignment 0, 1 or 2 only
//
#define DECODE_SET_32_SIMD_LDSTN_BHWD_A012(_NAME, _OPA, _OPL, _OPB) \
	DECODE_SET_32_SIMD_LDSTN_BHWD(_NAME, _OPA, _OPL, _OPB), \
	DECODE_LAST(5, "1111|1001|" _OPA "." _OPL "0|....|....|" _OPB "|..11|....")

//
// Decode entries for SIMD VSTn/VLDn multiple n element structs  Byte, half or word only, any alignment
//
#define DECODE_SET_32_SIMD_LDSTN_BHW(_NAME, _OPA, _OPL, _OPB) \
	DECODE_LAST(4,           "1111|1001|" _OPA "." _OPL "0|....|....|" _OPB "|....|...."), \
	DECODE_TT32(5, _NAME##_W,"1111|1001|" _OPA "." _OPL "0|....|....|" _OPB "|10..|...."), \
	DECODE_TT32(5, _NAME##_H,"1111|1001|" _OPA "." _OPL "0|....|....|" _OPB "|01..|...."), \
	DECODE_TT32(5, _NAME##_B,"1111|1001|" _OPA "." _OPL "0|....|....|" _OPB "|00..|....")

//
// Decode entries for SIMD VSTn/VLDn multiple n element structs  Byte, half or word only, alignment 0, 1 or 2
//
#define DECODE_SET_32_SIMD_LDSTN_BHW_A012(_NAME, _OPA, _OPL, _OPB) \
	DECODE_SET_32_SIMD_LDSTN_BHW(_NAME, _OPA, _OPL, _OPB), \
	DECODE_LAST(6,       "1111|1001|" _OPA "." _OPL "0|....|....|" _OPB "|..11|....")

//
// Decode entries for SIMD VSTn/VLDn multiple n element structs  Byte, half or word only, alignment 0 or 1
//
#define DECODE_SET_32_SIMD_LDSTN_BHW_A01(_NAME, _OPA, _OPL, _OPB) \
	DECODE_SET_32_SIMD_LDSTN_BHW(_NAME, _OPA, _OPL, _OPB), \
	DECODE_LAST(6,       "1111|1001|" _OPA "." _OPL "0|....|....|" _OPB "|..1.|....")

//
// Decode entries for SIMD VST1/VLD1 Single element to one lane.  Byte, half or word only
//
#define DECODE_SET_32_SIMD_LDST1Z1_BHW(_NAME, _OPA, _OPL, _OPB) \
	DECODE_LAST(4,           "1111|1001|" _OPA "." _OPL "0|....|....|.." _OPB "|....|...."), \
	DECODE_TT32(5, _NAME##_W,"1111|1001|" _OPA "." _OPL "0|....|....|10" _OPB "|.0..|...."), \
	DECODE_LAST(6,           "1111|1001|" _OPA "." _OPL "0|....|....|10" _OPB "|..01|...."), \
	DECODE_LAST(6,           "1111|1001|" _OPA "." _OPL "0|....|....|10" _OPB "|..10|...."), \
	DECODE_TT32(5, _NAME##_H,"1111|1001|" _OPA "." _OPL "0|....|....|01" _OPB "|..0.|...."), \
	DECODE_TT32(5, _NAME##_B,"1111|1001|" _OPA "." _OPL "0|....|....|00" _OPB "|...0|....")

//
// Decode entries for SIMD VLD1 Single element to all lanes.  Byte, half or word only
//
#define DECODE_SET_32_SIMD_LD1ZA_BHW(_NAME, _OPA, _OPL, _OPB) \
	DECODE_LAST(5,           "1111|1001|" _OPA "." _OPL "0|....|....|11" _OPB "|....|...."), \
	DECODE_TT32(6, _NAME##_W,"1111|1001|" _OPA "." _OPL "0|....|....|11" _OPB "|10..|...."), \
	DECODE_TT32(6, _NAME##_H,"1111|1001|" _OPA "." _OPL "0|....|....|11" _OPB "|01..|...."), \
	DECODE_TT32(6, _NAME##_B,"1111|1001|" _OPA "." _OPL "0|....|....|11" _OPB "|00.0|....")

//
// Decode entries for SIMD VST2/VLD2 Single element to one lane.  Byte, half or word only
//
#define DECODE_SET_32_SIMD_LDST2Z1_BHW(_NAME, _OPA, _OPL, _OPB) \
	DECODE_LAST(4,           "1111|1001|" _OPA "." _OPL "0|....|....|.." _OPB "|....|...."), \
	DECODE_TT32(5, _NAME##_W,"1111|1001|" _OPA "." _OPL "0|....|....|10" _OPB "|..0.|...."), \
	DECODE_TT32(5, _NAME##_H,"1111|1001|" _OPA "." _OPL "0|....|....|01" _OPB "|....|...."), \
	DECODE_TT32(5, _NAME##_B,"1111|1001|" _OPA "." _OPL "0|....|....|00" _OPB "|....|....")

//
// Decode entries for SIMD VLD2 Single element to all lanes.  Byte, half or word only
//
#define DECODE_SET_32_SIMD_LD2ZA_BHW(_NAME, _OPA, _OPL, _OPB) \
	DECODE_LAST(5,           "1111|1001|" _OPA "." _OPL "0|....|....|11" _OPB "|....|...."), \
	DECODE_TT32(6, _NAME##_W,"1111|1001|" _OPA "." _OPL "0|....|....|11" _OPB "|10..|...."), \
	DECODE_TT32(6, _NAME##_H,"1111|1001|" _OPA "." _OPL "0|....|....|11" _OPB "|01..|...."), \
	DECODE_TT32(6, _NAME##_B,"1111|1001|" _OPA "." _OPL "0|....|....|11" _OPB "|00..|....")

//
// Decode entries for SIMD VST3/VLD3 Single element to one lane.  Byte, half or word only
//
#define DECODE_SET_32_SIMD_LDST3Z1_BHW(_NAME, _OPA, _OPL, _OPB) \
	DECODE_LAST(4,           "1111|1001|" _OPA "." _OPL "0|....|....|.." _OPB "|....|...."), \
	DECODE_TT32(5, _NAME##_W,"1111|1001|" _OPA "." _OPL "0|....|....|10" _OPB "|..00|...."), \
	DECODE_TT32(5, _NAME##_H,"1111|1001|" _OPA "." _OPL "0|....|....|01" _OPB "|...0|...."), \
	DECODE_TT32(5, _NAME##_B,"1111|1001|" _OPA "." _OPL "0|....|....|00" _OPB "|...0|....")

//
// Decode entries for SIMD VLD3 Single element to all lanes.  Byte, half or word only
//
#define DECODE_SET_32_SIMD_LD3ZA_BHW(_NAME, _OPA, _OPL, _OPB) \
	DECODE_LAST(5,           "1111|1001|" _OPA "." _OPL "0|....|....|11" _OPB "|....|...."), \
	DECODE_TT32(6, _NAME##_W,"1111|1001|" _OPA "." _OPL "0|....|....|11" _OPB "|10.0|...."), \
	DECODE_TT32(6, _NAME##_H,"1111|1001|" _OPA "." _OPL "0|....|....|11" _OPB "|01.0|...."), \
	DECODE_TT32(6, _NAME##_B,"1111|1001|" _OPA "." _OPL "0|....|....|11" _OPB "|00.0|....")


//
// Decode entries for SIMD VST4/VLD4 Single element to one lane.  Byte, half or word only
//
#define DECODE_SET_32_SIMD_LDST4Z1_BHW(_NAME, _OPA, _OPL, _OPB) \
	DECODE_LAST(4,           "1111|1001|" _OPA "." _OPL "0|....|....|.." _OPB "|....|...."), \
	DECODE_TT32(5, _NAME##_W,"1111|1001|" _OPA "." _OPL "0|....|....|10" _OPB "|....|...."), \
	DECODE_LAST(6,           "1111|1001|" _OPA "." _OPL "0|....|....|10" _OPB "|..11|...."), \
	DECODE_TT32(5, _NAME##_H,"1111|1001|" _OPA "." _OPL "0|....|....|01" _OPB "|....|...."), \
	DECODE_TT32(5, _NAME##_B,"1111|1001|" _OPA "." _OPL "0|....|....|00" _OPB "|....|....")

//
// Decode entries for SIMD VLD4 Single element to all lanes.  Byte, half or word only
// Note size (7:6) = 11 and align (4) = 1 is overloaded to handle size=16 align-128 case (F%^&#@ Crazy S#*&!)
//
#define DECODE_SET_32_SIMD_LD4ZA_BHW(_NAME, _OPA, _OPL, _OPB) \
	DECODE_LAST(5,            "1111|1001|" _OPA "." _OPL "0|....|....|11" _OPB "|11.0|...."), \
	DECODE_TT32(5, _NAME##_W1,"1111|1001|" _OPA "." _OPL "0|....|....|11" _OPB "|11.1|...."), \
	DECODE_TT32(5, _NAME##_W, "1111|1001|" _OPA "." _OPL "0|....|....|11" _OPB "|10..|...."), \
	DECODE_TT32(5, _NAME##_H, "1111|1001|" _OPA "." _OPL "0|....|....|11" _OPB "|01..|...."), \
	DECODE_TT32(5, _NAME##_B, "1111|1001|" _OPA "." _OPL "0|....|....|11" _OPB "|00..|....")

#endif

