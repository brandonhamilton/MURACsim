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

// VMI header files
#include "vmi/vmiCxt.h"
#include "vmi/vmiDecode.h"
#include "vmi/vmiMessage.h"
#include "vmi/vmiRt.h"

// model header files
#include "armAttributeEntriesARM.h"
#include "armBitMacros.h"
#include "armDecode.h"
#include "armDecodeEntriesARM.h"
#include "armDecodeARM.h"
#include "armRegisters.h"
#include "armStructure.h"
#include "armVariant.h"


//
// Prefix for messages from this module
//
#define CPU_PREFIX "ARM_DECODE_ARM"


////////////////////////////////////////////////////////////////////////////////
// FIELD EXTRACTION MACROS
////////////////////////////////////////////////////////////////////////////////

#define OP_R0(_I)         WIDTH(4,(_I)>> 0)
#define OP_R8(_I)         WIDTH(4,(_I)>> 8)
#define OP_R12(_I)        WIDTH(4,(_I)>>12)
#define OP_R16(_I)        WIDTH(4,(_I)>>16)
#define OP_AIF(_I)        WIDTH(3,(_I)>> 6)
#define OP_MA(_I)         WIDTH(1,(_I)>>17)
#define OP_IMOD(_I)       WIDTH(2,(_I)>>18)
#define OP_F(_I)          WIDTH(1,(_I)>>20)
#define OP_WB(_I)         WIDTH(1,(_I)>>21)
#define OP_U(_I)          WIDTH(1,(_I)>>23)
#define OP_H(_I)          WIDTH(1,(_I)>>24)
#define OP_PI(_I)         WIDTH(1,(_I)>>24)
#define OP_COND(_I)       WIDTH(4,(_I)>>28)
#define OP_U_1_4(_I)      WIDTH(1,(_I)>> 4)
#define OP_U_1_5(_I)      WIDTH(1,(_I)>> 5)
#define OP_U_1_6(_I)      WIDTH(1,(_I)>> 6)
#define OP_U_1_7(_I)      WIDTH(1,(_I)>> 7)
#define OP_U_1_19(_I)     WIDTH(1,(_I)>>19)
#define OP_U_1_21(_I)     WIDTH(1,(_I)>>21)
#define OP_U_2_4(_I)      WIDTH(2,(_I)>> 4)
#define OP_U_2_6(_I)      WIDTH(2,(_I)>> 6)
#define OP_U_2_5_3(_I)    (((WIDTH(1,(_I)>>5 ))<<1)|WIDTH(1,(_I)>>3))
#define OP_U_2_8(_I)      WIDTH(2,(_I)>> 8)
#define OP_U_2_18(_I)     WIDTH(2,(_I)>>18)
#define OP_U_2_21_6(_I)   (((WIDTH(1,(_I)>>21))<<1)|WIDTH(1,(_I)>>6))
#define OP_U_3_0(_I)      WIDTH(3,(_I)>> 0)
#define OP_U_3_5(_I)      WIDTH(3,(_I)>> 5)
#define OP_U_3_16(_I)     WIDTH(3,(_I)>>16)
#define OP_U_3_17(_I)     WIDTH(3,(_I)>>17)
#define OP_U_3_21_6_5(_I) (((WIDTH(1,(_I)>>21))<<2)|WIDTH(2,(_I)>>5))
#define OP_U_4_0(_I)      WIDTH(4,(_I)>> 0)
#define OP_U_4_8(_I)      WIDTH(4,(_I)>> 8)
#define OP_U_4_16(_I)     WIDTH(4,(_I)>>16)
#define OP_U_5_0(_I)      WIDTH(5,(_I)>> 0)
#define OP_U_5_0_5(_I)    (((WIDTH(4,(_I)>>0))<<1)|WIDTH(1,(_I)>>5))
#define OP_U_5_7(_I)      WIDTH(5,(_I)>> 7)
#define OP_U_5_16(_I)     WIDTH(5,(_I)>>16)
#define OP_U_6_16(_I)     WIDTH(6,(_I)>>16)
#define OP_U_7_1(_I)      WIDTH(7,(_I)>> 1)
#define OP_U_8_0(_I)      WIDTH(8,(_I)>> 0)
#define OP_U_8_8_0(_I)    (((WIDTH(4,(_I)>>8))<<4)  | WIDTH(4,(_I)>>0))
#define OP_U_8_16_0(_I)   (((WIDTH(4,(_I)>>16))<<4) | WIDTH(4,(_I)>>0))
#define OP_U_8_SIMD_MI(_I) (((WIDTH(1,(_I)>>24))<<7)|((WIDTH(3,(_I)>>16))<<4)| WIDTH(4,(_I)>>0))
#define OP_U_9(_I)        WIDTH(1,(_I)>> 9)
#define OP_U_11_10(_I)    WIDTH(2,(_I)>>10)
#define OP_U_12_0(_I)     WIDTH(12,(_I)>>0)
#define OP_U_16_8_0(_I)   (((WIDTH(12,(_I)>>8))<<4) | WIDTH(4,(_I)>>0))
#define OP_U_16_16_0(_I)  (((WIDTH(4,(_I)>>16))<<12)| WIDTH(12,(_I)>>0))
#define OP_U_24_0(_I)     WIDTH(24,(_I)>>0)
#define OP_TS24(_I)       (((Int32)((_I)<<8))>>6)
#define OP_TS24H(_I)      (OP_TS24(_I) | (OP_H(_I)<<1))
#define OP_RLIST(_I)      WIDTH(16,(_I)>>0)
#define OP_6_5(_I)        WIDTH(2,(_I)>> 5)
#define OP_ROTATE(_I)     (WIDTH(4,(_I)>>8)<<1)
#define OP_CPNUM(_I)      WIDTH(4,(_I)>> 8)
#define OP_CPOP1_4_4(_I)  WIDTH(4,(_I)>> 4)
#define OP_CPOP1_4_20(_I) WIDTH(4,(_I)>>20)
#define OP_CPOP1_3_21(_I) WIDTH(3,(_I)>>21)
#define OP_CPOP2(_I)      WIDTH(3,(_I)>> 5)
#define OP_LL(_I)         WIDTH(1,(_I)>>22)
#define OP_V0_5(_I)       ((OP_R0(_I) <<1) | WIDTH(1,(_I)>> 5))
#define OP_V16_7(_I)      ((OP_R16(_I)<<1) | WIDTH(1,(_I)>> 7))
#define OP_V12_22(_I)     ((OP_R12(_I)<<1) | WIDTH(1,(_I)>>22))
#define OP_V5_0(_I)       ((WIDTH(1,(_I)>>5) <<4) | OP_R0(_I))
#define OP_V22_12(_I)     ((WIDTH(1,(_I)>>22)<<4) | OP_R12(_I))
#define OP_V7_16(_I)      ((WIDTH(1,(_I)>>7) <<4) | OP_R16(_I))


////////////////////////////////////////////////////////////////////////////////
// INSTRUCTION ATTRIBUTE TABLE
////////////////////////////////////////////////////////////////////////////////

//
// Define the location of register in an instruction
//
typedef enum rSpecE {
    R_NA,           // no register
    R_0,            // register at x:0
    R_8,            // register at x:8
    R_12,           // register at x:12
    R_12P1,         // register at x:12+1
    R_16,           // register at x:16
    R_SP,           // stack pointer
    V_0_5,          // register at 3:0,5
    V_12_22,        // register at 15:12,22
    V_16_7,         // register at 19:16,7
    V_5_0,          // register at 5,3:0
    V_22_12,        // register at 22,15:12
    V_7_16,         // register at 7,19:16
    V3_0,           // register is 3bits wide at 2:0
} rSpec;

//
// Define the location of a constant in an instruction
//
typedef enum constSpecE {
    CS_NA,          // instruction has no constant
    CS_U_9,         // 1-bit unsigned constant at 9
    CS_U_2_10x8,    // 2-bit unsigned constant at 11:10 * 8
    CS_U_3_16N,     // 3-bit unsigned constant at 18:16, value inverted
    CS_U_3_16P,     // 3-bit unsigned constant at 18:16, value not inverted
    CS_U_4_0,       // 4-bit unsigned constant at 3:0
    CS_U_4_8,       // 4-bit unsigned constant at 11:8
    CS_U_4_16N,     // 4-bit unsigned constant at 19:16, value inverted
    CS_U_4_16P,     // 4-bit unsigned constant at 19:16, value not inverted
    CS_U_5_0,       // 5-bit unsigned constant at 4:0
    CS_U_5_7,       // 5-bit unsigned constant at 11:7
    CS_U_5_0_5M16,  // 5-bit unsigned constant at 3:0, 5. Subtract from 16 to get value
    CS_U_5_0_5M32,  // 5-bit unsigned constant at 3:0, 5. Subtract from 32 to get value
    CS_U_5_16N,     // 5-bit unsigned constant at 20:16, value inverted
    CS_U_5_16P,     // 5-bit unsigned constant at 20:16, value not inverted
    CS_U_6_16N,     // 6-bit unsigned constant at 21:16, value inverted
    CS_U_6_16P,     // 6-bit unsigned constant at 21:16, value not inverted
    CS_U_8_0,       // 8-bit unsigned constant at 7:0
    CS_U_8_0_R,     // 8-bit unsigned constant at 7:0, rotated by 2*11:8
    CS_U_8_0x4_U,   // 8-bit unsigned constant at 7:0*4, negated if U=0
    CS_U_8_8_0_U,   // 8-bit unsigned constant at 11:8, 3:0
    CS_U_12_0_U,    // 12-bit unsigned constant at 11:0, negated if U=0
    CS_U_16_8_0,    // 16-bit unsigned constant at 19:8,3:0
    CS_U_16_16_0,   // 16-bit unsigned constant at 19:16,11:0
    CS_U_24_0,      // 24-bit unsigned constant at 23:0
} constSpec;

//
// This defines target address field in the instruction
//
typedef enum targetSpecE {
    TC_NA,      // no target
    TC_S24,     // target PC + s24 field (4-byte aligned)
    TC_S24H,    // target PC + s24 field (4-byte aligned) + H bit
} targetSpec;

//
// This defines shift in the instruction
//
typedef enum shiftSpecE {
    SS_NA,      // no shift operation
    SS_6_5,     // shift spec at offset 6:5
    SS_RRX      // RRX
} shiftSpec;

//
// This defines whether the instruction specifies post-indexed addressing
//
typedef enum postIndexSpecE {
    PI_0,       // not post-indexed
    PI_24,      // post-indexed at standard location in ARM instruction
} postIndexSpec;

//
// This defines whether the instruction specifides writeback
//
typedef enum writebackSpecE {
    WB_0,       // no writeback
    WB_21       // writeback at standard location in ARM instruction
} writebackSpec;

//
// This defines coprocessor opcode field in the instruction
//
typedef enum cpOp1SpecE {
    COP_NA,     // no opcode1 field
    COP_4_4,    // 4-bit constant at offset 7:4
    COP_4_20,   // 4-bit constant at offset 23:20
    COP_3_21    // 3-bit constant at offset 23:21
} cpOp1Spec;

//
// This defines register list field in the instruction
//
typedef enum rListSpecE {
    RL_NA,      // no register list
    RL_32,      // register list in ARM opcode
} rListSpec;

//
// This defines increment/decrement specification in the instruction
//
typedef enum incDecSpecE {
    ID_NA,      // no increment/decrement specification
    ID_U_P_IAI, // increment/decrement defined by U and P bits, implicit IA (UAL only)
    ID_U_P,     // increment/decrement defined by U and P bits, explicit IA
} incDecSpec;

//
// This defines width field in the instruction
//
typedef enum widthSpecE {
    WS_NA,      // no width specification
    WS_WIDTH,   // width in field 20:16
    WS_WIDTHM1, // width in field 20:16+1
    WS_MSB      // width in fields 20:16 - 11:7 + 1
} widthSpec;

//
// Define a SIMD/VFP modified immediate constant type
//
typedef enum sdfpMISpecE {
    SDFP_MI_NA,       // instruction has no SIMD/VFP modified immediate constant
    SDFP_MI_VFP_S,    // single precision VFP modified immediate value
    SDFP_MI_VFP_D,    // double precision VFP modified immediate value
    SDFP_MI_SIMD,     // 64 bit SIMD modified immediate value
} sdfpMISpec;

//
// This defines the SIMD scalar index field in the instruction
//
typedef enum indexSpecE {
    IDX_NA,       // no index specification
    IDX_21_6_5,   // index is 3 bits wide, in bits 21, 6:5
    IDX_21_6,     // index is 2 bits wide, in bits 21, 6
    IDX_21,       // index is 1 bit  wide, in bit  21
    IDX_5_3,      // index is 2 bits wide, in bits 5 and 3
    IDX_5,        // index is 1 bit  wide, in bit  5
    IDX_7,        // index is 1 bit  wide, in bit  7
    IDX_7_6,      // index is 2 bits wide, in bits 7:6
    IDX_7_6_5,    // index is 3 bits wide, in bits 7:5
    IDX_19,       // index is 1 bit  wide, in bit  19
    IDX_19_18,    // index is 2 bits wide, in bits 19:16
    IDX_19_18_17, // index is 3 bits wide, in bits 19:17
} indexSpec;

//
// This defines the SIMD element/structure alignment field in the instruction
//
typedef enum alignSpecE {
    ALIGN_NA,       // no alignment specification
    ALIGN_2_4_S4,   // alignment = 4 << (bits 5:4) or 0 if bits 5:4 == 0
    ALIGN_C2_4,     // alignment = 2  if bit 4 is 1 or 0 if bit 4 == 0
    ALIGN_C4_4,     // alignment = 4  if bit 4 is 1 or 0 if bit 4 == 0
    ALIGN_C8_4,     // alignment = 8  if bit 4 is 1 or 0 if bit 4 == 0
    ALIGN_C16_4,    // alignment = 16 if bit 4 is 1 or 0 if bit 4 == 0
} alignSpec;

//
// This defines the inc (or stride) for a SIMD register list
//
typedef enum incrSpecE {
    INCR_0,        // no incr/stride specification
    INCR_1,        // INCR = 1
    INCR_2,        // INCR = 2
    INCR_1_5_B,    // INCR is 2 if bit 5 is 1 else INCR is 1
    INCR_1_6_B,    // INCR is 2 if bit 6 is 1 else INCR is 1
} incrSpec;

//
// This defines the number of regs in a SIMD register list
//
typedef enum nregSpecE {
    NREG_NA,       // no alignment specification
    NREG_1,        // Nregs = 1
    NREG_2,        // Nregs = 2
    NREG_3,        // Nregs = 3
    NREG_4,        // Nregs = 4
    NREG_2_8_P1,   // Nregs is bit value in bits 9:8 + 1
    NREG_7_1,      // Nregs is 7 bits wide in bits 7:1
    NREG_8_0,      // Nregs is 8 bits wide in bits 7:0
    NREG_1_5_B,    // Nregs is 2 if bit 5 is 1 else Nregs is 1
    NREG_1_6_B,    // Nregs is 2 if bit 6 is 1 else Nregs is 1
} nregSpec;

//
// Structure defining characteristics of each opcode type
//
typedef struct opAttrsS {
    const char        *opcode;      // opcode name
    const char        *format;      // format string
    armArchitecture    support:16;  // variants on which instruction supported
    armISARSupport     isar   : 8;  // ISAR instruction support
    armSetFlags        f      : 4;  // does this opcode set flags?
    Bool               cond   : 1;  // does instruction have condition?
    rSpec              r1     : 4;  // does instruction have r1?
    rSpec              r2     : 4;  // does instruction have r2?
    rSpec              r3     : 4;  // does instruction have r3?
    rSpec              r4     : 4;  // does instruction have r4?
    constSpec          cs     : 8;  // location of constant
    targetSpec         ts     : 4;  // target specification
    shiftSpec          ss     : 4;  // shifter specification
    Uns8               sz     : 4;  // load/store size
    Uns8               xs     : 4;  // sign extend?
    Uns8               tl     : 4;  // translate?
    postIndexSpec      pi     : 4;  // instruction specifies post-indexed address?
    writebackSpec      wb     : 4;  // instruction specifies writeback?
    Uns8               ll     : 4;  // instruction specifies long load?
    Uns8               cpNum  : 4;  // does instruction have coprocessor number?
    cpOp1Spec          cpOp1  : 4;  // does instruction have coprocessor op 1?
    Uns8               cpOp2  : 4;  // does instruction have coprocessor op 2?
    rListSpec          rList  : 4;  // does instruction have register list?
    incDecSpec         incDec : 4;  // does instruction have increment/decrement?
    armUnalignedAction ua45   : 4;  // action if unaligned (SCTLR.U=0)
    armUnalignedAction ua67   : 4;  // action if unaligned (SCTLR.U=1)
    Bool               ea     : 1;  // exclusive access?
    Bool               imod   : 1;  // imod field present?
    Bool               m      : 1;  // M field present?
    Bool               aif    : 1;  // A/I/F fields present?
    widthSpec          w      : 4;  // width specification?
    indexSpec          index  : 4;  // SIMD scalar index specification?
    alignSpec          align  : 4;  // SIMD element/structure alignment specification?
    nregSpec           nregs  : 4;  // number of regs in SIMD/VFP register list specification?
    Uns8               nels   : 4;  // number of elements in SIMD element load/store instruction
    incrSpec           incr   : 4;  // inc (or stride) for SIMD register list?
    sdfpMISpec         sdfpMI : 4;  // SIMD/floating point modified immediate constant?
    armSDFPType        dt1    : 8;  // SIMD/VFP first data type?
    armSDFPType        dt2    : 8;  // SIMD/VFP second data type?
} opAttrs;

typedef const struct opAttrsS *opAttrsCP;

