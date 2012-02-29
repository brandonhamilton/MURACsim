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
#include "vmi/vmiMessage.h"

// model header files
#include "armEmit.h"
#include "armMorph.h"
#include "armMorphEntries.h"
#include "armMorphFunctions.h"
#include "armRegisters.h"
#include "armStructure.h"
#include "armUtils.h"


////////////////////////////////////////////////////////////////////////////////
// FLAGS
////////////////////////////////////////////////////////////////////////////////

const static vmiFlags flagsZNCV = {
    ARM_CF_CONST,
    {
        [vmi_CF] = ARM_CF_CONST,
        [vmi_PF] = VMI_NOFLAG_CONST,
        [vmi_ZF] = ARM_ZF_CONST,
        [vmi_SF] = ARM_NF_CONST,
        [vmi_OF] = ARM_VF_CONST
    }
};

const static vmiFlags flagsZNBV = {
    ARM_CF_CONST,
    {
        [vmi_CF] = ARM_CF_CONST,
        [vmi_PF] = VMI_NOFLAG_CONST,
        [vmi_ZF] = ARM_ZF_CONST,
        [vmi_SF] = ARM_NF_CONST,
        [vmi_OF] = ARM_VF_CONST
    },
    vmi_FN_CF_IN|vmi_FN_CF_OUT
};

const static vmiFlags flagsZN = {
    ARM_CF_CONST,
    {
        [vmi_CF] = VMI_NOFLAG_CONST,
        [vmi_PF] = VMI_NOFLAG_CONST,
        [vmi_ZF] = ARM_ZF_CONST,
        [vmi_SF] = ARM_NF_CONST,
        [vmi_OF] = VMI_NOFLAG_CONST
    }
};

const static vmiFlags flagsCIn = {
    ARM_CF_CONST,
    {
        [vmi_CF] = VMI_NOFLAG_CONST,
        [vmi_PF] = VMI_NOFLAG_CONST,
        [vmi_ZF] = VMI_NOFLAG_CONST,
        [vmi_SF] = VMI_NOFLAG_CONST,
        [vmi_OF] = VMI_NOFLAG_CONST
    }
};

const static vmiFlags flagsBIn = {
    ARM_CF_CONST,
    {
        [vmi_CF] = VMI_NOFLAG_CONST,
        [vmi_PF] = VMI_NOFLAG_CONST,
        [vmi_ZF] = VMI_NOFLAG_CONST,
        [vmi_SF] = VMI_NOFLAG_CONST,
        [vmi_OF] = VMI_NOFLAG_CONST
    },
    vmi_FN_CF_IN
};

// Macro accessors for flags
#define FLAGS_ZNCV &flagsZNCV
#define FLAGS_ZNBV &flagsZNBV
#define FLAGS_ZN   &flagsZN
#define FLAGS_CIN  &flagsCIn
#define FLAGS_BIN  &flagsBIn


////////////////////////////////////////////////////////////////////////////////
// MORPHER DISPATCH TABLE
////////////////////////////////////////////////////////////////////////////////

