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

#ifndef ARM_ATTRIBUTE_ENTRIES_ARM_H
#define ARM_ATTRIBUTE_ENTRIES_ARM_H

#include "armDisassembleFormats.h"

//
// Attribute entries for instructions like ADC
//
#define ATTR_SET_ADC(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_IMM]         = {opcode:_OPCODE, format:FMT_R1_R2_SIMM,          support:_SUPPORT, isar:_ISAR, f:ARM_SF_V, cond:1, r1:R_12, r2:R_16, r3:0,   r4:0,   cs:CS_U_8_0_R, ss:SS_NA }, \
    [ARM_IT_##_NAME##_RM]          = {opcode:_OPCODE, format:FMT_R1_R2_R3,            support:_SUPPORT, isar:_ISAR, f:ARM_SF_V, cond:1, r1:R_12, r2:R_16, r3:R_0, r4:0,   cs:CS_NA,      ss:SS_NA }, \
    [ARM_IT_##_NAME##_RM_SHFT_IMM] = {opcode:_OPCODE, format:FMT_R1_R2_R3_SHIFT_SIMM, support:_SUPPORT, isar:_ISAR, f:ARM_SF_V, cond:1, r1:R_12, r2:R_16, r3:R_0, r4:0,   cs:CS_U_5_7,   ss:SS_6_5}, \
    [ARM_IT_##_NAME##_RM_SHFT_RS]  = {opcode:_OPCODE, format:FMT_R1_R2_R3_SHIFT_R4,   support:_SUPPORT, isar:_ISAR, f:ARM_SF_V, cond:1, r1:R_12, r2:R_16, r3:R_0, r4:R_8, cs:CS_NA,      ss:SS_6_5}, \
    [ARM_IT_##_NAME##_RM_RRX]      = {opcode:_OPCODE, format:FMT_R1_R2_R3_SHIFT,      support:_SUPPORT, isar:_ISAR, f:ARM_SF_V, cond:1, r1:R_12, r2:R_16, r3:R_0, r4:0,   cs:CS_NA,      ss:SS_RRX}

//
// Attribute entries for instructions like MOV
//
#define ATTR_SET_MOV(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_IMM]         = {opcode:_OPCODE, format:FMT_R1_SIMM,          support:_SUPPORT, isar:_ISAR, f:ARM_SF_V, cond:1,   r1:R_12, r2:0,   r3:0,   cs:CS_U_8_0_R, ss:SS_NA,}, \
    [ARM_IT_##_NAME##_RM]          = {opcode:_OPCODE, format:FMT_R1_R2,            support:_SUPPORT, isar:_ISAR, f:ARM_SF_V, cond:1,   r1:R_12, r2:R_0, r3:0,   cs:CS_NA,      ss:SS_NA,}, \
    [ARM_IT_##_NAME##_RM_SHFT_IMM] = {opcode:_OPCODE, format:FMT_R1_R2_SHIFT_SIMM, support:_SUPPORT, isar:_ISAR, f:ARM_SF_V, cond:1,   r1:R_12, r2:R_0, r3:0,   cs:CS_U_5_7,   ss:SS_6_5}, \
    [ARM_IT_##_NAME##_RM_SHFT_RS]  = {opcode:_OPCODE, format:FMT_R1_R2_SHIFT_R3,   support:_SUPPORT, isar:_ISAR, f:ARM_SF_V, cond:1,   r1:R_12, r2:R_0, r3:R_8, cs:CS_NA,      ss:SS_6_5}, \
    [ARM_IT_##_NAME##_RM_RRX]      = {opcode:_OPCODE, format:FMT_R1_R2_SHIFT,      support:_SUPPORT, isar:_ISAR, f:ARM_SF_V, cond:1,   r1:R_12, r2:R_0, r3:0,   cs:CS_NA,      ss:SS_RRX}

//
// Attribute entries for instructions like MOVT
//
#define ATTR_SET_MOVT(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_SIMM, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, cs:CS_U_16_16_0} \

//
// Attribute entries for instructions like MLA
//
#define ATTR_SET_MLA(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_R3_R4, support:_SUPPORT, isar:_ISAR, f:ARM_SF_V, cond:1, r1:R_16, r2:R_0, r3:R_8, r4:R_12}

//
// Attribute entries for instructions like MUL
//
#define ATTR_SET_MUL(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_R3, support:_SUPPORT, isar:_ISAR, f:ARM_SF_V, cond:1, r1:R_16, r2:R_0, r3:R_8}

//
// Attribute entries for instructions like SMLAL
//
#define ATTR_SET_SMLAL(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_R3_R4, support:_SUPPORT, isar:_ISAR, f:ARM_SF_V, cond:1, r1:R_12, r2:R_16, r3:R_0, r4:R_8}

//
// Attribute entries for instructions like CMN
//
#define ATTR_SET_CMN(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_IMM]         = {opcode:_OPCODE, format:FMT_R1_SIMM,          support:_SUPPORT, isar:_ISAR, f:ARM_SF_I, cond:1, r1:R_16, r2:0,   r3:0,   cs:CS_U_8_0_R, ss:SS_NA }, \
    [ARM_IT_##_NAME##_RM]          = {opcode:_OPCODE, format:FMT_R1_R2,            support:_SUPPORT, isar:_ISAR, f:ARM_SF_I, cond:1, r1:R_16, r2:R_0, r3:0,   cs:CS_NA,      ss:SS_NA }, \
    [ARM_IT_##_NAME##_RM_SHFT_IMM] = {opcode:_OPCODE, format:FMT_R1_R2_SHIFT_SIMM, support:_SUPPORT, isar:_ISAR, f:ARM_SF_I, cond:1, r1:R_16, r2:R_0, r3:0,   cs:CS_U_5_7,   ss:SS_6_5}, \
    [ARM_IT_##_NAME##_RM_SHFT_RS]  = {opcode:_OPCODE, format:FMT_R1_R2_SHIFT_R3,   support:_SUPPORT, isar:_ISAR, f:ARM_SF_I, cond:1, r1:R_16, r2:R_0, r3:R_8, cs:CS_NA,      ss:SS_6_5}, \
    [ARM_IT_##_NAME##_RM_RRX]      = {opcode:_OPCODE, format:FMT_R1_R2_SHIFT,      support:_SUPPORT, isar:_ISAR, f:ARM_SF_I, cond:1, r1:R_16, r2:R_0, r3:0,   cs:CS_NA,      ss:SS_RRX}

//
// Attribute entries for instructions like B
//
#define ATTR_SET_B(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_T,  support:_SUPPORT, isar:_ISAR, cond:1, ts:TC_S24}

//
// Attribute entries for instructions like BLX (1)
//
#define ATTR_SET_BLX1(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_T, support:_SUPPORT, isar:_ISAR, ts:TC_S24H}

//
// Attribute entries for instructions like BLX (2)
//
#define ATTR_SET_BLX2(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_0}

//
// Attribute entries for instructions like BKPT
//
#define ATTR_SET_BKPT(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_XIMM, support:_SUPPORT, isar:_ISAR, cs:CS_U_16_8_0}

//
// Attribute entries for instructions like CLZ
//
#define ATTR_SET_CLZ(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_0}

//
// Attribute entries for instructions like SWI
//
#define ATTR_SET_SWI(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_XIMM, support:_SUPPORT, isar:_ISAR, cond:1, cs:CS_U_24_0}

//
// Attribute entries for instructions like LDM(1)
//
#define ATTR_SET_LDM1(_NAME, _SUPPORT, _ISAR, _OPCODE, _UA45, _UA67) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_RLIST, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, wb:WB_21, rList:RL_32, incDec:ID_U_P_IAI, ua45:_UA45, ua67:_UA67}

//
// Attribute entries for instructions like LDM(2)
//
#define ATTR_SET_LDM2(_NAME, _SUPPORT, _ISAR, _OPCODE, _UA45, _UA67) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_RLIST_UM, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, wb:WB_21, rList:RL_32, incDec:ID_U_P_IAI, ua45:_UA45, ua67:_UA67}

//
// Attribute entries for instructions like LDR
//
#define ATTR_SET_LDR(_NAME, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [ARM_IT_##_NAME##_IMM]         = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_SIMM,          support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:0,   cs:CS_U_12_0_U, ss:SS_NA,  sz:_SZ, xs:_XS, pi:PI_24, wb:WB_21, ua45:_UA45, ua67:_UA67}, \
    [ARM_IT_##_NAME##_RM]          = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_R3,            support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0, cs:CS_NA,       ss:SS_NA,  sz:_SZ, xs:_XS, pi:PI_24, wb:WB_21, ua45:_UA45, ua67:_UA67}, \
    [ARM_IT_##_NAME##_RM_SHFT_IMM] = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_R3_SHIFT_SIMM, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0, cs:CS_U_5_7,    ss:SS_6_5, sz:_SZ, xs:_XS, pi:PI_24, wb:WB_21, ua45:_UA45, ua67:_UA67}, \
    [ARM_IT_##_NAME##_RM_RRX]      = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_R3_SHIFT,      support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0, cs:CS_NA,       ss:SS_RRX, sz:_SZ, xs:_XS, pi:PI_24, wb:WB_21, ua45:_UA45, ua67:_UA67}

//
// Attribute entries for instructions like LDRBT
//
#define ATTR_SET_LDRBT(_NAME, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [ARM_IT_##_NAME##_IMM]         = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_SIMM,          support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:0,   cs:CS_U_12_0_U, ss:SS_NA,  sz:_SZ, xs:_XS, tl:1, pi:PI_24, wb:WB_21, ua45:_UA45, ua67:_UA67}, \
    [ARM_IT_##_NAME##_RM]          = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_R3,            support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0, cs:CS_NA,       ss:SS_NA,  sz:_SZ, xs:_XS, tl:1, pi:PI_24, wb:WB_21, ua45:_UA45, ua67:_UA67}, \
    [ARM_IT_##_NAME##_RM_SHFT_IMM] = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_R3_SHIFT_SIMM, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0, cs:CS_U_5_7,    ss:SS_6_5, sz:_SZ, xs:_XS, tl:1, pi:PI_24, wb:WB_21, ua45:_UA45, ua67:_UA67}, \
    [ARM_IT_##_NAME##_RM_RRX]      = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_R3_SHIFT,      support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0, cs:CS_NA,       ss:SS_RRX, sz:_SZ, xs:_XS, tl:1, pi:PI_24, wb:WB_21, ua45:_UA45, ua67:_UA67}

//
// Attribute entries for instructions like LDRH
//
#define ATTR_SET_LDRH(_NAME, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [ARM_IT_##_NAME##_IMM] = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_SIMM, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:0,   r4:R_12P1, cs:CS_U_8_8_0_U, sz:_SZ, xs:_XS, pi:PI_24, wb:WB_21, ua45:_UA45, ua67:_UA67}, \
    [ARM_IT_##_NAME##_RM]  = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_R3,   support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0, r4:R_12P1, cs:CS_NA,        sz:_SZ, xs:_XS, pi:PI_24, wb:WB_21, ua45:_UA45, ua67:_UA67}

//
// Attribute entries for instructions like LDRHT
//
#define ATTR_SET_LDRHT(_NAME, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [ARM_IT_##_NAME##_IMM] = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_SIMM, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:0,   cs:CS_U_8_8_0_U, sz:_SZ, xs:_XS, tl:1, pi:PI_24, wb:WB_21, ua45:_UA45, ua67:_UA67}, \
    [ARM_IT_##_NAME##_RM]  = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_R3,   support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0, cs:CS_NA,        sz:_SZ, xs:_XS, tl:1, pi:PI_24, wb:WB_21, ua45:_UA45, ua67:_UA67}

//
// Attribute entries for instructions like SWP
//
#define ATTR_SET_SWP(_NAME, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_ADDR_R3_SIMM, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_0, r3:R_16, sz:_SZ, xs:_XS, ua45:_UA45, ua67:_UA67}

//
// Attribute entries for instructions like LDREX
//
#define ATTR_SET_LDREX(_NAME, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_ADDR_R2_SIMM, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, sz:_SZ, xs:_XS, ua45:_UA45, ua67:_UA67, ea:1}

//
// Attribute entries for instructions like STREX
//
#define ATTR_SET_STREX(_NAME, _SUPPORT, _ISAR, _OPCODE, _SZ, _XS, _UA45, _UA67) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_ADDR_R3_SIMM, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_0, r3:R_16, sz:_SZ, xs:_XS, ua45:_UA45, ua67:_UA67, ea:1}

//
// Attribute entries for instructions like CDP
//
#define ATTR_SET_CDP(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_CPNUM_COP1_CR1_CR2_CR3_COP2, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0, cpNum:1, cpOp1:COP_4_20, cpOp2:1}

//
// Attribute entries for instructions like CDP2
//
#define ATTR_SET_CDP2(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_CPNUM_COP1_CR1_CR2_CR3_COP2, support:_SUPPORT, isar:_ISAR, r1:R_12, r2:R_16, r3:R_0, cpNum:1, cpOp1:COP_4_20, cpOp2:1}

//
// Attribute entries for instructions like LDC
//
#define ATTR_SET_LDC(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_IMM]       = {opcode:_OPCODE, format:FMT_CPNUM_CR1_SIMM,      support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, cs:CS_U_8_0x4_U, pi:PI_24, wb:WB_21, ll:1, cpNum:1}, \
    [ARM_IT_##_NAME##_UNINDEXED] = {opcode:_OPCODE, format:FMT_CPNUM_CR1_UNINDEXED, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, cs:CS_U_8_0,     pi:PI_24, wb:WB_21, ll:1, cpNum:1}

//
// Attribute entries for instructions like LDC2
//
#define ATTR_SET_LDC2(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_IMM]       = {opcode:_OPCODE, format:FMT_CPNUM_CR1_SIMM,      support:_SUPPORT, isar:_ISAR, r1:R_12, r2:R_16, cs:CS_U_8_0x4_U, pi:PI_24, wb:WB_21, ll:1, cpNum:1}, \
    [ARM_IT_##_NAME##_UNINDEXED] = {opcode:_OPCODE, format:FMT_CPNUM_CR1_UNINDEXED, support:_SUPPORT, isar:_ISAR, r1:R_12, r2:R_16, cs:CS_U_8_0,     pi:PI_24, wb:WB_21, ll:1, cpNum:1}

//
// Attribute entries for instructions like MCR
//
#define ATTR_SET_MCR(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_CPNUM_COP1_R1_CR2_CR3_COP2, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0, cpNum:1, cpOp1:COP_3_21, cpOp2:1}

//
// Attribute entries for instructions like MCR2
//
#define ATTR_SET_MCR2(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_CPNUM_COP1_R1_CR2_CR3_COP2, support:_SUPPORT, isar:_ISAR, r1:R_12, r2:R_16, r3:R_0, cpNum:1, cpOp1:COP_3_21, cpOp2:1}

//
// Attribute entries for instructions like MRC
//
#define ATTR_SET_MRC(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_CPNUM_COP1_R1F_CR2_CR3_COP2, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0, cpNum:1, cpOp1:COP_3_21, cpOp2:1}

//
// Attribute entries for instructions like MRC2
//
#define ATTR_SET_MRC2(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_CPNUM_COP1_R1F_CR2_CR3_COP2, support:_SUPPORT, isar:_ISAR, r1:R_12, r2:R_16, r3:R_0, cpNum:1, cpOp1:COP_3_21, cpOp2:1}

//
// Attribute entries for instructions like MRS
//
#define ATTR_SET_MRS(_NAME, _SUPPORT, _ISAR, _OPCODE, _REG) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_COMMA _REG, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12}

//
// Attribute entries for instructions like MSR
//
#define ATTR_SET_MSR(_NAME, _SUPPORT, _ISAR, _OPCODE, _REG) \
    [ARM_IT_##_NAME##_IMM] = {opcode:_OPCODE, format:_REG FMT_FIELDSR1_SIMM, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:0,   cs:CS_U_8_0_R}, \
    [ARM_IT_##_NAME##_RM]  = {opcode:_OPCODE, format:_REG FMT_FIELDSR1_R2,   support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, cs:CS_NA     }

//
// Attribute entries for instructions like SETEND
//
#define ATTR_SET_SETEND(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_ENDIAN, support:_SUPPORT, isar:_ISAR, cs:CS_U_9}

//
// Attribute entries for instructions like NOP (conditional)
//
#define ATTR_SET_NOP_C(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_NONE, support:_SUPPORT, isar:_ISAR, cond:1}

//
// Attribute entries for instructions like NOP (unconditional)
//
#define ATTR_SET_NOP_UC(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_NONE, support:_SUPPORT, isar:_ISAR}

//
// Attribute entries for instructions like DBG
//
#define ATTR_SET_DBG(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_UIMM, support:_SUPPORT, isar:_ISAR, cond:1, cs:CS_U_4_0}

//
// Attribute entries for instructions like SRS
//
#define ATTR_SET_SRS(_NAME, _SUPPORT, _ISAR, _OPCODE, _UA45, _UA67) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_WB_UIMM, support:_SUPPORT, isar:_ISAR, r1:R_SP, cs:CS_U_5_0, wb:WB_21, incDec:ID_U_P, ua45:_UA45, ua67:_UA67}

//
// Attribute entries for instructions like RFE
//
#define ATTR_SET_RFE(_NAME, _SUPPORT, _ISAR, _OPCODE, _UA45, _UA67) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_WB, support:_SUPPORT, isar:_ISAR, r1:R_16, wb:WB_21, incDec:ID_U_P, ua45:_UA45, ua67:_UA67}

//
// Attribute entries for instructions like CPS
//
#define ATTR_SET_CPS(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_FLAGS_OPT_MODE, support:_SUPPORT, isar:_ISAR, cs:CS_U_5_0, imod:1, m:1, aif:1}

//
// Attribute entries for instructions like DMB
//
#define ATTR_SET_DMB(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_LIM, support:_SUPPORT, isar:_ISAR, cs:CS_U_4_0}

//
// Attribute entries for DSP instructions like QADD
//
#define ATTR_SET_QADD(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_0, r3:R_16}

//
// Attribute entries for DSP instructions like SMLA<x><y>
//
#define ATTR_SET_SMLA_XY(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##BB] = {opcode:_OPCODE"bb", format:FMT_R1_R2_R3_R4, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8, r4:R_12}, \
    [ARM_IT_##_NAME##BT] = {opcode:_OPCODE"bt", format:FMT_R1_R2_R3_R4, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8, r4:R_12}, \
    [ARM_IT_##_NAME##TB] = {opcode:_OPCODE"tb", format:FMT_R1_R2_R3_R4, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8, r4:R_12}, \
    [ARM_IT_##_NAME##TT] = {opcode:_OPCODE"tt", format:FMT_R1_R2_R3_R4, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8, r4:R_12}

//
// Attribute entries for DSP instructions like SMLAL<x><y>
//
#define ATTR_SET_SMLAL_XY(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##BB] = {opcode:_OPCODE"bb", format:FMT_R1_R2_R3_R4, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0, r4:R_8}, \
    [ARM_IT_##_NAME##BT] = {opcode:_OPCODE"bt", format:FMT_R1_R2_R3_R4, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0, r4:R_8}, \
    [ARM_IT_##_NAME##TB] = {opcode:_OPCODE"tb", format:FMT_R1_R2_R3_R4, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0, r4:R_8}, \
    [ARM_IT_##_NAME##TT] = {opcode:_OPCODE"tt", format:FMT_R1_R2_R3_R4, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0, r4:R_8}

//
// Attribute entries for DSP instructions like SMLAW<y>
//
#define ATTR_SET_SMLAW_Y(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##B] = {opcode:_OPCODE"b", format:FMT_R1_R2_R3_R4, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8, r4:R_12}, \
    [ARM_IT_##_NAME##T] = {opcode:_OPCODE"t", format:FMT_R1_R2_R3_R4, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8, r4:R_12}

//
// Attribute entries for DSP instructions like SMUL<x><y>
//
#define ATTR_SET_SMUL_XY(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##BB] = {opcode:_OPCODE"bb", format:FMT_R1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8}, \
    [ARM_IT_##_NAME##BT] = {opcode:_OPCODE"bt", format:FMT_R1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8}, \
    [ARM_IT_##_NAME##TB] = {opcode:_OPCODE"tb", format:FMT_R1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8}, \
    [ARM_IT_##_NAME##TT] = {opcode:_OPCODE"tt", format:FMT_R1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8}

//
// Attribute entries for DSP instructions like SMULW<y>
//
#define ATTR_SET_SMULW_Y(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##B] = {opcode:_OPCODE"b", format:FMT_R1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8}, \
    [ARM_IT_##_NAME##T] = {opcode:_OPCODE"t", format:FMT_R1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8}

//
// Attribute entries for instructions like PLD
//
#define ATTR_SET_PLD(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_IMM]         = {opcode:_OPCODE, format:FMT_ADDR_R1_SIMM,          support:_SUPPORT, isar:_ISAR, r1:R_16, r2:0,   cs:CS_U_12_0_U, ss:SS_NA }, \
    [ARM_IT_##_NAME##_RM]          = {opcode:_OPCODE, format:FMT_ADDR_R1_R2,            support:_SUPPORT, isar:_ISAR, r1:R_16, r2:R_0, cs:CS_NA,       ss:SS_NA }, \
    [ARM_IT_##_NAME##_RM_SHFT_IMM] = {opcode:_OPCODE, format:FMT_ADDR_R1_R2_SHIFT_SIMM, support:_SUPPORT, isar:_ISAR, r1:R_16, r2:R_0, cs:CS_U_5_7,    ss:SS_6_5}, \
    [ARM_IT_##_NAME##_RM_RRX]      = {opcode:_OPCODE, format:FMT_ADDR_R1_R2_SHIFT,      support:_SUPPORT, isar:_ISAR, r1:R_16, r2:R_0, cs:CS_NA,       ss:SS_RRX}

//
// Attribute entries for unallocated memory hints
//
#define ATTR_SET_UHINT(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_NONE, support:_SUPPORT, isar:_ISAR}

//
// Attribute entries for instructions like MCRR
//
#define ATTR_SET_MCRR(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_CPNUM_COP1_R1_R2_CR3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0, cpNum:1, cpOp1:COP_4_4}

//
// Attribute entries for instructions like MCRR2
//
#define ATTR_SET_MCRR2(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_CPNUM_COP1_R1_R2_CR3, support:_SUPPORT, isar:_ISAR, r1:R_12, r2:R_16, r3:R_0, cpNum:1, cpOp1:COP_4_4}

//
// Attribute entries for instructions like USAD8
//
#define ATTR_SET_USAD8(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8}

//
// Attribute entries for instructions like USADA8
//
#define ATTR_SET_USADA8(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_R3_R4, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8, r4:R_12}

//
// Attribute entries for instructions like SBFX
//
#define ATTR_SET_SBFX(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_LSB_WIDTH, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_0, cs:CS_U_5_7, w:WS_WIDTHM1}

//
// Attribute entries for instructions like BFC
//
#define ATTR_SET_BFC(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_LSB_WIDTH, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, cs:CS_U_5_7, w:WS_MSB}

//
// Attribute entries for instructions like BFI
//
#define ATTR_SET_BFI(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_LSB_WIDTH, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_0, cs:CS_U_5_7, w:WS_MSB}

//
// Attribute entries for parallel add/subtract instructions
//
#define ATTR_SET_PAS(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_S##_NAME]  = {opcode:"s" _OPCODE, format:FMT_R1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0}, \
    [ARM_IT_Q##_NAME]  = {opcode:"q" _OPCODE, format:FMT_R1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0}, \
    [ARM_IT_SH##_NAME] = {opcode:"sh"_OPCODE, format:FMT_R1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0}, \
    [ARM_IT_U##_NAME]  = {opcode:"u" _OPCODE, format:FMT_R1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0}, \
    [ARM_IT_UQ##_NAME] = {opcode:"uq"_OPCODE, format:FMT_R1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0}, \
    [ARM_IT_UH##_NAME] = {opcode:"uh"_OPCODE, format:FMT_R1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0}

//
// Attribute entries for instructions like PKHBT
//
#define ATTR_SET_PKHBT(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_R3_SHIFT_SIMM, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0, cs:CS_U_5_7, ss:SS_6_5}

//
// Attribute entries for instructions like SSAT
//
#define ATTR_SET_SSAT(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_WIDTH_R2_SHIFT_SIMM, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_0, cs:CS_U_5_7, ss:SS_6_5, w:WS_WIDTHM1}

//
// Attribute entries for instructions like USAT
//
#define ATTR_SET_USAT(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_WIDTH_R2_SHIFT_SIMM, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_0, cs:CS_U_5_7, ss:SS_6_5, w:WS_WIDTH}

//
// Attribute entries for instructions like SXTAB16
//
#define ATTR_SET_SXTAB16(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_R3_SHIFT_SIMM, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0, cs:CS_U_2_10x8, ss:SS_6_5}

//
// Attribute entries for instructions like SXTB16
//
#define ATTR_SET_SXTB16(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_SHIFT_SIMM, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_0, cs:CS_U_2_10x8, ss:SS_6_5}

//
// Attribute entries for instructions like SEL
//
#define ATTR_SET_SEL(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0}

//
// Attribute entries for instructions like SSAT16
//
#define ATTR_SET_SSAT16(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_WIDTH_R2, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_0, w:WS_WIDTHM1}

//
// Attribute entries for instructions like USAT16
//
#define ATTR_SET_USAT16(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_WIDTH_R2, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_0, w:WS_WIDTH}

//
// Attribute entries for instructions like SMLAD
//
#define ATTR_SET_SMLAD(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME]    = {opcode:_OPCODE,    format:FMT_R1_R2_R3_R4, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8, r4:R_12}, \
    [ARM_IT_##_NAME##X] = {opcode:_OPCODE"x", format:FMT_R1_R2_R3_R4, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8, r4:R_12}

//
// Attribute entries for instructions like SMUAD
//
#define ATTR_SET_SMUAD(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME]    = {opcode:_OPCODE,    format:FMT_R1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8}, \
    [ARM_IT_##_NAME##X] = {opcode:_OPCODE"x", format:FMT_R1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8}

//
// Attribute entries for instructions like SMLALD
//
#define ATTR_SET_SMLALD(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME]    = {opcode:_OPCODE,    format:FMT_R1_R2_R3_R4, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0, r4:R_8}, \
    [ARM_IT_##_NAME##X] = {opcode:_OPCODE"x", format:FMT_R1_R2_R3_R4, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:R_0, r4:R_8}

//
// Attribute entries for instructions like SMMLA
//
#define ATTR_SET_SMMLA(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME]    = {opcode:_OPCODE,    format:FMT_R1_R2_R3_R4, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8, r4:R_12}, \
    [ARM_IT_##_NAME##R] = {opcode:_OPCODE"r", format:FMT_R1_R2_R3_R4, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8, r4:R_12}

//
// Attribute entries for instructions like SMMUL
//
#define ATTR_SET_SMMUL(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME]    = {opcode:_OPCODE,    format:FMT_R1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8}, \
    [ARM_IT_##_NAME##R] = {opcode:_OPCODE"r", format:FMT_R1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_0, r3:R_8}

//
// Attribute entries for instructions like VMRS
//
#define ATTR_SET_VMRS(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1F_VESR2, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16}

//
// Attribute entries for instructions like VMSR
//
#define ATTR_SET_VMSR(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_VESR1_R2, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:R_12}

//
// Attribute entries for instructions like VMOVRS
//
#define ATTR_SET_VMOVRS(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_S2, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:V_16_7}

//
// Attribute entries for instructions like VMOVSR
//
#define ATTR_SET_VMOVSR(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_S1_R2, support:_SUPPORT, isar:_ISAR, cond:1, r1:V_16_7, r2:R_12}

//
// Attribute entries for instructions VDUPR
//
#define ATTR_SET_VDUPR(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_W_D] = {opcode:_OPCODE, format:FMT_D1_R2, support:_SUPPORT, isar:_ISAR, cond:1, r1:V_7_16, r2:R_12, dt1:ARM_SDFPT_32}, \
    [ARM_IT_##_NAME##_H_D] = {opcode:_OPCODE, format:FMT_D1_R2, support:_SUPPORT, isar:_ISAR, cond:1, r1:V_7_16, r2:R_12, dt1:ARM_SDFPT_16}, \
    [ARM_IT_##_NAME##_B_D] = {opcode:_OPCODE, format:FMT_D1_R2, support:_SUPPORT, isar:_ISAR, cond:1, r1:V_7_16, r2:R_12, dt1:ARM_SDFPT_8}, \
    [ARM_IT_##_NAME##_W_Q] = {opcode:_OPCODE, format:FMT_Q1_R2, support:_SUPPORT, isar:_ISAR, cond:1, r1:V_7_16, r2:R_12, dt1:ARM_SDFPT_32}, \
    [ARM_IT_##_NAME##_H_Q] = {opcode:_OPCODE, format:FMT_Q1_R2, support:_SUPPORT, isar:_ISAR, cond:1, r1:V_7_16, r2:R_12, dt1:ARM_SDFPT_16}, \
    [ARM_IT_##_NAME##_B_Q] = {opcode:_OPCODE, format:FMT_Q1_R2, support:_SUPPORT, isar:_ISAR, cond:1, r1:V_7_16, r2:R_12, dt1:ARM_SDFPT_8}

//
// Attribute entries for instructions VMOVZR
//
#define ATTR_SET_VMOVZR(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_W] = {opcode:_OPCODE, format:FMT_Z1_R2, support:_SUPPORT, isar:_ISAR, cond:1, r1:V_7_16, r2:R_12, index:IDX_21,     dt1:ARM_SDFPT_32}, \
    [ARM_IT_##_NAME##_H] = {opcode:_OPCODE, format:FMT_Z1_R2, support:_SUPPORT, isar:_ISAR, cond:1, r1:V_7_16, r2:R_12, index:IDX_21_6,   dt1:ARM_SDFPT_16}, \
    [ARM_IT_##_NAME##_B] = {opcode:_OPCODE, format:FMT_Z1_R2, support:_SUPPORT, isar:_ISAR, cond:1, r1:V_7_16, r2:R_12, index:IDX_21_6_5, dt1:ARM_SDFPT_8}

//
// Attribute entries for instructions VMOVRZ
//
#define ATTR_SET_VMOVRZ(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_W ] = {opcode:_OPCODE, format:FMT_R1_Z2, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:V_7_16, index:IDX_21,     dt1:ARM_SDFPT_32}, \
    [ARM_IT_##_NAME##S_H] = {opcode:_OPCODE, format:FMT_R1_Z2, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:V_7_16, index:IDX_21_6,   dt1:ARM_SDFPT_S16}, \
    [ARM_IT_##_NAME##U_H] = {opcode:_OPCODE, format:FMT_R1_Z2, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:V_7_16, index:IDX_21_6,   dt1:ARM_SDFPT_U16}, \
    [ARM_IT_##_NAME##S_B] = {opcode:_OPCODE, format:FMT_R1_Z2, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:V_7_16, index:IDX_21_6_5, dt1:ARM_SDFPT_S8}, \
    [ARM_IT_##_NAME##U_B] = {opcode:_OPCODE, format:FMT_R1_Z2, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:V_7_16, index:IDX_21_6_5, dt1:ARM_SDFPT_U8}

//
// Attribute entries for instructions like VMOVRRD
//
#define ATTR_SET_VMOVRRD(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_D3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:V_5_0}

//
// Attribute entries for instructions like VMOVDRR
//
#define ATTR_SET_VMOVDRR(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_D1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:V_5_0, r2:R_12, r3:R_16}

//
// Attribute entries for instructions like VMOVRRSS
//
#define ATTR_SET_VMOVRRSS(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_R1_R2_SS3, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_12, r2:R_16, r3:V_0_5}

//
// Attribute entries for instructions like VMOVSSRR
//
#define ATTR_SET_VMOVSSRR(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_SS1_R2_R3, support:_SUPPORT, isar:_ISAR, cond:1, r1:V_0_5, r2:R_12, r3:R_16}

//
// Attribute entries for SIMD data processing instruction VEXT:  3 regs same length:
//   regs in D, N, M order   Q/D versions   No esize                dt1 qualifier is 8  immediate value
//
#define ATTR_SET_VEXT(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_Q3_UIMM, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_8, r1:V_22_12, r2:V_7_16, r3:V_5_0, cs:CS_U_4_8}, \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_D1_D2_D3_UIMM, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_8, r1:V_22_12, r2:V_7_16, r3:V_5_0, cs:CS_U_4_8}

//
// Attribute entries for SIMD data processing instruction VTBL and VTBLX:  3 regs same length:
//   regs in D, N, M order   D version only   No esize               dt1 qualifier is 8  immediate value
//
#define ATTR_SET_VTBL(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_D1_RL_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_8, r1:V_22_12, r2:V_7_16, r3:V_5_0, nregs:NREG_2_8_P1}

//
// Attribute entries for SIMD data processing instruction VDUP:  Register and a scalar
//   regs in D, N, M order   Q/D versions   Esize 8, 16 or 32        dt1 qualifier matches size  Scalar index encoded with size
//
#define ATTR_SET_VDUPZ(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_W_Q] = {opcode:_OPCODE, format:FMT_Q1_Z2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_32, r1:V_22_12, r2:V_5_0, index:IDX_19}, \
    [ARM_IT_##_NAME##_W_D] = {opcode:_OPCODE, format:FMT_D1_Z2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_32, r1:V_22_12, r2:V_5_0, index:IDX_19}, \
    [ARM_IT_##_NAME##_H_Q] = {opcode:_OPCODE, format:FMT_Q1_Z2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_16, r1:V_22_12, r2:V_5_0, index:IDX_19_18}, \
    [ARM_IT_##_NAME##_H_D] = {opcode:_OPCODE, format:FMT_D1_Z2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_16, r1:V_22_12, r2:V_5_0, index:IDX_19_18}, \
    [ARM_IT_##_NAME##_B_Q] = {opcode:_OPCODE, format:FMT_Q1_Z2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_8,  r1:V_22_12, r2:V_5_0, index:IDX_19_18_17}, \
    [ARM_IT_##_NAME##_B_D] = {opcode:_OPCODE, format:FMT_D1_Z2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_8,  r1:V_22_12, r2:V_5_0, index:IDX_19_18_17}

//
// Attribute entries for SIMD data processing instructions w/ 3 regs same length:
//   regs in D, N, M order   Q/D versions   No esize                No dt1
//
#define ATTR_SET_SIMD_RRR_QD(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_Q3, support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_D1_D2_D3, support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:V_7_16, r3:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 3 regs same length:
//   regs in D, N, M order   D vers no Q    No esize                dt1 qualifier specified
//
#define ATTR_SET_SIMD_RRR_D_TS(_NAME, _SUPPORT, _ISAR, _OPCODE, _TS) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_D1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_TS, r1:V_22_12, r2:V_7_16, r3:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 3 regs same length:
//   regs in D, N, M order   Q/D versions   No esize                dt1 qualifier specified
//
#define ATTR_SET_SIMD_RRR_QD_TS(_NAME, _SUPPORT, _ISAR, _OPCODE, _TS) \
    [ARM_IT_##_NAME##_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_Q3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_TS, r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_D1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_TS, r1:V_22_12, r2:V_7_16, r3:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 3 regs same length:
//   regs in D, N, M order   Q/D versions   Esize 8, 16 or 32       dt1 qualifier specified
//
#define ATTR_SET_SIMD_RRR_QD_BHW_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_W_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_Q3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_H_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_Q3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_B_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_Q3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##8,  r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_W_D] = {opcode:_OPCODE, format:FMT_D1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_H_D] = {opcode:_OPCODE, format:FMT_D1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_B_D] = {opcode:_OPCODE, format:FMT_D1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##8,  r1:V_22_12, r2:V_7_16, r3:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 3 regs same length:
//   regs in D, N, M order   Q/D versions   Esize 16 or 32          dt1 qualifier specified
//
#define ATTR_SET_SIMD_RRR_QD_HW_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_W_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_Q3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_H_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_Q3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_W_D] = {opcode:_OPCODE, format:FMT_D1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_H_D] = {opcode:_OPCODE, format:FMT_D1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_7_16, r3:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 3 regs same length:
//   regs in D, N, M order   D only, no Q   Esize 8, 16 or 32       dt1 qualifier specified
//
#define ATTR_SET_SIMD_RRR_D_BHW_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_W] = {opcode:_OPCODE, format:FMT_D1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_H] = {opcode:_OPCODE, format:FMT_D1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_B] = {opcode:_OPCODE, format:FMT_D1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##8,  r1:V_22_12, r2:V_7_16, r3:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 3 regs same length:
//   regs in D, N, M order   Q/D versions   Esize 8, 16, 32 or 64   dt1 qualifier specified
//
#define ATTR_SET_SIMD_RRR_QD_BHWD_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
	ATTR_SET_SIMD_RRR_QD_BHW_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T), \
    [ARM_IT_##_NAME##_D_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_Q3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##64, r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_D_D] = {opcode:_OPCODE, format:FMT_D1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##64, r1:V_22_12, r2:V_7_16, r3:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 3 regs same length:
//   regs in D, M, N order   Q/D versions   Esize 8, 16 or 32       dt1 qualifier specified
//
#define ATTR_SET_SIMD_DMN_QD_BHW_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_W_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_Q3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_5_0, r3:V_7_16}, \
    [ARM_IT_##_NAME##_H_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_Q3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_5_0, r3:V_7_16}, \
    [ARM_IT_##_NAME##_B_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_Q3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##8,  r1:V_22_12, r2:V_5_0, r3:V_7_16}, \
    [ARM_IT_##_NAME##_W_D] = {opcode:_OPCODE, format:FMT_D1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_5_0, r3:V_7_16}, \
    [ARM_IT_##_NAME##_H_D] = {opcode:_OPCODE, format:FMT_D1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_5_0, r3:V_7_16}, \
    [ARM_IT_##_NAME##_B_D] = {opcode:_OPCODE, format:FMT_D1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##8,  r1:V_22_12, r2:V_5_0, r3:V_7_16}

//
// Attribute entries for SIMD data processing instructions w/ 3 regs same length:
//   regs in D, M, N order   Q/D versions   Esize 8, 16, 32 or 64   dt1 qualifier specified
//
#define ATTR_SET_SIMD_DMN_QD_BHWD_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
	ATTR_SET_SIMD_DMN_QD_BHW_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T), \
    [ARM_IT_##_NAME##_D_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_Q3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##64, r1:V_22_12, r2:V_5_0, r3:V_7_16}, \
    [ARM_IT_##_NAME##_D_D] = {opcode:_OPCODE, format:FMT_D1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##64, r1:V_22_12, r2:V_5_0, r3:V_7_16}

//
// Polynomial instruction:
// Attribute entries for SIMD data processing instructions w/ 3 regs different length:
//   Long = QDD     No Esize             dt1 qualifier is P8
//
#define ATTR_SET_SIMD_L_P(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_Q1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_P8, r1:V_22_12, r2:V_7_16, r3:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 3 regs different length:
//   Long = QDD     Esize 16 or 32       dt1 qualifier specified
//
#define ATTR_SET_SIMD_L_HW_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_W] = {opcode:_OPCODE, format:FMT_Q1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_H] = {opcode:_OPCODE, format:FMT_Q1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_7_16, r3:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 3 regs different length:
//   Long = QDD     Esize 8, 16 or 32       dt1 qualifier specified
//
#define ATTR_SET_SIMD_L_BHW_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_W] = {opcode:_OPCODE, format:FMT_Q1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_H] = {opcode:_OPCODE, format:FMT_Q1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_B] = {opcode:_OPCODE, format:FMT_Q1_D2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##8,  r1:V_22_12, r2:V_7_16, r3:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 3 regs different length:
//   Wide = QQD     Esize 8, 16 or 32       dt1 qualifier specified
//
#define ATTR_SET_SIMD_W_BHW_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_W] = {opcode:_OPCODE, format:FMT_Q1_Q2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_H] = {opcode:_OPCODE, format:FMT_Q1_Q2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_B] = {opcode:_OPCODE, format:FMT_Q1_Q2_D3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##8,  r1:V_22_12, r2:V_7_16, r3:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 3 regs different length:
//   Narrow = DQQ   Esize 16, 32 or 64      dt1 qualifier specified
//
#define ATTR_SET_SIMD_N_HWD_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_D1_Q2_Q3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##64, r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_W] = {opcode:_OPCODE, format:FMT_D1_Q2_Q3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_H] = {opcode:_OPCODE, format:FMT_D1_Q2_Q3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_7_16, r3:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 2 regs same length and a scalar:
//   regs RRZ, scalar index uses top of r3  Q/D versions   Esize  32 only          dt1 qualifier specified
//
#define ATTR_SET_SIMD_RRZ_QD_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_Z3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_7_16, r3:R_0, index:IDX_5}, \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_D1_D2_Z3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_7_16, r3:R_0, index:IDX_5}

//
// Attribute entries for SIMD data processing instructions w/ 2 regs same length and a scalar:
//   regs RRZ, scalar index uses top of r3  Q/D versions   Esize 16 or 32          dt1 qualifier specified
//
#define ATTR_SET_SIMD_RRZ_QD_HW_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_W_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_Z3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_7_16, r3:R_0,  index:IDX_5}, \
    [ARM_IT_##_NAME##_H_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_Z3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_7_16, r3:V3_0, index:IDX_5_3}, \
    [ARM_IT_##_NAME##_W_D] = {opcode:_OPCODE, format:FMT_D1_D2_Z3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_7_16, r3:R_0,  index:IDX_5}, \
    [ARM_IT_##_NAME##_H_D] = {opcode:_OPCODE, format:FMT_D1_D2_Z3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_7_16, r3:V3_0, index:IDX_5_3}

//
// Attribute entries for SIMD data processing instructions w/ 2 regs same length and a scalar:
//   regs QDZ, scalar index uses top of r3  Q/D versions   Esize 16 or 32          dt1 qualifier specified
//
#define ATTR_SET_SIMD_LZ_HW_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_W] = {opcode:_OPCODE, format:FMT_Q1_D2_Z3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_7_16, r3:R_0,  index:IDX_5}, \
    [ARM_IT_##_NAME##_H] = {opcode:_OPCODE, format:FMT_Q1_D2_Z3, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_7_16, r3:V3_0, index:IDX_5_3}

//
// Attribute entries for SIMD data processing instructions w/ 2 regs same length and shift amount:
//   regs RRI Q/D versions   Esize 8, 16, 32 or 64 (immediate and esize encoded together)  dt1 qualifier specified   Imm invert specified
//
#define ATTR_SET_SIMD_RRI_QD_BHWD_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T, _I) \
    [ARM_IT_##_NAME##_D_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_UIMM, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##64, r1:V_22_12, r2:V_5_0, cs:CS_U_6_16##_I}, \
    [ARM_IT_##_NAME##_D_D] = {opcode:_OPCODE, format:FMT_D1_D2_UIMM, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##64, r1:V_22_12, r2:V_5_0, cs:CS_U_6_16##_I}, \
    [ARM_IT_##_NAME##_W_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_UIMM, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_5_0, cs:CS_U_5_16##_I}, \
    [ARM_IT_##_NAME##_W_D] = {opcode:_OPCODE, format:FMT_D1_D2_UIMM, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_5_0, cs:CS_U_5_16##_I}, \
    [ARM_IT_##_NAME##_H_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_UIMM, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_5_0, cs:CS_U_4_16##_I}, \
    [ARM_IT_##_NAME##_H_D] = {opcode:_OPCODE, format:FMT_D1_D2_UIMM, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_5_0, cs:CS_U_4_16##_I}, \
    [ARM_IT_##_NAME##_B_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_UIMM, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##8,  r1:V_22_12, r2:V_5_0, cs:CS_U_3_16##_I}, \
    [ARM_IT_##_NAME##_B_D] = {opcode:_OPCODE, format:FMT_D1_D2_UIMM, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##8,  r1:V_22_12, r2:V_5_0, cs:CS_U_3_16##_I}

//
// Attribute entries for SIMD data processing instructions w/ 2 regs same length and shift amount:
//   regs RRI Q/D versions   Esize 32 only   2 dt1 qualifiers specified
//
#define ATTR_SET_SIMD_RRI_QD_W_T2(_NAME, _SUPPORT, _ISAR, _OPCODE, _T1, _T2) \
    [ARM_IT_##_NAME##_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_UIMM, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T1##32, dt2:ARM_SDFPT##_T2##32, r1:V_22_12, r2:V_5_0, cs:CS_U_6_16N}, \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_D1_D2_UIMM, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T1##32, dt2:ARM_SDFPT##_T2##32, r1:V_22_12, r2:V_5_0, cs:CS_U_6_16N}

//
// Attribute entries for SIMD data processing instructions w/ 2 regs different length and shift amount:
//   Long = QDI            Esize 8, 16, or 32 (immediate and esize encoded together)   dt1 qualifier specified
//
#define ATTR_SET_SIMD_LI_BHW_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_W] = {opcode:_OPCODE, format:FMT_Q1_D2_UIMM, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_5_0, cs:CS_U_5_16P}, \
    [ARM_IT_##_NAME##_H] = {opcode:_OPCODE, format:FMT_Q1_D2_UIMM, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_5_0, cs:CS_U_4_16P}, \
    [ARM_IT_##_NAME##_B] = {opcode:_OPCODE, format:FMT_Q1_D2_UIMM, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##8,  r1:V_22_12, r2:V_5_0, cs:CS_U_3_16P}

//
// Attribute entries for SIMD data processing instruction VMOVL - special case of VSHLL with imm=0
//   Long = QD             Esize 8, 16, or 32    dt1 qualifier specified    no constant value
//
#define ATTR_SET_SIMD_VMOVL(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_W] = {opcode:_OPCODE, format:FMT_Q1_D2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_H] = {opcode:_OPCODE, format:FMT_Q1_D2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_B] = {opcode:_OPCODE, format:FMT_Q1_D2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##8,  r1:V_22_12, r2:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 2 regs different length and shift amount:
//   Narrow = DQI            Esize 16, 32 or 64 (immediate and esize encoded together)   dt1 qualifier specified
//
#define ATTR_SET_SIMD_NI_HWD_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_D1_Q2_UIMM, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##64, r1:V_22_12, r2:V_5_0, cs:CS_U_5_16N}, \
    [ARM_IT_##_NAME##_W] = {opcode:_OPCODE, format:FMT_D1_Q2_UIMM, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_5_0, cs:CS_U_4_16N}, \
    [ARM_IT_##_NAME##_H] = {opcode:_OPCODE, format:FMT_D1_Q2_UIMM, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_5_0, cs:CS_U_3_16N}

//
// Attribute entries for SIMD data processing instructions vrev16, vrev32 and vrev64
//   regs RR Q/D versions   Esize 8, 16 or 32 (Esize must be < region size)   no dt1 qualifier
//
#define ATTR_SET_SIMD_VREV(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##16_B_Q] = {opcode:_OPCODE"16", format:FMT_Q1_Q2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_8,  r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##16_B_D] = {opcode:_OPCODE"16", format:FMT_D1_D2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_8,  r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##32_B_Q] = {opcode:_OPCODE"32", format:FMT_Q1_Q2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_8,  r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##32_B_D] = {opcode:_OPCODE"32", format:FMT_D1_D2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_8,  r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##32_H_Q] = {opcode:_OPCODE"32", format:FMT_Q1_Q2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_16, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##32_H_D] = {opcode:_OPCODE"32", format:FMT_D1_D2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_16, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##64_B_Q] = {opcode:_OPCODE"64", format:FMT_Q1_Q2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_8,  r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##64_B_D] = {opcode:_OPCODE"64", format:FMT_D1_D2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_8,  r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##64_H_Q] = {opcode:_OPCODE"64", format:FMT_Q1_Q2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_16, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##64_H_D] = {opcode:_OPCODE"64", format:FMT_D1_D2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_16, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##64_W_Q] = {opcode:_OPCODE"64", format:FMT_Q1_Q2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_32, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##64_W_D] = {opcode:_OPCODE"64", format:FMT_D1_D2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_32, r1:V_22_12, r2:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 2 regs same length:
//   regs RR  Q/D versions   Esize 8, 16 or 32   dt1 qualifier specified
//
#define ATTR_SET_SIMD_RR_QD_BHW_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_W_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_W_D] = {opcode:_OPCODE, format:FMT_D1_D2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_H_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_H_D] = {opcode:_OPCODE, format:FMT_D1_D2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_B_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##8,  r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_B_D] = {opcode:_OPCODE, format:FMT_D1_D2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##8,  r1:V_22_12, r2:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 2 regs different lengths and a constant immediate:
//   Long QDI     Esize 8, 16 or 32   dt1 qualifier specified
//
#define ATTR_SET_SIMD_L2_BHW_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_W] = {opcode:_OPCODE, format:FMT_Q1_D2_U32, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_H] = {opcode:_OPCODE, format:FMT_Q1_D2_U16, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_B] = {opcode:_OPCODE, format:FMT_Q1_D2_U8,  support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##8,  r1:V_22_12, r2:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 2 regs different lengths:
//   Narrow DQ     Esize 16, 32 or 64   dt1 qualifier specified
//
#define ATTR_SET_SIMD_N2_HWD_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_D1_Q2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##64, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_W] = {opcode:_OPCODE, format:FMT_D1_Q2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_H] = {opcode:_OPCODE, format:FMT_D1_Q2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 2 regs different lengths:
//   Long QD       Single unspecified esize   2 dt1 sizes specified
//
#define ATTR_SET_SIMD_L2_S2(_NAME, _SUPPORT, _ISAR, _OPCODE, _S1, _S2) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_Q1_D2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_F##_S1, dt2:ARM_SDFPT_F##_S2, r1:V_22_12, r2:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 2 regs different lengths:
//   Narrow DQ     Single unspecified esize   2 dt1 sizes specified
//
#define ATTR_SET_SIMD_N2_S2(_NAME, _SUPPORT, _ISAR, _OPCODE, _S1, _S2) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_D1_Q2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT_F##_S1, dt2:ARM_SDFPT_F##_S2, r1:V_22_12, r2:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 2 regs same length:
//   regs RR  Q version for Esize 8, 16 or 32, D version for Esize 8 or 16   dt1 qualifier specified
//
#define ATTR_SET_SIMD_RR_QBHW_DBH_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_W_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_H_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_H_D] = {opcode:_OPCODE, format:FMT_D1_D2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_B_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##8,  r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_B_D] = {opcode:_OPCODE, format:FMT_D1_D2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##8,  r1:V_22_12, r2:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 2 regs same length and an implied immediate 0 value
//   regs RR0  Q/D versions   Esize 8, 16 or 32   dt1 qualifier specified
//
#define ATTR_SET_SIMD_RR0_QD_BHW_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_W_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_U0, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_W_D] = {opcode:_OPCODE, format:FMT_D1_D2_U0, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_H_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_U0, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_H_D] = {opcode:_OPCODE, format:FMT_D1_D2_U0, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##16, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_B_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_U0, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##8,  r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_B_D] = {opcode:_OPCODE, format:FMT_D1_D2_U0, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##8,  r1:V_22_12, r2:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 2 regs same length and an implied immediate 0 value
//   regs RR0  Q/D versions   Single unspecified esize   dt1 qualifier specified
//
#define ATTR_SET_SIMD_RR0_QD_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2_U0, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_D1_D2_U0, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 2 regs same length:
//   regs RR  Q/D versions   Single unspecified esize    dt1 qualifier and size specified
//
#define ATTR_SET_SIMD_RR_QD_TS(_NAME, _SUPPORT, _ISAR, _OPCODE, _TS) \
    [ARM_IT_##_NAME##_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_TS,  r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_D1_D2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_TS,  r1:V_22_12, r2:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 2 regs same length:
//   regs RR  Q/D versions   Single unspecified esize    2 dt1 qualifiers and sizes specified
//
#define ATTR_SET_SIMD_RR_QD_TS2(_NAME, _SUPPORT, _ISAR, _OPCODE, _TS1, _TS2) \
    [ARM_IT_##_NAME##_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_TS1, dt2:ARM_SDFPT##_TS2,  r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_D1_D2, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_TS1, dt2:ARM_SDFPT##_TS2,  r1:V_22_12, r2:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 2 regs same length:
//   regs RR  Q/D versions   Single unspecified esize    No dt1 qualifier
//
#define ATTR_SET_SIMD_RR_QD(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_Q] = {opcode:_OPCODE, format:FMT_Q1_Q2, support:_SUPPORT, isar:_ISAR,  r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_D1_D2, support:_SUPPORT, isar:_ISAR,  r1:V_22_12, r2:V_5_0}

//
// Attribute entries for SIMD data processing instructions w/ 1 reg and modified immediate:
//   D/S versions    dt1 and size specified
// Note: immediate value 'abcdefgh' is in c (for disassembly purposes). Must still be converted to modified imm value at morph time
//
#define ATTR_SET_SIMD_RI_DS(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_D1_SDFP_MI, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T, r1:V_22_12, sdfpMI:SDFP_MI_SIMD}, \
    [ARM_IT_##_NAME##_Q] = {opcode:_OPCODE, format:FMT_Q1_SDFP_MI, support:_SUPPORT, isar:_ISAR, dt1:ARM_SDFPT##_T, r1:V_22_12, sdfpMI:SDFP_MI_SIMD}

//
// Attribute entries for VFP data processing instructions w/ 3 regs same length:
//   D/S versions    dt1 matches D/S
//
#define ATTR_SET_VFP_RRR_DS(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_D1_D2_D3, support:_SUPPORT, isar:_ISAR, cond:1, dt1:ARM_SDFPT_F64, r1:V_22_12, r2:V_7_16, r3:V_5_0}, \
    [ARM_IT_##_NAME##_S] = {opcode:_OPCODE, format:FMT_S1_S2_S3, support:_SUPPORT, isar:_ISAR, cond:1, dt1:ARM_SDFPT_F32, r1:V_12_22, r2:V_16_7, r3:V_0_5}

//
// Attribute entries for VFP data processing instructions w/ 2 regs same length:
//   D/S versions    dt1 matches D/S
//
#define ATTR_SET_VFP_RR_DS(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_D1_D2, support:_SUPPORT, isar:_ISAR, cond:1, dt1:ARM_SDFPT_F64, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_S] = {opcode:_OPCODE, format:FMT_S1_S2, support:_SUPPORT, isar:_ISAR, cond:1, dt1:ARM_SDFPT_F32, r1:V_12_22, r2:V_0_5}

//
// Attribute entries for VFP data processing instructions w/ 1 reg and modified immediate:
//   D/S versions    dt1 matches D/S
// Note: immediate value 'abcdefgh' is in c (for disassembly purposes). Must still be converted to modified imm value at morph time
//
#define ATTR_SET_VFP_RI_DS(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_D1_SDFP_MI, support:_SUPPORT, isar:_ISAR, cond:1, dt1:ARM_SDFPT_F64, r1:V_22_12, sdfpMI:SDFP_MI_VFP_D}, \
    [ARM_IT_##_NAME##_S] = {opcode:_OPCODE, format:FMT_S1_SDFP_MI, support:_SUPPORT, isar:_ISAR, cond:1, dt1:ARM_SDFPT_F32, r1:V_12_22, sdfpMI:SDFP_MI_VFP_S}

//
// Attribute entries for VFP data processing instructions w/ 2 regs same length:
//   S version only   2 dt1 sizes specified
//
#define ATTR_SET_VFP_RR_S_S2(_NAME, _SUPPORT, _ISAR, _OPCODE, _S1, _S2) \
    [ARM_IT_##_NAME] = {opcode:_OPCODE, format:FMT_S1_S2, support:_SUPPORT, isar:_ISAR, cond:1, dt1:ARM_SDFPT_F##_S1, dt2:ARM_SDFPT_F##_S2, r1:V_12_22, r2:V_0_5}

//
// Attribute entries for VFP data processing instructions w/ 1 reg and implied immediate 0.0 value
//   D/S versions    dt1 matches D/S
//
#define ATTR_SET_VFP_R0_DS(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_D1_F0, support:_SUPPORT, isar:_ISAR, cond:1, dt1:ARM_SDFPT_F64, r1:V_22_12, r2:V_5_0}, \
    [ARM_IT_##_NAME##_S] = {opcode:_OPCODE, format:FMT_S1_F0, support:_SUPPORT, isar:_ISAR, cond:1, dt1:ARM_SDFPT_F32, r1:V_12_22, r2:V_0_5}

//
// Attribute entries for VFP data processing instructions w/ 2 regs different lengths:
//   Narrow (S<-D) and Long (D<-S) versions    dt1 matches D/S direction
//
#define ATTR_SET_VFP_NL_DS(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_S] = {opcode:_OPCODE, format:FMT_D1_S2, support:_SUPPORT, isar:_ISAR, cond:1, dt1:ARM_SDFPT_F64, dt2:ARM_SDFPT_F32, r1:V_22_12, r2:V_0_5}, \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_S1_D2, support:_SUPPORT, isar:_ISAR, cond:1, dt1:ARM_SDFPT_F32, dt2:ARM_SDFPT_F64, r1:V_12_22, r2:V_5_0}

//
// Attribute entries for VFP data processing instructions w/ 2 regs SS or DS:
//   Long (D<-S) and matching (S<-S) versions    Type for dt2  specified
//
#define ATTR_SET_VFP_LS_DS_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_S] = {opcode:_OPCODE, format:FMT_S1_S2, support:_SUPPORT, isar:_ISAR, cond:1, dt1:ARM_SDFPT_F32, dt2:ARM_SDFPT##_T##32, r1:V_12_22, r2:V_0_5}, \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_D1_S2, support:_SUPPORT, isar:_ISAR, cond:1, dt1:ARM_SDFPT_F64, dt2:ARM_SDFPT##_T##32, r1:V_22_12, r2:V_0_5}

//
// Attribute entries for VFP data processing instructions w/ 2 regs SS or SD:
//   Narrow (S<-D) and matching (S<-S) versions    Type for dt1 specified
//
#define ATTR_SET_VFP_NS_DS_T(_NAME, _SUPPORT, _ISAR, _OPCODE, _T) \
    [ARM_IT_##_NAME##_S] = {opcode:_OPCODE, format:FMT_S1_S2, support:_SUPPORT, isar:_ISAR, cond:1, dt1:ARM_SDFPT##_T##32, dt2:ARM_SDFPT_F32, r1:V_12_22, r2:V_0_5}, \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_S1_D2, support:_SUPPORT, isar:_ISAR, cond:1, dt1:ARM_SDFPT##_T##32, dt2:ARM_SDFPT_F64, r1:V_12_22, r2:V_5_0}

//
// Attribute entries for VFP data processing instructions w/ 1 reg and an immediate:
//   D/S versions    2 ftypes with sizes specified for each of D and S     A modifier for the constant specified
//
#define ATTR_SET_VFP_RI_DS_T2C(_NAME, _SUPPORT, _ISAR, _OPCODE, _TS1, _TS2, _TD1, _TD2, _C) \
    [ARM_IT_##_NAME##_S] = {opcode:_OPCODE, format:FMT_S1_S2_UIMM, support:_SUPPORT, isar:_ISAR, cond:1, dt1:ARM_SDFPT##_TS1, dt2:ARM_SDFPT##_TS2, r1:V_12_22, r2:V_12_22, cs:CS_U_5_0_5M##_C}, \
    [ARM_IT_##_NAME##_D] = {opcode:_OPCODE, format:FMT_D1_D2_UIMM, support:_SUPPORT, isar:_ISAR, cond:1, dt1:ARM_SDFPT##_TD1, dt2:ARM_SDFPT##_TD2, r1:V_22_12, r2:V_22_12, cs:CS_U_5_0_5M##_C}

//
// Attribute entries for SIMD/VFP load/store multiple instructions w/ 1 arm reg and a reg list (initial reg + number of consecutive regs)
//   D/S versions
//
#define ATTR_SET_SDFP_LDSTM(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_D]  = {opcode:_OPCODE, format:FMT_R1_SIMD_RL, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:V_22_12, nregs:NREG_7_1, incDec:ID_U_P, wb:WB_21, ua67:ARM_UA_DABORT}, \
    [ARM_IT_##_NAME##_S]  = {opcode:_OPCODE, format:FMT_R1_VFP_RL,  support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:V_12_22, nregs:NREG_8_0, incDec:ID_U_P, wb:WB_21, ua67:ARM_UA_DABORT}

//
// Attribute entries for SIMD/VFP load/store instructions w/ 1 reg and 1 arm reg and an immediate
//   D/S versions
//
#define ATTR_SET_SDFP_LDST(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_D]  = {opcode:_OPCODE, format:FMT_D1_ADDR_R2_SIMM, support:_SUPPORT, isar:_ISAR, cond:1, r1:V_22_12, r2:R_16, cs:CS_U_8_0x4_U, ua67:ARM_UA_DABORT}, \
    [ARM_IT_##_NAME##_S]  = {opcode:_OPCODE, format:FMT_S1_ADDR_R2_SIMM, support:_SUPPORT, isar:_ISAR, cond:1, r1:V_12_22, r2:R_16, cs:CS_U_8_0x4_U, ua67:ARM_UA_DABORT}

//
// Attribute entries for SIMD/VFP push/pop instructions w/ arm reg SP and a reg list (initial reg + number of consecutive regs)
//   D/S versions
//
#define ATTR_SET_SDFP_PUSH(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_D]  = {opcode:_OPCODE, format:FMT_SIMD_RL, support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:V_22_12, nregs:NREG_7_1, ua67:ARM_UA_DABORT}, \
    [ARM_IT_##_NAME##_S]  = {opcode:_OPCODE, format:FMT_VFP_RL,  support:_SUPPORT, isar:_ISAR, cond:1, r1:R_16, r2:V_12_22, nregs:NREG_8_0, ua67:ARM_UA_DABORT}

//
// Attribute entries for SIMD VLDn/VSTn multiple n-element structs
//   Byte, half and word versions
//
#define ATTR_SET_SIMD_LDSTN_BHW(_NAME, _SUPPORT, _ISAR, _OPCODE, _ELS, _REGS, _INCR) \
    [ARM_IT_##_NAME##_W]  = {opcode:_OPCODE, format:FMT_SIMD_ES, support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_32, align:ALIGN_2_4_S4, nels:_ELS, nregs:NREG_##_REGS, incr:INCR_##_INCR, ua67:ARM_UA_UNALIGNED}, \
    [ARM_IT_##_NAME##_H]  = {opcode:_OPCODE, format:FMT_SIMD_ES, support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_16, align:ALIGN_2_4_S4, nels:_ELS, nregs:NREG_##_REGS, incr:INCR_##_INCR, ua67:ARM_UA_UNALIGNED}, \
    [ARM_IT_##_NAME##_B]  = {opcode:_OPCODE, format:FMT_SIMD_ES, support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_8,  align:ALIGN_2_4_S4, nels:_ELS, nregs:NREG_##_REGS, incr:INCR_##_INCR, ua67:ARM_UA_UNALIGNED}

//
// Attribute entries for SIMD VLDn/VSTn multiple n-element structs
//   Byte, half and word versions
//
#define ATTR_SET_SIMD_LDSTN_BHWD(_NAME, _SUPPORT, _ISAR, _OPCODE, _ELS, _REGS, _INCR) \
    [ARM_IT_##_NAME##_D]  = {opcode:_OPCODE, format:FMT_SIMD_ES, support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_64, align:ALIGN_2_4_S4, nels:_ELS, nregs:NREG_##_REGS, incr:INCR_##_INCR, ua67:ARM_UA_UNALIGNED}, \
	ATTR_SET_SIMD_LDSTN_BHW(_NAME, _SUPPORT, _ISAR, _OPCODE, _ELS, _REGS, _INCR)

//
// Attribute entries for SIMD VLD1/VST1 single element to one lane
//   Byte, half and word versions
//
#define ATTR_SET_SIMD_LDST1Z1_BHW(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_W]  = {opcode:_OPCODE, format:FMT_SIMD_ESZ,  support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_32, align:ALIGN_C4_4, nels:1, nregs:NREG_1, index:IDX_7,     ua67:ARM_UA_UNALIGNED},   \
    [ARM_IT_##_NAME##_H]  = {opcode:_OPCODE, format:FMT_SIMD_ESZ,  support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_16, align:ALIGN_C2_4, nels:1, nregs:NREG_1, index:IDX_7_6,   ua67:ARM_UA_UNALIGNED}, \
    [ARM_IT_##_NAME##_B]  = {opcode:_OPCODE, format:FMT_SIMD_ESZ,  support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_8,                    nels:1, nregs:NREG_1, index:IDX_7_6_5, ua67:ARM_UA_UNALIGNED}

//
// Attribute entries for SIMD VLD1 single element to all lanes
//   Byte, half and word versions
//
#define ATTR_SET_SIMD_LD1ZA_BHW(_NAME, _SUPPORT, _ISAR, _OPCODE) \
	[ARM_IT_##_NAME##_W]  = {opcode:_OPCODE, format:FMT_SIMD_ESZA, support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_32, align:ALIGN_C4_4, nels:1, nregs:NREG_1_5_B, ua67:ARM_UA_UNALIGNED}, \
    [ARM_IT_##_NAME##_H]  = {opcode:_OPCODE, format:FMT_SIMD_ESZA, support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_16, align:ALIGN_C2_4, nels:1, nregs:NREG_1_5_B, ua67:ARM_UA_UNALIGNED}, \
    [ARM_IT_##_NAME##_B]  = {opcode:_OPCODE, format:FMT_SIMD_ESZA, support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_8,                    nels:1, nregs:NREG_1_5_B, ua67:ARM_UA_UNALIGNED}

//
// Attribute entries for SIMD VLD2/VST2 single element to one lane
//   Byte, half and word versions
//
#define ATTR_SET_SIMD_LDST2Z1_BHW(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_W]  = {opcode:_OPCODE, format:FMT_SIMD_ESZ,  support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_32, align:ALIGN_C8_4, nels:2, nregs:NREG_1, incr:INCR_1_6_B, index:IDX_7,     ua67:ARM_UA_UNALIGNED},   \
    [ARM_IT_##_NAME##_H]  = {opcode:_OPCODE, format:FMT_SIMD_ESZ,  support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_16, align:ALIGN_C4_4, nels:2, nregs:NREG_1, incr:INCR_1_5_B, index:IDX_7_6,   ua67:ARM_UA_UNALIGNED}, \
    [ARM_IT_##_NAME##_B]  = {opcode:_OPCODE, format:FMT_SIMD_ESZ,  support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_8,  align:ALIGN_C2_4, nels:2, nregs:NREG_1, incr:INCR_1,     index:IDX_7_6_5, ua67:ARM_UA_UNALIGNED}

//
// Attribute entries for SIMD VLD2 single element to all lanes
//   Byte, half and word versions
//
#define ATTR_SET_SIMD_LD2ZA_BHW(_NAME, _SUPPORT, _ISAR, _OPCODE) \
	[ARM_IT_##_NAME##_W]  = {opcode:_OPCODE, format:FMT_SIMD_ESZA, support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_32, align:ALIGN_C8_4, nels:2, nregs:NREG_1, incr:INCR_1_5_B, ua67:ARM_UA_UNALIGNED}, \
    [ARM_IT_##_NAME##_H]  = {opcode:_OPCODE, format:FMT_SIMD_ESZA, support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_16, align:ALIGN_C4_4, nels:2, nregs:NREG_1, incr:INCR_1_5_B, ua67:ARM_UA_UNALIGNED}, \
    [ARM_IT_##_NAME##_B]  = {opcode:_OPCODE, format:FMT_SIMD_ESZA, support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_8,  align:ALIGN_C2_4, nels:2, nregs:NREG_1, incr:INCR_1_5_B, ua67:ARM_UA_UNALIGNED}

//
// Attribute entries for SIMD VLD3/VST3 single element to one lane
//   Byte, half and word versions
//
#define ATTR_SET_SIMD_LDST3Z1_BHW(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_W]  = {opcode:_OPCODE, format:FMT_SIMD_ESZ,  support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_32, nels:3, nregs:NREG_1, incr:INCR_1_6_B, index:IDX_7,     ua67:ARM_UA_UNALIGNED},   \
    [ARM_IT_##_NAME##_H]  = {opcode:_OPCODE, format:FMT_SIMD_ESZ,  support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_16, nels:3, nregs:NREG_1, incr:INCR_1_5_B, index:IDX_7_6,   ua67:ARM_UA_UNALIGNED}, \
    [ARM_IT_##_NAME##_B]  = {opcode:_OPCODE, format:FMT_SIMD_ESZ,  support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_8,  nels:3, nregs:NREG_1, incr:INCR_1,     index:IDX_7_6_5, ua67:ARM_UA_UNALIGNED}

//
// Attribute entries for SIMD VLD3 single element to all lanes
//   Byte, half and word versions
//
#define ATTR_SET_SIMD_LD3ZA_BHW(_NAME, _SUPPORT, _ISAR, _OPCODE) \
	[ARM_IT_##_NAME##_W]  = {opcode:_OPCODE, format:FMT_SIMD_ESZA, support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_32, nels:3, nregs:NREG_1, incr:INCR_1_5_B, ua67:ARM_UA_UNALIGNED}, \
    [ARM_IT_##_NAME##_H]  = {opcode:_OPCODE, format:FMT_SIMD_ESZA, support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_16, nels:3, nregs:NREG_1, incr:INCR_1_5_B, ua67:ARM_UA_UNALIGNED}, \
    [ARM_IT_##_NAME##_B]  = {opcode:_OPCODE, format:FMT_SIMD_ESZA, support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_8,  nels:3, nregs:NREG_1, incr:INCR_1_5_B, ua67:ARM_UA_UNALIGNED}

//
// Attribute entries for SIMD VLD4/VST4 single element to one lane
//   Byte, half and word versions
//
#define ATTR_SET_SIMD_LDST4Z1_BHW(_NAME, _SUPPORT, _ISAR, _OPCODE) \
    [ARM_IT_##_NAME##_W]  = {opcode:_OPCODE, format:FMT_SIMD_ESZ,  support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_32, align:ALIGN_2_4_S4, nels:4, nregs:NREG_1, incr:INCR_1_6_B, index:IDX_7,     ua67:ARM_UA_UNALIGNED},   \
    [ARM_IT_##_NAME##_H]  = {opcode:_OPCODE, format:FMT_SIMD_ESZ,  support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_16, align:ALIGN_C8_4,   nels:4, nregs:NREG_1, incr:INCR_1_5_B, index:IDX_7_6,   ua67:ARM_UA_UNALIGNED}, \
    [ARM_IT_##_NAME##_B]  = {opcode:_OPCODE, format:FMT_SIMD_ESZ,  support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_8,  align:ALIGN_C4_4,   nels:4, nregs:NREG_1, incr:INCR_1,     index:IDX_7_6_5, ua67:ARM_UA_UNALIGNED}

//
// Attribute entries for SIMD VLD4 single element to all lanes
//   Byte, half and word versions
//
#define ATTR_SET_SIMD_LD4ZA_BHW(_NAME, _SUPPORT, _ISAR, _OPCODE) \
	[ARM_IT_##_NAME##_W1] = {opcode:_OPCODE, format:FMT_SIMD_ESZA, support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_32, align:ALIGN_C16_4, nels:4, nregs:NREG_1, incr:INCR_1_5_B, ua67:ARM_UA_UNALIGNED}, \
	[ARM_IT_##_NAME##_W]  = {opcode:_OPCODE, format:FMT_SIMD_ESZA, support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_32, align:ALIGN_C8_4,  nels:4, nregs:NREG_1, incr:INCR_1_5_B, ua67:ARM_UA_UNALIGNED}, \
    [ARM_IT_##_NAME##_H]  = {opcode:_OPCODE, format:FMT_SIMD_ESZA, support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_16, align:ALIGN_C8_4,  nels:4, nregs:NREG_1, incr:INCR_1_5_B, ua67:ARM_UA_UNALIGNED}, \
    [ARM_IT_##_NAME##_B]  = {opcode:_OPCODE, format:FMT_SIMD_ESZA, support:_SUPPORT, isar:_ISAR, r1:V_22_12, r2:R_16, r3:R_0, dt1:ARM_SDFPT_8,  align:ALIGN_C4_4,  nels:4, nregs:NREG_1, incr:INCR_1_5_B, ua67:ARM_UA_UNALIGNED}

#endif