//
// This specifies attributes for each opcode
//
const static opAttrs attrsArray[ARM_IT_LAST+1] = {

    ////////////////////////////////////////////////////////////////////////////
    // NORMAL INSTRUCTIONS
    ////////////////////////////////////////////////////////////////////////////

    // data processing instructions
    ATTR_SET_ADC (ADC,  0, ARM_ISAR_NA, "adc" ),
    ATTR_SET_ADC (ADD,  0, ARM_ISAR_NA, "add" ),
    ATTR_SET_ADC (AND,  0, ARM_ISAR_NA, "and" ),
    ATTR_SET_ADC (BIC,  0, ARM_ISAR_NA, "bic" ),
    ATTR_SET_ADC (EOR,  0, ARM_ISAR_NA, "eor" ),
    ATTR_SET_MOV (MOV,  0, ARM_ISAR_NA, "mov" ),
    ATTR_SET_MOV (MVN,  0, ARM_ISAR_NA, "mvn" ),
    ATTR_SET_ADC (ORR,  0, ARM_ISAR_NA, "orr" ),
    ATTR_SET_ADC (RSB,  0, ARM_ISAR_NA, "rsb" ),
    ATTR_SET_ADC (RSC,  0, ARM_ISAR_NA, "rsc" ),
    ATTR_SET_ADC (SBC,  0, ARM_ISAR_NA, "sbc" ),
    ATTR_SET_ADC (SUB,  0, ARM_ISAR_NA, "sub" ),

    // ARMv6T2 move instructions
    ATTR_SET_MOVT (MOVT, ARM_VT2, ARM_ISAR_MOVT, "movt"),
    ATTR_SET_MOVT (MOVW, ARM_VT2, ARM_ISAR_MOVT, "movw"),

    // multiply instructions
    ATTR_SET_MLA   (MLA,   2,       ARM_ISAR_MLA,   "mla"  ),
    ATTR_SET_MLA   (MLS,   ARM_VT2, ARM_ISAR_MLS,   "mls"  ),
    ATTR_SET_MUL   (MUL,   2,       ARM_ISAR_NA,    "mul"  ),
    ATTR_SET_SMLAL (SMLAL, ARM_VM,  ARM_ISAR_SMULL, "smlal"),
    ATTR_SET_SMLAL (SMULL, ARM_VM,  ARM_ISAR_SMULL, "smull"),
    ATTR_SET_SMLAL (UMAAL, 6,       ARM_ISAR_UMAAL, "umaal"),
    ATTR_SET_SMLAL (UMLAL, ARM_VM,  ARM_ISAR_UMULL, "umlal"),
    ATTR_SET_SMLAL (UMULL, ARM_VM,  ARM_ISAR_UMULL, "umull"),

    // compare instructions
    ATTR_SET_CMN (CMN, 0, ARM_ISAR_NA, "cmn"),
    ATTR_SET_CMN (CMP, 0, ARM_ISAR_NA, "cmp"),
    ATTR_SET_CMN (TEQ, 0, ARM_ISAR_NA, "teq"),
    ATTR_SET_CMN (TST, 0, ARM_ISAR_NA, "tst"),

    // branch instructions
    ATTR_SET_B    (B,    0,      ARM_ISAR_NA,  "b"  ),
    ATTR_SET_B    (BL,   0,      ARM_ISAR_NA,  "bl" ),
    ATTR_SET_BLX1 (BLX1, 5,      ARM_ISAR_BLX, "blx"),
    ATTR_SET_BLX2 (BLX2, 5,      ARM_ISAR_BLX, "blx"),
    ATTR_SET_BLX2 (BX,   ARM_BX, ARM_ISAR_BX,  "bx" ),
    ATTR_SET_BLX2 (BXJ,  ARM_J,  ARM_ISAR_BXJ, "bxj"),

    // MURAC instructions
    ATTR_SET_BLX2 (BAA,  ARM_BAA,  ARM_ISAR_BAA, "baa"),

    // miscellaneous instructions
    ATTR_SET_BKPT (BKPT, 5, ARM_ISAR_BKPT, "bkpt"),
    ATTR_SET_CLZ  (CLZ,  5, ARM_ISAR_CLZ,  "clz" ),
    ATTR_SET_SWI  (SWI,  0, ARM_ISAR_SVC,  "svc" ),

    // load and store instructions
    ATTR_SET_LDR   (LDR,   0, ARM_ISAR_NA,    "ldr", 4, False, ARM_UA_ROTATE, ARM_UA_UNALIGNED),
    ATTR_SET_LDR   (LDRB,  0, ARM_ISAR_NA,    "ldr", 1, False, ARM_UA_ALIGN,  ARM_UA_UNALIGNED),
    ATTR_SET_LDRBT (LDRBT, 0, ARM_ISAR_LDRBT, "ldr", 1, False, ARM_UA_ALIGN,  ARM_UA_UNALIGNED),
    ATTR_SET_LDRH  (LDRH,  4, ARM_ISAR_NA,    "ldr", 2, False, ARM_UA_ALIGN,  ARM_UA_UNALIGNED),
    ATTR_SET_LDRH  (LDRSB, 4, ARM_ISAR_NA,    "ldr", 1, True,  ARM_UA_ALIGN,  ARM_UA_UNALIGNED),
    ATTR_SET_LDRH  (LDRSH, 4, ARM_ISAR_NA,    "ldr", 2, True,  ARM_UA_ALIGN,  ARM_UA_UNALIGNED),
    ATTR_SET_LDRBT (LDRT,  0, ARM_ISAR_LDRBT, "ldr", 4, False, ARM_UA_ROTATE, ARM_UA_UNALIGNED),
    ATTR_SET_LDR   (STR,   0, ARM_ISAR_NA,    "str", 4, False, ARM_UA_ALIGN,  ARM_UA_UNALIGNED),
    ATTR_SET_LDR   (STRB,  0, ARM_ISAR_NA,    "str", 1, False, ARM_UA_ALIGN,  ARM_UA_UNALIGNED),
    ATTR_SET_LDRBT (STRBT, 0, ARM_ISAR_LDRBT, "str", 1, False, ARM_UA_ALIGN,  ARM_UA_UNALIGNED),
    ATTR_SET_LDRH  (STRH,  4, ARM_ISAR_NA,    "str", 2, False, ARM_UA_ALIGN,  ARM_UA_UNALIGNED),
    ATTR_SET_LDRBT (STRT,  0, ARM_ISAR_LDRBT, "str", 4, False, ARM_UA_ALIGN,  ARM_UA_UNALIGNED),

    // load and store multiple instructions
    ATTR_SET_LDM1 (LDM1,  0, ARM_ISAR_NA,     "ldm", ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_LDM2 (LDM2,  0, ARM_ISAR_LDM_UR, "ldm", ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_LDM2 (LDM3,  0, ARM_ISAR_LDM_UR, "ldm", ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_LDM1 (STM1,  0, ARM_ISAR_NA,     "stm", ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_LDM2 (STM2,  0, ARM_ISAR_LDM_UR, "stm", ARM_UA_ALIGN, ARM_UA_DABORT),

    // ARMv6T2 load and store instructions
    ATTR_SET_LDRHT (LDRHT,  ARM_VT2, ARM_ISAR_LDRHT, "ldr", 2, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_LDRHT (LDRSBT, ARM_VT2, ARM_ISAR_LDRHT, "ldr", 1, True,  ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_LDRHT (LDRSHT, ARM_VT2, ARM_ISAR_LDRHT, "ldr", 2, True,  ARM_UA_ALIGN, ARM_UA_UNALIGNED),
    ATTR_SET_LDRHT (STRHT,  ARM_VT2, ARM_ISAR_LDRHT, "str", 2, False, ARM_UA_ALIGN, ARM_UA_UNALIGNED),

    // semaphore instructions
    ATTR_SET_SWP (SWP,  3, ARM_ISAR_SWP, "swp", 4, False, ARM_UA_ROTATE, ARM_UA_DABORT),
    ATTR_SET_SWP (SWPB, 3, ARM_ISAR_SWP, "swp", 1, False, ARM_UA_ALIGN,  ARM_UA_DABORT),

    // synchronization primitives
    ATTR_SET_LDREX (LDREX,  6,     ARM_ISAR_LDREX,  "ldr", 4, False, ARM_UA_DABORT, ARM_UA_DABORT),
    ATTR_SET_LDREX (LDREXB, ARM_K, ARM_ISAR_CLREX,  "ldr", 1, False, ARM_UA_DABORT, ARM_UA_DABORT),
    ATTR_SET_LDREX (LDREXH, ARM_K, ARM_ISAR_CLREX,  "ldr", 2, False, ARM_UA_DABORT, ARM_UA_DABORT),
    ATTR_SET_LDREX (LDREXD, ARM_K, ARM_ISAR_LDREXD, "ldr", 8, False, ARM_UA_DABORT, ARM_UA_DABORT),
    ATTR_SET_STREX (STREX,  6,     ARM_ISAR_LDREX,  "str", 4, False, ARM_UA_DABORT, ARM_UA_DABORT),
    ATTR_SET_STREX (STREXB, ARM_K, ARM_ISAR_CLREX,  "str", 1, False, ARM_UA_DABORT, ARM_UA_DABORT),
    ATTR_SET_STREX (STREXH, ARM_K, ARM_ISAR_CLREX,  "str", 2, False, ARM_UA_DABORT, ARM_UA_DABORT),
    ATTR_SET_STREX (STREXD, ARM_K, ARM_ISAR_LDREXD, "str", 8, False, ARM_UA_DABORT, ARM_UA_DABORT),

    // coprocessor instructions
    ATTR_SET_CDP  (CDP,  2, ARM_ISAR_NA, "cdp" ),
    ATTR_SET_CDP2 (CDP2, 5, ARM_ISAR_NA, "cdp2"),
    ATTR_SET_LDC  (LDC,  2, ARM_ISAR_NA, "ldc" ),
    ATTR_SET_LDC2 (LDC2, 5, ARM_ISAR_NA, "ldc2"),
    ATTR_SET_MCR  (MCR,  2, ARM_ISAR_NA, "mcr" ),
    ATTR_SET_MCR2 (MCR2, 5, ARM_ISAR_NA, "mcr2"),
    ATTR_SET_MRC  (MRC,  2, ARM_ISAR_NA, "mrc" ),
    ATTR_SET_MRC2 (MRC2, 5, ARM_ISAR_NA, "mrc2"),
    ATTR_SET_LDC  (STC,  2, ARM_ISAR_NA, "stc" ),
    ATTR_SET_LDC2 (STC2, 5, ARM_ISAR_NA, "stc2"),

    // status register access instructions
    ATTR_SET_MRS (MRSC, 3, ARM_ISAR_MRS_AR, "mrs", "CPSR"),
    ATTR_SET_MRS (MRSS, 3, ARM_ISAR_MRS_AR, "mrs", "SPSR"),
    ATTR_SET_MSR (MSRC, 3, ARM_ISAR_MRS_AR, "msr", "CPSR"),
    ATTR_SET_MSR (MSRS, 3, ARM_ISAR_MRS_AR, "msr", "SPSR"),

    // hint instructions
    ATTR_SET_NOP_C (NOP,   0, ARM_ISAR_NOP, "nop"  ),
    ATTR_SET_NOP_C (YIELD, 0, ARM_ISAR_NOP, "yield"),
    ATTR_SET_NOP_C (WFE,   0, ARM_ISAR_NOP, "wfe"  ),
    ATTR_SET_NOP_C (WFI,   0, ARM_ISAR_NOP, "wfi"  ),
    ATTR_SET_NOP_C (SEV,   0, ARM_ISAR_NOP, "sev"  ),
    ATTR_SET_DBG   (DBG,   0, ARM_ISAR_NOP, "dbg"  ),

    // ARMv6 exception instructions
    ATTR_SET_SRS (SRS, 6, ARM_ISAR_SRS, "srs", ARM_UA_DABORT, ARM_UA_DABORT),
    ATTR_SET_RFE (RFE, 6, ARM_ISAR_SRS, "rfe", ARM_UA_DABORT, ARM_UA_DABORT),

    // ARMv6 miscellaneous instructions
    ATTR_SET_SETEND (SETEND, 6,     ARM_ISAR_SETEND, "setend"),
    ATTR_SET_CPS    (CPS,    6,     ARM_ISAR_SRS,    "cps"   ),
    ATTR_SET_NOP_UC (CLREX,  ARM_K, ARM_ISAR_CLREX,  "clrex" ),

    // ARMv6/ARMv7 memory hint instructions
    ATTR_SET_PLD   (PLD,   ARM_VD2, ARM_ISAR_PLD,  "pld" ),
    ATTR_SET_PLD   (PLI,   7,       ARM_ISAR_PLI,  "pli" ),
    ATTR_SET_PLD   (PLDW,  ARM_VD2, ARM_ISAR_PLD,  "pldw"), // NOTE: nop on ARMv6T2, ARMv7
    ATTR_SET_DMB   (DMB,   7,       ARM_ISAR_DMB,  "dmb" ),
    ATTR_SET_DMB   (DSB,   ARM_VT2, ARM_ISAR_DMB,  "dsb" ),
    ATTR_SET_DMB   (ISB,   ARM_VT2, ARM_ISAR_DMB,  "isb" ),
    ATTR_SET_UHINT (UHINT, 7,       ARM_ISAR_PLDW, "nop" ),

    ////////////////////////////////////////////////////////////////////////////
    // DSP INSTRUCTIONS
    ////////////////////////////////////////////////////////////////////////////

    // data processing instructions
    ATTR_SET_QADD (QADD,  ARM_VD, ARM_ISAR_QADD, "qadd" ),
    ATTR_SET_QADD (QDADD, ARM_VD, ARM_ISAR_QADD, "qdadd"),
    ATTR_SET_QADD (QDSUB, ARM_VD, ARM_ISAR_QADD, "qdsub"),
    ATTR_SET_QADD (QSUB,  ARM_VD, ARM_ISAR_QADD, "qsub" ),

    // multiply instructions
    ATTR_SET_SMLA_XY  (SMLA,  ARM_VD, ARM_ISAR_SMLABB, "smla" ),
    ATTR_SET_SMLAL_XY (SMLAL, ARM_VD, ARM_ISAR_SMLABB, "smlal"),
    ATTR_SET_SMLAW_Y  (SMLAW, ARM_VD, ARM_ISAR_SMLABB, "smlaw"),
    ATTR_SET_SMUL_XY  (SMUL,  ARM_VD, ARM_ISAR_SMLABB, "smul" ),
    ATTR_SET_SMULW_Y  (SMULW, ARM_VD, ARM_ISAR_SMLABB, "smulw"),

    // load and store instructions
    ATTR_SET_LDRH (LDRD, ARM_VD2, ARM_ISAR_LDRD, "ldr", 8, False, ARM_UA_ALIGN, ARM_UA_DABORT),
    ATTR_SET_LDRH (STRD, ARM_VD2, ARM_ISAR_LDRD, "str", 8, False, ARM_UA_ALIGN, ARM_UA_DABORT),

    // coprocessor instructions
    ATTR_SET_MCRR  (MCRR,  ARM_VD2, ARM_ISAR_NA, "mcrr" ),
    ATTR_SET_MCRR2 (MCRR2, 6,       ARM_ISAR_NA, "mcrr2"),
    ATTR_SET_MCRR  (MRRC,  ARM_VD2, ARM_ISAR_NA, "mrrc" ),
    ATTR_SET_MCRR2 (MRRC2, 6,       ARM_ISAR_NA, "mrrc2"),

    ////////////////////////////////////////////////////////////////////////////
    // MEDIA INSTRUCTIONS
    ////////////////////////////////////////////////////////////////////////////

    // basic instructions
    ATTR_SET_USAD8  (USAD8,  6,       ARM_ISAR_PKHBT, "usad8" ),
    ATTR_SET_USADA8 (USADA8, 6,       ARM_ISAR_PKHBT, "usada8"),
    ATTR_SET_SBFX   (SBFX,   ARM_VT2, ARM_ISAR_BFC,   "sbfx"  ),
    ATTR_SET_BFC    (BFC,    ARM_VT2, ARM_ISAR_BFC,   "bfc"   ),
    ATTR_SET_BFI    (BFI,    ARM_VT2, ARM_ISAR_BFC,   "bfi"   ),
    ATTR_SET_SBFX   (UBFX,   ARM_VT2, ARM_ISAR_BFC,   "ubfx"  ),

    // parallel add/subtract instructions
    ATTR_SET_PAS (ADD16, 6, ARM_ISAR_PKHBT, "add16"),
    ATTR_SET_PAS (ASX,   6, ARM_ISAR_PKHBT, "asx"  ),
    ATTR_SET_PAS (SAX,   6, ARM_ISAR_PKHBT, "sax"  ),
    ATTR_SET_PAS (SUB16, 6, ARM_ISAR_PKHBT, "sub16"),
    ATTR_SET_PAS (ADD8,  6, ARM_ISAR_PKHBT, "add8" ),
    ATTR_SET_PAS (SUB8,  6, ARM_ISAR_PKHBT, "sub8" ),

    // packing, unpacking, saturation and reversal instructions
    ATTR_SET_PKHBT   (PKHBT,   6,       ARM_ISAR_PKHBT,  "pkhbt"  ),
    ATTR_SET_PKHBT   (PKHTB,   6,       ARM_ISAR_PKHBT,  "pkhtb"  ),
    ATTR_SET_SSAT    (SSAT,    6,       ARM_ISAR_SSAT,   "ssat"   ),
    ATTR_SET_SSAT16  (SSAT16,  6,       ARM_ISAR_PKHBT,  "ssat16" ),
    ATTR_SET_USAT    (USAT,    6,       ARM_ISAR_SSAT,   "usat"   ),
    ATTR_SET_USAT16  (USAT16,  6,       ARM_ISAR_PKHBT,  "usat16" ),
    ATTR_SET_SXTAB16 (SXTAB,   6,       ARM_ISAR_SXTAB,  "sxtab"  ),
    ATTR_SET_SXTAB16 (UXTAB,   6,       ARM_ISAR_SXTAB,  "uxtab"  ),
    ATTR_SET_SXTAB16 (SXTAB16, 6,       ARM_ISAR_SXTB16, "sxtab16"),
    ATTR_SET_SXTAB16 (UXTAB16, 6,       ARM_ISAR_SXTB16, "uxtab16"),
    ATTR_SET_SXTAB16 (SXTAH,   6,       ARM_ISAR_SXTAB,  "sxtah"  ),
    ATTR_SET_SXTAB16 (UXTAH,   6,       ARM_ISAR_SXTAB,  "uxtah"  ),
    ATTR_SET_SXTB16  (SXTB,    6,       ARM_ISAR_SXTB,   "sxtb"   ),
    ATTR_SET_SXTB16  (UXTB,    6,       ARM_ISAR_SXTB,   "uxtb"   ),
    ATTR_SET_SXTB16  (SXTB16,  6,       ARM_ISAR_SXTB16, "sxtb16" ),
    ATTR_SET_SXTB16  (UXTB16,  6,       ARM_ISAR_SXTB16, "uxtb16" ),
    ATTR_SET_SXTB16  (SXTH,    6,       ARM_ISAR_SXTB,   "sxth"   ),
    ATTR_SET_SXTB16  (UXTH,    6,       ARM_ISAR_SXTB,   "uxth"   ),
    ATTR_SET_SEL     (SEL,     6,       ARM_ISAR_PKHBT,  "sel"    ),
    ATTR_SET_CLZ     (REV,     6,       ARM_ISAR_REV,    "rev"    ),
    ATTR_SET_CLZ     (REV16,   6,       ARM_ISAR_REV,    "rev16"  ),
    ATTR_SET_CLZ     (RBIT,    ARM_VT2, ARM_ISAR_RBIT,   "rbit"   ),
    ATTR_SET_CLZ     (REVSH,   6,       ARM_ISAR_REV,    "revsh"  ),

    // signed multiply instructions
    ATTR_SET_SMLAD  (SMLAD,  ARM_VT2, ARM_ISAR_SMLAD, "smlad" ),
    ATTR_SET_SMUAD  (SMUAD,  ARM_VT2, ARM_ISAR_SMLAD, "smuad" ),
    ATTR_SET_SMLAD  (SMLSD,  ARM_VT2, ARM_ISAR_SMLAD, "smlsd" ),
    ATTR_SET_SMUAD  (SMUSD,  ARM_VT2, ARM_ISAR_SMLAD, "smusd" ),
    ATTR_SET_SMLALD (SMLALD, ARM_VT2, ARM_ISAR_SMLAD, "smlald"),
    ATTR_SET_SMLALD (SMLSLD, ARM_VT2, ARM_ISAR_SMLAD, "smlsld"),
    ATTR_SET_SMMLA  (SMMLA,  ARM_VT2, ARM_ISAR_SMLAD, "smmla" ),
    ATTR_SET_SMMUL  (SMMUL,  ARM_VT2, ARM_ISAR_SMLAD, "smmul" ),
    ATTR_SET_SMMLA  (SMMLS,  ARM_VT2, ARM_ISAR_SMLAD, "smmls" ),

    ////////////////////////////////////////////////////////////////////////////
    // SIMD/VFP INSTRUCTIONS
    ////////////////////////////////////////////////////////////////////////////

    // SIMD data processing instructions - Misc
    ATTR_SET_VEXT  (VEXT,        7, ARM_ISAR_SIMDINT, "vext"),
    ATTR_SET_VTBL  (VTBL,        7, ARM_ISAR_SIMDINT, "vtbl"),
    ATTR_SET_VTBL  (VTBX,        7, ARM_ISAR_SIMDINT, "vtbx"),
    ATTR_SET_VDUPZ (VDUPZ,       7, ARM_ISAR_SIMDINT, "vdup"),

    // SIMD data processing instructions - 3 regs same length
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VHADDU,      7, ARM_ISAR_SIMDINT, "vhadd",   _U),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VHADDS,      7, ARM_ISAR_SIMDINT, "vhadd",   _S),
    ATTR_SET_SIMD_RRR_QD_BHWD_T (VQADDU,      7, ARM_ISAR_SIMDINT, "vqadd",   _U),
    ATTR_SET_SIMD_RRR_QD_BHWD_T (VQADDS,      7, ARM_ISAR_SIMDINT, "vqadd",   _S),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VRHADDU,     7, ARM_ISAR_SIMDINT, "vrhadd",  _U),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VRHADDS,     7, ARM_ISAR_SIMDINT, "vrhadd",  _S),
    ATTR_SET_SIMD_RRR_QD        (VAND,        7, ARM_ISAR_SIMDINT, "vand"),
    ATTR_SET_SIMD_RRR_QD        (VBIC,        7, ARM_ISAR_SIMDINT, "vbic"),
    ATTR_SET_SIMD_RRR_QD        (VORR,        7, ARM_ISAR_SIMDINT, "vorr"),
    ATTR_SET_SIMD_RRR_QD        (VORN,        7, ARM_ISAR_SIMDINT, "vorn"),
    ATTR_SET_SIMD_RRR_QD        (VEOR,        7, ARM_ISAR_SIMDINT, "veor"),
    ATTR_SET_SIMD_RRR_QD        (VBSL,        7, ARM_ISAR_SIMDINT, "vbsl"),
    ATTR_SET_SIMD_RRR_QD        (VBIT,        7, ARM_ISAR_SIMDINT, "vbit"),
    ATTR_SET_SIMD_RRR_QD        (VBIF,        7, ARM_ISAR_SIMDINT, "vbif"),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VHSUBU,      7, ARM_ISAR_SIMDINT, "vhsub",   _U),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VHSUBS,      7, ARM_ISAR_SIMDINT, "vhsub",   _S),
    ATTR_SET_SIMD_RRR_QD_BHWD_T (VQSUBU,      7, ARM_ISAR_SIMDINT, "vqsub",   _U),
    ATTR_SET_SIMD_RRR_QD_BHWD_T (VQSUBS,      7, ARM_ISAR_SIMDINT, "vqsub",   _S),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VCGTU,       7, ARM_ISAR_SIMDINT, "vcgt",    _U),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VCGTS,       7, ARM_ISAR_SIMDINT, "vcgt",    _S),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VCGEU,       7, ARM_ISAR_SIMDINT, "vcge",    _U),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VCGES,       7, ARM_ISAR_SIMDINT, "vcge",    _S),
    ATTR_SET_SIMD_DMN_QD_BHWD_T (VSHLU,       7, ARM_ISAR_SIMDINT, "vshl",    _U),
    ATTR_SET_SIMD_DMN_QD_BHWD_T (VSHLS,       7, ARM_ISAR_SIMDINT, "vshl",    _S),
    ATTR_SET_SIMD_DMN_QD_BHWD_T (VQSHLU,      7, ARM_ISAR_SIMDINT, "vqshl",   _U),
    ATTR_SET_SIMD_DMN_QD_BHWD_T (VQSHLS,      7, ARM_ISAR_SIMDINT, "vqshl",   _S),
    ATTR_SET_SIMD_DMN_QD_BHWD_T (VRSHLU,      7, ARM_ISAR_SIMDINT, "vrshl",   _U),
    ATTR_SET_SIMD_DMN_QD_BHWD_T (VRSHLS,      7, ARM_ISAR_SIMDINT, "vrshl",   _S),
    ATTR_SET_SIMD_DMN_QD_BHWD_T (VQRSHLU,     7, ARM_ISAR_SIMDINT, "vqrshl",  _U),
    ATTR_SET_SIMD_DMN_QD_BHWD_T (VQRSHLS,     7, ARM_ISAR_SIMDINT, "vqrshl",  _S),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VMAXU,       7, ARM_ISAR_SIMDINT, "vmax",    _U),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VMAXS,       7, ARM_ISAR_SIMDINT, "vmax",    _S),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VMINU,       7, ARM_ISAR_SIMDINT, "vmin",    _U),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VMINS,       7, ARM_ISAR_SIMDINT, "vmin",    _S),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VABDU,       7, ARM_ISAR_SIMDINT, "vabd",    _U),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VABDS,       7, ARM_ISAR_SIMDINT, "vabd",    _S),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VABAU,       7, ARM_ISAR_SIMDINT, "vaba",    _U),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VABAS,       7, ARM_ISAR_SIMDINT, "vaba",    _S),
    ATTR_SET_SIMD_RRR_QD_BHWD_T (VADD,        7, ARM_ISAR_SIMDINT, "vadd",    _I),
    ATTR_SET_SIMD_RRR_QD_BHWD_T (VSUB,        7, ARM_ISAR_SIMDINT, "vsub",    _I),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VTST,        7, ARM_ISAR_SIMDINT, "vtst",    _),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VCEQ,        7, ARM_ISAR_SIMDINT, "vceq",    _I),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VMLA,        7, ARM_ISAR_SIMDINT, "vmla",    _I),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VMLS,        7, ARM_ISAR_SIMDINT, "vmls",    _I),
    ATTR_SET_SIMD_RRR_QD_BHW_T  (VMUL,        7, ARM_ISAR_SIMDINT, "vmul",    _I),
    ATTR_SET_SIMD_RRR_QD_TS     (VMUL_P,      7, ARM_ISAR_SIMDINT, "vmul",    _P8),
    ATTR_SET_SIMD_RRR_D_BHW_T   (VPMAXU,      7, ARM_ISAR_SIMDINT, "vpmax",   _U),
    ATTR_SET_SIMD_RRR_D_BHW_T   (VPMAXS,      7, ARM_ISAR_SIMDINT, "vpmax",   _S),
    ATTR_SET_SIMD_RRR_D_BHW_T   (VPMINU,      7, ARM_ISAR_SIMDINT, "vpmin",   _U),
    ATTR_SET_SIMD_RRR_D_BHW_T   (VPMINS,      7, ARM_ISAR_SIMDINT, "vpmin",   _S),
    ATTR_SET_SIMD_RRR_QD_HW_T   (VQDMULH,     7, ARM_ISAR_SIMDINT, "vqdmulh", _S),
    ATTR_SET_SIMD_RRR_QD_HW_T   (VQRDMULH,    7, ARM_ISAR_SIMDINT, "vqrdmulh",_S),
    ATTR_SET_SIMD_RRR_D_BHW_T   (VPADD,       7, ARM_ISAR_SIMDINT, "vpadd",   _I),
    ATTR_SET_SIMD_RRR_QD_TS     (VADD_F,      7, ARM_ISAR_SIMDSP,  "vadd",    _F32),
    ATTR_SET_SIMD_RRR_QD_TS     (VSUB_F,      7, ARM_ISAR_SIMDSP,  "vsub",    _F32),
    ATTR_SET_SIMD_RRR_D_TS      (VPADD_F,     7, ARM_ISAR_SIMDSP,  "vpadd",   _F32),
    ATTR_SET_SIMD_RRR_QD_TS     (VABD_F,      7, ARM_ISAR_SIMDSP,  "vabd",    _F32),
    ATTR_SET_SIMD_RRR_QD_TS     (VMLA_F,      7, ARM_ISAR_SIMDSP,  "vmla",    _F32),
    ATTR_SET_SIMD_RRR_QD_TS     (VMLS_F,      7, ARM_ISAR_SIMDSP,  "vmls",    _F32),
    ATTR_SET_SIMD_RRR_QD_TS     (VMUL_F,      7, ARM_ISAR_SIMDSP,  "vmul",    _F32),
    ATTR_SET_SIMD_RRR_QD_TS     (VCEQ_F,      7, ARM_ISAR_SIMDSP,  "vceq",    _F32),
    ATTR_SET_SIMD_RRR_QD_TS     (VCGE_F,      7, ARM_ISAR_SIMDSP,  "vcge",    _F32),
    ATTR_SET_SIMD_RRR_QD_TS     (VCGT_F,      7, ARM_ISAR_SIMDSP,  "vcgt",    _F32),
    ATTR_SET_SIMD_RRR_QD_TS     (VACGE_F,     7, ARM_ISAR_SIMDSP,  "vacge",   _F32),
    ATTR_SET_SIMD_RRR_QD_TS     (VACGT_F,     7, ARM_ISAR_SIMDSP,  "vacgt",   _F32),
    ATTR_SET_SIMD_RRR_QD_TS     (VMAX_F,      7, ARM_ISAR_SIMDSP,  "vmax",    _F32),
    ATTR_SET_SIMD_RRR_QD_TS     (VMIN_F,      7, ARM_ISAR_SIMDSP,  "vmin",    _F32),
    ATTR_SET_SIMD_RRR_D_TS      (VPMAX_F,     7, ARM_ISAR_SIMDSP,  "vpmax",   _F32),
    ATTR_SET_SIMD_RRR_D_TS      (VPMIN_F,     7, ARM_ISAR_SIMDSP,  "vpmin",   _F32),
    ATTR_SET_SIMD_RRR_QD_TS     (VRECPS,      7, ARM_ISAR_SIMDSP,  "vrecps",  _F32),
    ATTR_SET_SIMD_RRR_QD_TS     (VRSQRTS,     7, ARM_ISAR_SIMDSP,  "vrsqrts", _F32),

    // SIMD data processing instructions - 3 regs different lengths
    ATTR_SET_SIMD_L_BHW_T (VADDLS,    7, ARM_ISAR_SIMDINT, "vaddl",   _S),
    ATTR_SET_SIMD_L_BHW_T (VADDLU,    7, ARM_ISAR_SIMDINT, "vaddl",   _U),
    ATTR_SET_SIMD_W_BHW_T (VADDWS,    7, ARM_ISAR_SIMDINT, "vaddw",   _S),
    ATTR_SET_SIMD_W_BHW_T (VADDWU,    7, ARM_ISAR_SIMDINT, "vaddw",   _U),
    ATTR_SET_SIMD_L_BHW_T (VSUBLS,    7, ARM_ISAR_SIMDINT, "vsubl",   _S),
    ATTR_SET_SIMD_L_BHW_T (VSUBLU,    7, ARM_ISAR_SIMDINT, "vsubl",   _U),
    ATTR_SET_SIMD_W_BHW_T (VSUBWS,    7, ARM_ISAR_SIMDINT, "vsubw",   _S),
    ATTR_SET_SIMD_W_BHW_T (VSUBWU,    7, ARM_ISAR_SIMDINT, "vsubw",   _U),
    ATTR_SET_SIMD_N_HWD_T (VADDHN,    7, ARM_ISAR_SIMDINT, "vaddhn",  _I),
    ATTR_SET_SIMD_N_HWD_T (VRADDHN,   7, ARM_ISAR_SIMDINT, "vraddhn", _I),
    ATTR_SET_SIMD_L_BHW_T (VABALS,    7, ARM_ISAR_SIMDINT, "vabal",   _S),
    ATTR_SET_SIMD_L_BHW_T (VABALU,    7, ARM_ISAR_SIMDINT, "vabal",   _U),
    ATTR_SET_SIMD_N_HWD_T (VSUBHN,    7, ARM_ISAR_SIMDINT, "vsubhn",  _I),
    ATTR_SET_SIMD_N_HWD_T (VRSUBHN,   7, ARM_ISAR_SIMDINT, "vrsubhn", _I),
    ATTR_SET_SIMD_L_BHW_T (VABDLS,    7, ARM_ISAR_SIMDINT, "vabdl",   _S),
    ATTR_SET_SIMD_L_BHW_T (VABDLU,    7, ARM_ISAR_SIMDINT, "vabdl",   _U),
    ATTR_SET_SIMD_L_BHW_T (VMLALS,    7, ARM_ISAR_SIMDINT, "vmlal",   _S),
    ATTR_SET_SIMD_L_BHW_T (VMLALU,    7, ARM_ISAR_SIMDINT, "vmlal",   _U),
    ATTR_SET_SIMD_L_BHW_T (VMLSLS,    7, ARM_ISAR_SIMDINT, "vmlsl",   _S),
    ATTR_SET_SIMD_L_BHW_T (VMLSLU,    7, ARM_ISAR_SIMDINT, "vmlsl",   _U),
    ATTR_SET_SIMD_L_HW_T  (VQDMLAL,   7, ARM_ISAR_SIMDINT, "vqdmlal", _S),
    ATTR_SET_SIMD_L_HW_T  (VQDMLSL,   7, ARM_ISAR_SIMDINT, "vqdmlsl", _S),
    ATTR_SET_SIMD_L_BHW_T (VMULLS,    7, ARM_ISAR_SIMDINT, "vmull",   _S),
    ATTR_SET_SIMD_L_BHW_T (VMULLU,    7, ARM_ISAR_SIMDINT, "vmull",   _U),
    ATTR_SET_SIMD_L_HW_T  (VQDMULL,   7, ARM_ISAR_SIMDINT, "vqdmull", _S),
    ATTR_SET_SIMD_L_P     (VMULL_P,   7, ARM_ISAR_SIMDINT, "vmull"),

    // SIMD data processing instructions - 2 regs and a scalar
    ATTR_SET_SIMD_RRZ_QD_HW_T (VMLAZ,     7, ARM_ISAR_SIMDINT, "vmla",     _I),
    ATTR_SET_SIMD_RRZ_QD_T    (VMLAZ_F,   7, ARM_ISAR_SIMDSP,  "vmla",     _F),
    ATTR_SET_SIMD_RRZ_QD_HW_T (VMLSZ,     7, ARM_ISAR_SIMDINT, "vmls",     _I),
    ATTR_SET_SIMD_RRZ_QD_T    (VMLSZ_F,   7, ARM_ISAR_SIMDSP,  "vmls",     _F),
    ATTR_SET_SIMD_LZ_HW_T     (VMLALZS,   7, ARM_ISAR_SIMDINT, "vmlal",    _S),
    ATTR_SET_SIMD_LZ_HW_T     (VMLALZU,   7, ARM_ISAR_SIMDINT, "vmlal",    _U),
    ATTR_SET_SIMD_LZ_HW_T     (VMLSLZS,   7, ARM_ISAR_SIMDINT, "vmlsl",    _S),
    ATTR_SET_SIMD_LZ_HW_T     (VMLSLZU,   7, ARM_ISAR_SIMDINT, "vmlsl",    _U),
    ATTR_SET_SIMD_LZ_HW_T     (VQDMLALZ,  7, ARM_ISAR_SIMDINT, "vqdmlal",  _S),
    ATTR_SET_SIMD_LZ_HW_T     (VQDMLSLZ,  7, ARM_ISAR_SIMDINT, "vqdmlsl",  _S),
    ATTR_SET_SIMD_RRZ_QD_HW_T (VMULZ,     7, ARM_ISAR_SIMDINT, "vmul",     _I),
    ATTR_SET_SIMD_RRZ_QD_T    (VMULZ_F,   7, ARM_ISAR_SIMDSP,  "vmul",     _F),
    ATTR_SET_SIMD_LZ_HW_T     (VMULLZS,   7, ARM_ISAR_SIMDINT, "vmull",    _S),
    ATTR_SET_SIMD_LZ_HW_T     (VMULLZU,   7, ARM_ISAR_SIMDINT, "vmull",    _U),
    ATTR_SET_SIMD_LZ_HW_T     (VQDMULLZ,  7, ARM_ISAR_SIMDINT, "vqdmull",  _S),
    ATTR_SET_SIMD_RRZ_QD_HW_T (VQDMULHZ,  7, ARM_ISAR_SIMDINT, "vqdmulh",  _S),
    ATTR_SET_SIMD_RRZ_QD_HW_T (VQRDMULHZ, 7, ARM_ISAR_SIMDINT, "vqrdmulh", _S),

    // SIMD data processing instructions - 2 regs and a shift amount
    ATTR_SET_SIMD_RRI_QD_BHWD_T (VSHRS,    7, ARM_ISAR_SIMDINT, "vshr",    _S, N),
    ATTR_SET_SIMD_RRI_QD_BHWD_T (VSHRU,    7, ARM_ISAR_SIMDINT, "vshr",    _U, N),
    ATTR_SET_SIMD_RRI_QD_BHWD_T (VSRAS,    7, ARM_ISAR_SIMDINT, "vsra",    _S, N),
    ATTR_SET_SIMD_RRI_QD_BHWD_T (VSRAU,    7, ARM_ISAR_SIMDINT, "vsra",    _U, N),
    ATTR_SET_SIMD_RRI_QD_BHWD_T (VRSHRS,   7, ARM_ISAR_SIMDINT, "vrshr",   _S, N),
    ATTR_SET_SIMD_RRI_QD_BHWD_T (VRSHRU,   7, ARM_ISAR_SIMDINT, "vrshr",   _U, N),
    ATTR_SET_SIMD_RRI_QD_BHWD_T (VRSRAS,   7, ARM_ISAR_SIMDINT, "vrsra",   _S, N),
    ATTR_SET_SIMD_RRI_QD_BHWD_T (VRSRAU,   7, ARM_ISAR_SIMDINT, "vrsra",   _U, N),
    ATTR_SET_SIMD_RRI_QD_BHWD_T (VSRI,     7, ARM_ISAR_SIMDINT, "vsri",    _,  N),
    ATTR_SET_SIMD_RRI_QD_BHWD_T (VSHL,     7, ARM_ISAR_SIMDINT, "vshl",    _S, P),
    ATTR_SET_SIMD_RRI_QD_BHWD_T (VSLI,     7, ARM_ISAR_SIMDINT, "vsli",    _,  P),
    ATTR_SET_SIMD_RRI_QD_BHWD_T (VQSHLUS,  7, ARM_ISAR_SIMDINT, "vqshlu",  _S, P),
    ATTR_SET_SIMD_RRI_QD_BHWD_T (VQSHLSS,  7, ARM_ISAR_SIMDINT, "vqshl",   _S, P),
    ATTR_SET_SIMD_RRI_QD_BHWD_T (VQSHLSU,  7, ARM_ISAR_SIMDINT, "vqshl",   _U, P),
    ATTR_SET_SIMD_NI_HWD_T      (VSHRN,    7, ARM_ISAR_SIMDINT, "vshrn",   _I),
    ATTR_SET_SIMD_NI_HWD_T      (VRSHRN,   7, ARM_ISAR_SIMDINT, "vrshrn",  _I),
    ATTR_SET_SIMD_NI_HWD_T      (VQSHRUNS, 7, ARM_ISAR_SIMDINT, "vqshrun", _S),
    ATTR_SET_SIMD_NI_HWD_T      (VQRSHRUNS,7, ARM_ISAR_SIMDINT, "vqrshrun",_S),
    ATTR_SET_SIMD_NI_HWD_T      (VQSHRNS,  7, ARM_ISAR_SIMDINT, "vqshrn",  _S),
    ATTR_SET_SIMD_NI_HWD_T      (VQSHRNU,  7, ARM_ISAR_SIMDINT, "vqshrn",  _U),
    ATTR_SET_SIMD_NI_HWD_T      (VQRSHRNS, 7, ARM_ISAR_SIMDINT, "vqrshrn", _S),
    ATTR_SET_SIMD_NI_HWD_T      (VQRSHRNU, 7, ARM_ISAR_SIMDINT, "vqrshrn", _U),
    ATTR_SET_SIMD_LI_BHW_T      (VSHLLS,   7, ARM_ISAR_SIMDINT, "vshll",   _S),
    ATTR_SET_SIMD_LI_BHW_T      (VSHLLU,   7, ARM_ISAR_SIMDINT, "vshll",   _U),
    ATTR_SET_SIMD_VMOVL         (VMOVLS,   7, ARM_ISAR_SIMDINT, "vmovl",   _S),
    ATTR_SET_SIMD_VMOVL         (VMOVLU,   7, ARM_ISAR_SIMDINT, "vmovl",   _U),
    ATTR_SET_SIMD_RRI_QD_W_T2   (VCVTFXS,  7, ARM_ISAR_SIMDSP,  "vcvt",    _F, _S),
    ATTR_SET_SIMD_RRI_QD_W_T2   (VCVTFXU,  7, ARM_ISAR_SIMDSP,  "vcvt",    _F, _U),
    ATTR_SET_SIMD_RRI_QD_W_T2   (VCVTXFS,  7, ARM_ISAR_SIMDSP,  "vcvt",    _S, _F),
    ATTR_SET_SIMD_RRI_QD_W_T2   (VCVTXFU,  7, ARM_ISAR_SIMDSP,  "vcvt",    _U, _F),

    // SIMD data processing instructions - Two registers, Miscellaneous
    ATTR_SET_SIMD_VREV          (VREV,     7, ARM_ISAR_SIMDINT, "vrev"),
    ATTR_SET_SIMD_RR_QD_BHW_T   (VPADDLS,  7, ARM_ISAR_SIMDINT, "vpaddl",  _S),
    ATTR_SET_SIMD_RR_QD_BHW_T   (VPADDLU,  7, ARM_ISAR_SIMDINT, "vpaddl",  _U),
    ATTR_SET_SIMD_RR_QD_BHW_T   (VCLS,     7, ARM_ISAR_SIMDINT, "vcls",    _S),
    ATTR_SET_SIMD_RR_QD_BHW_T   (VCLZ,     7, ARM_ISAR_SIMDINT, "vclz",    _I),
    ATTR_SET_SIMD_RR_QD_TS      (VCNT,     7, ARM_ISAR_SIMDINT, "vcnt",    _8),
    ATTR_SET_SIMD_RR_QD         (VMVN,     7, ARM_ISAR_SIMDINT, "vmvn"),
    ATTR_SET_SIMD_RR_QD_BHW_T   (VPADALS,  7, ARM_ISAR_SIMDINT, "vpadal",  _S),
    ATTR_SET_SIMD_RR_QD_BHW_T   (VPADALU,  7, ARM_ISAR_SIMDINT, "vpadal",  _U),
    ATTR_SET_SIMD_RR_QD_BHW_T   (VQABS,    7, ARM_ISAR_SIMDINT, "vqabs",   _S),
    ATTR_SET_SIMD_RR_QD_BHW_T   (VQNEG,    7, ARM_ISAR_SIMDINT, "vqneg",   _S),
    ATTR_SET_SIMD_RR0_QD_BHW_T  (VCGT0,    7, ARM_ISAR_SIMDINT, "vcgt",    _S),
    ATTR_SET_SIMD_RR0_QD_BHW_T  (VCGE0,    7, ARM_ISAR_SIMDINT, "vcge",    _S),
    ATTR_SET_SIMD_RR0_QD_BHW_T  (VCEQ0,    7, ARM_ISAR_SIMDINT, "vceq",    _I),
    ATTR_SET_SIMD_RR0_QD_BHW_T  (VCLE0,    7, ARM_ISAR_SIMDINT, "vcle",    _S),
    ATTR_SET_SIMD_RR0_QD_BHW_T  (VCLT0,    7, ARM_ISAR_SIMDINT, "vclt",    _S),
    ATTR_SET_SIMD_RR_QD_BHW_T   (VABS,     7, ARM_ISAR_SIMDINT, "vabs",    _S),
    ATTR_SET_SIMD_RR_QD_BHW_T   (VNEG,     7, ARM_ISAR_SIMDINT, "vneg",    _S),
    ATTR_SET_SIMD_RR0_QD_T      (VCGT0_F,  7, ARM_ISAR_SIMDSP,  "vcgt",    _F),
    ATTR_SET_SIMD_RR0_QD_T      (VCGE0_F,  7, ARM_ISAR_SIMDSP,  "vcge",    _F),
    ATTR_SET_SIMD_RR0_QD_T      (VCEQ0_F,  7, ARM_ISAR_SIMDSP,  "vceq",    _F),
    ATTR_SET_SIMD_RR0_QD_T      (VCLE0_F,  7, ARM_ISAR_SIMDSP,  "vcle",    _F),
    ATTR_SET_SIMD_RR0_QD_T      (VCLT0_F,  7, ARM_ISAR_SIMDSP,  "vclt",    _F),
    ATTR_SET_SIMD_RR_QD_TS      (VABS_F,   7, ARM_ISAR_SIMDSP,  "vabs",    _F32),
    ATTR_SET_SIMD_RR_QD_TS      (VNEG_F,   7, ARM_ISAR_SIMDSP,  "vneg",    _F32),
    ATTR_SET_SIMD_RR_QD         (VSWP,     7, ARM_ISAR_SIMDINT, "vswp"),
    ATTR_SET_SIMD_RR_QD_BHW_T   (VTRN,     7, ARM_ISAR_SIMDINT, "vtrn",    _),
    ATTR_SET_SIMD_RR_QBHW_DBH_T (VUZP,     7, ARM_ISAR_SIMDINT, "vuzp",    _),
    ATTR_SET_SIMD_RR_QBHW_DBH_T (VZIP,     7, ARM_ISAR_SIMDINT, "vzip",    _),
    ATTR_SET_SIMD_N2_HWD_T      (VMOVN,    7, ARM_ISAR_SIMDINT, "vmovn",   _I),
    ATTR_SET_SIMD_N2_HWD_T      (VQMOVUNS, 7, ARM_ISAR_SIMDINT, "vqmovun", _S),
    ATTR_SET_SIMD_N2_HWD_T      (VQMOVNS,  7, ARM_ISAR_SIMDINT, "vqmovn",  _S),
    ATTR_SET_SIMD_N2_HWD_T      (VQMOVNU,  7, ARM_ISAR_SIMDINT, "vqmovn",  _U),
    ATTR_SET_SIMD_L2_BHW_T      (VSHLLM,   7, ARM_ISAR_SIMDINT, "vshll",   _I),
    ATTR_SET_SIMD_N2_S2         (VCVTHS,   7, ARM_ISAR_SIMDHP,  "vcvt",     16, 32),
    ATTR_SET_SIMD_L2_S2         (VCVTSH,   7, ARM_ISAR_SIMDHP,  "vcvt",     32, 16),
    ATTR_SET_SIMD_RR_QD_TS      (VRECPE,   7, ARM_ISAR_SIMDINT, "vrecpe",  _U32),
    ATTR_SET_SIMD_RR_QD_TS      (VRECPE_F, 7, ARM_ISAR_SIMDSP,  "vrecpe",  _F32),
    ATTR_SET_SIMD_RR_QD_TS      (VRSQRTE,  7, ARM_ISAR_SIMDINT, "vrsqrte", _U32),
    ATTR_SET_SIMD_RR_QD_TS      (VRSQRTE_F,7, ARM_ISAR_SIMDSP,  "vrsqrte", _F32),
    ATTR_SET_SIMD_RR_QD_TS2     (VCVTFS,   7, ARM_ISAR_SIMDSP,  "vcvt",    _F32, _S32),
    ATTR_SET_SIMD_RR_QD_TS2     (VCVTFU,   7, ARM_ISAR_SIMDSP,  "vcvt",    _F32, _U32),
    ATTR_SET_SIMD_RR_QD_TS2     (VCVTSF,   7, ARM_ISAR_SIMDSP,  "vcvt",    _S32, _F32),
    ATTR_SET_SIMD_RR_QD_TS2     (VCVTUF,   7, ARM_ISAR_SIMDSP,  "vcvt",    _U32, _F32),

    // SIMD data processing instructions - One register and a modified immediate
    ATTR_SET_SIMD_RI_DS (VMOVI_W,    7, ARM_ISAR_SIMDINT, "vmov", _I32),
    ATTR_SET_SIMD_RI_DS (VORRI_W,    7, ARM_ISAR_SIMDINT, "vorr", _I32),
    ATTR_SET_SIMD_RI_DS (VMOVI_H,    7, ARM_ISAR_SIMDINT, "vmov", _I16),
    ATTR_SET_SIMD_RI_DS (VORRI_H,    7, ARM_ISAR_SIMDINT, "vorr", _I16),
    ATTR_SET_SIMD_RI_DS (VMOVI1_W,   7, ARM_ISAR_SIMDINT, "vmov", _I32),
    ATTR_SET_SIMD_RI_DS (VMOVI_B,    7, ARM_ISAR_SIMDINT, "vmov", _I8),
    ATTR_SET_SIMD_RI_DS (VMOVI_F_W,  7, ARM_ISAR_SIMDSP,  "vmov", _F32),
    ATTR_SET_SIMD_RI_DS (VMVNI_W,    7, ARM_ISAR_SIMDINT, "vmvn", _I32),
    ATTR_SET_SIMD_RI_DS (VBICI_W,    7, ARM_ISAR_SIMDINT, "vbic", _I32),
    ATTR_SET_SIMD_RI_DS (VMVNI_H,    7, ARM_ISAR_SIMDINT, "vmvn", _I16),
    ATTR_SET_SIMD_RI_DS (VBICI_H,    7, ARM_ISAR_SIMDINT, "vbic", _I16),
    ATTR_SET_SIMD_RI_DS (VMVNI1_W,   7, ARM_ISAR_SIMDINT, "vmvn", _I32),
    ATTR_SET_SIMD_RI_DS (VMOVI_D,    7, ARM_ISAR_SIMDINT, "vmov", _I64),

    // VFP data processing instructions w/ 3 regs
    ATTR_SET_VFP_RRR_DS  (VMLA_VFP,    7, ARM_ISAR_VFPV2,  "vmla"),
    ATTR_SET_VFP_RRR_DS  (VMLS_VFP,    7, ARM_ISAR_VFPV2,  "vmls"),
    ATTR_SET_VFP_RRR_DS  (VNMLS_VFP,   7, ARM_ISAR_VFPV2,  "vnmls"),
    ATTR_SET_VFP_RRR_DS  (VNMLA_VFP,   7, ARM_ISAR_VFPV2,  "vnmla"),
    ATTR_SET_VFP_RRR_DS  (VMUL_VFP,    7, ARM_ISAR_VFPV2,  "vmul"),
    ATTR_SET_VFP_RRR_DS  (VNMUL_VFP,   7, ARM_ISAR_VFPV2,  "vnmul"),
    ATTR_SET_VFP_RRR_DS  (VADD_VFP,    7, ARM_ISAR_VFPV2,  "vadd"),
    ATTR_SET_VFP_RRR_DS  (VSUB_VFP,    7, ARM_ISAR_VFPV2,  "vsub"),
    ATTR_SET_VFP_RRR_DS  (VDIV_VFP,    7, ARM_ISAR_VFPDIV, "vdiv"),

    // VFP data processing instructions - other
    ATTR_SET_VFP_RI_DS     (VMOVI_VFP,    7, ARM_ISAR_VFPV3,   "vmov"),
    ATTR_SET_VFP_RR_DS     (VMOVR_VFP,    7, ARM_ISAR_VFPV2,   "vmov"),
    ATTR_SET_VFP_RR_DS     (VABS_VFP,     7, ARM_ISAR_VFPV2,   "vabs"),
    ATTR_SET_VFP_RR_DS     (VNEG_VFP,     7, ARM_ISAR_VFPV2,   "vneg"),
    ATTR_SET_VFP_RR_DS     (VSQRT_VFP,    7, ARM_ISAR_VFPSQRT, "vsqrt"),
    ATTR_SET_VFP_RR_S_S2   (VCVTBFH_VFP,  7, ARM_ISAR_VFPHP,   "vcvtb", 32, 16),
    ATTR_SET_VFP_RR_S_S2   (VCVTTFH_VFP,  7, ARM_ISAR_VFPHP,   "vcvtt", 32, 16),
    ATTR_SET_VFP_RR_S_S2   (VCVTBHF_VFP,  7, ARM_ISAR_VFPHP,   "vcvtb", 16, 32),
    ATTR_SET_VFP_RR_S_S2   (VCVTTHF_VFP,  7, ARM_ISAR_VFPHP,   "vcvtt", 16, 32),
    ATTR_SET_VFP_RR_DS     (VCMP_VFP,     7, ARM_ISAR_VFPV2,   "vcmp"),
    ATTR_SET_VFP_RR_DS     (VCMPE_VFP,    7, ARM_ISAR_VFPV2,   "vcmpe"),
    ATTR_SET_VFP_R0_DS     (VCMP0_VFP,    7, ARM_ISAR_VFPV2,   "vcmp"),
    ATTR_SET_VFP_R0_DS     (VCMPE0_VFP,   7, ARM_ISAR_VFPV2,   "vcmpe"),
    ATTR_SET_VFP_NL_DS     (VCVT_VFP,     7, ARM_ISAR_VFPCVT2, "vcvt"),
    ATTR_SET_VFP_LS_DS_T   (VCVTFU_VFP,   7, ARM_ISAR_VFPCVT2, "vcvt",  _U),
    ATTR_SET_VFP_LS_DS_T   (VCVTFS_VFP,   7, ARM_ISAR_VFPCVT2, "vcvt",  _S),
    ATTR_SET_VFP_RI_DS_T2C (VCVTFXUH_VFP, 7, ARM_ISAR_VFPCVT3, "vcvt",  _F32, _U16, _F64, _U16, 16),
    ATTR_SET_VFP_RI_DS_T2C (VCVTFXUW_VFP, 7, ARM_ISAR_VFPCVT3, "vcvt",  _F32, _U32, _F64, _U32, 32),
    ATTR_SET_VFP_RI_DS_T2C (VCVTFXSH_VFP, 7, ARM_ISAR_VFPCVT3, "vcvt",  _F32, _S16, _F64, _S16, 16),
    ATTR_SET_VFP_RI_DS_T2C (VCVTFXSW_VFP, 7, ARM_ISAR_VFPCVT3, "vcvt",  _F32, _S32, _F64, _S32, 32),
    ATTR_SET_VFP_NS_DS_T   (VCVTRUF_VFP,  7, ARM_ISAR_VFPCVT2, "vcvtr", _U),
    ATTR_SET_VFP_NS_DS_T   (VCVTUF_VFP,   7, ARM_ISAR_VFPCVT2, "vcvt",  _U),
    ATTR_SET_VFP_NS_DS_T   (VCVTRSF_VFP,  7, ARM_ISAR_VFPCVT2, "vcvtr", _S),
    ATTR_SET_VFP_NS_DS_T   (VCVTSF_VFP,   7, ARM_ISAR_VFPCVT2, "vcvt",  _S),
    ATTR_SET_VFP_RI_DS_T2C (VCVTXFSH_VFP, 7, ARM_ISAR_VFPCVT3, "vcvt",  _S16, _F32, _S16, _F64, 16),
    ATTR_SET_VFP_RI_DS_T2C (VCVTXFSW_VFP, 7, ARM_ISAR_VFPCVT3, "vcvt",  _S32, _F32, _S32, _F64, 32),
    ATTR_SET_VFP_RI_DS_T2C (VCVTXFUH_VFP, 7, ARM_ISAR_VFPCVT3, "vcvt",  _U16, _F32, _U16, _F64, 16),
    ATTR_SET_VFP_RI_DS_T2C (VCVTXFUW_VFP, 7, ARM_ISAR_VFPCVT3, "vcvt",  _U32, _F32, _U32, _F64, 32),

    // Extension register load/store instructions
    ATTR_SET_SDFP_LDSTM    (VSTMIA,  7, ARM_ISAR_VMRS, "vstm"),
    ATTR_SET_SDFP_LDSTM    (VSTMIAW, 7, ARM_ISAR_VMRS, "vstm"),
    ATTR_SET_SDFP_LDST     (VSTR,    7, ARM_ISAR_VMRS, "vstr"),
    ATTR_SET_SDFP_LDSTM    (VSTMDBW, 7, ARM_ISAR_VMRS, "vstm"),
    ATTR_SET_SDFP_PUSH     (VPUSH,   7, ARM_ISAR_VMRS, "vpush"),
    ATTR_SET_SDFP_LDSTM    (VLDMIA,  7, ARM_ISAR_VMRS, "vldm"),
    ATTR_SET_SDFP_LDSTM    (VLDMIAW, 7, ARM_ISAR_VMRS, "vldm"),
    ATTR_SET_SDFP_PUSH     (VPOP,    7, ARM_ISAR_VMRS, "vpop"),
    ATTR_SET_SDFP_LDST     (VLDR,    7, ARM_ISAR_VMRS, "vldr"),
    ATTR_SET_SDFP_LDSTM    (VLDMDBW, 7, ARM_ISAR_VMRS, "vldm"),

    // SIMD element or structure load/store instructions
    ATTR_SET_SIMD_LDSTN_BHWD      (VSTN1_R4,   7, ARM_ISAR_SIMDLDST, "vst1", 1, 4, 0),
    ATTR_SET_SIMD_LDSTN_BHWD      (VSTN1_R3,   7, ARM_ISAR_SIMDLDST, "vst1", 1, 3, 0),
    ATTR_SET_SIMD_LDSTN_BHWD      (VSTN1_R2,   7, ARM_ISAR_SIMDLDST, "vst1", 1, 2, 0),
    ATTR_SET_SIMD_LDSTN_BHWD      (VSTN1_R1,   7, ARM_ISAR_SIMDLDST, "vst1", 1, 1, 0),
    ATTR_SET_SIMD_LDSTN_BHW       (VSTN2_R2I2, 7, ARM_ISAR_SIMDLDST, "vst2", 2, 2, 2),
    ATTR_SET_SIMD_LDSTN_BHW       (VSTN2_R1I2, 7, ARM_ISAR_SIMDLDST, "vst2", 2, 1, 2),
    ATTR_SET_SIMD_LDSTN_BHW       (VSTN2_R1I1, 7, ARM_ISAR_SIMDLDST, "vst2", 2, 1, 1),
    ATTR_SET_SIMD_LDSTN_BHW       (VSTN3_R3I1, 7, ARM_ISAR_SIMDLDST, "vst3", 3, 1, 1),
    ATTR_SET_SIMD_LDSTN_BHW       (VSTN3_R3I2, 7, ARM_ISAR_SIMDLDST, "vst3", 3, 1, 2),
    ATTR_SET_SIMD_LDSTN_BHW       (VSTN4_R4I1, 7, ARM_ISAR_SIMDLDST, "vst4", 4, 1, 1),
    ATTR_SET_SIMD_LDSTN_BHW       (VSTN4_R4I2, 7, ARM_ISAR_SIMDLDST, "vst4", 4, 1, 2),
    ATTR_SET_SIMD_LDSTN_BHWD      (VLDN1_R4,   7, ARM_ISAR_SIMDLDST, "vld1", 1, 4, 0),
    ATTR_SET_SIMD_LDSTN_BHWD      (VLDN1_R3,   7, ARM_ISAR_SIMDLDST, "vld1", 1, 3, 0),
    ATTR_SET_SIMD_LDSTN_BHWD      (VLDN1_R2,   7, ARM_ISAR_SIMDLDST, "vld1", 1, 2, 0),
    ATTR_SET_SIMD_LDSTN_BHWD      (VLDN1_R1,   7, ARM_ISAR_SIMDLDST, "vld1", 1, 1, 0),
    ATTR_SET_SIMD_LDSTN_BHW       (VLDN2_R2I2, 7, ARM_ISAR_SIMDLDST, "vld2", 2, 2, 2),
    ATTR_SET_SIMD_LDSTN_BHW       (VLDN2_R1I2, 7, ARM_ISAR_SIMDLDST, "vld2", 2, 1, 2),
    ATTR_SET_SIMD_LDSTN_BHW       (VLDN2_R1I1, 7, ARM_ISAR_SIMDLDST, "vld2", 2, 1, 1),
    ATTR_SET_SIMD_LDSTN_BHW       (VLDN3_R3I1, 7, ARM_ISAR_SIMDLDST, "vld3", 3, 1, 1),
    ATTR_SET_SIMD_LDSTN_BHW       (VLDN3_R3I2, 7, ARM_ISAR_SIMDLDST, "vld3", 3, 1, 2),
    ATTR_SET_SIMD_LDSTN_BHW       (VLDN4_R4I1, 7, ARM_ISAR_SIMDLDST, "vld4", 4, 1, 1),
    ATTR_SET_SIMD_LDSTN_BHW       (VLDN4_R4I2, 7, ARM_ISAR_SIMDLDST, "vld4", 4, 1, 2),

    ATTR_SET_SIMD_LDST1Z1_BHW     (VST1Z1,     7, ARM_ISAR_SIMDLDST, "vst1"),
    ATTR_SET_SIMD_LDST2Z1_BHW     (VST2Z1,     7, ARM_ISAR_SIMDLDST, "vst2"),
    ATTR_SET_SIMD_LDST3Z1_BHW     (VST3Z1,     7, ARM_ISAR_SIMDLDST, "vst3"),
    ATTR_SET_SIMD_LDST4Z1_BHW     (VST4Z1,     7, ARM_ISAR_SIMDLDST, "vst4"),

    ATTR_SET_SIMD_LDST1Z1_BHW     (VLD1Z1,     7, ARM_ISAR_SIMDLDST, "vld1"),
    ATTR_SET_SIMD_LD1ZA_BHW       (VLD1ZA,     7, ARM_ISAR_SIMDLDST, "vld1"),
    ATTR_SET_SIMD_LDST2Z1_BHW     (VLD2Z1,     7, ARM_ISAR_SIMDLDST, "vld2"),
    ATTR_SET_SIMD_LD2ZA_BHW       (VLD2ZA,     7, ARM_ISAR_SIMDLDST, "vld2"),
    ATTR_SET_SIMD_LDST3Z1_BHW     (VLD3Z1,     7, ARM_ISAR_SIMDLDST, "vld3"),
    ATTR_SET_SIMD_LD3ZA_BHW       (VLD3ZA,     7, ARM_ISAR_SIMDLDST, "vld3"),
    ATTR_SET_SIMD_LDST4Z1_BHW     (VLD4Z1,     7, ARM_ISAR_SIMDLDST, "vld4"),
    ATTR_SET_SIMD_LD4ZA_BHW       (VLD4ZA,     7, ARM_ISAR_SIMDLDST, "vld4"),

    // 8, 16 and 32-bit transfer instructions between ARM core regs and extension regs
    ATTR_SET_VMRS   (VMRS,   7, ARM_ISAR_VMRS, "vmrs"),
    ATTR_SET_VMSR   (VMSR,   7, ARM_ISAR_VMRS, "vmsr"),
    ATTR_SET_VMOVRS (VMOVRS, 7, ARM_ISAR_VMRS, "vmov"),
    ATTR_SET_VMOVSR (VMOVSR, 7, ARM_ISAR_VMRS, "vmov"),
    ATTR_SET_VMOVZR (VMOVZR, 7, ARM_ISAR_VMRS, "vmov"),
    ATTR_SET_VMOVRZ (VMOVRZ, 7, ARM_ISAR_VMRS, "vmov"),
    ATTR_SET_VDUPR  (VDUPR,  7, ARM_ISAR_VMRS, "vdup"),

    // 64-bit transfer instructions between ARM core regs and extension regs
    ATTR_SET_VMOVRRD  (VMOVRRD,  7, ARM_ISAR_VMRS, "vmov"),
    ATTR_SET_VMOVDRR  (VMOVDRR,  7, ARM_ISAR_VMRS, "vmov"),
    ATTR_SET_VMOVRRSS (VMOVRRSS, 7, ARM_ISAR_VMRS, "vmov"),
    ATTR_SET_VMOVSSRR (VMOVSSRR, 7, ARM_ISAR_VMRS, "vmov"),
};


