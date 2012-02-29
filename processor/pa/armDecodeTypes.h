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

#ifndef ARM_DECODE_TYPES_H
#define ARM_DECODE_TYPES_H

// basic number types
#include "hostapi/impTypes.h"

// model header files
#include "armTypeRefs.h"
#include "armVariant.h"


//
// Instruction type for an instruction with a single variant
//
#define ITYPE_SINGLE(_NAME) \
    ARM_IT_##_NAME

//
// Instruction types for normal instructions like ADC
//
#define ITYPE_SET_ADC(_NAME) \
    ARM_IT_##_NAME##_IMM,               \
    ARM_IT_##_NAME##_RM,                \
    ARM_IT_##_NAME##_RM_SHFT_IMM,       \
    ARM_IT_##_NAME##_RM_SHFT_RS,        \
    ARM_IT_##_NAME##_RM_RRX,            \
    ARM_IT_##_NAME##_IT,                \
    ARM_IT_##_NAME##_RT

//
// Instruction types for normal instructions like LDC
//
#define ITYPE_SET_LDC(_NAME) \
    ARM_IT_##_NAME##_IMM,               \
    ARM_IT_##_NAME##_UNINDEXED

//
// Instruction types for normal instructions like LDR
//
#define ITYPE_SET_LDR(_NAME) \
    ARM_IT_##_NAME##_IMM,               \
    ARM_IT_##_NAME##_RM,                \
    ARM_IT_##_NAME##_RM_SHFT_IMM,       \
    ARM_IT_##_NAME##_RM_RRX

//
// Instruction types for normal instructions like LDRH
//
#define ITYPE_SET_LDRH(_NAME) \
    ARM_IT_##_NAME##_IMM,               \
    ARM_IT_##_NAME##_RM                 \

//
// Instruction types for normal instructions like MOV
//
#define ITYPE_SET_MOV(_NAME) \
    ARM_IT_##_NAME##_IMM,               \
    ARM_IT_##_NAME##_RM,                \
    ARM_IT_##_NAME##_RM_SHFT_IMM,       \
    ARM_IT_##_NAME##_RM_SHFT_RS,        \
    ARM_IT_##_NAME##_RM_RRX,            \
    ARM_IT_##_NAME##_RM_SHFT_RST

//
// Instruction types for normal instructions like MSRC
//
#define ITYPE_SET_MSRC(_NAME) \
     ARM_IT_##_NAME##_IMM,              \
     ARM_IT_##_NAME##_RM

//
// Instruction types for DSP instructions like SMLA<x><y>
//
#define ITYPE_SET_SMLA_XY(_NAME) \
     ARM_IT_##_NAME##BB,                \
     ARM_IT_##_NAME##BT,                \
     ARM_IT_##_NAME##TB,                \
     ARM_IT_##_NAME##TT

//
// Instruction types for DSP instructions like SMLAW<y>
//
#define ITYPE_SET_SMLAW_Y(_NAME) \
     ARM_IT_##_NAME##B,                 \
     ARM_IT_##_NAME##T

//
// Instruction types for normal instructions like PLD
//
#define ITYPE_SET_PLD(_NAME) \
    ARM_IT_##_NAME##_IMM,               \
    ARM_IT_##_NAME##_RM,                \
    ARM_IT_##_NAME##_RM_SHFT_IMM,       \
    ARM_IT_##_NAME##_RM_RRX

//
// Instruction types for parallel add/subtract Media instructions
//
#define ITYPE_SET_PAS(_NAME) \
    ARM_IT_S##_NAME,                    \
    ARM_IT_Q##_NAME,                    \
    ARM_IT_SH##_NAME,                   \
    ARM_IT_U##_NAME,                    \
    ARM_IT_UQ##_NAME,                   \
    ARM_IT_UH##_NAME

//
// Instruction types for Media instructions with optional argument exchange
//
#define ITYPE_MEDIA_X(_NAME)   \
    ARM_IT_##_NAME,            \
    ARM_IT_##_NAME##X

//
// Instruction types for Media instructions with optional result rounding
//
#define ITYPE_MEDIA_R(_NAME)   \
    ARM_IT_##_NAME,            \
    ARM_IT_##_NAME##R

