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

#ifndef ARM_MORPH_H
#define ARM_MORPH_H

// VMI header files
#include "vmi/vmiTypes.h"

// model header files
#include "armDecodeTypes.h"
#include "armTypeRefs.h"
#include "armVariant.h"


//
// Dispatcher callback type
//
#define ARM_MORPH_FN(_NAME) void _NAME(armMorphStateP state)
typedef ARM_MORPH_FN((*armMorphFn));

//
// Instruction type
//
typedef enum armInstTypeE {
    ARM_TY_NORMAL,  // normal instructions type
    ARM_TY_SIMD,    // advanced SIMD instruction
    ARM_TY_VFP,     // VFP instruction
}  armInstType;

//
// This specifies the conditions under which this is an interworking instruction
// if it targets the program counter
//
typedef enum armInterworkE {
    ARM_IW_NEVER,   // never an interworking instruction
    ARM_IW_L4,      // an interworking instruction if Control.L4 is set
    ARM_IW_ARM_V7   // an interworking instruction if an ARM v7 instruction
} armInterwork;

typedef enum armSIMDShapeE {
    ASDS_NORMAL,    // Result is same width as operands (Q, Q, Q or D, D, D) (default)
    ASDS_LONG,     	// Result is twice the width of operands (Q, D, D)
    ASDS_WIDE,     	// Result and first op are twice width of second op (Q, Q, D)
    ASDS_NARROW,   	// Result is half the width of operands (D, Q, Q)
} armSIMDShape;

//
// This structure provides information required to morph code for an instruction
//
typedef struct armMorphAttrS {

    armMorphFn    morphCB;         // callback function to morph the instruction
    armInterwork  interwork   : 2; // whether an interworking instruction
    armInstType   iType       : 2; // Instruction type

    Bool          condJump    : 1; // is this instruction a conditional jump?
    Bool          jumpIfTrue  : 1; // take branch or jump if condition is True
    Bool          isLink      : 1; // whether branch saves link address
    Bool          shiftCOut   : 1; // whether to set carry with shifter carry
    Bool          exchange    : 1; // whether to exchange SIMD argument halves
    Bool          setGE       : 1; // whether to set SIMD GE fields
    Bool          halve       : 1; // whether to halve SIMD results
    Bool          round       : 1; // whether to round intermediate results
    Bool          satOnNarrow : 1; // whether to saturate results when narrowing
    Bool          ussat       : 1; // signed operands with unsigned saturating result
    Bool          accumulate  : 1; // whether to accumulate
    Bool          subtract    : 1; // whether to subtract
    Bool          highhalf    : 1; // whether to use the highhalf of result
    Bool          negate      : 1; // negate constant (ADR and VMVN)
    Bool          sextend     : 1; // Sign extend the operands
    Bool	 	  isLoad      : 1; // whether this is a load (vs store) operation
    Bool		  isScalar    : 1; // whether this is a scalar operation
    Bool          allowQNaN   : 1; // whether Quiet NaN's are allowed
    Bool          roundFPSCR  : 1; // Use FPSCR rounding mode on VFP vcvt instruction

    vmiFlagsCP    flagsRW;         // flags read and written by the instruction
    vmiFlagsCP    flagsR;          // flags read by the instruction

    vmiUnop       unop        : 8; // if a simple unary operation
    vmiBinop      binop       : 8; // if a simple binary operation
    vmiBinop      binop2      : 8; // second binary operation (SIMD)
    vmiCondition  cond        : 8; // condition to apply
    vmiFPRelation fpRelation  : 8; // Floating point relation to match

    vmiFUnop      funop       : 8; // if a simple fp unary operation
    vmiFBinop     fbinop      : 8; // if a simple fp binary operation
    vmiFBinop     fternop     : 8; // if a simple fp ternary operation

    Uns8          regs;            // number of regs in SIMD operation
    Uns8          elements;        // number of elements per reg in SIMD operation
    Uns8          ebytes;          // SIMD/VFP element size in bytes - SIMD: 1, 2, 4 or 8, VFP: 4 or 8
    Uns8          srcBytes;        // Size of source operand in bytes (for VFP VCVT only)
    Uns8          groupSize;       // VREV group size - 2, 4 or 8 elements
    armSIMDShape  shape;		   // SIMD instruction shape

} armMorphAttr;

typedef enum armSetPCE {
    ASPC_NA,        // no modification to PC
    ASPC_R15,       // indirect jump to address in R15
    ASPC_R15_RET,   // return to address in R15
    ASPC_IMM        // direct jump to immediate address
} armSetPC;

//
// This structure holds state for a single instruction as it is morphed
//
typedef struct armMorphStateS {
    armInstructionInfo info;        // instruction description (from decoder)
    armMorphAttrCP     attrs;       // instruction attributes
    armP               arm;         // current processor
    Uns32              nextPC;      // next instruction address in sequence
    vmiLabelP          skipLabel;   // label to skip instruction body
    Uns32              tempIdx;     // current temporary index
    Bool               pcFetched;   // PC value already fetched?
    armSetPC           pcSet;       // PC value updated
    Uns32              pcImmediate; // immediate value of PC
    Bool               loadCPSR;    // whether to load CPSR from SPSR
    Bool               setMode;     // test for mode switch?
} armMorphState;

//
// This array defines the morpher dispatch table
//
extern const armMorphAttr armMorphTable[ARM_IT_LAST+1];

#endif