////////////////////////////////////////////////////////////////////////////////
// ARM INSTRUCTION DECODE TABLE
////////////////////////////////////////////////////////////////////////////////

//
// This type specifies the pattern for a decode table entry
//
typedef struct decodeEntryS {
    armInstructionType type    :16;
    Uns32              priority:16;
    const char        *name;
    const char        *pattern;
} decodeEntry;

//
// Create the ARM instruction decode table
//
static vmidDecodeTableP createDecodeTableARM(void) {

    const static decodeEntry entries[] = {

        ////////////////////////////////////////////////////////////////////////
        // NORMAL INSTRUCTIONS
        ////////////////////////////////////////////////////////////////////////

        // data processing instructions
        DECODE_SET_ADC (ADC, "0101"),
        DECODE_SET_ADC (ADD, "0100"),
        DECODE_SET_ADC (AND, "0000"),
        DECODE_SET_ADC (BIC, "1110"),
        DECODE_SET_ADC (EOR, "0001"),
        DECODE_SET_MOV (MOV, "1101"),
        DECODE_SET_MOV (MVN, "1111"),
        DECODE_SET_ADC (ORR, "1100"),
        DECODE_SET_ADC (RSB, "0011"),
        DECODE_SET_ADC (RSC, "0111"),
        DECODE_SET_ADC (SBC, "0110"),
        DECODE_SET_ADC (SUB, "0010"),

        // ARMv6T2 move instructions
        DECODE_SET_MOVT (MOVT, "1010"),
        DECODE_SET_MOVT (MOVW, "1000"),

        // multiply instructions
        DECODE_SET_MLA (MLA,   "0000001."),
        DECODE_SET_MLA (MLS,   "00000110"),
        DECODE_SET_MLA (MUL,   "0000000."),
        DECODE_SET_MLA (SMLAL, "0000111."),
        DECODE_SET_MLA (SMULL, "0000110."),
        DECODE_SET_MLA (UMAAL, "00000100"),
        DECODE_SET_MLA (UMLAL, "0000101."),
        DECODE_SET_MLA (UMULL, "0000100."),

        // compare instructions
        DECODE_SET_CMN (CMN, "1011"),
        DECODE_SET_CMN (CMP, "1010"),
        DECODE_SET_CMN (TEQ, "1001"),
        DECODE_SET_CMN (TST, "1000"),

        // branch instructions
        DECODE_SET_B    (B,    "0"),
        DECODE_SET_B    (BL,   "1"),
        DECODE_SET_BLX1 (BLX1),
        DECODE_SET_BLX2 (BLX2, "0011"),
        DECODE_SET_BLX2 (BX,   "0001"),
        DECODE_SET_BLX2 (BXJ,  "0010"),

        // MURAC instructions
        DECODE_SET_BLX2 (BAA,  "0100"),

        // miscellaneous instructions
        DECODE_SET_BKPT (BKPT),
        DECODE_SET_CLZ  (CLZ),
        DECODE_SET_SWI  (SWI),

        // load and store instructions
        DECODE_SET_LDR   (LDR,   "0", "1"),
        DECODE_SET_LDR   (LDRB,  "1", "1"),
        DECODE_SET_LDRBT (LDRBT, "1", "1"),
        DECODE_SET_LDRH  (LDRH,  "1", ".", "01"),
        DECODE_SET_LDRH  (LDRSB, "1", ".", "10"),
        DECODE_SET_LDRH  (LDRSH, "1", ".", "11"),
        DECODE_SET_LDRBT (LDRT,  "0", "1"),
        DECODE_SET_LDR   (STR,   "0", "0"),
        DECODE_SET_LDR   (STRB,  "1", "0"),
        DECODE_SET_LDRBT (STRBT, "1", "0"),
        DECODE_SET_LDRH  (STRH,  "0", ".", "01"),
        DECODE_SET_LDRBT (STRT,  "0", "0"),

        // load and store multiple instructions
        DECODE_SET_LDM1  (LDM1, "1"),
        DECODE_SET_LDM2  (LDM2),
        DECODE_SET_LDM3  (LDM3),
        DECODE_SET_LDM1  (STM1, "0"),
        DECODE_SET_STM2  (STM2),

        // ARMv6T2 load and store instructions
        DECODE_SET_LDRHT (LDRHT,  "1", "01"),
        DECODE_SET_LDRHT (LDRSBT, "1", "10"),
        DECODE_SET_LDRHT (LDRSHT, "1", "11"),
        DECODE_SET_LDRHT (STRHT,  "0", "01"),

        // semaphore instructions
        DECODE_SET_SWP (SWP,    "0000"),
        DECODE_SET_SWP (SWPB,   "0100"),

        // synchronization primitives
        DECODE_SET_SWP (LDREX,  "1001"),
        DECODE_SET_SWP (LDREXB, "1101"),
        DECODE_SET_SWP (LDREXH, "1111"),
        DECODE_SET_SWP (LDREXD, "1011"),
        DECODE_SET_SWP (STREX,  "1000"),
        DECODE_SET_SWP (STREXB, "1100"),
        DECODE_SET_SWP (STREXH, "1110"),
        DECODE_SET_SWP (STREXD, "1010"),

        // coprocessor instructions
        DECODE_SET_CDP  (CDP),
        DECODE_SET_CDP2 (CDP2),
        DECODE_SET_LDC  (LDC,  "1"),
        DECODE_SET_LDC2 (LDC2, "1"),
        DECODE_SET_MCR  (MCR,  "0"),
        DECODE_SET_MCR2 (MCR2, "0"),
        DECODE_SET_MCR  (MRC,  "1"),
        DECODE_SET_MCR2 (MRC2, "1"),
        DECODE_SET_LDC  (STC,  "0"),
        DECODE_SET_LDC2 (STC2, "0"),

        // status register access instructions
        DECODE_SET_MRS (MRSC, "0"),
        DECODE_SET_MRS (MRSS, "1"),
        DECODE_SET_MSR (MSRC, "0"),
        DECODE_SET_MSR (MSRS, "1"),

        // hint instructions
        DECODE_SET_NOP (NOP,   "00000000"),
        DECODE_SET_NOP (YIELD, "00000001"),
        DECODE_SET_NOP (WFE,   "00000010"),
        DECODE_SET_NOP (WFI,   "00000011"),
        DECODE_SET_NOP (SEV,   "00000100"),
        DECODE_SET_NOP (DBG,   "1111...."),

        // ARMv6 exception instructions
        DECODE_SET_SRS  (SRS, "100..1.0"),
        DECODE_SET_SRS  (RFE, "100..0.1"),

        // ARMv6 miscellaneous instructions
        DECODE_SET_MISC (SETEND, "0010000", "0000", "...1"),
        DECODE_SET_MISC (CPS,    "0010000", "..0.", "...0"),
        DECODE_SET_MISC (CLREX,  "1010111", "0001", "...."),

        // ARMv6/ARMv7 memory hint instructions
        DECODE_SET_PLD  (PLD,   "1.1"),
        DECODE_SET_PLD  (PLI,   "0.1"),
        DECODE_SET_PLD  (PLDW,  "1.0"),
        DECODE_SET_MISC (DMB,   "1010111", "0101", "...."),
        DECODE_SET_MISC (DSB,   "1010111", "0100", "...."),
        DECODE_SET_MISC (ISB,   "1010111", "0110", "...."),
        DECODE_SET_MISC (UHINT, "100.001", "....", "...."),

        ////////////////////////////////////////////////////////////////////////
        // DSP INSTRUCTIONS
        ////////////////////////////////////////////////////////////////////////

        // data processing instructions
        DECODE_SET_QADD (QADD,  "00"),
        DECODE_SET_QADD (QDADD, "10"),
        DECODE_SET_QADD (QDSUB, "11"),
        DECODE_SET_QADD (QSUB,  "01"),

        // multiply instructions
        DECODE_SET_SMLA_XY (SMLA,  "00"),
        DECODE_SET_SMLA_XY (SMLAL, "10"),
        DECODE_SET_SMLAW_Y (SMLAW, "0" ),
        DECODE_SET_SMLA_XY (SMUL,  "11"),
        DECODE_SET_SMLAW_Y (SMULW, "1" ),

        // load and store instructions
        DECODE_SET_LDRH (LDRD, "0", "0", "10"),
        DECODE_SET_LDRH (STRD, "0", "0", "11"),

        // coprocessor instructions
        DECODE_SET_MCRR  (MCRR,  "0"),
        DECODE_SET_MCRR2 (MCRR2, "0"),
        DECODE_SET_MCRR  (MRRC,  "1"),
        DECODE_SET_MCRR2 (MRRC2, "1"),

        ////////////////////////////////////////////////////////////////////////
        // MEDIA INSTRUCTIONS
        ////////////////////////////////////////////////////////////////////////

        // basic instructions
        DECODE_SET_MEDIA (USAD8,  1, "11000", "000", "....", "1111", "...."),
        DECODE_SET_MEDIA (USADA8, 0, "11000", "000", "....", "....", "...."),
        DECODE_SET_MEDIA (SBFX,   0, "1101.", ".10", "....", "....", "...."),
        DECODE_SET_MEDIA (BFC,    1, "1110.", ".00", "....", "....", "1111"),
        DECODE_SET_MEDIA (BFI,    0, "1110.", ".00", "....", "....", "...."),
        DECODE_SET_MEDIA (UBFX,   0, "1111.", ".10", "....", "....", "...."),

        // parallel add/subtract instructions
        DECODE_SET_PAS (ADD16, "000"),
        DECODE_SET_PAS (ASX,   "001"),
        DECODE_SET_PAS (SAX,   "010"),
        DECODE_SET_PAS (SUB16, "011"),
        DECODE_SET_PAS (ADD8,  "100"),
        DECODE_SET_PAS (SUB8,  "111"),

        // packing, unpacking, saturation and reversal instructions
        DECODE_SET_MEDIA (PKHBT,   0, "01" "000", ".00", "....", "....", "...."),
        DECODE_SET_MEDIA (PKHTB,   0, "01" "000", ".10", "....", "....", "...."),
        DECODE_SET_MEDIA (SSAT,    0, "01" "01.", "..0", "....", "....", "...."),
        DECODE_SET_MEDIA (SSAT16,  0, "01" "010", "001", "....", "....", "...."),
        DECODE_SET_MEDIA (USAT,    0, "01" "11.", "..0", "....", "....", "...."),
        DECODE_SET_MEDIA (USAT16,  0, "01" "110", "001", "....", "....", "...."),
        DECODE_SET_MEDIA (SXTAB,   0, "01" "010", "011", "....", "....", "...."),
        DECODE_SET_MEDIA (UXTAB,   0, "01" "110", "011", "....", "....", "...."),
        DECODE_SET_MEDIA (SXTAB16, 0, "01" "000", "011", "....", "....", "...."),
        DECODE_SET_MEDIA (UXTAB16, 0, "01" "100", "011", "....", "....", "...."),
        DECODE_SET_MEDIA (SXTAH,   0, "01" "011", "011", "....", "....", "...."),
        DECODE_SET_MEDIA (UXTAH,   0, "01" "111", "011", "....", "....", "...."),
        DECODE_SET_MEDIA (SXTB,    1, "01" "010", "011", "1111", "....", "...."),
        DECODE_SET_MEDIA (UXTB,    1, "01" "110", "011", "1111", "....", "...."),
        DECODE_SET_MEDIA (SXTB16,  1, "01" "000", "011", "1111", "....", "...."),
        DECODE_SET_MEDIA (UXTB16,  1, "01" "100", "011", "1111", "....", "...."),
        DECODE_SET_MEDIA (SXTH,    1, "01" "011", "011", "1111", "....", "...."),
        DECODE_SET_MEDIA (UXTH,    1, "01" "111", "011", "1111", "....", "...."),
        DECODE_SET_MEDIA (SEL,     0, "01" "000", "101", "....", "....", "...."),
        DECODE_SET_MEDIA (REV,     0, "01" "011", "001", "....", "....", "...."),
        DECODE_SET_MEDIA (REV16,   0, "01" "011", "101", "....", "....", "...."),
        DECODE_SET_MEDIA (RBIT,    0, "01" "111", "001", "....", "....", "...."),
        DECODE_SET_MEDIA (REVSH,   0, "01" "111", "101", "....", "....", "...."),

        // signed multiply instructions
        DECODE_SET_MEDIA_X (SMLAD,  0, "10" "000", "00", "....", "....", "...."),
        DECODE_SET_MEDIA_X (SMUAD,  1, "10" "000", "00", "....", "1111", "...."),
        DECODE_SET_MEDIA_X (SMLSD,  0, "10" "000", "01", "....", "....", "...."),
        DECODE_SET_MEDIA_X (SMUSD,  1, "10" "000", "01", "....", "1111", "...."),
        DECODE_SET_MEDIA_X (SMLALD, 0, "10" "100", "00", "....", "....", "...."),
        DECODE_SET_MEDIA_X (SMLSLD, 0, "10" "100", "01", "....", "....", "...."),
        DECODE_SET_MEDIA_R (SMMLA,  0, "10" "101", "00", "....", "....", "...."),
        DECODE_SET_MEDIA_R (SMMUL,  1, "10" "101", "00", "....", "1111", "...."),
        DECODE_SET_MEDIA_R (SMMLS,  0, "10" "101", "11", "....", "....", "...."),

        ////////////////////////////////////////////////////////////////////////////
        // SIMD/VFP INSTRUCTIONS
        ////////////////////////////////////////////////////////////////////////////

        // SIMD data processing instructions - Miscellaneous
        DECODE_SET_VEXT  (VEXT),
        DECODE_SET_VTBL  (VTBL, "0"),
        DECODE_SET_VTBL  (VTBX, "1"),
        DECODE_SET_VDUPZ (VDUPZ),

        // SIMD data processing instructions - 3 regs same length
        DECODE_SET_SIMD_RRR_QD_BHW  (VHADDU,  "0000", "0", "1"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VHADDS,  "0000", "0", "0"),
        DECODE_SET_SIMD_RRR_QD_BHWD (VQADDU,  "0000", "1", "1"),
        DECODE_SET_SIMD_RRR_QD_BHWD (VQADDS,  "0000", "1", "0"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VRHADDU, "0001", "0", "1"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VRHADDS, "0001", "0", "0"),
        DECODE_SET_SIMD_RRR_QD      (VAND,    "0001", "1", "0", "00"),
        DECODE_SET_SIMD_RRR_QD      (VBIC,    "0001", "1", "0", "01"),
        DECODE_SET_SIMD_RRR_QD      (VORR,    "0001", "1", "0", "10"),
        DECODE_SET_SIMD_RRR_QD      (VORN,    "0001", "1", "0", "11"),
        DECODE_SET_SIMD_RRR_QD      (VEOR,    "0001", "1", "1", "00"),
        DECODE_SET_SIMD_RRR_QD      (VBSL,    "0001", "1", "1", "01"),
        DECODE_SET_SIMD_RRR_QD      (VBIT,    "0001", "1", "1", "10"),
        DECODE_SET_SIMD_RRR_QD      (VBIF,    "0001", "1", "1", "11"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VHSUBU,  "0010", "0", "1"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VHSUBS,  "0010", "0", "0"),
        DECODE_SET_SIMD_RRR_QD_BHWD (VQSUBU,  "0010", "1", "1"),
        DECODE_SET_SIMD_RRR_QD_BHWD (VQSUBS,  "0010", "1", "0"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VCGTU,   "0011", "0", "1"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VCGTS,   "0011", "0", "0"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VCGEU,   "0011", "1", "1"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VCGES,   "0011", "1", "0"),
        DECODE_SET_SIMD_RRR_QD_BHWD (VSHLU,   "0100", "0", "1"),
        DECODE_SET_SIMD_RRR_QD_BHWD (VSHLS,   "0100", "0", "0"),
        DECODE_SET_SIMD_RRR_QD_BHWD (VQSHLU,  "0100", "1", "1"),
        DECODE_SET_SIMD_RRR_QD_BHWD (VQSHLS,  "0100", "1", "0"),
        DECODE_SET_SIMD_RRR_QD_BHWD (VRSHLU,  "0101", "0", "1"),
        DECODE_SET_SIMD_RRR_QD_BHWD (VRSHLS,  "0101", "0", "0"),
        DECODE_SET_SIMD_RRR_QD_BHWD (VQRSHLU, "0101", "1", "1"),
        DECODE_SET_SIMD_RRR_QD_BHWD (VQRSHLS, "0101", "1", "0"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VMAXU,   "0110", "0", "1"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VMAXS,   "0110", "0", "0"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VMINU,   "0110", "1", "1"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VMINS,   "0110", "1", "0"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VABDU,   "0111", "0", "1"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VABDS,   "0111", "0", "0"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VABAU,   "0111", "1", "1"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VABAS,   "0111", "1", "0"),
        DECODE_SET_SIMD_RRR_QD_BHWD (VADD,    "1000", "0", "0"),
        DECODE_SET_SIMD_RRR_QD_BHWD (VSUB,    "1000", "0", "1"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VTST,    "1000", "1", "0"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VCEQ,    "1000", "1", "1"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VMLA,    "1001", "0", "0"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VMLS,    "1001", "0", "1"),
        DECODE_SET_SIMD_RRR_QD_BHW  (VMUL,    "1001", "1", "0"),
        DECODE_SET_SIMD_RRR_QD_P    (VMUL_P,  "1001", "1", "1"),
        DECODE_SET_SIMD_RRR_D_BHW   (VPMAXS,  "1010", "0", "0"),
        DECODE_SET_SIMD_RRR_D_BHW   (VPMAXU,  "1010", "0", "1"),
        DECODE_SET_SIMD_RRR_D_BHW   (VPMINS,  "1010", "1", "0"),
        DECODE_SET_SIMD_RRR_D_BHW   (VPMINU,  "1010", "1", "1"),
        DECODE_SET_SIMD_RRR_QD_HW   (VQDMULH, "1011", "0", "0"),
        DECODE_SET_SIMD_RRR_QD_HW   (VQRDMULH,"1011", "0", "1"),
        DECODE_SET_SIMD_RRR_D_BHW   (VPADD,   "1011", "1", "0"),
        DECODE_SET_SIMD_RRR_QD      (VADD_F,  "1101", "0", "0", "00"),
        DECODE_SET_SIMD_RRR_QD      (VSUB_F,  "1101", "0", "0", "10"),
        DECODE_SET_SIMD_RRR         (VPADD_F, "1101", "0", "1", "00"),
        DECODE_SET_SIMD_RRR_QD      (VABD_F,  "1101", "0", "1", "10"),
        DECODE_SET_SIMD_RRR_QD      (VMLA_F,  "1101", "1", "0", "00"),
        DECODE_SET_SIMD_RRR_QD      (VMLS_F,  "1101", "1", "0", "10"),
        DECODE_SET_SIMD_RRR_QD      (VMUL_F,  "1101", "1", "1", "00"),
        DECODE_SET_SIMD_RRR_QD      (VCEQ_F,  "1110", "0", "0", "00"),
        DECODE_SET_SIMD_RRR_QD      (VCGE_F,  "1110", "0", "1", "00"),
        DECODE_SET_SIMD_RRR_QD      (VCGT_F,  "1110", "0", "1", "10"),
        DECODE_SET_SIMD_RRR_QD      (VACGE_F, "1110", "1", "1", "00"),
        DECODE_SET_SIMD_RRR_QD      (VACGT_F, "1110", "1", "1", "10"),
        DECODE_SET_SIMD_RRR_QD      (VMAX_F,  "1111", "0", "0", "00"),
        DECODE_SET_SIMD_RRR_QD      (VMIN_F,  "1111", "0", "0", "10"),
        DECODE_SET_SIMD_RRR         (VPMAX_F, "1111", "0", "1", "00"),
        DECODE_SET_SIMD_RRR         (VPMIN_F, "1111", "0", "1", "10"),
        DECODE_SET_SIMD_RRR_QD      (VRECPS,  "1111", "1", "0", "00"),
        DECODE_SET_SIMD_RRR_QD      (VRSQRTS, "1111", "1", "0", "10"),

        // SIMD data processing instructions - 3 regs different lengths
        DECODE_SET_SIMD_L_BHW (VADDLS,  "0000", "0"),
        DECODE_SET_SIMD_L_BHW (VADDLU,  "0000", "1"),
        DECODE_SET_SIMD_W_BHW (VADDWS,  "0001", "0"),
        DECODE_SET_SIMD_W_BHW (VADDWU,  "0001", "1"),
        DECODE_SET_SIMD_L_BHW (VSUBLS,  "0010", "0"),
        DECODE_SET_SIMD_L_BHW (VSUBLU,  "0010", "1"),
        DECODE_SET_SIMD_W_BHW (VSUBWS,  "0011", "0"),
        DECODE_SET_SIMD_W_BHW (VSUBWU,  "0011", "1"),
        DECODE_SET_SIMD_N_HWD (VADDHN,  "0100", "0"),
        DECODE_SET_SIMD_N_HWD (VRADDHN, "0100", "1"),
        DECODE_SET_SIMD_L_BHW (VABALS,  "0101", "0"),
        DECODE_SET_SIMD_L_BHW (VABALU,  "0101", "1"),
        DECODE_SET_SIMD_N_HWD (VSUBHN,  "0110", "0"),
        DECODE_SET_SIMD_N_HWD (VRSUBHN, "0110", "1"),
        DECODE_SET_SIMD_L_BHW (VABDLS,  "0111", "0"),
        DECODE_SET_SIMD_L_BHW (VABDLU,  "0111", "1"),
        DECODE_SET_SIMD_L_BHW (VMLALS,  "1000", "0"),
        DECODE_SET_SIMD_L_BHW (VMLALU,  "1000", "1"),
        DECODE_SET_SIMD_L_BHW (VMLSLS,  "1010", "0"),
        DECODE_SET_SIMD_L_BHW (VMLSLU,  "1010", "1"),
        DECODE_SET_SIMD_L_HW  (VQDMLAL, "1001", "0"),
        DECODE_SET_SIMD_L_HW  (VQDMLSL, "1011", "0"),
        DECODE_SET_SIMD_L_BHW (VMULLS,  "1100", "0"),
        DECODE_SET_SIMD_L_BHW (VMULLU,  "1100", "1"),
        DECODE_SET_SIMD_L_HW  (VQDMULL, "1101", "0"),
        DECODE_SET_SIMD_L_P   (VMULL_P, "1110"),

        // SIMD data processing instructions - 2 regs and a scalar
        DECODE_SET_SIMD_RRZ_QD_HW (VMLAZ,     "0000"),
        DECODE_SET_SIMD_RRZ_QD    (VMLAZ_F,   "0001"),
        DECODE_SET_SIMD_RRZ_QD_HW (VMLSZ,     "0100"),
        DECODE_SET_SIMD_RRZ_QD    (VMLSZ_F,   "0101"),
        DECODE_SET_SIMD_LZ_HW     (VMLALZS,   "0010", "0"),
        DECODE_SET_SIMD_LZ_HW     (VMLALZU,   "0010", "1"),
        DECODE_SET_SIMD_LZ_HW     (VMLSLZS,   "0110", "0"),
        DECODE_SET_SIMD_LZ_HW     (VMLSLZU,   "0110", "1"),
        DECODE_SET_SIMD_LZ_HW     (VQDMLALZ,  "0011", "0"),
        DECODE_SET_SIMD_LZ_HW     (VQDMLSLZ,  "0111", "0"),
        DECODE_SET_SIMD_RRZ_QD_HW (VMULZ,     "1000"),
        DECODE_SET_SIMD_RRZ_QD    (VMULZ_F,   "1001"),
        DECODE_SET_SIMD_LZ_HW     (VMULLZS,   "1010", "0"),
        DECODE_SET_SIMD_LZ_HW     (VMULLZU,   "1010", "1"),
        DECODE_SET_SIMD_LZ_HW     (VQDMULLZ,  "1011", "0"),
        DECODE_SET_SIMD_RRZ_QD_HW (VQDMULHZ,  "1100"),
        DECODE_SET_SIMD_RRZ_QD_HW (VQRDMULHZ, "1101"),

        // SIMD data processing instructions - 2 regs and a shift amount
        DECODE_SET_SIMD_RRI_QD_BHWD(VSHRS,    "0000", "0"),
        DECODE_SET_SIMD_RRI_QD_BHWD(VSHRU,    "0000", "1"),
        DECODE_SET_SIMD_RRI_QD_BHWD(VSRAS,    "0001", "0"),
        DECODE_SET_SIMD_RRI_QD_BHWD(VSRAU,    "0001", "1"),
        DECODE_SET_SIMD_RRI_QD_BHWD(VRSHRS,   "0010", "0"),
        DECODE_SET_SIMD_RRI_QD_BHWD(VRSHRU,   "0010", "1"),
        DECODE_SET_SIMD_RRI_QD_BHWD(VRSRAS,   "0011", "0"),
        DECODE_SET_SIMD_RRI_QD_BHWD(VRSRAU,   "0011", "1"),
        DECODE_SET_SIMD_RRI_QD_BHWD(VSRI,     "0100", "1"),
        DECODE_SET_SIMD_RRI_QD_BHWD(VSHL,     "0101", "0"),
        DECODE_SET_SIMD_RRI_QD_BHWD(VSLI,     "0101", "1"),
        DECODE_SET_SIMD_VQSHLUS    (VQSHLUS,  "0110", "1"),    //VQSHLU specifies undefined instruction when U=0
        DECODE_SET_SIMD_RRI_QD_BHWD(VQSHLSS,  "0111", "0"),
        DECODE_SET_SIMD_RRI_QD_BHWD(VQSHLSU,  "0111", "1"),
        DECODE_SET_SIMD_NI_HWD     (VSHRN,    "1000", "0", "0"),
        DECODE_SET_SIMD_NI_HWD     (VRSHRN,   "1000", "0", "1"),
        DECODE_SET_SIMD_NI_HWD     (VQSHRUNS, "1000", "1", "0"),
        DECODE_SET_SIMD_NI_HWD     (VQRSHRUNS,"1000", "1", "1"),
        DECODE_SET_SIMD_NI_HWD     (VQSHRNS,  "1001", "0", "0"),
        DECODE_SET_SIMD_NI_HWD     (VQRSHRNS, "1001", "0", "1"),
        DECODE_SET_SIMD_NI_HWD     (VQSHRNU,  "1001", "1", "0"),
        DECODE_SET_SIMD_NI_HWD     (VQRSHRNU, "1001", "1", "1"),
        DECODE_SET_SIMD_LI_BHW     (VSHLLS,   "1010", "0"),
        DECODE_SET_SIMD_LI_BHW     (VSHLLU,   "1010", "1"),
        DECODE_SET_SIMD_VMOVL      (VMOVLS,   "1010", "0"),
        DECODE_SET_SIMD_VMOVL      (VMOVLU,   "1010", "1"),
        DECODE_SET_SIMD_RRI_QD_W   (VCVTFXS,  "1110", "0"),
        DECODE_SET_SIMD_RRI_QD_W   (VCVTFXU,  "1110", "1"),
        DECODE_SET_SIMD_RRI_QD_W   (VCVTXFS,  "1111", "0"),
        DECODE_SET_SIMD_RRI_QD_W   (VCVTXFU,  "1111", "1"),

        // SIMD data processing instructions - Two registers, Miscellaneous
        //  Note: These use the esize (21:20)='11' space of other instructions and must have priority > 2
        DECODE_SET_SIMD_VREV       (VREV),
        DECODE_SET_SIMD_RR_QD_BHW  (VPADDLS,  "00", "0100"),
        DECODE_SET_SIMD_RR_QD_BHW  (VPADDLU,  "00", "0101"),
        DECODE_SET_SIMD_RR_QD_BHW  (VCLS,     "00", "1000"),
        DECODE_SET_SIMD_RR_QD_BHW  (VCLZ,     "00", "1001"),
        DECODE_SET_SIMD_RR_QD_B    (VCNT,     "00", "1010"),
        DECODE_SET_SIMD_RR_QD_B    (VMVN,     "00", "1011"),
        DECODE_SET_SIMD_RR_QD_BHW  (VPADALS,  "00", "1100"),
        DECODE_SET_SIMD_RR_QD_BHW  (VPADALU,  "00", "1101"),
        DECODE_SET_SIMD_RR_QD_BHW  (VQABS,    "00", "1110"),
        DECODE_SET_SIMD_RR_QD_BHW  (VQNEG,    "00", "1111"),
        DECODE_SET_SIMD_RR_QD_BHW  (VCGT0,    "01", "0000"),
        DECODE_SET_SIMD_RR_QD_BHW  (VCGE0,    "01", "0001"),
        DECODE_SET_SIMD_RR_QD_BHW  (VCEQ0,    "01", "0010"),
        DECODE_SET_SIMD_RR_QD_BHW  (VCLE0,    "01", "0011"),
        DECODE_SET_SIMD_RR_QD_BHW  (VCLT0,    "01", "0100"),
        DECODE_SET_SIMD_RR_QD_BHW  (VABS,     "01", "0110"),
        DECODE_SET_SIMD_RR_QD_BHW  (VNEG,     "01", "0111"),
        DECODE_SET_SIMD_RR_QD_W    (VCGT0_F,  "01", "1000"),
        DECODE_SET_SIMD_RR_QD_W    (VCGE0_F,  "01", "1001"),
        DECODE_SET_SIMD_RR_QD_W    (VCEQ0_F,  "01", "1010"),
        DECODE_SET_SIMD_RR_QD_W    (VCLE0_F,  "01", "1011"),
        DECODE_SET_SIMD_RR_QD_W    (VCLT0_F,  "01", "1100"),
        DECODE_SET_SIMD_RR_QD_W    (VABS_F,   "01", "1110"),
        DECODE_SET_SIMD_RR_QD_W    (VNEG_F,   "01", "1111"),
        DECODE_SET_SIMD_RR_QD_B    (VSWP,     "10", "0000"),
        DECODE_SET_SIMD_RR_QD_BHW  (VTRN,     "10", "0001"),
        DECODE_SET_SIMD_RR_QBHW_DBH(VUZP,     "10", "0010"),
        DECODE_SET_SIMD_RR_QBHW_DBH(VZIP,     "10", "0011"),
        DECODE_SET_SIMD_N2_HWD     (VMOVN,    "10", "01000"),
        DECODE_SET_SIMD_N2_HWD     (VQMOVUNS, "10", "01001"),
        DECODE_SET_SIMD_N2_HWD     (VQMOVNS,  "10", "01010"),
        DECODE_SET_SIMD_N2_HWD     (VQMOVNU,  "10", "01011"),
        DECODE_SET_SIMD_L2_BHW     (VSHLLM,   "10", "01100"),
        DECODE_SET_SIMD_N2_H       (VCVTHS,   "10", "11000"),
        DECODE_SET_SIMD_L2_H       (VCVTSH,   "10", "11100"),
        DECODE_SET_SIMD_RR_QD_W    (VRECPE,   "11", "1000"),
        DECODE_SET_SIMD_RR_QD_W    (VRECPE_F, "11", "1010"),
        DECODE_SET_SIMD_RR_QD_W    (VRSQRTE,  "11", "1001"),
        DECODE_SET_SIMD_RR_QD_W    (VRSQRTE_F,"11", "1011"),
        DECODE_SET_SIMD_RR_QD_W    (VCVTFS,   "11", "1100"),
        DECODE_SET_SIMD_RR_QD_W    (VCVTFU,   "11", "1101"),
        DECODE_SET_SIMD_RR_QD_W    (VCVTSF,   "11", "1110"),
        DECODE_SET_SIMD_RR_QD_W    (VCVTUF,   "11", "1111"),

        // SIMD data processing instructions - One register and a modified immediate
        DECODE_SET_SIMD_RI_QD (VMOVI_W,   "0",  "0..0"),
        DECODE_SET_SIMD_RI_QD (VORRI_W,   "0",  "0..1"),
        DECODE_SET_SIMD_RI_QD (VMOVI_H,   "0",  "10.0"),
        DECODE_SET_SIMD_RI_QD (VORRI_H,   "0",  "10.1"),
        DECODE_SET_SIMD_RI_QD (VMOVI1_W,  "0",  "110."),
        DECODE_SET_SIMD_RI_QD (VMOVI_B,   "0",  "1110"),
        DECODE_SET_SIMD_RI_QD (VMOVI_F_W, "0",  "1111"),
        DECODE_SET_SIMD_RI_QD (VMVNI_W,   "1",  "0..0"),
        DECODE_SET_SIMD_RI_QD (VBICI_W,   "1",  "0..1"),
        DECODE_SET_SIMD_RI_QD (VMVNI_H,   "1",  "10.0"),
        DECODE_SET_SIMD_RI_QD (VBICI_H,   "1",  "10.1"),
        DECODE_SET_SIMD_RI_QD (VMVNI1_W,  "1",  "110."),
        DECODE_SET_SIMD_RI_QD (VMOVI_D,   "1",  "1110"),

        // VFP data processing instructions - 3 regs
        DECODE_SET_VFP_DS (VMLA_VFP,    "0.00", "....", ".0"),
        DECODE_SET_VFP_DS (VMLS_VFP,    "0.00", "....", ".1"),
        DECODE_SET_VFP_DS (VNMLS_VFP,   "0.01", "....", ".0"),
        DECODE_SET_VFP_DS (VNMLA_VFP,   "0.01", "....", ".1"),
        DECODE_SET_VFP_DS (VMUL_VFP,    "0.10", "....", ".0"),
        DECODE_SET_VFP_DS (VNMUL_VFP,   "0.10", "....", ".1"),
        DECODE_SET_VFP_DS (VADD_VFP,    "0.11", "....", ".0"),
        DECODE_SET_VFP_DS (VSUB_VFP,    "0.11", "....", ".1"),
        DECODE_SET_VFP_DS (VDIV_VFP,    "1.00", "....", ".0"),

        // VFP data processing instructions - Other
        DECODE_SET_VFP_DS (VMOVI_VFP,    "1.11", "....", ".0"),
        DECODE_SET_VFP_DS (VMOVR_VFP,    "1.11", "0000", "01"),
        DECODE_SET_VFP_DS (VABS_VFP,     "1.11", "0000", "11"),
        DECODE_SET_VFP_DS (VNEG_VFP,     "1.11", "0001", "01"),
        DECODE_SET_VFP_DS (VSQRT_VFP,    "1.11", "0001", "11"),
        DECODE_SET_VFP_S  (VCVTBFH_VFP,  "1.11", "0010", "01"),
        DECODE_SET_VFP_S  (VCVTTFH_VFP,  "1.11", "0010", "11"),
        DECODE_SET_VFP_S  (VCVTBHF_VFP,  "1.11", "0011", "01"),
        DECODE_SET_VFP_S  (VCVTTHF_VFP,  "1.11", "0011", "11"),
        DECODE_SET_VFP_DS (VCMP_VFP,     "1.11", "0100", "01"),
        DECODE_SET_VFP_DS (VCMPE_VFP,    "1.11", "0100", "11"),
        DECODE_SET_VFP_DS (VCMP0_VFP,    "1.11", "0101", "01"),
        DECODE_SET_VFP_DS (VCMPE0_VFP,   "1.11", "0101", "11"),
        DECODE_SET_VFP_DS (VCVT_VFP,     "1.11", "0111", "11"),
        DECODE_SET_VFP_DS (VCVTFU_VFP,   "1.11", "1000", "01"),
        DECODE_SET_VFP_DS (VCVTFS_VFP,   "1.11", "1000", "11"),
        DECODE_SET_VFP_DS (VCVTFXSH_VFP, "1.11", "1010", "01"),
        DECODE_SET_VFP_DS (VCVTFXSW_VFP, "1.11", "1010", "11"),
        DECODE_SET_VFP_DS (VCVTFXUH_VFP, "1.11", "1011", "01"),
        DECODE_SET_VFP_DS (VCVTFXUW_VFP, "1.11", "1011", "11"),
        DECODE_SET_VFP_DS (VCVTRUF_VFP,  "1.11", "1100", "01"),
        DECODE_SET_VFP_DS (VCVTUF_VFP,   "1.11", "1100", "11"),
        DECODE_SET_VFP_DS (VCVTRSF_VFP,  "1.11", "1101", "01"),
        DECODE_SET_VFP_DS (VCVTSF_VFP,   "1.11", "1101", "11"),
        DECODE_SET_VFP_DS (VCVTXFSH_VFP, "1.11", "1110", "01"),
        DECODE_SET_VFP_DS (VCVTXFSW_VFP, "1.11", "1110", "11"),
        DECODE_SET_VFP_DS (VCVTXFUH_VFP, "1.11", "1111", "01"),
        DECODE_SET_VFP_DS (VCVTXFUW_VFP, "1.11", "1111", "11"),

        // Extension register load/store instructions
        DECODE_SET_SDFP_LDST     (VSTMIA,  "01.00"),
        DECODE_SET_SDFP_LDST     (VSTMIAW, "01.10"),
        DECODE_SET_SDFP_LDST     (VSTR,    "1..00"),
        DECODE_SET_SDFP_LDST     (VSTMDBW, "10.10"),
        DECODE_SET_SDFP_PUSH_POP (VPUSH,   "10.10"),
        DECODE_SET_SDFP_LDST     (VLDMIA,  "01.01"),
        DECODE_SET_SDFP_LDST     (VLDMIAW, "01.11"),
        DECODE_SET_SDFP_PUSH_POP (VPOP,    "01.11"),
        DECODE_SET_SDFP_LDST     (VLDR,    "1..01"),
        DECODE_SET_SDFP_LDST     (VLDMDBW, "10.11"),

        // SIMD element or structure load/store instructions
        DECODE_SET_SIMD_LDSTN_BHWD      (VSTN1_R4,   "0", "0", "0010"),
        DECODE_SET_SIMD_LDSTN_BHWD_A01  (VSTN1_R3,   "0", "0", "0110"),
        DECODE_SET_SIMD_LDSTN_BHWD_A012 (VSTN1_R2,   "0", "0", "1010"),
        DECODE_SET_SIMD_LDSTN_BHWD_A01  (VSTN1_R1,   "0", "0", "0111"),
        DECODE_SET_SIMD_LDSTN_BHW       (VSTN2_R2I2, "0", "0", "0011"),
        DECODE_SET_SIMD_LDSTN_BHW_A012  (VSTN2_R1I2, "0", "0", "1001"),
        DECODE_SET_SIMD_LDSTN_BHW_A012  (VSTN2_R1I1, "0", "0", "1000"),
        DECODE_SET_SIMD_LDSTN_BHW_A01   (VSTN3_R3I1, "0", "0", "0100"),
        DECODE_SET_SIMD_LDSTN_BHW_A01   (VSTN3_R3I2, "0", "0", "0101"),
        DECODE_SET_SIMD_LDSTN_BHW       (VSTN4_R4I1, "0", "0", "0000"),
        DECODE_SET_SIMD_LDSTN_BHW       (VSTN4_R4I2, "0", "0", "0001"),

        DECODE_SET_SIMD_LDSTN_BHWD      (VLDN1_R4,   "0", "1", "0010"),
        DECODE_SET_SIMD_LDSTN_BHWD_A01  (VLDN1_R3,   "0", "1", "0110"),
        DECODE_SET_SIMD_LDSTN_BHWD_A012 (VLDN1_R2,   "0", "1", "1010"),
        DECODE_SET_SIMD_LDSTN_BHWD_A01  (VLDN1_R1,   "0", "1", "0111"),
        DECODE_SET_SIMD_LDSTN_BHW       (VLDN2_R2I2, "0", "1", "0011"),
        DECODE_SET_SIMD_LDSTN_BHW_A012  (VLDN2_R1I2, "0", "1", "1001"),
        DECODE_SET_SIMD_LDSTN_BHW_A012  (VLDN2_R1I1, "0", "1", "1000"),
        DECODE_SET_SIMD_LDSTN_BHW_A01   (VLDN3_R3I1, "0", "1", "0100"),
        DECODE_SET_SIMD_LDSTN_BHW_A01   (VLDN3_R3I2, "0", "1", "0101"),
        DECODE_SET_SIMD_LDSTN_BHW       (VLDN4_R4I1, "0", "1", "0000"),
        DECODE_SET_SIMD_LDSTN_BHW       (VLDN4_R4I2, "0", "1", "0001"),

        DECODE_SET_SIMD_LDST1Z1_BHW     (VST1Z1,    "1", "0", "00"),
        DECODE_SET_SIMD_LDST2Z1_BHW     (VST2Z1,    "1", "0", "01"),
        DECODE_SET_SIMD_LDST3Z1_BHW     (VST3Z1,    "1", "0", "10"),
        DECODE_SET_SIMD_LDST4Z1_BHW     (VST4Z1,    "1", "0", "11"),

        DECODE_SET_SIMD_LDST1Z1_BHW     (VLD1Z1,    "1", "1", "00"),
        DECODE_SET_SIMD_LD1ZA_BHW       (VLD1ZA,    "1", "1", "00"),
        DECODE_SET_SIMD_LDST2Z1_BHW     (VLD2Z1,    "1", "1", "01"),
        DECODE_SET_SIMD_LD2ZA_BHW       (VLD2ZA,    "1", "1", "01"),
        DECODE_SET_SIMD_LDST3Z1_BHW     (VLD3Z1,    "1", "1", "10"),
        DECODE_SET_SIMD_LD3ZA_BHW       (VLD3ZA,    "1", "1", "10"),
        DECODE_SET_SIMD_LDST4Z1_BHW     (VLD4Z1,    "1", "1", "11"),
        DECODE_SET_SIMD_LD4ZA_BHW       (VLD4ZA,    "1", "1", "11"),

        // 8, 16 and 32-bit transfer instructions between ARM core regs and extension regs
        DECODE_SET_VMRS   (VMRS,   "1", "0", "111", ".."),
        DECODE_SET_VMRS   (VMSR,   "0", "0", "111", ".."),
        DECODE_SET_VMRS   (VMOVRS, "1", "0", "000", ".."),
        DECODE_SET_VMRS   (VMOVSR, "0", "0", "000", ".."),
        DECODE_SET_VMOVZR (VMOVZR),
        DECODE_SET_VMOVRZ (VMOVRZ),
        DECODE_SET_VDUPR  (VDUPR),

        // 64-bit transfer instructions between ARM core regs and extension regs
        DECODE_SET_VMOVRRD (VMOVRRD,  "1", "1", "00.1"),
        DECODE_SET_VMOVRRD (VMOVDRR,  "0", "1", "00.1"),
        DECODE_SET_VMOVRRD (VMOVRRSS, "1", "0", "00.1"),
        DECODE_SET_VMOVRRD (VMOVSSRR, "0", "0", "00.1"),

        // terminator
        {0}
    };

    // create the table
    vmidDecodeTableP   table = vmidNewDecodeTable(32, ARM_IT_LAST);
    const decodeEntry *entry;

    // add all entries to the decode table
    for(entry=entries; entry->pattern; entry++) {
        vmidNewEntryFmtBin(
            table,
            entry->name,
            entry->type,
            entry->pattern,
            entry->priority
        );
    }

    return table;
}