const armMorphAttr armMorphTable[ARM_IT_LAST+1] = {

    ////////////////////////////////////////////////////////////////////////////
    // ARM INSTRUCTIONS
    ////////////////////////////////////////////////////////////////////////////

    // data processing instructions
    MORPH_SET_ADC (ADC,  vmi_ADC,  FLAGS_ZNCV, FLAGS_CIN, False),
    MORPH_SET_ADC (ADD,  vmi_ADD,  FLAGS_ZNCV, FLAGS_CIN, False),
    MORPH_SET_ADC (AND,  vmi_AND,  FLAGS_ZN,   0,         True ),
    MORPH_SET_ADC (BIC,  vmi_ANDN, FLAGS_ZN,   0,         True ),
    MORPH_SET_ADC (EOR,  vmi_XOR,  FLAGS_ZN,   0,         True ),
    MORPH_SET_MOV (MOV,  vmi_MOV,  FLAGS_ZN,   0,         True ),
    MORPH_SET_ADC (MUL,  vmi_MUL,  FLAGS_ZN,   0,         True ),
    MORPH_SET_MOV (MVN,  vmi_NOT,  FLAGS_ZN,   0,         True ),
    MORPH_SET_MOV (NEG,  vmi_NEG,  FLAGS_ZNBV, 0,         False),
    MORPH_SET_ADC (ORN,  vmi_ORN,  FLAGS_ZN,   0,         True ),
    MORPH_SET_ADC (ORR,  vmi_OR,   FLAGS_ZN,   0,         True ),
    MORPH_SET_ADC (RSB,  vmi_RSUB, FLAGS_ZNBV, FLAGS_BIN, False),
    MORPH_SET_ADC (RSC,  vmi_RSBB, FLAGS_ZNBV, FLAGS_BIN, False),
    MORPH_SET_ADC (SBC,  vmi_SBB,  FLAGS_ZNBV, FLAGS_BIN, False),
    MORPH_SET_ADC (SUB,  vmi_SUB,  FLAGS_ZNBV, FLAGS_BIN, False),

    // ARMv6T2 move instructions
    MORPH_SINGLE (MOVT),
    MORPH_SINGLE (MOVW),

    // multiply instructions
    MORPH_SET_MLA   (MLA,                   FLAGS_ZN),
    MORPH_SET_MLA   (MLS,                   FLAGS_ZN),
    MORPH_SET_MLA   (MUL,                   FLAGS_ZN),
    MORPH_SET_SMULL (SMLAL, MLAL, vmi_IMUL, FLAGS_ZN),
    MORPH_SET_SMULL (SMULL, MULL, vmi_IMUL, FLAGS_ZN),
    MORPH_SET_SMULL (UMAAL, MAAL, vmi_MUL,  FLAGS_ZN),
    MORPH_SET_SMULL (UMLAL, MLAL, vmi_MUL,  FLAGS_ZN),
    MORPH_SET_SMULL (UMULL, MULL, vmi_MUL,  FLAGS_ZN),

    // compare instructions
    MORPH_SET_CMN (CMN, vmi_ADD, FLAGS_ZNCV, FLAGS_CIN, False),
    MORPH_SET_CMN (CMP, vmi_SUB, FLAGS_ZNBV, FLAGS_BIN, False),
    MORPH_SET_CMN (TEQ, vmi_XOR, FLAGS_ZN,   0,         True ),
    MORPH_SET_CMN (TST, vmi_AND, FLAGS_ZN,   0,         True ),

    // branch instructions
    MORPH_SET_B    (B,    False),
    MORPH_SET_B    (BL,   True ),
    MORPH_SET_BLX1 (BLX1, True ),
    MORPH_SET_BLX2 (BLX2, True ),
    MORPH_SET_BLX2 (BX,   False),
    MORPH_SET_BLX2 (BXJ,  False),

    // MURAC instructions
    MORPH_SET_BAA (BAA,  True),

    // miscellaneous instructions
    MORPH_SINGLE (BKPT),
    MORPH_SINGLE (CLZ ),
    MORPH_SINGLE (SWI ),

    // load and store instructions
    MORPH_SET_LDR  (LDR,   LDR ),
    MORPH_SET_LDR  (LDRB,  LDR ),
    MORPH_SET_LDR  (LDRBT, LDR ),
    MORPH_SET_LDRH (LDRH,  LDR ),
    MORPH_SET_LDRH (LDRSB, LDR ),
    MORPH_SET_LDRH (LDRSH, LDR ),
    MORPH_SET_LDR  (LDRT,  LDR ),
    MORPH_SET_LDR  (STR,   STR ),
    MORPH_SET_LDR  (STRB,  STR ),
    MORPH_SET_LDR  (STRBT, STR ),
    MORPH_SET_LDRH (STRH,  STR ),
    MORPH_SET_LDR  (STRT,  STR ),

    // load and store multiple instructions
    MORPH_SET_LDM1 (LDM1),
    MORPH_SINGLE   (LDM2),
    MORPH_SINGLE   (LDM3),
    MORPH_SINGLE   (STM1),
    MORPH_SINGLE   (STM2),

    // ARMv6T2 load and store instructions
    MORPH_SET_LDRH (LDRHT,  LDR),
    MORPH_SET_LDRH (LDRSBT, LDR),
    MORPH_SET_LDRH (LDRSHT, LDR),
    MORPH_SET_LDRH (STRHT,  STR),

    // semaphore instructions
    MORPH_SET_SWP (SWP,  SWP),
    MORPH_SET_SWP (SWPB, SWP),

    // synchronization primitives
    MORPH_SET_SWP (LDREX,  LDREX),
    MORPH_SET_SWP (LDREXB, LDREX),
    MORPH_SET_SWP (LDREXH, LDREX),
    MORPH_SET_SWP (LDREXD, LDREX),
    MORPH_SET_SWP (STREX,  STREX),
    MORPH_SET_SWP (STREXB, STREX),
    MORPH_SET_SWP (STREXH, STREX),
    MORPH_SET_SWP (STREXD, STREX),

    // coprocessor instructions (mostly undefined)
    MORPH_SET_SWP (CDP,  CDP),
    MORPH_SET_SWP (CDP2, CDP),
    MORPH_SET_LDC (LDC,  LDC),
    MORPH_SET_LDC (LDC2, LDC),
    MORPH_SET_SWP (MCR,  MCR),
    MORPH_SET_SWP (MCR2, MCR),
    MORPH_SET_SWP (MRC,  MRC),
    MORPH_SET_SWP (MRC2, MRC),
    MORPH_SET_LDC (STC,  STC),
    MORPH_SET_LDC (STC2, STC),

    // status register access instructions
    MORPH_SINGLE  (MRSC  ),
    MORPH_SINGLE  (MRSS  ),
    MORPH_SET_MSR (MSRC  ),
    MORPH_SET_MSR (MSRS  ),

    // hint instructions
    MORPH_SET_NOP (NOP  ),
    MORPH_SET_NOP (YIELD),
    MORPH_SINGLE  (WFE  ),
    MORPH_SINGLE  (WFI  ),
    MORPH_SINGLE  (SEV  ),
    MORPH_SET_NOP (DBG  ),

    // ARMv6 exception instructions
    MORPH_SINGLE (SRS),
    MORPH_SINGLE (RFE),

    // ARMv6 miscellaneous instructions
    MORPH_SINGLE (SETEND),
    MORPH_SINGLE (CPS   ),
    MORPH_SINGLE (CLREX ),

    // ARMv6/ARMv7 memory hint instructions
    MORPH_SET_PLD (PLD  ),
    MORPH_SET_PLD (PLI  ),
    MORPH_SET_PLD (PLDW ),
    MORPH_SET_NOP (DMB  ),
    MORPH_SET_NOP (DSB  ),
    MORPH_SET_NOP (ISB  ),
    MORPH_SET_NOP (UHINT),

    ////////////////////////////////////////////////////////////////////////////
    // DSP INSTRUCTIONS
    ////////////////////////////////////////////////////////////////////////////

    // data processing instructions
    MORPH_SINGLE (QADD ),
    MORPH_SINGLE (QDADD),
    MORPH_SINGLE (QDSUB),
    MORPH_SINGLE (QSUB ),

    // multiply instructions
    MORPH_SET_SMLA_XY (SMLA ),
    MORPH_SET_SMLA_XY (SMLAL),
    MORPH_SET_SMLAW_Y (SMLAW),
    MORPH_SET_SMLA_XY (SMUL ),
    MORPH_SET_SMLAW_Y (SMULW),

    // load and store instructions
    MORPH_SET_LDRH (LDRD, LDR),
    MORPH_SET_LDRH (STRD, STR),

    // coprocessor instructions
    MORPH_SET_SWP (MCRR,  MCRR),
    MORPH_SET_SWP (MCRR2, MCRR),
    MORPH_SET_SWP (MRRC,  MRRC),
    MORPH_SET_SWP (MRRC2, MRRC),

    ////////////////////////////////////////////////////////////////////////////
    // MEDIA INSTRUCTIONS
    ////////////////////////////////////////////////////////////////////////////

    // basic instructions
    MORPH_SINGLE (USAD8 ),
    MORPH_SINGLE (USADA8),
    MORPH_SINGLE (SBFX  ),
    MORPH_SINGLE (BFC   ),
    MORPH_SINGLE (BFI   ),
    MORPH_SINGLE (UBFX  ),

    // parallel add/subtract instructions
    MORPH_SET_PAS (S,  vmi_ADD,   vmi_SUB,   True,  True,  False),
    MORPH_SET_PAS (Q,  vmi_ADDSQ, vmi_SUBSQ, False, False, False),
    MORPH_SET_PAS (SH, vmi_ADD,   vmi_SUB,   True,  False, True ),
    MORPH_SET_PAS (U,  vmi_ADD,   vmi_SUB,   False, True,  False),
    MORPH_SET_PAS (UQ, vmi_ADDUQ, vmi_SUBUQ, False, False, False),
    MORPH_SET_PAS (UH, vmi_ADD,   vmi_SUB,   False, False, True ),

    // packing, unpacking, saturation and reversal instructions
    MORPH_SINGLE (PKHBT  ),
    MORPH_SINGLE (PKHTB  ),
    MORPH_SINGLE (SSAT   ),
    MORPH_SINGLE (SSAT16 ),
    MORPH_SINGLE (USAT   ),
    MORPH_SINGLE (USAT16 ),
    MORPH_SINGLE (SXTAB  ),
    MORPH_SINGLE (UXTAB  ),
    MORPH_SINGLE (SXTAB16),
    MORPH_SINGLE (UXTAB16),
    MORPH_SINGLE (SXTAH  ),
    MORPH_SINGLE (UXTAH  ),
    MORPH_SINGLE (SXTB   ),
    MORPH_SINGLE (UXTB   ),
    MORPH_SINGLE (SXTB16 ),
    MORPH_SINGLE (UXTB16 ),
    MORPH_SINGLE (SXTH   ),
    MORPH_SINGLE (UXTH   ),
    MORPH_SINGLE (SEL    ),
    MORPH_SINGLE (REV    ),
    MORPH_SINGLE (REV16  ),
    MORPH_SINGLE (RBIT   ),
    MORPH_SINGLE (REVSH  ),

    // signed multiply instructions
    MORPH_SET_MEDIA_X (SMLAD,  SMLXD,  vmi_ADD),
    MORPH_SET_MEDIA_X (SMUAD,  SMUXD,  vmi_ADD),
    MORPH_SET_MEDIA_X (SMLSD,  SMLXD,  vmi_SUB),
    MORPH_SET_MEDIA_X (SMUSD,  SMUXD,  vmi_SUB),
    MORPH_SET_MEDIA_X (SMLALD, SMLXLD, vmi_ADD),
    MORPH_SET_MEDIA_X (SMLSLD, SMLXLD, vmi_SUB),
    MORPH_SET_MEDIA_R (SMMLA,  SMMLX,  vmi_ADD,  True ),
    MORPH_SET_MEDIA_R (SMMUL,  SMMLX,  vmi_ADD,  False),
    MORPH_SET_MEDIA_R (SMMLS,  SMMLX,  vmi_RSUB, True ),

    ////////////////////////////////////////////////////////////////////////////
    // THUMB INSTRUCTIONS (WHEN DISTINCT FROM ARM INSTRUCTIONS)
    ////////////////////////////////////////////////////////////////////////////

    // data processing instructions
    MORPH_SET_ADD4 (ADD4, vmi_ADD, 0, 0, False),
    MORPH_SET_ADD6 (ADD6, vmi_ADD, 0, 0, False),
    MORPH_SET_ADD7 (ADD7, vmi_ADD, 0, 0, False),
    MORPH_SET_ADD7 (SUB4, vmi_SUB, 0, 0, False),
    MORPH_SET_MOV3 (MOV3, vmi_MOV, 0, 0, False),

    // address instructions
    MORPH_SET_ADR (ADD_ADR, False),
    MORPH_SET_ADR (SUB_ADR, True ),

    // branch instructions
    MORPH_SET_CBZ (CBNZ, False),
    MORPH_SET_CBZ (CBZ,  True ),
    MORPH_SINGLE  (TB),

    // divide instructions
    MORPH_SET_SDIV (SDIV, vmi_IDIV),
    MORPH_SET_SDIV (UDIV, vmi_DIV ),

    // 16-bit branch instructions
    MORPH_SINGLE (BL_H10),
    MORPH_SINGLE (BL_H11),
    MORPH_SINGLE (BL_H01),

    // ThumbEE instructions
    MORPH_SINGLE (UND_EE           ),
    MORPH_SET_HB (HB,   HB,   False),
    MORPH_SET_HB (HBL,  HB,   True ),
    MORPH_SET_HB (HBP,  HBP,  False),
    MORPH_SET_HB (HBLP, HBP,  True ),
    MORPH_SET_HB (CHKA, CHKA, True ),
    MORPH_SINGLE (ENTERX           ),
    MORPH_SINGLE (LEAVEX           ),

    ////////////////////////////////////////////////////////////////////////////
    // SIMD/VFP INSTRUCTIONS
    ////////////////////////////////////////////////////////////////////////////

    // SIMD data processing instructions - Misc
    MORPH_SET_SIMD_QD     (VEXT, VEXT),
    MORPH_SET_SINGLE_SIMD (VTBL),
    MORPH_SET_SINGLE_SIMD (VTBX),
    MORPH_SET_SIMD_QD_BHW (VDUPZ),

    // SIMD data processing instructions - 3 regs same length
    MORPH_SET_SIMD_I_QD_BHW   (VHADDS,    VBinop,     vmi_ADDSH, 1, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHW   (VHADDU,    VBinop,     vmi_ADDUH, 0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHWD  (VQADDS,    VBinop,     vmi_ADDSQ, 1, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHWD  (VQADDU,    VBinop,     vmi_ADDUQ, 0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHW   (VRHADDS,   VBinop,     vmi_ADDSHR,1, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHW   (VRHADDU,   VBinop,     vmi_ADDUHR,0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD       (VAND,      VBinop,     vmi_AND,   0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD       (VBIC,      VBinop,     vmi_ANDN,  0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD       (VORR,      VBinop,     vmi_OR,    0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD       (VORN,      VBinop,     vmi_ORN,   0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD       (VEOR,      VBinop,     vmi_XOR,   0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_QD         (VBIF,      VBIF),
    MORPH_SET_SIMD_QD         (VBIT,      VBIT),
    MORPH_SET_SIMD_QD         (VBSL,      VBSL),
    MORPH_SET_SIMD_I_QD_BHW   (VHSUBS,    VBinop,     vmi_SUBSH, 1, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHW   (VHSUBU,    VBinop,     vmi_SUBUH, 0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHWD  (VQSUBS,    VBinop,     vmi_SUBSQ, 1, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHWD  (VQSUBU,    VBinop,     vmi_SUBUQ, 0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_VCMP_QD_BHW     (VCGTU,     VCmpBool,   vmi_COND_NBE),
    MORPH_SET_VCMP_QD_BHW     (VCGTS,     VCmpBool,   vmi_COND_NLE),
    MORPH_SET_VCMP_QD_BHW     (VCGEU,     VCmpBool,   vmi_COND_NB),
    MORPH_SET_VCMP_QD_BHW     (VCGES,     VCmpBool,   vmi_COND_NL),
    MORPH_SET_SIMD_VSH_R      (VSHLU,     vmi_SHL,    vmi_SHR,   0, 0),
    MORPH_SET_SIMD_VSH_R      (VSHLS,     vmi_SHL,    vmi_SAR,   1, 0),
    MORPH_SET_SIMD_VSH_R      (VQSHLU,    vmi_SHLUQ,  vmi_SHR,   0, 1),
    MORPH_SET_SIMD_VSH_R      (VQSHLS,    vmi_SHLSQ,  vmi_SAR,   1, 1),
    MORPH_SET_SIMD_VSH_R      (VRSHLU,    vmi_SHL,    vmi_SHRR,  0, 0),
    MORPH_SET_SIMD_VSH_R      (VRSHLS,    vmi_SHL,    vmi_SARR,  1, 0),
    MORPH_SET_SIMD_VSH_R      (VQRSHLU,   vmi_SHLUQ,  vmi_SHRR,  0, 1),
    MORPH_SET_SIMD_VSH_R      (VQRSHLS,   vmi_SHLSQ,  vmi_SARR,  1, 1),
    MORPH_SET_VCMP_QD_BHW     (VMAXU,     VCmpReg,    vmi_COND_NB),
    MORPH_SET_VCMP_QD_BHW     (VMAXS,     VCmpReg,    vmi_COND_NL),
    MORPH_SET_VCMP_QD_BHW     (VMINU,     VCmpReg,    vmi_COND_B),
    MORPH_SET_VCMP_QD_BHW     (VMINS,     VCmpReg,    vmi_COND_L),
    MORPH_SET_SIMD_I_QD_BHW   (VABAU,     VAbsDiff,   vmi_ADD,   0, 0, 1, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHW   (VABAS,     VAbsDiff,   vmi_ADD,   1, 0, 1, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHW   (VABDU,     VAbsDiff,   vmi_ADD,   0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHW   (VABDS,     VAbsDiff,   vmi_ADD,   1, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHWD  (VADD,      VBinop,     vmi_ADD,   0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHWD  (VSUB,      VBinop,     vmi_SUB,   0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_VCMP_QD_BHW     (VTST,      VTst,       vmi_COND_Z),
    MORPH_SET_VCMP_QD_BHW     (VCEQ,      VCmpBool,   vmi_COND_EQ),
    MORPH_SET_SIMD_I_QD_BHW   (VMLA,      VMulAcc,    vmi_ADD,   0, 0, 1, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHW   (VMLS,      VMulAcc,    vmi_SUB,   0, 0, 1, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHW   (VMUL,      VBinop,     vmi_IMUL,  0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_QD         (VMUL_P,    VMUL_P),
    MORPH_SET_VCMP_BHW        (VPMAXU,    VPCmpReg,   vmi_COND_NB),
    MORPH_SET_VCMP_BHW        (VPMAXS,    VPCmpReg,   vmi_COND_NL),
    MORPH_SET_VCMP_BHW        (VPMINU,    VPCmpReg,   vmi_COND_B),
    MORPH_SET_VCMP_BHW        (VPMINS,    VPCmpReg,   vmi_COND_L),
    MORPH_SET_SIMD_I_QD_HW    (VQDMULH,   VQDMul,     vmi_ADDSQ, 1, 0, 0, 1, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_HW    (VQRDMULH,  VQDMul,     vmi_ADDSQ, 1, 1, 0, 1, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_BHW      (VPADD,     VPairOp,    vmi_ADD,   0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_F_QD       (VADD_F,    VBinop_F,   vmi_FADD),
    MORPH_SET_SIMD_F_QD       (VSUB_F,    VBinop_F,   vmi_FSUB),
    MORPH_SET_SIMD_F_D        (VPADD_F,   VPairOp_F,  vmi_FADD),
    MORPH_SET_SIMD_F_QD       (VMUL_F,    VBinop_F,   vmi_FMUL),
    MORPH_SET_SIMD_QD         (VABD_F,    VABD_F),
    MORPH_SET_SIMD_F_T_QD     (VMLA_F,    VTernop_F,  vmi_FMADD),
    MORPH_SET_SIMD_F_T_QD     (VMLS_F,    VTernop_F,  vmi_FMSUBR),
    MORPH_SET_VCMP_F_QD       (VCEQ_F,    VCmpBool_F, 1, vmi_FPRL_EQUAL),
    MORPH_SET_VCMP_F_QD       (VCGE_F,    VCmpBool_F, 0, vmi_FPRL_EQUAL | vmi_FPRL_GREATER),
    MORPH_SET_VCMP_F_QD       (VCGT_F,    VCmpBool_F, 0, vmi_FPRL_GREATER),
    MORPH_SET_VCMP_F_QD       (VACGE_F,   VAbsCmp_F,  0, vmi_FPRL_EQUAL | vmi_FPRL_GREATER),
    MORPH_SET_VCMP_F_QD       (VACGT_F,   VAbsCmp_F,  0, vmi_FPRL_GREATER),
    MORPH_SET_VCMP_F_QD       (VMAX_F,    VCmpReg_F,  1, vmi_FPRL_GREATER),
    MORPH_SET_VCMP_F_QD       (VMIN_F,    VCmpReg_F,  1, vmi_FPRL_LESS),
    MORPH_SET_VCMP_F_D        (VPMAX_F,   VPCmpReg_F, 1, vmi_FPRL_GREATER),
    MORPH_SET_VCMP_F_D        (VPMIN_F,   VPCmpReg_F, 1, vmi_FPRL_LESS),

    // SIMD data processing instructions - 3 regs different lengths
    MORPH_SET_SIMD_I_BHW      (VADDLU,    VBinop,     vmi_ADD,   0, 0, 0, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_BHW      (VADDLS,    VBinop,     vmi_ADD,   1, 0, 0, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_BHW      (VADDWU,    VBinop,     vmi_ADD,   0, 0, 0, 0, 0, 0, ASDS_WIDE),
    MORPH_SET_SIMD_I_BHW      (VADDWS,    VBinop,     vmi_ADD,   1, 0, 0, 0, 0, 0, ASDS_WIDE),
    MORPH_SET_SIMD_I_BHW      (VSUBLU,    VBinop,     vmi_SUB,   0, 0, 0, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_BHW      (VSUBLS,    VBinop,     vmi_SUB,   1, 0, 0, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_BHW      (VSUBWU,    VBinop,     vmi_SUB,   0, 0, 0, 0, 0, 0, ASDS_WIDE),
    MORPH_SET_SIMD_I_BHW      (VSUBWS,    VBinop,     vmi_SUB,   1, 0, 0, 0, 0, 0, ASDS_WIDE),
    MORPH_SET_SIMD_I_HWD      (VADDHN,    VBinop,     vmi_ADD,   0, 0, 0, 1, 0, 0, ASDS_NARROW),
    MORPH_SET_SIMD_I_HWD      (VRADDHN,   VBinop,     vmi_ADD,   0, 1, 0, 1, 0, 0, ASDS_NARROW),
    MORPH_SET_SIMD_I_BHW      (VABALU,    VAbsDiff,   vmi_ADD,   0, 0, 1, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_BHW      (VABALS,    VAbsDiff,   vmi_ADD,   1, 0, 1, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_HWD      (VSUBHN,    VBinop,     vmi_SUB,   0, 0, 0, 1, 0, 0, ASDS_NARROW),
    MORPH_SET_SIMD_I_HWD      (VRSUBHN,   VBinop,     vmi_SUB,   0, 1, 0, 1, 0, 0, ASDS_NARROW),
    MORPH_SET_SIMD_I_BHW      (VABDLU,    VAbsDiff,   vmi_ADD,   0, 0, 0, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_BHW      (VABDLS,    VAbsDiff,   vmi_ADD,   1, 0, 0, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_BHW      (VMULLU,    VBinop,     vmi_MUL,   0, 0, 0, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_BHW      (VMULLS,    VBinop,     vmi_IMUL,  1, 0, 0, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_BHW      (VMLALU,    VMulAcc,    vmi_ADD,   0, 0, 1, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_BHW      (VMLALS,    VMulAcc,    vmi_ADD,   1, 0, 1, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_BHW      (VMLSLU,    VMulAcc,    vmi_SUB,   0, 0, 1, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_BHW      (VMLSLS,    VMulAcc,    vmi_SUB,   1, 0, 1, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_HW       (VQDMLAL,   VQDMul,     vmi_ADDSQ, 1, 0, 1, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_HW       (VQDMLSL,   VQDMul,     vmi_SUBSQ, 1, 0, 1, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_B          (VMULL_P,   VMULL_P),
    MORPH_SET_SIMD_I_HW       (VQDMULL,   VQDMul,     vmi_ADDSQ, 1, 0, 0, 0, 0, 0, ASDS_LONG),

    // SIMD data processing instructions - Two Registers and a scalar
    MORPH_SET_SIMD_I_QD_HW    (VMLAZ,     VMulAccZ,   vmi_ADD,    0, 0, 1, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_HW    (VMLSZ,     VMulAccZ,   vmi_SUB,    0, 0, 1, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_HW       (VMLALZU,   VMulAccZ,   vmi_ADD,    0, 0, 1, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_HW       (VMLALZS,   VMulAccZ,   vmi_ADD,    1, 0, 1, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_HW       (VMLSLZU,   VMulAccZ,   vmi_SUB,    0, 0, 1, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_HW       (VMLSLZS,   VMulAccZ,   vmi_SUB,    1, 0, 1, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_HW       (VQDMLALZ,  VQDMulZ,    vmi_ADDSQ,  1, 0, 1, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_HW       (VQDMLSLZ,  VQDMulZ,    vmi_SUBSQ,  1, 0, 1, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_QD_HW    (VMULZ,     VBinopZ,    vmi_IMUL,   0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_HW       (VMULLZU,   VBinopZ,    vmi_IMUL,   0, 0, 0, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_HW       (VMULLZS,   VBinopZ,    vmi_IMUL,   1, 0, 0, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_HW       (VQDMULLZ,  VQDMulZ,    vmi_ADDSQ,  1, 0, 0, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_QD_HW    (VQDMULHZ,  VQDMulZ,    vmi_ADDSQ,  1, 0, 0, 1, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_HW    (VQRDMULHZ, VQDMulZ,    vmi_ADDSQ,  1, 1, 0, 1, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_F_QD       (VMULZ_F,   VBinopZ_F,  vmi_FMUL),
    MORPH_SET_SIMD_F_T_QD     (VMLAZ_F,   VTernopZ_F, vmi_FMADD),
    MORPH_SET_SIMD_F_T_QD     (VMLSZ_F,   VTernopZ_F, vmi_FMSUBR),

    // SIMD data processing instructions - Two Registers and a shift amount
    MORPH_SET_SIMD_I_QD_BHWD  (VSHRU,     VShiftImm,  vmi_SHR,    0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHWD  (VSHRS,     VShiftImm,  vmi_SAR,    1, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHWD  (VSRAU,     VShiftImm,  vmi_SHR,    0, 0, 1, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHWD  (VSRAS,     VShiftImm,  vmi_SAR,    1, 0, 1, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHWD  (VRSHRU,    VShiftImm,  vmi_SHRR,   0, 1, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHWD  (VRSHRS,    VShiftImm,  vmi_SARR,   1, 1, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHWD  (VRSRAU,    VShiftImm,  vmi_SHRR,   0, 1, 1, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHWD  (VRSRAS,    VShiftImm,  vmi_SARR,   1, 1, 1, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHWD  (VSRI,      VShiftIns,  vmi_SHR,    0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHWD  (VSHL,      VShiftImm,  vmi_SHL,    0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHWD  (VSLI,      VShiftIns,  vmi_SHL,    0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHWD  (VQSHLSU,   VShiftImm,  vmi_SHLUQ,  0, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHWD  (VQSHLSS,   VShiftImm,  vmi_SHLSQ,  1, 0, 0, 0, 0, 0, ASDS_NORMAL),
    MORPH_SET_SIMD_I_QD_BHWD  (VQSHLUS,   VShiftImm,  vmi_SHLUQ,  1, 0, 0, 0, 0, 1, ASDS_NORMAL),
    MORPH_SET_SIMD_I_HWD      (VSHRN,     VShiftImm,  vmi_SHR,    0, 0, 0, 0, 0, 0, ASDS_NARROW),
    MORPH_SET_SIMD_I_HWD      (VRSHRN,    VShiftImm,  vmi_SHRR,   0, 1, 0, 0, 0, 0, ASDS_NARROW),
    MORPH_SET_SIMD_I_HWD      (VQSHRNU,   VShiftImm,  vmi_SHR,    0, 0, 0, 0, 1, 0, ASDS_NARROW),
    MORPH_SET_SIMD_I_HWD      (VQSHRNS,   VShiftImm,  vmi_SAR,    1, 0, 0, 0, 1, 0, ASDS_NARROW),
    MORPH_SET_SIMD_I_HWD      (VQSHRUNS,  VShiftImm,  vmi_SHR,    1, 0, 0, 0, 1, 1, ASDS_NARROW),
    MORPH_SET_SIMD_I_HWD      (VQRSHRNU,  VShiftImm,  vmi_SHRR,   0, 1, 0, 0, 1, 0, ASDS_NARROW),
    MORPH_SET_SIMD_I_HWD      (VQRSHRNS,  VShiftImm,  vmi_SARR,   1, 1, 0, 0, 1, 0, ASDS_NARROW),
    MORPH_SET_SIMD_I_HWD      (VQRSHRUNS, VShiftImm,  vmi_SHRR,   1, 1, 0, 0, 1, 1, ASDS_NARROW),
    MORPH_SET_SIMD_I_BHW      (VSHLLU,    VShiftImm,  vmi_SHL,    0, 0, 0, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_BHW      (VSHLLS,    VShiftImm,  vmi_SHL,    1, 0, 0, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_BHW      (VMOVLU,    VShiftImm,  vmi_SHL,    0, 0, 0, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_BHW      (VMOVLS,    VShiftImm,  vmi_SHL,    1, 0, 0, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_VCVT       (VCVTFXU,   VCVT_FX_SIMD, 0),
    MORPH_SET_SIMD_VCVT       (VCVTFXS,   VCVT_FX_SIMD, 1),
    MORPH_SET_SIMD_VCVT       (VCVTXFU,   VCVT_XF_SIMD, 0),
    MORPH_SET_SIMD_VCVT       (VCVTXFS,   VCVT_XF_SIMD, 1),

    // SIMD data processing instructions - Two Registers, miscellaneous
    MORPH_SET_SIMD_VREV       (VREV),
    MORPH_SET_SIMD_I_QD_BHW   (VPADDLU,   VPairOp,    vmi_ADD,    0, 0, 0, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_QD_BHW   (VPADDLS,   VPairOp,    vmi_ADD,    1, 0, 0, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_QD_BHW   (VPADALU,   VPairOp,    vmi_ADD,    0, 0, 1, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_QD_BHW   (VPADALS,   VPairOp,    vmi_ADD,    1, 0, 1, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_I_QD_BHW   (VPADALS,   VPairOp,    vmi_ADD,    1, 0, 1, 0, 0, 0, ASDS_LONG),
    MORPH_SET_SIMD_QD_BHW     (VCLS),
    MORPH_SET_SIMD_I_U_QD_BHW (VCLZ,      VUnop,      vmi_CLZ,                      ASDS_NORMAL),
    MORPH_SET_SIMD_QD_BHW     (VCLS),
    MORPH_SET_SIMD_QD_B       (VCNT,      VCNT),
    MORPH_SET_SIMD_I_U_QD     (VMVN,      VUnopReg,   vmi_NOT,                      ASDS_NORMAL),
    MORPH_SET_SIMD_I_U_QD_BHW (VQABS,     VUnop,      vmi_ABSSQ,                    ASDS_NORMAL),
    MORPH_SET_SIMD_I_U_QD_BHW (VQNEG,     VUnop,      vmi_NEGSQ,                    ASDS_NORMAL),
    MORPH_SET_VCMP_QD_BHW     (VCGT0,     VCmp0,      vmi_COND_NLE),
    MORPH_SET_VCMP_QD_BHW     (VCGE0,     VCmp0,      vmi_COND_NL),
    MORPH_SET_VCMP_QD_BHW     (VCEQ0,     VCmp0,      vmi_COND_EQ),
    MORPH_SET_VCMP_QD_BHW     (VCLE0,     VCmp0,      vmi_COND_LE),
    MORPH_SET_VCMP_QD_BHW     (VCLT0,     VCmp0,      vmi_COND_L),
    MORPH_SET_VCMP_F_QD       (VCGT0_F,   VCmp0_F, 0, vmi_FPRL_GREATER),
    MORPH_SET_VCMP_F_QD       (VCGE0_F,   VCmp0_F, 0, vmi_FPRL_EQUAL | vmi_FPRL_GREATER),
    MORPH_SET_VCMP_F_QD       (VCEQ0_F,   VCmp0_F, 1, vmi_FPRL_EQUAL),
    MORPH_SET_VCMP_F_QD       (VCLT0_F,   VCmp0_F, 0, vmi_FPRL_LESS),
    MORPH_SET_VCMP_F_QD       (VCLE0_F,   VCmp0_F, 0, vmi_FPRL_EQUAL | vmi_FPRL_LESS),
    MORPH_SET_SIMD_I_U_QD_BHW (VABS,      VUnop,      vmi_ABS,                      ASDS_NORMAL),
    MORPH_SET_SIMD_QD         (VABS_F,    VABS_F),
    MORPH_SET_SIMD_I_U_QD_BHW (VNEG,      VUnop,      vmi_NEG,                      ASDS_NORMAL),
    MORPH_SET_SIMD_QD         (VNEG_F,    VNEG_F),
    MORPH_SET_SIMD_QD         (VSWP,      VSWP),
    MORPH_SET_SIMD_QD_BHW     (VTRN),
    MORPH_SET_VZIP            (VZIP),
    MORPH_SET_VZIP            (VUZP),
    MORPH_SET_VZIP            (VTRN),
    MORPH_SET_SIMD_I_U_HWD    (VMOVN,     VUnop,      vmi_MOV,    0, 0, 0,           ASDS_NARROW),
    MORPH_SET_SIMD_I_U_HWD    (VQMOVNU,   VUnop,      vmi_MOV,    0, 1, 0,           ASDS_NARROW),
    MORPH_SET_SIMD_I_U_HWD    (VQMOVNS,   VUnop,      vmi_MOV,    1, 1, 0,           ASDS_NARROW),
    MORPH_SET_SIMD_I_U_HWD    (VQMOVUNS,  VUnop,      vmi_MOV,    1, 1, 1,           ASDS_NARROW),
    MORPH_SET_SIMD_I_BHW      (VSHLLM,    VShiftMax,  vmi_SHL,    0, 0, 0, 0, 0, 0,  ASDS_LONG),
    MORPH_SET_SIMD_VCVT_FH    (VCVTSH,    VCVT_FH_SIMD),
    MORPH_SET_SIMD_VCVT_HF    (VCVTHS,    VCVT_HF_SIMD),
    MORPH_SET_SIMD_QD         (VRECPE,    VRECPE_U),
    MORPH_SET_SIMD_QD         (VRECPE_F,  VRECPE_F),
    MORPH_SET_SIMD_QD         (VRSQRTE,   VRSQRTE_U),
    MORPH_SET_SIMD_QD         (VRSQRTE_F, VRSQRTE_F),
    MORPH_SET_SIMD_QD         (VRECPS,    VRECPS_F),
    MORPH_SET_SIMD_QD         (VRSQRTS,   VRSQRTS_F),
    MORPH_SET_SIMD_VCVT       (VCVTFU,    VCVT_FX_SIMD, 0),
    MORPH_SET_SIMD_VCVT       (VCVTFS,    VCVT_FX_SIMD, 1),
    MORPH_SET_SIMD_VCVT       (VCVTUF,    VCVT_XF_SIMD, 0),
    MORPH_SET_SIMD_VCVT       (VCVTSF,    VCVT_XF_SIMD, 1),

    // SIMD data processing instructions - One register and a modified immediate
    MORPH_SET_SIMD_I_RI_QD (VMOVI_B,  VOpRI, vmi_BINOP_LAST, 0),
    MORPH_SET_SIMD_I_RI_QD (VMOVI_H,  VOpRI, vmi_BINOP_LAST, 0),
    MORPH_SET_SIMD_I_RI_QD (VMOVI_W,  VOpRI, vmi_BINOP_LAST, 0),
    MORPH_SET_SIMD_I_RI_QD (VMOVI1_W, VOpRI, vmi_BINOP_LAST, 0),
    MORPH_SET_SIMD_I_RI_QD (VMOVI_D,  VOpRI, vmi_BINOP_LAST, 0),
    MORPH_SET_SIMD_I_RI_QD (VMOVI_F_W,VOpRI, vmi_BINOP_LAST, 0),
    MORPH_SET_SIMD_I_RI_QD (VMVNI_H,  VOpRI, vmi_BINOP_LAST, 1),
    MORPH_SET_SIMD_I_RI_QD (VMVNI_W,  VOpRI, vmi_BINOP_LAST, 1),
    MORPH_SET_SIMD_I_RI_QD (VMVNI1_W, VOpRI, vmi_BINOP_LAST, 1),
    MORPH_SET_SIMD_I_RI_QD (VORRI_H,  VOpRI, vmi_OR,         0),
    MORPH_SET_SIMD_I_RI_QD (VORRI_W,  VOpRI, vmi_OR,         0),
    MORPH_SET_SIMD_I_RI_QD (VBICI_H,  VOpRI, vmi_AND,        1),
    MORPH_SET_SIMD_I_RI_QD (VBICI_W,  VOpRI, vmi_AND,        1),

    // VFP data processing instructions - 3 registers
    MORPH_SET_VFP_RRR_F_DS (VMLA_VFP,  VMulAcc_VFP, vmi_FADD, 0),
    MORPH_SET_VFP_RRR_F_DS (VMLS_VFP,  VMulAcc_VFP, vmi_FSUB, 0),
    MORPH_SET_VFP_RRR_F_DS (VNMLA_VFP, VMulAcc_VFP, vmi_FSUB, 1),
    MORPH_SET_VFP_RRR_F_DS (VNMLS_VFP, VMulAcc_VFP, vmi_FADD, 1),
    MORPH_SET_VFP_RRR_F_DS (VNMUL_VFP, VFPBinop,    vmi_FMUL, 1),
    MORPH_SET_VFP_RRR_F_DS (VMUL_VFP,  VFPBinop,    vmi_FMUL, 0),
    MORPH_SET_VFP_RRR_F_DS (VADD_VFP,  VFPBinop,    vmi_FADD, 0),
    MORPH_SET_VFP_RRR_F_DS (VSUB_VFP,  VFPBinop,    vmi_FSUB, 0),
    MORPH_SET_VFP_RRR_F_DS (VDIV_VFP,  VFPBinop,    vmi_FDIV, 0),

    // VFP data processing instructions - Other
    MORPH_SET_VFP_DS       (VMOVI_VFP,      VMOVI_VFP),
    MORPH_SET_VFP_DS       (VMOVR_VFP,      VMOVR_VFP),
    MORPH_SET_VFP_DS       (VABS_VFP,       VABS_VFP),
    MORPH_SET_VFP_DS       (VNEG_VFP,       VNEG_VFP),
    MORPH_SET_VFP_RR_F_DS  (VSQRT_VFP,      vmi_FSQRT),
    MORPH_SET_VFP_VCMP     (VCMP_VFP,       VCMP_VFP,    1),
    MORPH_SET_VFP_VCMP     (VCMPE_VFP,      VCMP_VFP,    0),
    MORPH_SET_VFP_VCMP     (VCMP0_VFP,      VCMP0_VFP,   1),
    MORPH_SET_VFP_VCMP     (VCMPE0_VFP,     VCMP0_VFP,   0),
    //    VCVT between single and double precision
    MORPH_SET_VFP_VCVT     (VCVT_VFP_S,     VCVT_SD_VFP, 8, 4, 0, 1),
    MORPH_SET_VFP_VCVT     (VCVT_VFP_D,     VCVT_SD_VFP, 4, 8, 0, 1),
    //    VCVT between half precision and single precision
    MORPH_SET_VFP_VCVT_H   (VCVTTFH_VFP,    VCVT_SH_VFP, 4, 2, 1),
    MORPH_SET_VFP_VCVT_H   (VCVTBFH_VFP,    VCVT_SH_VFP, 4, 2, 0),
    MORPH_SET_VFP_VCVT_H   (VCVTTHF_VFP,    VCVT_HS_VFP, 2, 4, 1),
    MORPH_SET_VFP_VCVT_H   (VCVTBHF_VFP,    VCVT_HS_VFP, 2, 4, 0),
    //    VCVT from Float (double or single) to Integer (signed or unsigned) with round option
    MORPH_SET_VFP_VCVT     (VCVTRUF_VFP_S,  VCVT_IF_VFP, 4, 4, 0, 1),
    MORPH_SET_VFP_VCVT     (VCVTRUF_VFP_D,  VCVT_IF_VFP, 4, 8, 0, 1),
    MORPH_SET_VFP_VCVT     (VCVTUF_VFP_S,   VCVT_IF_VFP, 4, 4, 0, 0),
    MORPH_SET_VFP_VCVT     (VCVTUF_VFP_D,   VCVT_IF_VFP, 4, 8, 0, 0),
    MORPH_SET_VFP_VCVT     (VCVTRSF_VFP_S,  VCVT_IF_VFP, 4, 4, 1, 1),
    MORPH_SET_VFP_VCVT     (VCVTRSF_VFP_D,  VCVT_IF_VFP, 4, 8, 1, 1),
    MORPH_SET_VFP_VCVT     (VCVTSF_VFP_S,   VCVT_IF_VFP, 4, 4, 1, 0),
    MORPH_SET_VFP_VCVT     (VCVTSF_VFP_D,   VCVT_IF_VFP, 4, 8, 1, 0),
    //    VCVT from Float (double or single) to Fixed (signed or unsigned, word or half)
    MORPH_SET_VFP_VCVT     (VCVTXFUW_VFP_S, VCVT_XF_VFP, 4, 4, 0, 0),
    MORPH_SET_VFP_VCVT     (VCVTXFUW_VFP_D, VCVT_XF_VFP, 4, 8, 0, 0),
    MORPH_SET_VFP_VCVT     (VCVTXFSW_VFP_S, VCVT_XF_VFP, 4, 4, 1, 0),
    MORPH_SET_VFP_VCVT     (VCVTXFSW_VFP_D, VCVT_XF_VFP, 4, 8, 1, 0),
    MORPH_SET_VFP_VCVT     (VCVTXFUH_VFP_S, VCVT_XF_VFP, 2, 4, 0, 0),
    MORPH_SET_VFP_VCVT     (VCVTXFUH_VFP_D, VCVT_XF_VFP, 2, 8, 0, 0),
    MORPH_SET_VFP_VCVT     (VCVTXFSH_VFP_S, VCVT_XF_VFP, 2, 4, 1, 0),
    MORPH_SET_VFP_VCVT     (VCVTXFSH_VFP_D, VCVT_XF_VFP, 2, 8, 1, 0),
    //    VCVT from Fixed (signed or unsigned, word or half) to Float (double or single)
    MORPH_SET_VFP_VCVT     (VCVTFXUW_VFP_S, VCVT_FX_VFP, 4, 4, 0, 0),
    MORPH_SET_VFP_VCVT     (VCVTFXUW_VFP_D, VCVT_FX_VFP, 8, 4, 0, 0),
    MORPH_SET_VFP_VCVT     (VCVTFXSW_VFP_S, VCVT_FX_VFP, 4, 4, 1, 0),
    MORPH_SET_VFP_VCVT     (VCVTFXSW_VFP_D, VCVT_FX_VFP, 8, 4, 1, 0),
    MORPH_SET_VFP_VCVT     (VCVTFXUH_VFP_S, VCVT_FX_VFP, 4, 2, 0, 0),
    MORPH_SET_VFP_VCVT     (VCVTFXUH_VFP_D, VCVT_FX_VFP, 8, 2, 0, 0),
    MORPH_SET_VFP_VCVT     (VCVTFXSH_VFP_S, VCVT_FX_VFP, 4, 2, 1, 0),
    MORPH_SET_VFP_VCVT     (VCVTFXSH_VFP_D, VCVT_FX_VFP, 8, 2, 1, 0),
    //    VCVT From Integer (signed or unsigned) to Float (double or single)
    MORPH_SET_VFP_VCVT     (VCVTFU_VFP_S,   VCVT_FI_VFP, 4, 4, 0, 1),
    MORPH_SET_VFP_VCVT     (VCVTFU_VFP_D,   VCVT_FI_VFP, 8, 4, 0, 1),
    MORPH_SET_VFP_VCVT     (VCVTFS_VFP_S,   VCVT_FI_VFP, 4, 4, 1, 1),
    MORPH_SET_VFP_VCVT     (VCVTFS_VFP_D,   VCVT_FI_VFP, 8, 4, 1, 1),

    // 8, 16 and 32-bit transfer instructions
    MORPH_SET_SINGLE_VFP (VMRS),
    MORPH_SET_SINGLE_VFP (VMSR),
    MORPH_SET_SINGLE_VFP (VMOVRS),
    MORPH_SET_SINGLE_VFP (VMOVSR),
    MORPH_SET_VMOVRZ_BHW (VMOVRZ),
    MORPH_SET_VMOVZR_BHW (VMOVZR),
    MORPH_SET_SIMD_QD_BHW(VDUPR),

    // Extension register load/store instructions
    MORPH_SET_VFP_DS    (VLDR,    VLDR),
    MORPH_SET_VFP_DS    (VSTR,    VSTR),
    MORPH_SET_VFP_DS    (VLDR,    VLDR),
    MORPH_SET_VFP_DS    (VSTR,    VSTR),
    MORPH_SET_VFP_DS    (VLDMIA,  VLDM),
    MORPH_SET_VFP_DS    (VLDMIAW, VLDM),
    MORPH_SET_VFP_DS    (VLDMDBW, VLDM),
    MORPH_SET_VFP_DS    (VPOP,    VLDM),
    MORPH_SET_VFP_DS    (VSTMIA,  VSTM),
    MORPH_SET_VFP_DS    (VSTMIAW, VSTM),
    MORPH_SET_VFP_DS    (VSTMDBW, VSTM),
    MORPH_SET_VFP_DS    (VPUSH,   VSTM),

    // SIMD element or structure load/store instructions
    MORPH_SET_SIMD_LDST1   (VSTN),
    MORPH_SET_SIMD_LDST1   (VLDN),
    MORPH_SET_SIMD_LDST234 (VSTN),
    MORPH_SET_SIMD_LDST234 (VLDN),
    MORPH_SET_SIMD_BHW     (VST1Z1,  VSTNZ1),
    MORPH_SET_SIMD_BHW     (VST2Z1,  VSTNZ1),
    MORPH_SET_SIMD_BHW     (VST3Z1,  VSTNZ1),
    MORPH_SET_SIMD_BHW     (VST4Z1,  VSTNZ1),
    MORPH_SET_SIMD_BHW     (VLD1Z1,  VLDNZ1),
    MORPH_SET_SIMD_BHW     (VLD2Z1,  VLDNZ1),
    MORPH_SET_SIMD_BHW     (VLD3Z1,  VLDNZ1),
    MORPH_SET_SIMD_BHW     (VLD4Z1,  VLDNZ1),
    MORPH_SET_SIMD_BHW     (VLD1ZA,  VLDNZA),
    MORPH_SET_SIMD_BHW     (VLD2ZA,  VLDNZA),
    MORPH_SET_SIMD_BHW     (VLD3ZA,  VLDNZA),
    MORPH_SET_SIMD_BHW     (VLD4ZA,  VLDNZA),

    // 64-bit transfer instructions
    MORPH_SET_SINGLE_VFP (VMOVRRD),
    MORPH_SET_SINGLE_VFP (VMOVDRR),
    MORPH_SET_SINGLE_VFP (VMOVRRSS),
    MORPH_SET_SINGLE_VFP (VMOVSSRR),

};