//
// Instruction types for SIMD instructions with esize 16 variant
//
#define ITYPE_SIMD_H(_NAME)    \
    ITYPE_SINGLE(_NAME##_H)
//
// Instruction types for SIMD instructions with esize 8 and 16 variants
//
#define ITYPE_SIMD_BH(_NAME)    \
    ITYPE_SINGLE(_NAME##_H),    \
    ITYPE_SINGLE(_NAME##_B)

//
// Instruction types for SIMD instructions with esize 16 and 32 variants
//
#define ITYPE_SIMD_HW(_NAME)    \
    ITYPE_SINGLE(_NAME##_W),    \
    ITYPE_SINGLE(_NAME##_H)

//
// Instruction types for SIMD instructions with esize 8, 16 and 32 variants
//
#define ITYPE_SIMD_BHW(_NAME)   \
    ITYPE_SINGLE(_NAME##_W),    \
    ITYPE_SINGLE(_NAME##_H),    \
    ITYPE_SINGLE(_NAME##_B)

//
// Instruction types for SIMD instructions with esize 16, 32 and 64 variants
//
#define ITYPE_SIMD_HWD(_NAME)   \
    ITYPE_SINGLE(_NAME##_D),    \
    ITYPE_SINGLE(_NAME##_W),    \
    ITYPE_SINGLE(_NAME##_H)

//
// Instruction types for SIMD instructions with esize 8, 16, 32 and 64 variants
//
#define ITYPE_SIMD_BHWD(_NAME)   \
    ITYPE_SINGLE(_NAME##_D),    \
    ITYPE_SINGLE(_NAME##_W),    \
    ITYPE_SINGLE(_NAME##_H),    \
    ITYPE_SINGLE(_NAME##_B)

//
// Instruction types for SIMD instructions with Q and D variants
//
#define ITYPE_SIMD_QD(_NAME)   \
    ITYPE_SINGLE(_NAME##_Q),   \
    ITYPE_SINGLE(_NAME##_D)

//
// Instruction types for SIMD instructions with D/Q and esize 8 variant only
//
#define ITYPE_SIMD_QD_B(_NAME)  \
    ITYPE_SIMD_QD(_NAME##_B)

//
// Instruction types for SIMD instructions with D/Q and esize 8 and 16 variants
//
#define ITYPE_SIMD_QD_BH(_NAME)  \
    ITYPE_SIMD_QD(_NAME##_H),    \
    ITYPE_SIMD_QD(_NAME##_B)

//
// Instruction types for SIMD instructions with D/Q and esize 16 and 32 variants
//
#define ITYPE_SIMD_QD_HW(_NAME)  \
    ITYPE_SIMD_QD(_NAME##_W),    \
    ITYPE_SIMD_QD(_NAME##_H)

//
// Instruction types for SIMD instructions with D/Q and esize 32, 16 and 8 variants
//
#define ITYPE_SIMD_QD_BHW(_NAME) \
    ITYPE_SIMD_QD(_NAME##_W),  \
    ITYPE_SIMD_QD(_NAME##_H),    \
    ITYPE_SIMD_QD(_NAME##_B)

//
// Instruction types for SIMD instructions with Q esize 32, 16 and 8 variants and D esize 16 and 8 variants
//
#define ITYPE_SIMD_Q_BHW_D_BH(_NAME) \
    ITYPE_SINGLE (_NAME##_W_Q),  \
    ITYPE_SIMD_QD(_NAME##_H),    \
    ITYPE_SIMD_QD(_NAME##_B)

//
// Instruction types for SIMD instructions with D/Q and esize 64, 32, 16 and 8 variants
//
#define ITYPE_SIMD_QD_BHWD(_NAME) \
    ITYPE_SIMD_QD(_NAME##_D),     \
    ITYPE_SIMD_QD(_NAME##_W),     \
    ITYPE_SIMD_QD(_NAME##_H),     \
    ITYPE_SIMD_QD(_NAME##_B)

//
// Instruction types for SIMD instructions vrev16, vrev32 and vrev64
//
#define ITYPE_SIMD_VREV(_NAME)    \
    ITYPE_SIMD_QD_B  (_NAME##16), \
    ITYPE_SIMD_QD_BH (_NAME##32), \
    ITYPE_SIMD_QD_BHW(_NAME##64)

//
// Instruction types for VFP instructions with D and S variants
//
#define ITYPE_VFP_DS(_NAME)       \
    ARM_IT_##_NAME##_D,           \
    ARM_IT_##_NAME##_S

//
// Instruction type enumeration
//
typedef enum armInstructionTypeE {

    ////////////////////////////////////////////////////////////////////////////
    // ARM INSTRUCTIONS
    ////////////////////////////////////////////////////////////////////////////

    // data processing instructions
    ITYPE_SET_ADC (ADC),
    ITYPE_SET_ADC (ADD),
    ITYPE_SET_ADC (AND),
    ITYPE_SET_ADC (BIC),
    ITYPE_SET_ADC (EOR),
    ITYPE_SET_MOV (MOV),
    ITYPE_SET_ADC (MUL),
    ITYPE_SET_MOV (MVN),
    ITYPE_SET_MOV (NEG),
    ITYPE_SET_ADC (ORN),
    ITYPE_SET_ADC (ORR),
    ITYPE_SET_ADC (RSB),
    ITYPE_SET_ADC (RSC),
    ITYPE_SET_ADC (SBC),
    ITYPE_SET_ADC (SUB),

    // ARMv6T2 move instructions
    ITYPE_SINGLE (MOVT),
    ITYPE_SINGLE (MOVW),

    // multiply instructions
    ITYPE_SINGLE (MLA  ),
    ITYPE_SINGLE (MLS  ),
    ITYPE_SINGLE (MUL  ),
    ITYPE_SINGLE (SMLAL),
    ITYPE_SINGLE (SMULL),
    ITYPE_SINGLE (UMAAL),
    ITYPE_SINGLE (UMLAL),
    ITYPE_SINGLE (UMULL),

    // compare instructions
    ITYPE_SET_ADC (CMN),
    ITYPE_SET_ADC (CMP),
    ITYPE_SET_ADC (TEQ),
    ITYPE_SET_ADC (TST),

    // branch instructions
    ITYPE_SINGLE (B   ),
    ITYPE_SINGLE (BL  ),
    ITYPE_SINGLE (BLX1),
    ITYPE_SINGLE (BLX2),
    ITYPE_SINGLE (BX  ),
    ITYPE_SINGLE (BXJ ),

    // MURAC instructions
    ITYPE_SINGLE (BAA ),

    // miscellaneous instructions
    ITYPE_SINGLE (BKPT),
    ITYPE_SINGLE (CLZ ),
    ITYPE_SINGLE (SWI ),

    // load and store instructions
    ITYPE_SET_LDR  (LDR  ),
    ITYPE_SET_LDR  (LDRB ),
    ITYPE_SET_LDR  (LDRBT),
    ITYPE_SET_LDRH (LDRH ),
    ITYPE_SET_LDRH (LDRSB),
    ITYPE_SET_LDRH (LDRSH),
    ITYPE_SET_LDR  (LDRT ),
    ITYPE_SET_LDR  (STR  ),
    ITYPE_SET_LDR  (STRB ),
    ITYPE_SET_LDR  (STRBT),
    ITYPE_SET_LDRH (STRH ),
    ITYPE_SET_LDR  (STRT ),

    // load and store multiple instructions
    ITYPE_SINGLE (LDM1),
    ITYPE_SINGLE (LDM2),
    ITYPE_SINGLE (LDM3),
    ITYPE_SINGLE (STM1),
    ITYPE_SINGLE (STM2),

    // ARMv6T2 load and store instructions
    ITYPE_SET_LDRH (LDRHT ),
    ITYPE_SET_LDRH (LDRSBT),
    ITYPE_SET_LDRH (LDRSHT),
    ITYPE_SET_LDRH (STRHT ),

    // semaphore instructions
    ITYPE_SINGLE (SWP ),
    ITYPE_SINGLE (SWPB),

    // synchronization primitives
    ITYPE_SINGLE (LDREX ),
    ITYPE_SINGLE (LDREXB),
    ITYPE_SINGLE (LDREXH),
    ITYPE_SINGLE (LDREXD),
    ITYPE_SINGLE (STREX ),
    ITYPE_SINGLE (STREXB),
    ITYPE_SINGLE (STREXH),
    ITYPE_SINGLE (STREXD),

    // coprocessor instructions
    ITYPE_SINGLE  (CDP ),
    ITYPE_SINGLE  (CDP2),
    ITYPE_SET_LDC (LDC ),
    ITYPE_SET_LDC (LDC2),
    ITYPE_SINGLE  (MCR ),
    ITYPE_SINGLE  (MCR2),
    ITYPE_SINGLE  (MRC ),
    ITYPE_SINGLE  (MRC2),
    ITYPE_SET_LDC (STC ),
    ITYPE_SET_LDC (STC2),

    // status register access instructions
    ITYPE_SINGLE   (MRSC),
    ITYPE_SINGLE   (MRSS),
    ITYPE_SET_MSRC (MSRC),
    ITYPE_SET_MSRC (MSRS),

    // hints
    ITYPE_SINGLE (NOP  ),
    ITYPE_SINGLE (YIELD),
    ITYPE_SINGLE (WFE  ),
    ITYPE_SINGLE (WFI  ),
    ITYPE_SINGLE (SEV  ),
    ITYPE_SINGLE (DBG  ),

    // ARMv6 exception instructions
    ITYPE_SINGLE (SRS),
    ITYPE_SINGLE (RFE),

    // ARMv6 miscellaneous instructions
    ITYPE_SINGLE (SETEND),
    ITYPE_SINGLE (CPS   ),
    ITYPE_SINGLE (CLREX ),

    // ARMv6/ARMv7 hint instructions
    ITYPE_SET_PLD (PLD  ),
    ITYPE_SET_PLD (PLI  ),
    ITYPE_SET_PLD (PLDW ),
    ITYPE_SINGLE  (DMB  ),
    ITYPE_SINGLE  (DSB  ),
    ITYPE_SINGLE  (ISB  ),
    ITYPE_SINGLE  (UHINT),

    ////////////////////////////////////////////////////////////////////////////
    // DSP INSTRUCTIONS
    ////////////////////////////////////////////////////////////////////////////

    // data processing instructions
    ITYPE_SINGLE (QADD ),
    ITYPE_SINGLE (QDADD),
    ITYPE_SINGLE (QDSUB),
    ITYPE_SINGLE (QSUB ),

    // multiply instructions
    ITYPE_SET_SMLA_XY (SMLA ),
    ITYPE_SET_SMLA_XY (SMLAL),
    ITYPE_SET_SMLAW_Y (SMLAW),
    ITYPE_SET_SMLA_XY (SMUL ),
    ITYPE_SET_SMLAW_Y (SMULW),

    // load and store instructions
    ITYPE_SET_LDRH (LDRD),
    ITYPE_SET_LDRH (STRD),

    // coprocessor instructions
    ITYPE_SINGLE (MCRR ),
    ITYPE_SINGLE (MCRR2),
    ITYPE_SINGLE (MRRC ),
    ITYPE_SINGLE (MRRC2),

    ////////////////////////////////////////////////////////////////////////////
    // MEDIA INSTRUCTIONS
    ////////////////////////////////////////////////////////////////////////////

    // basic instructions
    ITYPE_SINGLE (USAD8 ),
    ITYPE_SINGLE (USADA8),
    ITYPE_SINGLE (SBFX  ),
    ITYPE_SINGLE (BFC   ),
    ITYPE_SINGLE (BFI   ),
    ITYPE_SINGLE (UBFX  ),

    // parallel add/subtract instructions
    ITYPE_SET_PAS (ADD16),
    ITYPE_SET_PAS (ASX  ),
    ITYPE_SET_PAS (SAX  ),
    ITYPE_SET_PAS (SUB16),
    ITYPE_SET_PAS (ADD8 ),
    ITYPE_SET_PAS (SUB8 ),

    // packing, unpacking, saturation and reversal instructions
    ITYPE_SINGLE (PKHBT  ),
    ITYPE_SINGLE (PKHTB  ),
    ITYPE_SINGLE (SSAT   ),
    ITYPE_SINGLE (SSAT16 ),
    ITYPE_SINGLE (USAT   ),
    ITYPE_SINGLE (USAT16 ),
    ITYPE_SINGLE (SXTAB  ),
    ITYPE_SINGLE (UXTAB  ),
    ITYPE_SINGLE (SXTAB16),
    ITYPE_SINGLE (UXTAB16),
    ITYPE_SINGLE (SXTAH  ),
    ITYPE_SINGLE (UXTAH  ),
    ITYPE_SINGLE (SXTB   ),
    ITYPE_SINGLE (UXTB   ),
    ITYPE_SINGLE (SXTB16 ),
    ITYPE_SINGLE (UXTB16 ),
    ITYPE_SINGLE (SXTH   ),
    ITYPE_SINGLE (UXTH   ),
    ITYPE_SINGLE (SEL    ),
    ITYPE_SINGLE (REV    ),
    ITYPE_SINGLE (REV16  ),
    ITYPE_SINGLE (RBIT   ),
    ITYPE_SINGLE (REVSH  ),

    // signed multiply instructions
    ITYPE_MEDIA_X (SMLAD ),
    ITYPE_MEDIA_X (SMUAD ),
    ITYPE_MEDIA_X (SMLSD ),
    ITYPE_MEDIA_X (SMUSD ),
    ITYPE_MEDIA_X (SMLALD),
    ITYPE_MEDIA_X (SMLSLD),
    ITYPE_MEDIA_R (SMMLA ),
    ITYPE_MEDIA_R (SMMUL ),
    ITYPE_MEDIA_R (SMMLS ),

    ////////////////////////////////////////////////////////////////////////////
    // THUMB INSTRUCTIONS (WHEN DISTINCT FROM ARM INSTRUCTIONS)
    ////////////////////////////////////////////////////////////////////////////

    // data processing instructions
    ITYPE_SINGLE (ADD4),
    ITYPE_SINGLE (ADD6),
    ITYPE_SINGLE (ADD7),
    ITYPE_SINGLE (SUB4),
    ITYPE_SINGLE (MOV3),

    // address instructions
    ITYPE_SINGLE (ADD_ADR),
    ITYPE_SINGLE (SUB_ADR),

    // branch instructions
    ITYPE_SINGLE (CBNZ),
    ITYPE_SINGLE (CBZ ),
    ITYPE_SINGLE (TB  ),

    // divide instructions
    ITYPE_SINGLE (SDIV),
    ITYPE_SINGLE (UDIV),

    // 16-bit branch instructions
    ITYPE_SINGLE (BL_H10),
    ITYPE_SINGLE (BL_H11),
    ITYPE_SINGLE (BL_H01),

    // ThumbEE instructions
    ITYPE_SINGLE (UND_EE),
    ITYPE_SINGLE (HBP   ),
    ITYPE_SINGLE (HB    ),
    ITYPE_SINGLE (HBL   ),
    ITYPE_SINGLE (HBLP  ),
    ITYPE_SINGLE (CHKA  ),
    ITYPE_SINGLE (ENTERX),
    ITYPE_SINGLE (LEAVEX),

    ////////////////////////////////////////////////////////////////////////////
    // SIMD/VFP INSTRUCTIONS
    ////////////////////////////////////////////////////////////////////////////

    // SIMD data processing instructions: Miscellaneous
    ITYPE_SIMD_QD      (VEXT),
    ITYPE_SINGLE       (VTBL),
    ITYPE_SINGLE       (VTBX),
    ITYPE_SIMD_QD_BHW  (VDUPZ),

    // SIMD data processing instructions: 3 regs same length
    ITYPE_SIMD_QD_BHW  (VHADDU),
    ITYPE_SIMD_QD_BHW  (VHADDS),
    ITYPE_SIMD_QD_BHWD (VQADDU),
    ITYPE_SIMD_QD_BHWD (VQADDS),
    ITYPE_SIMD_QD_BHW  (VRHADDU),
    ITYPE_SIMD_QD_BHW  (VRHADDS),
    ITYPE_SIMD_QD      (VAND),
    ITYPE_SIMD_QD      (VBIC),
    ITYPE_SIMD_QD      (VORR),
    ITYPE_SIMD_QD      (VORN),
    ITYPE_SIMD_QD      (VEOR),
    ITYPE_SIMD_QD      (VBSL),
    ITYPE_SIMD_QD      (VBIT),
    ITYPE_SIMD_QD      (VBIF),
    ITYPE_SIMD_QD_BHW  (VHSUBU),
    ITYPE_SIMD_QD_BHW  (VHSUBS),
    ITYPE_SIMD_QD_BHWD (VQSUBU),
    ITYPE_SIMD_QD_BHWD (VQSUBS),
    ITYPE_SIMD_QD_BHW  (VCGTU),
    ITYPE_SIMD_QD_BHW  (VCGTS),
    ITYPE_SIMD_QD_BHW  (VCGEU),
    ITYPE_SIMD_QD_BHW  (VCGES),
    ITYPE_SIMD_QD_BHWD (VSHLU),
    ITYPE_SIMD_QD_BHWD (VSHLS),
    ITYPE_SIMD_QD_BHWD (VQSHLU),
    ITYPE_SIMD_QD_BHWD (VQSHLS),
    ITYPE_SIMD_QD_BHWD (VRSHLU),
    ITYPE_SIMD_QD_BHWD (VRSHLS),
    ITYPE_SIMD_QD_BHWD (VQRSHLU),
    ITYPE_SIMD_QD_BHWD (VQRSHLS),
    ITYPE_SIMD_QD_BHW  (VMAXU),
    ITYPE_SIMD_QD_BHW  (VMAXS),
    ITYPE_SIMD_QD_BHW  (VMINU),
    ITYPE_SIMD_QD_BHW  (VMINS),
    ITYPE_SIMD_QD_BHW  (VABDU),
    ITYPE_SIMD_QD_BHW  (VABDS),
    ITYPE_SIMD_QD_BHW  (VABAU),
    ITYPE_SIMD_QD_BHW  (VABAS),
    ITYPE_SIMD_QD_BHWD (VADD),
    ITYPE_SIMD_QD_BHWD (VSUB),
    ITYPE_SIMD_QD_BHW  (VTST),
    ITYPE_SIMD_QD_BHW  (VCEQ),
    ITYPE_SIMD_QD_BHW  (VMLA),
    ITYPE_SIMD_QD_BHW  (VMLS),
    ITYPE_SIMD_QD_BHW  (VMUL),
    ITYPE_SIMD_QD      (VMUL_P),
    ITYPE_SIMD_BHW     (VPMAXU),
    ITYPE_SIMD_BHW     (VPMAXS),
    ITYPE_SIMD_BHW     (VPMINU),
    ITYPE_SIMD_BHW     (VPMINS),
    ITYPE_SIMD_QD_HW   (VQDMULH),
    ITYPE_SIMD_QD_HW   (VQRDMULH),
    ITYPE_SIMD_BHW     (VPADD),
    ITYPE_SIMD_QD      (VADD_F),
    ITYPE_SIMD_QD      (VSUB_F),
    ITYPE_SINGLE       (VPADD_F),
    ITYPE_SIMD_QD      (VABD_F),
    ITYPE_SIMD_QD      (VMLA_F),
    ITYPE_SIMD_QD      (VMLS_F),
    ITYPE_SIMD_QD      (VFMA_F),
    ITYPE_SIMD_QD      (VFMS_F),
    ITYPE_SIMD_QD      (VMUL_F),
    ITYPE_SIMD_QD      (VCEQ_F),
    ITYPE_SIMD_QD      (VCGE_F),
    ITYPE_SIMD_QD      (VCGT_F),
    ITYPE_SIMD_QD      (VACGE_F),
    ITYPE_SIMD_QD      (VACGT_F),
    ITYPE_SIMD_QD      (VMAX_F),
    ITYPE_SIMD_QD      (VMIN_F),
    ITYPE_SINGLE       (VPMAX_F),
    ITYPE_SINGLE       (VPMIN_F),
    ITYPE_SIMD_QD      (VRECPS),
    ITYPE_SIMD_QD      (VRSQRTS),

    // SIMD data processing instructions: 3 regs different length
    ITYPE_SIMD_BHW    (VADDLS),
    ITYPE_SIMD_BHW    (VADDLU),
    ITYPE_SIMD_BHW    (VADDWS),
    ITYPE_SIMD_BHW    (VADDWU),
    ITYPE_SIMD_BHW    (VSUBLS),
    ITYPE_SIMD_BHW    (VSUBLU),
    ITYPE_SIMD_BHW    (VSUBWS),
    ITYPE_SIMD_BHW    (VSUBWU),
    ITYPE_SIMD_HWD    (VADDHN),
    ITYPE_SIMD_HWD    (VRADDHN),
    ITYPE_SIMD_BHW    (VABALS),
    ITYPE_SIMD_BHW    (VABALU),
    ITYPE_SIMD_HWD    (VSUBHN),
    ITYPE_SIMD_HWD    (VRSUBHN),
    ITYPE_SIMD_BHW    (VABDLS),
    ITYPE_SIMD_BHW    (VABDLU),
    ITYPE_SIMD_BHW    (VMLALS),
    ITYPE_SIMD_BHW    (VMLALU),
    ITYPE_SIMD_BHW    (VMLSLS),
    ITYPE_SIMD_BHW    (VMLSLU),
    ITYPE_SIMD_HW     (VQDMLAL),
    ITYPE_SIMD_HW     (VQDMLSL),
    ITYPE_SIMD_BHW    (VMULLS),
    ITYPE_SIMD_BHW    (VMULLU),
    ITYPE_SIMD_HW     (VQDMULL),
    ITYPE_SINGLE      (VMULL_P),

    // SIMD data processing instructions - 2 regs and a scalar
    ITYPE_SIMD_QD_HW  (VMLAZ),
    ITYPE_SIMD_QD     (VMLAZ_F),
    ITYPE_SIMD_QD_HW  (VMLSZ),
    ITYPE_SIMD_QD     (VMLSZ_F),
    ITYPE_SIMD_HW     (VMLALZS),
    ITYPE_SIMD_HW     (VMLALZU),
    ITYPE_SIMD_HW     (VMLSLZS),
    ITYPE_SIMD_HW     (VMLSLZU),
    ITYPE_SIMD_HW     (VQDMLALZ),
    ITYPE_SIMD_HW     (VQDMLSLZ),
    ITYPE_SIMD_QD_HW  (VMULZ),
    ITYPE_SIMD_QD     (VMULZ_F),
    ITYPE_SIMD_HW     (VMULLZS),
    ITYPE_SIMD_HW     (VMULLZU),
    ITYPE_SIMD_HW     (VQDMULLZ),
    ITYPE_SIMD_QD_HW  (VQDMULHZ),
    ITYPE_SIMD_QD_HW  (VQRDMULHZ),

    // SIMD data processing instructions - 2 regs and a shift amount
    ITYPE_SIMD_QD_BHWD (VSHRS),
    ITYPE_SIMD_QD_BHWD (VSHRU),
    ITYPE_SIMD_QD_BHWD (VSRAS),
    ITYPE_SIMD_QD_BHWD (VSRAU),
    ITYPE_SIMD_QD_BHWD (VRSHRS),
    ITYPE_SIMD_QD_BHWD (VRSHRU),
    ITYPE_SIMD_QD_BHWD (VRSRAS),
    ITYPE_SIMD_QD_BHWD (VRSRAU),
    ITYPE_SIMD_QD_BHWD (VSRI),
    ITYPE_SIMD_QD_BHWD (VSHL),
    ITYPE_SIMD_QD_BHWD (VSLI),
    ITYPE_SIMD_QD_BHWD (VQSHLUS),
    ITYPE_SIMD_QD_BHWD (VQSHLSS),
    ITYPE_SIMD_QD_BHWD (VQSHLSU),
    ITYPE_SIMD_HWD     (VSHRN),
    ITYPE_SIMD_HWD     (VRSHRN),
    ITYPE_SIMD_HWD     (VQSHRUNS),
    ITYPE_SIMD_HWD     (VQRSHRUNS),
    ITYPE_SIMD_HWD     (VQSHRNS),
    ITYPE_SIMD_HWD     (VQSHRNU),
    ITYPE_SIMD_HWD     (VQRSHRNS),
    ITYPE_SIMD_HWD     (VQRSHRNU),
    ITYPE_SIMD_BHW     (VSHLLS),
    ITYPE_SIMD_BHW     (VSHLLU),
    ITYPE_SIMD_BHW     (VMOVLS),
    ITYPE_SIMD_BHW     (VMOVLU),
    ITYPE_SIMD_QD      (VCVTFXS),
    ITYPE_SIMD_QD      (VCVTFXU),
    ITYPE_SIMD_QD      (VCVTXFS),
    ITYPE_SIMD_QD      (VCVTXFU),

    // SIMD data processing instructions - Two registers, Miscellaneous
    ITYPE_SIMD_VREV    (VREV),
    ITYPE_SIMD_QD_BHW  (VPADDLS),
    ITYPE_SIMD_QD_BHW  (VPADDLU),
    ITYPE_SIMD_QD_BHW  (VCLS),
    ITYPE_SIMD_QD_BHW  (VCLZ),
    ITYPE_SIMD_QD      (VCNT),
    ITYPE_SIMD_QD      (VMVN),
    ITYPE_SIMD_QD_BHW  (VPADALS),
    ITYPE_SIMD_QD_BHW  (VPADALU),
    ITYPE_SIMD_QD_BHW  (VQABS),
    ITYPE_SIMD_QD_BHW  (VQNEG),
    ITYPE_SIMD_QD_BHW  (VCGT0),
    ITYPE_SIMD_QD_BHW  (VCGE0),
    ITYPE_SIMD_QD_BHW  (VCEQ0),
    ITYPE_SIMD_QD_BHW  (VCLE0),
    ITYPE_SIMD_QD_BHW  (VCLT0),
    ITYPE_SIMD_QD_BHW  (VABS),
    ITYPE_SIMD_QD_BHW  (VNEG),
    ITYPE_SIMD_QD      (VCGT0_F),
    ITYPE_SIMD_QD      (VCGE0_F),
    ITYPE_SIMD_QD      (VCEQ0_F),
    ITYPE_SIMD_QD      (VCLE0_F),
    ITYPE_SIMD_QD      (VCLT0_F),
    ITYPE_SIMD_QD      (VABS_F),
    ITYPE_SIMD_QD      (VNEG_F),
    ITYPE_SIMD_QD      (VSWP),
    ITYPE_SIMD_QD_BHW  (VTRN),
    ITYPE_SIMD_Q_BHW_D_BH(VUZP),
    ITYPE_SIMD_Q_BHW_D_BH(VZIP),
    ITYPE_SIMD_HWD     (VMOVN),
    ITYPE_SIMD_HWD     (VQMOVUNS),
    ITYPE_SIMD_HWD     (VQMOVNS),
    ITYPE_SIMD_HWD     (VQMOVNU),
    ITYPE_SIMD_BHW     (VSHLLM),
    ITYPE_SINGLE       (VCVTHS),
    ITYPE_SINGLE       (VCVTSH),
    ITYPE_SIMD_QD      (VRECPE),
    ITYPE_SIMD_QD      (VRECPE_F),
    ITYPE_SIMD_QD      (VRSQRTE),
    ITYPE_SIMD_QD      (VRSQRTE_F),
    ITYPE_SIMD_QD      (VCVTFS),
    ITYPE_SIMD_QD      (VCVTFU),
    ITYPE_SIMD_QD      (VCVTSF),
    ITYPE_SIMD_QD      (VCVTUF),

    // SIMD data processing instructions - One register and a modified immediate
    ITYPE_SIMD_QD      (VMOVI_W),
    ITYPE_SIMD_QD      (VORRI_W),
    ITYPE_SIMD_QD      (VMOVI_H),
    ITYPE_SIMD_QD      (VORRI_H),
    ITYPE_SIMD_QD      (VMOVI1_W),
    ITYPE_SIMD_QD      (VMOVI_B),
    ITYPE_SIMD_QD      (VMOVI_F_W),
    ITYPE_SIMD_QD      (VMVNI_W),
    ITYPE_SIMD_QD      (VBICI_W),
    ITYPE_SIMD_QD      (VMVNI_H),
    ITYPE_SIMD_QD      (VBICI_H),
    ITYPE_SIMD_QD      (VMVNI1_W),
    ITYPE_SIMD_QD      (VMOVI_D),

    // VFP data processing instructions w/ 3 regs same length and D/S versions
    ITYPE_VFP_DS (VMLA_VFP),
    ITYPE_VFP_DS (VMLS_VFP),
    ITYPE_VFP_DS (VNMLS_VFP),
    ITYPE_VFP_DS (VNMLA_VFP),
    ITYPE_VFP_DS (VMUL_VFP),
    ITYPE_VFP_DS (VNMUL_VFP),
    ITYPE_VFP_DS (VADD_VFP),
    ITYPE_VFP_DS (VSUB_VFP),
    ITYPE_VFP_DS (VDIV_VFP),
    ITYPE_VFP_DS (VFMA_VFP),
    ITYPE_VFP_DS (VFMS_VFP),
    ITYPE_VFP_DS (VFNMA_VFP),
    ITYPE_VFP_DS (VFNMS_VFP),

    // VFP data processing instructions - other
    ITYPE_VFP_DS (VMOVI_VFP),
    ITYPE_VFP_DS (VMOVR_VFP),
    ITYPE_VFP_DS (VABS_VFP),
    ITYPE_VFP_DS (VNEG_VFP),
    ITYPE_VFP_DS (VSQRT_VFP),
    ITYPE_SINGLE (VCVTBFH_VFP),
    ITYPE_SINGLE (VCVTTFH_VFP),
    ITYPE_SINGLE (VCVTBHF_VFP),
    ITYPE_SINGLE (VCVTTHF_VFP),
    ITYPE_VFP_DS (VCMP_VFP),
    ITYPE_VFP_DS (VCMPE_VFP),
    ITYPE_VFP_DS (VCMP0_VFP),
    ITYPE_VFP_DS (VCMPE0_VFP),
    ITYPE_VFP_DS (VCVT_VFP),
    ITYPE_VFP_DS (VCVTFU_VFP),
    ITYPE_VFP_DS (VCVTFS_VFP),
    ITYPE_VFP_DS (VCVTFXUH_VFP),
    ITYPE_VFP_DS (VCVTFXUW_VFP),
    ITYPE_VFP_DS (VCVTFXSH_VFP),
    ITYPE_VFP_DS (VCVTFXSW_VFP),
    ITYPE_VFP_DS (VCVTUF_VFP),
    ITYPE_VFP_DS (VCVTRUF_VFP),
    ITYPE_VFP_DS (VCVTSF_VFP),
    ITYPE_VFP_DS (VCVTRSF_VFP),
    ITYPE_VFP_DS (VCVTXFSH_VFP),
    ITYPE_VFP_DS (VCVTXFSW_VFP),
    ITYPE_VFP_DS (VCVTXFUH_VFP),
    ITYPE_VFP_DS (VCVTXFUW_VFP),

    // Extension register load/store instructions
    ITYPE_VFP_DS (VSTMIA),
    ITYPE_VFP_DS (VSTMIAW),
    ITYPE_VFP_DS (VSTR),
    ITYPE_VFP_DS (VSTMDBW),
    ITYPE_VFP_DS (VPUSH),
    ITYPE_VFP_DS (VLDMIA),
    ITYPE_VFP_DS (VLDMIAW),
    ITYPE_VFP_DS (VPOP),
    ITYPE_VFP_DS (VLDR),
    ITYPE_VFP_DS (VLDMDBW),

    // SIMD element or structure load/store instructions
    ITYPE_SIMD_BHWD (VSTN1_R4),
    ITYPE_SIMD_BHWD (VSTN1_R3),
    ITYPE_SIMD_BHWD (VSTN1_R2),
    ITYPE_SIMD_BHWD (VSTN1_R1),
    ITYPE_SIMD_BHW  (VSTN2_R2I2),
    ITYPE_SIMD_BHW  (VSTN2_R1I2),
    ITYPE_SIMD_BHW  (VSTN2_R1I1),
    ITYPE_SIMD_BHW  (VSTN3_R3I1),
    ITYPE_SIMD_BHW  (VSTN3_R3I2),
    ITYPE_SIMD_BHW  (VSTN4_R4I1),
    ITYPE_SIMD_BHW  (VSTN4_R4I2),

    ITYPE_SIMD_BHWD (VLDN1_R4),
    ITYPE_SIMD_BHWD (VLDN1_R3),
    ITYPE_SIMD_BHWD (VLDN1_R2),
    ITYPE_SIMD_BHWD (VLDN1_R1),
    ITYPE_SIMD_BHW  (VLDN2_R2I2),
    ITYPE_SIMD_BHW  (VLDN2_R1I2),
    ITYPE_SIMD_BHW  (VLDN2_R1I1),
    ITYPE_SIMD_BHW  (VLDN3_R3I1),
    ITYPE_SIMD_BHW  (VLDN3_R3I2),
    ITYPE_SIMD_BHW  (VLDN4_R4I1),
    ITYPE_SIMD_BHW  (VLDN4_R4I2),

    ITYPE_SIMD_BHW  (VST1Z1),
    ITYPE_SIMD_BHW  (VST2Z1),
    ITYPE_SIMD_BHW  (VST3Z1),
    ITYPE_SIMD_BHW  (VST4Z1),

    ITYPE_SIMD_BHW  (VLD1Z1),
    ITYPE_SIMD_BHW  (VLD1ZA),
    ITYPE_SIMD_BHW  (VLD2Z1),
    ITYPE_SIMD_BHW  (VLD2ZA),
    ITYPE_SIMD_BHW  (VLD3Z1),
    ITYPE_SIMD_BHW  (VLD3ZA),
    ITYPE_SIMD_BHW  (VLD4Z1),
    ITYPE_SIMD_BHW  (VLD4ZA),
    ITYPE_SINGLE    (VLD4ZA_W1),

    // 8, 16 and 32-bit transfer instructions between ARM core regs and extension regs
    ITYPE_SINGLE       (VMRS),
    ITYPE_SINGLE       (VMSR),
    ITYPE_SINGLE       (VMOVRS),
    ITYPE_SINGLE       (VMOVSR),
    ITYPE_SIMD_BHW     (VMOVZR),
    ITYPE_SINGLE       (VMOVRZ_W),    // MOVRZ is unusual - no sign on the 32 bit version
    ITYPE_SIMD_BH      (VMOVRZU),
    ITYPE_SIMD_BH      (VMOVRZS),
    ITYPE_SIMD_QD_BHW  (VDUPR),

    // 64-bit transfer instructions between ARM core regs and extension regs
    ITYPE_SINGLE (VMOVRRD),
    ITYPE_SINGLE (VMOVDRR),
    ITYPE_SINGLE (VMOVRRSS),
    ITYPE_SINGLE (VMOVSSRR),

    // KEEP LAST
    ITYPE_SINGLE (LAST)

} armInstructionType;

//
// Condition code enumeration
//
typedef enum armConditionE {

    ARM_C_EQ,       // ZF==1
    ARM_C_NE,       // ZF==0
    ARM_C_CS,       // CF==1
    ARM_C_CC,       // CF==0
    ARM_C_MI,       // NF==1
    ARM_C_PL,       // NF==0
    ARM_C_VS,       // VF==1
    ARM_C_VC,       // VF==0
    ARM_C_HI,       // (CF==1) && (ZF==0)
    ARM_C_LS,       // (CF==0) || (ZF==1)
    ARM_C_GE,       // NF==VF
    ARM_C_LT,       // NF!=VF
    ARM_C_GT,       // (ZF==0) && (NF==VF)
    ARM_C_LE,       // (ZF==1) || (NF!=VF)
    ARM_C_AL,       // always
    ARM_C_NV,       // never

    // KEEP LAST
    ARM_C_LAST

} armCondition;

//
// This defines whether the instruction sets flags
//
typedef enum armSetFlagsE {
    ARM_SF_0,       // don't set flags
    ARM_SF_V,       // set flags, show in disassembly using "s" suffix
    ARM_SF_I,       // set flags, not shown in instruction disassembly
    ARM_SF_IT,      // only when not in if-then block
} armSetFlags;

//
// This defines shift operations
//
typedef enum armShiftOpE {
    ARM_SO_NA,      // no shift operation
    ARM_SO_LSL,     // logical shift left
    ARM_SO_LSR,     // logical shift right
    ARM_SO_ASR,     // arithmetic shift right
    ARM_SO_ROR,     // rotate right
    ARM_SO_RRX      // rotate right with extend
} armShiftOp;

//
// This defines increment/decrement actions
//
typedef enum armIncDecE {
    ARM_ID_NA  = 0x0,                               // no increment/decrement spec present
    ARM_ID_D   = 0x0,                               // decrement
    ARM_ID_I   = 0x1,                               // increment
    ARM_ID_A   = 0x0,                               // after
    ARM_ID_B   = 0x2,                               // before
    ARM_ID_NS  = 0x4,                               // not shown in disassembly
    ARM_ID_P   = 0x8,                               // increment/decrement spec present
    ARM_ID_DA  = ARM_ID_P | ARM_ID_D  | ARM_ID_A,   // decrement after
    ARM_ID_IA  = ARM_ID_P | ARM_ID_I  | ARM_ID_A,   // increment after
    ARM_ID_DB  = ARM_ID_P | ARM_ID_D  | ARM_ID_B,   // decrement before
    ARM_ID_IB  = ARM_ID_P | ARM_ID_I  | ARM_ID_B,   // increment before
    ARM_ID_IAI = ARM_ID_IA | ARM_ID_NS,             // IA, not shown in disassembly
    ARM_ID_DBI = ARM_ID_DB | ARM_ID_NS              // DB, not shown in disassembly
} armIncDec;

//
// This defines bits in a field mask
//
typedef enum armSRFieldMaskE {
    ARM_SR_C = 0x1, // control field mask bit
    ARM_SR_X = 0x2, // extension field mask bit
    ARM_SR_S = 0x4, // status field mask bit
    ARM_SR_F = 0x8  // flags field mask bit
} armSRFieldMask;

//
// This defines actions to be taken for unaligned memory accesses
//
typedef enum armUnalignedActionE {
    ARM_UA_DABORT,      // take data abort exception
    ARM_UA_ROTATE,      // rotate if unaligned (some ARMv4 and ARMv5 reads)
    ARM_UA_ALIGN,       // force alignment
    ARM_UA_UNALIGNED,   // allow unaligned access
} armUnalignedAction;

//
// This specifies the effect on any interrupt flags of this instruction
//
typedef enum armFlagActionE {
    ARM_FACT_NA  = 0,    // no flag action
    ARM_FACT_BAD = 1,    // (illegal value)
    ARM_FACT_IE  = 2,    // interrupts enabled
    ARM_FACT_ID  = 3     // interrupts disabled
} armFlagAction;

//
// This specifies flags affected by this instruction
//
typedef enum armFlagAffectE {
    ARM_FAFF_NA = 0x0,  // no flags affected
    ARM_FAFF_F  = 0x1,  // F flag affected
    ARM_FAFF_I  = 0x2,  // I flag affected
    ARM_FAFF_A  = 0x4,  // A flag affected
} armFlagAffect;

//
// This specifies SIMD/VFP extension system register ids
//
typedef enum armVESRIdE {
    ARM_VESR_FPSID = 0,
    ARM_VESR_FPSCR = 1,
    ARM_VESR_MVFR1 = 6,
    ARM_VESR_MVFR0 = 7,
    ARM_VESR_FPEXC = 8
} armVESRId;

//
// This specifies SIMD/VFP type
//
typedef enum armSDFPTypeE {
    ARM_SDFPT_NA,   // no SIMD/VFP type
    ARM_SDFPT_8,    // 8 bit value - type not specified
    ARM_SDFPT_16,   // 16 bit value - type not specified
    ARM_SDFPT_32,   // 32 bit value - type not specified
    ARM_SDFPT_64,   // 64 bit value - type not specified
    ARM_SDFPT_F16,  // floating point 16 bit value
    ARM_SDFPT_F32,  // floating point 32 bit value
    ARM_SDFPT_F64,  // floating point 64 bit value
    ARM_SDFPT_I8,   // integer 8 bit value
    ARM_SDFPT_I16,  // integer 16 bit value
    ARM_SDFPT_I32,  // integer 32 bit value
    ARM_SDFPT_I64,  // integer 64 bit value
    ARM_SDFPT_P8,   // polynomial 8 bit value
    ARM_SDFPT_S8,   // signed 8 bit value
    ARM_SDFPT_S16,  // signed 16 bit value
    ARM_SDFPT_S32,  // signed 32 bit value
    ARM_SDFPT_S64,  // signed 64 bit value
    ARM_SDFPT_U8,   // unsigned 8 bit value
    ARM_SDFPT_U16,  // unsigned 16 bit value
    ARM_SDFPT_U32,  // unsigned 32 bit value
    ARM_SDFPT_U64,  // unsigned 64 bit value
} armSDFPType;

//
// Type to hold a modified immediate constant value for SIMD and VFP instructions
//
typedef union armSdfpMItypeU {
    Uns64   u64;
    Flt64   f64;
    Flt32   f32;
    struct {
        Uns32 w0;
        Uns32 w1;
    } u32;
    struct {
        Uns16 h0;
        Uns16 h1;
        Uns16 h2;
        Uns16 h3;
    } u16;
    struct {
        Uns8 b0;
        Uns8 b1;
        Uns8 b2;
        Uns8 b3;
        Uns8 b4;
        Uns8 b5;
        Uns8 b6;
        Uns8 b7;
    } u8;
} armSdfpMItype;

//
// This specifies instruction support implied by ISAR registers
//
typedef enum armISARSupportE {
    ARM_ISAR_NA = 0,    // no ISAR restriction
    ARM_ISAR_DIV,       // SDIV/UDIV support
    ARM_ISAR_BKPT,      // BKPT support
    ARM_ISAR_CBZ,       // CBZ/CBNZ support
    ARM_ISAR_BFC,       // BFC/BFI/SBFX/UBFX support
    ARM_ISAR_CLZ,       // CLZ support
    ARM_ISAR_SWP,       // SWP/SWPB support
    ARM_ISAR_BXJ,       // BXJ support
    ARM_ISAR_BX,        // BX support
    ARM_ISAR_BLX,       // BX support
    ARM_ISAR_BAA,       // MURAC BAA support
    ARM_ISAR_MOVT,      // MOVT/MOV(16)/ADD(12) etc support
    ARM_ISAR_IT,        // IT support
    ARM_ISAR_SXTB,      // SXTB/SXTH/UXTB/UXTH support
    ARM_ISAR_SXTAB,     // SXTAB/SXTAH/UXTAB/UXTAH support
    ARM_ISAR_SXTB16,    // SXTB16/SXTAB16/UXTB16/UXTAB16 support
    ARM_ISAR_SRS,       // SRS/RFE and A/R profile CPS
    ARM_ISAR_LDM_UR,    // user mode LDM/STM, exception return LDM
    ARM_ISAR_SETEND,    // SETEND support
    ARM_ISAR_REV,       // REV/REV16/REVSH support
    ARM_ISAR_RBIT,      // RBIT support
    ARM_ISAR_MRS_AR,    // A/R profile MRS/MSR and exception return support
    ARM_ISAR_UMULL,     // UMULL/UMLAL support
    ARM_ISAR_UMAAL,     // UMAAL support
    ARM_ISAR_SMULL,     // SMULL/SMLAL support
    ARM_ISAR_SMLABB,    // SMLABB/SMLABT ... SMULWB/SMULWT support
    ARM_ISAR_SMLAD,     // SMLAD/SMLADX ... SMUSD/SMUSDX support
    ARM_ISAR_MLA,       // MLA support
    ARM_ISAR_MLS,       // MLS support
    ARM_ISAR_PLD,       // PLD support
    ARM_ISAR_PLDW,      // PLDW support
    ARM_ISAR_PLI,       // PLI support
    ARM_ISAR_LDRD,      // LDRD/STRD support
    ARM_ISAR_THUMBEE,   // ThumbEE support
    ARM_ISAR_NOP,       // NOP support
    ARM_ISAR_MOVLL,     // Thumb MOV low->low support
    ARM_ISAR_TBB,       // TBB/TBH support
    ARM_ISAR_LDREX,     // LDREX/STREX support
    ARM_ISAR_CLREX,     // CLREX/LDREXB/LDREXH/STREXB/STREXH support
    ARM_ISAR_LDREXD,    // LDREXD/STREXD support
    ARM_ISAR_SVC,       // SVC support
    ARM_ISAR_SSAT,      // SSAT/USAT support
    ARM_ISAR_PKHBT,     // PKHBT/PKHTB ... USUB8/USAX support
    ARM_ISAR_QADD,      // QADD/QDADD/QDSUB/QSUB support
    ARM_ISAR_MRS_M,     // M profile CPS/MRS/MSR support
    ARM_ISAR_DMB,       // DMB/DSB/ISB support
    ARM_ISAR_LDRBT,     // LDRBT/LDRT/STRBT/STRT support
    ARM_ISAR_LDRHT,     // LDRHT/LDRSBT/LDRSHT/STRHT support
    ARM_ISAR_VMRS,      // load, store, cp moves to SIMD regs supported (VFP2,VFP3,SIMD)
    ARM_ISAR_VFPSQRT,   // VFP VSQRT support
    ARM_ISAR_VFPDIV,    // VFP VDIV support
    ARM_ISAR_VFPV3,     // VFP v3-only support (also check double/single precision support)
    ARM_ISAR_VFPV2,     // VFP v2 and later support (also check double/single precision support)
    ARM_ISAR_VFPCVT3,   // VFP v3-only conversion support
    ARM_ISAR_VFPCVT2,   // VFP v2 and later conversion support
    ARM_ISAR_VFPHP,     // VFP half precision convert support
    ARM_ISAR_SIMDHP,    // SIMD half precision convert support
    ARM_ISAR_SIMDSP,    // SIMD single precision FP support
    ARM_ISAR_SIMDINT,   // SIMD integer instruction support
    ARM_ISAR_SIMDLDST,  // SIMD LD/ST instruction support
} armISARSupport;

//
// This structure is filled with information extracted from the decoded
// instruction
//
typedef struct armInstructionInfoS {
    const char        *opcode;      // opcode name
    const char        *format;      // disassembly format string
    armArchitecture    support;     // variants on which instruction supported
    armISARSupport     isar;        // ISAR instruction support
    Uns32              thisPC;      // instruction address
    Uns32              instruction; // instruction word
    armInstructionType type;        // instruction type
    armCondition       cond;        // condition under which instruction executes
    armSetFlags        f;           // set flags?
    armShiftOp         so;          // shifter operation to apply to arg 2
    armIncDec          incDec;      // increment/decrement action
    Uns32              c;           // constant value
    Uns32              t;           // target address
    Uns32              rList;       // register list
    Uns8               crotate;     // constant rotation from instruction
    Uns8               bytes;       // instruction size in bytes (1, 2 or 4)
    Uns8               r1;          // register 1
    Uns8               r2;          // register 2
    Uns8               r3;          // register 3
    Uns8               r4;          // register 4
    Uns8               sz;          // load/store size
    Int8               w;           // bit operation width
    Uns8               cpNum;       // coprocessor number
    Uns8               cpOp1;       // coprocessor opcode1
    Uns8               cpOp2;       // coprocessor opcode2
    Uns8               handler;     // ThumbEE handler
    Bool               xs;          // sign extend?
    Bool               tl;          // translate?
    Bool               pi;          // post-indexed?
    Bool               wb;          // instruction specifies writeback?
    Bool               u;           // instruction U bit set?
    Bool               ll;          // instruction specifies long load?
    Bool               ea;          // load/store is exclusive access?
    armUnalignedAction ua;          // unaligned action
    Bool               ma;          // mode action (CPS)
    armFlagAction      fact;        // flag action (CPS)
    armFlagAffect      faff;        // flags affected (CPS)
    Uns8               it;          // IT block specification
    armSDFPType        dt1;         // SIMD/VFP first data type
    armSDFPType        dt2;         // SIMD/VFP second data type
    armSdfpMItype      sdfpMI;      // SIMD/VFP modified immediate constant value
    Uns8               index;       // index for simd scalars
    Uns8               align;       // alignment for simd element/structure instructions
    Uns8               nregs;       // number of registers for simd element/structure instructions
    Uns8               nels;        // number of elements in SIMD element load/store instruction
    Uns8               incr;        // register increment value for simd element/structure instructions
} armInstructionInfo;

#endif