//
// Return register index encoded in the ARM instruction
//
static Uns8 getRegisterARM(Uns32 instr, rSpec r) {

    Uns8 result = 0;

    switch(r) {
        case R_NA:    break;
        case R_0:     result = OP_R0(instr);         break;
        case R_8:     result = OP_R8(instr);         break;
        case R_12:    result = OP_R12(instr);        break;
        case R_12P1:  result = (OP_R12(instr)+1)&15; break;
        case R_16:    result = OP_R16(instr);        break;
        case R_SP:    result = ARM_REG_SP;           break;
        case V_0_5:   result = OP_V0_5(instr);       break;
        case V_16_7:  result = OP_V16_7(instr);      break;
        case V_12_22: result = OP_V12_22(instr);     break;
        case V_5_0:   result = OP_V5_0(instr);       break;
        case V_22_12: result = OP_V22_12(instr);     break;
        case V_7_16:  result = OP_V7_16(instr);      break;
        case V3_0:    result = OP_U_3_0(instr);      break;
        default:      VMI_ABORT("%s: unimplemented case", FUNC_NAME); break;
    }

    return result;
}

//
// Return shift operation encoded in the ARM instruction
//
static armShiftOp getShiftOpARM(Uns32 instr, shiftSpec ss) {

    Uns32 result = ARM_SO_NA;

    const static armShiftOp shiftMap[] = {
        ARM_SO_LSL, ARM_SO_LSR, ARM_SO_ASR, ARM_SO_ROR
    };

    switch(ss) {
        case SS_NA:
            break;
        case SS_6_5:
            result = shiftMap[OP_6_5(instr)];
            break;
        case SS_RRX:
            result = ARM_SO_RRX;
            break;
        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME);
            break;
    }

    return result;
}

//
// Return bit operation width encoded in the ARM instruction
//
static Int8 getWidthARM(Uns32 instr, widthSpec w) {

    Int8 result = 0;

    switch(w) {
        case WS_NA:
            break;
        case WS_WIDTH:
            result = OP_U_5_16(instr);
            break;
        case WS_WIDTHM1:
            result = OP_U_5_16(instr)+1;
            break;
        case WS_MSB:
            result = OP_U_5_16(instr)-OP_U_5_7(instr)+1;
            break;
        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME);
            break;
    }

    return result;
}

//
// Return the SIMD scalar index encoded in the ARM instruction
//
static Int8 getIndexARM(Uns32 instr, indexSpec index) {

    Int8 result = 0;

    switch(index) {
        case IDX_NA:
            break;
        case IDX_21_6_5:
            result = OP_U_3_21_6_5(instr);
            break;
        case IDX_21_6:
            result = OP_U_2_21_6(instr);
            break;
        case IDX_21:
            result = OP_U_1_21(instr);
            break;
        case IDX_5_3:
            result = OP_U_2_5_3(instr);
            break;
        case IDX_5:
            result = OP_U_1_5(instr);
            break;
        case IDX_7:
            result = OP_U_1_7(instr);
            break;
        case IDX_7_6:
            result = OP_U_2_6(instr);
            break;
        case IDX_7_6_5:
            result = OP_U_3_5(instr);
            break;
        case IDX_19:
            result = OP_U_1_19(instr);
            break;
        case IDX_19_18:
            result = OP_U_2_18(instr);
            break;
        case IDX_19_18_17:
            result = OP_U_3_17(instr);
            break;
        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME);
            break;
    }

    return result;
}

//
// Return the SIMD element/structure alignment encoded in the ARM instruction
//
static Uns8 getAlignARM(Uns32 instr, alignSpec align) {

    Uns8 result = 0;

    switch(align) {
        case ALIGN_NA:
            break;
        case ALIGN_2_4_S4:
            result = OP_U_2_4(instr);
            if (result) result = 4 << (result);
            break;
        case ALIGN_C2_4:
            result = OP_U_1_4(instr);
            if (result) result = 2;
            break;
        case ALIGN_C4_4:
            result = OP_U_1_4(instr);
            if (result) result = 4;
            break;
        case ALIGN_C8_4:
            result = OP_U_1_4(instr);
            if (result) result = 8;
            break;
        case ALIGN_C16_4:
            result = OP_U_1_4(instr);
            if (result) result = 16;
            break;
        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME);
            break;
    }

    return result;
}

//
// Return nregs, fixed or encoded within the ARM instruction
//
static Uns8 getNRegsARM(Uns32 instr, nregSpec nRegs) {

    Uns8 result = 0;

    switch(nRegs) {
        case NREG_NA:
            break;
        case NREG_1:
            result = 1;
            break;
        case NREG_2:
            result = 2;
            break;
        case NREG_3:
            result = 3;
            break;
        case NREG_4:
            result = 4;
            break;
        case NREG_2_8_P1:
            result = OP_U_2_8(instr) + 1;
            break;
        case NREG_7_1:
            result = OP_U_7_1(instr);
            break;
        case NREG_8_0:
            result = OP_U_8_0(instr);
            break;
        case NREG_1_5_B:
            result = OP_U_1_5(instr) ? 2 : 1;
            break;
        case NREG_1_6_B:
            result = OP_U_1_6(instr) ? 2 : 1;
            break;
        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME); break;
    }

    return result;
}

//
// Return incr value, fixed or encoded within the ARM instruction
//
static Uns8 getIncrARM(Uns32 instr, incrSpec incr) {

    Uns8 result = 0;

    switch(incr) {
        case INCR_0:     break;
        case INCR_1:     result = 1; break;
        case INCR_2:     result = 2; break;
        case INCR_1_5_B: result = OP_U_1_5(instr) ? 2 : 1; break;
        case INCR_1_6_B: result = OP_U_1_6(instr) ? 2 : 1; break;
        default: VMI_ABORT("%s: unimplemented case", FUNC_NAME); break;
    }

    return result;
}

//
// Negate the argument if U bit of instruction is zero
//
inline static Uns32 negateIfU(armInstructionInfoP info, Uns32 result) {
    return info->u ? result : -result;
}

//
// Rotate result by rotate
//
static Uns32 rotate(Uns32 result, Uns32 rotate) {

    if(rotate) {
        Uns32 part1 = (result>>rotate);
        Uns32 part2 = (result<<(32-rotate));
        result = part1 | part2;
    }

    return result;
}

//
// Allow for cases where constant value of zero is interpreted as 32
//
static Uns32 adjustShift(Uns32 result, constSpec c, armShiftOp so) {

    if((result==0) && (c!=CS_NA) && ((so==ARM_SO_LSR) || (so==ARM_SO_ASR))) {
        result = 32;
    }

    return result;
}

//
// Return a constant encoded within the ARM instruction
//
static Uns32 getConstantARM(
    armInstructionInfoP info,
    Uns32               instr,
    constSpec           c,
    armShiftOp          so
) {
    Uns32 result = 0;

    switch(c) {
        case CS_NA:
            break;
        case CS_U_9:
            result = OP_U_9(instr);
            break;
        case CS_U_2_10x8:
            result = OP_U_11_10(instr)*8;
            break;
        case CS_U_3_16N:
            result = 8 - OP_U_3_16(instr);
            break;
        case CS_U_3_16P:
            result = OP_U_3_16(instr);
            break;
        case CS_U_4_0:
            result = OP_U_4_0(instr);
            break;
        case CS_U_4_8:
            result = OP_U_4_8(instr);
            break;
        case CS_U_4_16N:
            result = 16 - OP_U_4_16(instr);
            break;
        case CS_U_4_16P:
            result = OP_U_4_16(instr);
            break;
        case CS_U_5_0:
            result = OP_U_5_0(instr);
            break;
        case CS_U_5_7:
            result = OP_U_5_7(instr);
            break;
        case CS_U_5_0_5M16:
            result = OP_U_5_0_5(instr);
            result = 16 - (result > 16 ? 0 : result);
            break;
        case CS_U_5_0_5M32:
            result = 32 - OP_U_5_0_5(instr);
            break;
        case CS_U_5_16N:
            result = 32 - OP_U_5_16(instr);
            break;
        case CS_U_5_16P:
            result = OP_U_5_16(instr);
            break;
        case CS_U_6_16N:
            result = 64 - OP_U_6_16(instr);
            break;
        case CS_U_6_16P:
            result = OP_U_6_16(instr);
            break;
        case CS_U_8_0:
            result = OP_U_8_0(instr);
            break;
        case CS_U_8_0_R:
            result = rotate(OP_U_8_0(instr), OP_ROTATE(instr));
            break;
        case CS_U_8_8_0_U:
            result = negateIfU(info, OP_U_8_8_0(instr));
            break;
        case CS_U_8_0x4_U:
            result = negateIfU(info, OP_U_8_0(instr)*4);
            break;
        case CS_U_12_0_U:
            result = negateIfU(info, OP_U_12_0(instr));
            break;
        case CS_U_16_8_0:
            result = OP_U_16_8_0(instr);
            break;
        case CS_U_16_16_0:
            result = OP_U_16_16_0(instr);
            break;
        case CS_U_24_0:
            result = OP_U_24_0(instr);
            break;
        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME);
            break;
    }

    // allow for cases where constant value of zero is interpreted as 32
    return adjustShift(result, c, so);
}

//
// Return a modified immediate constant for the given immediate value
//  If single is true then form a single precision value,
//  otherwise form the high order 32 bits of a double precision value (low order 32 bits are always all 0's)
//
static armSdfpMItype modifiedImmediateVFPARM(Uns32 imm, Bool single) {

    armSdfpMItype result;

    if (single) {

        // Set the sign bit to be a
        result.u32.w0 = (imm & 0x80) ? 0x80000000 : 0;

        // Set the exponent sign to be Bbbbbb  - where B = Not b
        result.u32.w0 |= (imm & 0x40) ? 0x3e000000 : 0x40000000;

        // set the mantissa to cdefgh
        result.u32.w0 |= (imm & 0x3f) << 19;

        // CLear the high order bits just to be safe
        result.u32.w1 = 0;

    } else {
        // Set the sign bit to be a
        result.u64 = (imm & 0x80) ? 0x8000000000000000ULL : 0ULL;

        // Set the exponent sign to be Bbbbbbbbb  - where B = Not b
        result.u64 |= (imm & 0x40) ? 0x3fc0000000000000ULL : 0x4000000000000000ULL;

        // set the mantissa to cdefgh
        result.u64 |= ((Uns64) (imm & 0x3f)) << 48;

    }

    return result;
}

//
// Return a modified immediate constant encoded within the SIMD instruction
//
static armSdfpMItype modifiedImmediateSIMDARM(Uns32 instr, armSDFPType dt) {

    armSdfpMItype result;

    // bitfield to extract instruction parts
    union {
        Uns32 instr;
        struct {
            Uns32 efgh  :  4;
            Uns32 _u1   :  1;
            Uns32 op    :  1;
            Uns32 _u2   :  2;
            Uns32 cmode :  4;
            Uns32 _u3   :  4;
            Uns32 bcd   :  3;
            Uns32 _u4   :  5;
            Uns32 a     :  1;
        } f;
    } u1 = {instr};

    // compose code
    Uns32 abcdefgh = (u1.f.a<<7) | (u1.f.bcd<<4) | u1.f.efgh;

    // derive result from cmode
    switch(u1.f.cmode) {

        case 0x00: case 0x01:
            VMI_ASSERT(dt == ARM_SDFPT_I32, "SIMD Modified immediate constant type does not match dt");
            result.u32.w0 = result.u32.w1 = abcdefgh;
            break;

        case 0x02: case 0x03:
            VMI_ASSERT(dt == ARM_SDFPT_I32, "SIMD Modified immediate constant type does not match dt");
            result.u32.w0 = result.u32.w1 = abcdefgh<<8;
            break;

        case 0x04: case 0x05:
            VMI_ASSERT(dt == ARM_SDFPT_I32, "SIMD Modified immediate constant type does not match dt");
            result.u32.w0 = result.u32.w1 = abcdefgh<<16;
            break;

        case 0x06: case 0x07:
            VMI_ASSERT(dt == ARM_SDFPT_I32, "SIMD Modified immediate constant type does not match dt");
            result.u32.w0 = result.u32.w1 = abcdefgh<<24;
            break;

        case 0x08: case 0x09:
            VMI_ASSERT(dt == ARM_SDFPT_I16, "SIMD Modified immediate constant type does not match dt");
            result.u16.h0 = result.u16.h1 = result.u16.h2 = result.u16.h3 = (Uns16) abcdefgh;
            break;

        case 0x0a: case 0x0b:
            VMI_ASSERT(dt == ARM_SDFPT_I16, "SIMD Modified immediate constant type does not match dt");
            result.u16.h0 = result.u16.h1 = result.u16.h2 = result.u16.h3 = (Uns16) (abcdefgh << 8);
            break;

        case 0x0c:
            VMI_ASSERT(dt == ARM_SDFPT_I32, "SIMD Modified immediate constant type does not match dt");
            result.u32.w0 = result.u32.w1 = (abcdefgh<<8)  | 0x000000ff;
            break;

        case 0x0d:
            VMI_ASSERT(dt == ARM_SDFPT_I32, "SIMD Modified immediate constant type does not match dt");
            result.u32.w0 = result.u32.w1 = (abcdefgh<<16) | 0x0000ffff;
            break;

        case 0x0e:
            if (u1.f.op == 0) {
                VMI_ASSERT(dt == ARM_SDFPT_I8, "SIMD Modified immediate constant type does not match dt");
                result.u8.b0 = result.u8.b1 = result.u8.b2 = result.u8.b3 =
                result.u8.b4 = result.u8.b5 = result.u8.b6 = result.u8.b7 = (Uns8) abcdefgh;
            } else {
                VMI_ASSERT(dt == ARM_SDFPT_I64, "SIMD Modified immediate constant type does not match dt");
                result.u8.b0 = (abcdefgh & 0x01) ? 0xff : 0x00;
                result.u8.b1 = (abcdefgh & 0x02) ? 0xff : 0x00;
                result.u8.b2 = (abcdefgh & 0x04) ? 0xff : 0x00;
                result.u8.b3 = (abcdefgh & 0x08) ? 0xff : 0x00;
                result.u8.b4 = (abcdefgh & 0x10) ? 0xff : 0x00;
                result.u8.b5 = (abcdefgh & 0x20) ? 0xff : 0x00;
                result.u8.b6 = (abcdefgh & 0x40) ? 0xff : 0x00;
                result.u8.b7 = (abcdefgh & 0x80) ? 0xff : 0x00;
            }
            break;

        case 0x0f:
            if (u1.f.op == 0) {

                VMI_ASSERT(dt == ARM_SDFPT_F32, "SIMD Modified immediate constant type does not match dt");
                result = modifiedImmediateVFPARM(abcdefgh, True);

                // For SIMD, F32 type value is duplicated in high order 32 bits also
                result.u32.w1 = result.u32.w0;

            } else {

                // Undefined instruction - Should be caught by decoder
                VMI_ABORT("SIMD modified immediate with op=1 and cmode=0xf found");

            }

            break;

        default:
            // shouldn't happen - only a 4 bit field
            VMI_ABORT("Unknown SIMD modified immediate cmode value=0x%x found", u1.f.cmode);
            break;
    }

    return result;
}

//
// Decode and return a SIMD/VFP modified immediate constant encoded within the ARM instruction
//
static armSdfpMItype getSdfpMIARM(
    Uns32       instr,
    sdfpMISpec  s,
    armSDFPType dt
) {

    armSdfpMItype result;

    result.u64 = 0;

    switch (s) {

        case SDFP_MI_NA:
            break;
        case SDFP_MI_SIMD:
            result = modifiedImmediateSIMDARM(instr, dt);
            break;
        case SDFP_MI_VFP_S:
            VMI_ASSERT(dt == ARM_SDFPT_F32, "VFP Modified immediate constant type does not match dt");
            result = modifiedImmediateVFPARM(OP_U_8_16_0(instr), True);
            break;
        case SDFP_MI_VFP_D:
            VMI_ASSERT(dt == ARM_SDFPT_F64, "VFP Modified immediate constant type does not match dt");
            result = modifiedImmediateVFPARM(OP_U_8_16_0(instr), False);
            break;
        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME);
            break;
    }

    return result;
}

//
// Get post-indexed attributes for the ARM instruction
//
static Bool getPostIndexedARM(Uns32 instr, postIndexSpec pi) {

    Bool result = False;

    switch(pi) {
        case PI_0:
            result = False;
            break;
        case PI_24:
            result = !OP_PI(instr);
            break;
        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME);
            break;
    }

    return result;
}

//
// Get writeback attrtibutes for the ARM instruction
//
static Bool getWritebackARM(Uns32 instr, writebackSpec wb) {

    Bool result = False;

    switch(wb) {
        case WB_0:
            result = False;
            break;
        case WB_21:
            result = OP_WB(instr);
            break;
        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME);
            break;
    }

    return result;
}

//
// Return target address calculated by adding the current PC and the displacement
//
static Uns32 getTarget(armInstructionInfoP info, Int32 displacement) {
    return info->thisPC + displacement;
}

//
// Return a target address encoded within the ARM instruction
//
static Uns32 getTargetARM(
    armInstructionInfoP info,
    Uns32               instr,
    targetSpec          t
) {
    Uns32 result = 0;

    switch(t) {
        case TC_NA:
            break;
        case TC_S24:
            result = getTarget(info, OP_TS24(instr)+8);
            break;
        case TC_S24H:
            result = getTarget(info, OP_TS24H(instr)+8);
            break;
        default:
            VMI_ABORT("%s: unimplemented case", FUNC_NAME);
            break;
    }

    return result;
}

//
// Return coprocessor opcode1 encoded within the ARM instruction
//
static Uns32 getCpOp1ARM(
    armInstructionInfoP info,
    Uns32               instr,
    cpOp1Spec           cpOp1
) {
    Uns32 result = 0;

    switch(cpOp1) {
        case COP_NA:   break;
        case COP_4_4:  result = OP_CPOP1_4_4(instr);  break;
        case COP_4_20: result = OP_CPOP1_4_20(instr); break;
        case COP_3_21: result = OP_CPOP1_3_21(instr); break;
        default:       VMI_ABORT("%s: unimplemented case", FUNC_NAME); break;
    }

    return result;
}

//
// Return register list encoded within the ARM instruction
//
static Uns32 getRListARM(Uns32 instr, rListSpec rList) {

    Uns32 result = 0;

    switch(rList) {
        case RL_NA: break;
        case RL_32:   result = OP_RLIST(instr); break;
        default:    VMI_ABORT("%s: unimplemented case", FUNC_NAME); break;
    }

    return result;
}

//
// Return increment/decrement action encoded within the ARM instruction
//
static armIncDec getIncDecARM(
    armInstructionInfoP info,
    armP                arm,
    Uns32               instr,
    incDecSpec          incDec
) {
    armIncDec result = ARM_ID_NA;

    if(incDec) {

        armIncDec USpec = OP_U (instr) ? ARM_ID_I : ARM_ID_D;
        armIncDec PSpec = OP_PI(instr) ? ARM_ID_B : ARM_ID_A;

        result = (ARM_ID_P | USpec | PSpec);

        if(arm->UAL && (result==ARM_ID_IA) && (incDec==ID_U_P_IAI)) {
            result = ARM_ID_IAI;
        }
    }

    return result;
}

//
// Return flags affected by the ARM instruction
//
static armFlagAffect getFlagAffectARM(armInstructionInfoP info, Uns32 instr) {

    armFlagAffect result = ARM_FAFF_NA;

    if((info->fact==ARM_FACT_IE) || (info->fact==ARM_FACT_ID)) {
        result = OP_AIF(instr);
    }

    return result;
}


////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE
////////////////////////////////////////////////////////////////////////////////

//
// Do simple decode of an ARM instruction
//
armInstructionType armGetARMInstructionType(
    armP   arm,
    Uns32  thisPC,
    Uns32 *instrP
) {
    static vmidDecodeTableP decodeTable;

    Uns32 instr = vmicxtFetch4Byte((vmiProcessorP)arm, thisPC);

    // create instruction decode table if required
    if(!decodeTable) {
        decodeTable = createDecodeTableARM();
    }

    // return instruction using byref argument
    if(instrP) *instrP = instr;

    // decode the instruction using decode table
    return vmidDecode(decodeTable, instr);
}

//
// Decode the ARM instruction at the passed address. The 'info' structure is
// filled with details of the instruction.
//
void armDecodeARM(armP arm, Uns32 thisPC, armInstructionInfoP info) {

    Uns32 instr;

    // decode the instruction using decode table
    info->type = armGetARMInstructionType(arm, thisPC, &instr);

    // save instruction
    info->instruction = instr;

    // get instruction attributes based on type
    opAttrsCP attrs = &attrsArray[info->type];

    // specify the name and format for the opcode
    info->opcode = attrs->opcode;
    info->format = attrs->format;

    // specify required architecture for the instruction
    info->support = attrs->support;
    info->isar    = attrs->isar;

    // instruction is always 4 bytes
    info->bytes = 4;

    // indicate whether the instruction sets flags
    info->f = (attrs->f && OP_F(instr)) ? attrs->f : ARM_SF_0;

    // indicate whether the instruction is conditional
    info->cond = attrs->cond ? OP_COND(instr) : ARM_C_AL;

    // get registers used by this instruction
    info->r1 = getRegisterARM(instr, attrs->r1);
    info->r2 = getRegisterARM(instr, attrs->r2);
    info->r3 = getRegisterARM(instr, attrs->r3);
    info->r4 = getRegisterARM(instr, attrs->r4);

    // get shiftop for the instruction
    info->so = getShiftOpARM(instr, attrs->ss);

    // get load/store size, sign extension, translate and exclusive access
    // settings
    info->sz    = attrs->sz;
    info->w     = getWidthARM(instr, attrs->w);
    info->xs    = attrs->xs;
    info->tl    = attrs->tl;
    info->ea    = attrs->ea;

    // Get SIMD instruction settings
    info->index = getIndexARM(instr, attrs->index);
    info->align = getAlignARM(instr, attrs->align);
    info->nregs = getNRegsARM(instr, attrs->nregs);
    info->incr  = getIncrARM (instr, attrs->incr);
    info->nels  = attrs->nels;

    // does the opcode have a long load field?
    info->ll = attrs->ll ? OP_LL(instr) : 0;

    // get U bit (must be valid *before* getConstantARM which uses it)
    info->u = OP_U(instr);

    // get any constant value and constant rotate associated with the instruction
    info->c       = getConstantARM(info, instr, attrs->cs, info->so);
    info->crotate = (attrs->cs==CS_U_8_0_R) ? OP_ROTATE(instr) : 0;

    // get any SIMD modified immediate constant value (64 bits long)
    info->sdfpMI = getSdfpMIARM (instr, attrs->sdfpMI, attrs->dt1);

    // get any constant target address associated with the instruction
    info->t = getTargetARM(info, instr, attrs->ts);

    // get any coprocessor fields associated with the instruction
    info->cpNum = attrs->cpNum ? OP_CPNUM(instr) : 0;
    info->cpOp1 = getCpOp1ARM(info, instr, attrs->cpOp1);
    info->cpOp2 = attrs->cpOp2 ? OP_CPOP2(instr) : 0;

    // get any register list and increment/decrement specification
    info->rList  = getRListARM(instr, attrs->rList);
    info->incDec = getIncDecARM(info, arm, instr, attrs->incDec);

    // get post-indexed and writeback attrtibutes for the instruction
    info->pi = getPostIndexedARM(instr, attrs->pi);
    info->wb = info->pi || getWritebackARM(instr, attrs->wb);

    // specify action on unaligned access
    if(info->align==0) {
        info->ua = CP_FIELD(arm, SCTLR, U) ? attrs->ua67 : attrs->ua45;
    } else {
        // Instructions that specify alignment ALWAYS do a DABORT on unaligned address
        info->ua = ARM_UA_DABORT;
    }


    // get flag and mode effect fields (CPS instruction)
    info->ma   = attrs->m    ? OP_MA(instr)   : 0;
    info->fact = attrs->imod ? OP_IMOD(instr) : 0;
    info->faff = getFlagAffectARM(info, instr);

    // get if-then specification
    info->it = 0;

    // get ThumbEE mode handler specification
    info->handler = 0;

    // get floating point type specification
    info->dt1 = attrs->dt1;
    info->dt2 = attrs->dt2;
}
