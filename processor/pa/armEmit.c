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
#include "stdio.h"
// VMI header files
#include "vmi/vmiAttrs.h"
#include "vmi/vmiMt.h"
#include "vmi/vmiRt.h"
#include "vmi/vmiMessage.h"

// model header files
#include "armDecode.h"
#include "armEmit.h"
#include "armFunctions.h"
#include "armFPConstants.h"
#include "armMode.h"
#include "armMorph.h"
#include "armStructure.h"
#include "armSIMDVFP.h"
#include "armRegisters.h"
#include "armUtils.h"
#include "armMurac.h"

//
// Prefix for messages from this module
//
#define CPU_PREFIX "ARM_EMIT"


////////////////////////////////////////////////////////////////////////////////
// SPECIAL HANDLING FOR PC UPDATES
////////////////////////////////////////////////////////////////////////////////

//
// Is the passed register the link register?
//
inline static Bool isLinkReg(vmiReg ra) {
    return VMI_REG_EQUAL(ra, ARM_LR);
}

//
// Invalidate derived flags dependent on ZF
//
inline static void invalidateZF(armMorphStateP state) {
    state->arm->validLE = False;
    state->arm->validHI = False;
}

//
// Invalidate derived flags dependent on NF
//
inline static void invalidateNF(armMorphStateP state) {
    state->arm->validLE = False;
    state->arm->validLT = False;
}

//
// Invalidate derived flags dependent on CF
//
inline static void invalidateCF(armMorphStateP state) {
    state->arm->validHI = False;
}

//
// Invalidate derived flags dependent on VF
//
inline static void invalidateVF(armMorphStateP state) {
    state->arm->validLE = False;
    state->arm->validLT = False;
}

//
// Invalidate any derived state when a register is written
//
static void invalidateDerivedReg(armMorphStateP state, vmiReg rd) {

    Uns32 gprIndex;

    // remove GPR index from ThumbEE zero check mask, or invalidate any derived
    // flags if a flag register is being assigned
    if(armGetGPRIndex(state, rd, &gprIndex)) {
        state->arm->teeNZMask &= ~(1<<gprIndex);
    } else if(VMI_REG_EQUAL(rd, ARM_ZF)) {
        invalidateZF(state);
    } else if(VMI_REG_EQUAL(rd, ARM_NF)) {
        invalidateNF(state);
    } else if(VMI_REG_EQUAL(rd, ARM_CF)) {
        invalidateCF(state);
    } else if(VMI_REG_EQUAL(rd, ARM_VF)) {
        invalidateVF(state);
    }
}

//
// Invalidate derived flags for all flags assigned in the flags structure
//
static void invalidateDerivedFlags(armMorphStateP state, vmiFlagsCP flags) {
    if(flags) {
        invalidateDerivedReg(state, flags->f[vmi_CF]);
        invalidateDerivedReg(state, flags->f[vmi_ZF]);
        invalidateDerivedReg(state, flags->f[vmi_SF]);
        invalidateDerivedReg(state, flags->f[vmi_OF]);
    }
}

//
// Called when register is assigned a variable value
//
static void setVariable(armMorphStateP state, vmiReg rd, Bool isReturn) {

    armP arm = state->arm;

    // assignment to a flag should invalidate any derived flags
    invalidateDerivedReg(state, rd);

    // possibly a special jump if PC is being assigned
    if(VMI_REG_EQUAL(rd, ARM_PC)) {

        Uns32 version   = ARM_INSTRUCTION_VERSION(arm->configInfo.arch);
        Bool  interwork = False;

        switch(state->attrs->interwork) {

            case ARM_IW_L4:
                // an interworking instruction if ARMv5 or above and SCTLR.L4
                // is set
                interwork = ((version>=5) && !CP_FIELD(arm, SCTLR, L4));
                // validate block mode if SCTLR.L4 ever changes
                if(arm->checkL4) {
                    vmimtValidateBlockMask(ARM_BM_L4);
                }
                break;

            case ARM_IW_ARM_V7:
                // an interworking instruction if ARMv7 or above and an ARM
                // instruction that does not set flags (not Thumb)
                interwork = ((version>=7) && !IN_THUMB_MODE(arm) && !state->info.f);
                break;

            default:
                // never an interworking instruction (or an explicit
                // interworking instruction)
                break;
        }

        state->pcSet   = isReturn ? ASPC_R15_RET : ASPC_R15;
        state->setMode = interwork;
    }
}

//
// Called when register is assigned a constant value
//
static Bool setConstant(armMorphStateP state, vmiReg rd, Uns32 c) {

    // assignment to a flag should invalidate any derived flags
    invalidateDerivedReg(state, rd);

    // special jump if PC is being assigned
    if(!VMI_REG_EQUAL(rd, ARM_PC)) {
        return False;
    } else {
        state->pcSet       = ASPC_IMM;
        state->pcImmediate = c;
        return True;
    }
}


////////////////////////////////////////////////////////////////////////////////
// MODE SWITCH
////////////////////////////////////////////////////////////////////////////////

//
// Emit code to toggle a bit in the blockMask
//
static void emitToggleBlockMask(Uns32 mask) {

    // toggle Thumb mode bit in blockMask
    vmimtBinopRC(32, vmi_XOR, ARM_BLOCK_MASK, mask, 0);

    // modify the block mask
    vmimtArgProcessor();
    vmimtArgReg(32, ARM_BLOCK_MASK);
    vmimtCall((vmiCallFn)vmirtSetBlockMask);

    // end the code block on a blockMask change
    vmimtEndBlock();
}

//
// Enable interwork checking
//
static void enableInterworkCheck(armP arm) {

    // sanity check mode switch checking is not already enabled
    VMI_ASSERT(!arm->checkInterwork, "interwork checking already enabled");

    // discard the current code dictionaries (they need to be regenerated with
    // interwork checking enabled in the block mask)
    vmirtFlushAllDicts((vmiProcessorP)arm);

    // indicate that checking is now enabled
    arm->checkInterwork = True;
}

//
// Enable Thumb/ThumbEE checking
//
static void enableThumbEECheck(armP arm) {

    // sanity check mode switch checking is not already enabled
    VMI_ASSERT(!arm->checkThumbEE, "Thumb/ThumbEE checking already enabled");

    // discard the current code dictionaries (they need to be regenerated with
    // Thumb/ThumbEE checking enabled in the block mask)
    vmirtFlushAllDicts((vmiProcessorP)arm);

    // indicate that checking is now enabled
    arm->checkThumbEE = True;
}

//
// Emit code to switch state between normal and Thumb mode
//
void armEmitInterwork(armMorphStateP state) {

    // toggle Thumb mode bit in blockMask
    emitToggleBlockMask(ARM_BM_THUMB);

    // on first call, enable interwork checking
    if(!state->arm->checkInterwork) {
        vmimtArgProcessor();
        vmimtCall((vmiCallFn)enableInterworkCheck);
    }
}

//
// Emit code to switch state between Thumb and ThumbEE mode
//
void armEmitEnterLeaveX(armMorphStateP state) {

    // toggle Thumb mode bit in blockMask
    emitToggleBlockMask(ARM_BM_JAZ_EE);

    // on first call, enable interwork checking
    if(!state->arm->checkThumbEE) {
        vmimtArgProcessor();
        vmimtCall((vmiCallFn)enableThumbEECheck);
    }
}


////////////////////////////////////////////////////////////////////////////////
// INTEGER OPCODES
////////////////////////////////////////////////////////////////////////////////

//
// r15 = simPC
//
void armEmitGetPC(armMorphStateP state) {
    if(!state->pcFetched) {
        state->pcFetched = True;
        vmimtMoveRSimPC(ARM_GPR_BITS, ARM_PC);
        vmimtBinopRC(ARM_GPR_BITS, vmi_ADD, ARM_PC, ARM_PC_DELTA(state->arm), 0);
    }
}

//
// rd = c
//
void armEmitMoveRC(
    armMorphStateP state,
    Uns32          bits,
    vmiReg         rd,
    Uns64          c
) {
    if(!setConstant(state, rd, c)) {
        vmimtMoveRC(bits, rd, c);
    }
}

//
// rd = ra
//
void armEmitMoveRR(
    armMorphStateP state,
    Uns32          bits,
    vmiReg         rd,
    vmiReg         ra
) {
    setVariable(state, rd, isLinkReg(ra));
    vmimtMoveRR(bits, rd, ra);
}

//
// rd<destBits> = ra<srcBits>
//
void armEmitMoveExtendRR(
    armMorphStateP state,
    Uns32          destBits,
    vmiReg         rd,
    Uns32          srcBits,
    vmiReg         ra,
    Bool           signExtend
) {
    setVariable(state, rd, False);
    vmimtMoveExtendRR(destBits, rd, srcBits, ra, signExtend);
}

//
// rd = (flag==select1) ? c1 : c2
//
void armEmitCondMoveRCC(
    armMorphStateP state,
    Uns32          bits,
    vmiReg         flag,
    Bool           select1,
    vmiReg         rd,
    Uns64          c1,
    Uns64          c2
) {
    setVariable(state, rd, False);
    vmimtCondMoveRCC(bits, flag, select1, rd, c1, c2);
}

//
// rd = (flag==select1) ? r1 : c2
//
void armEmitCondMoveRRC(
    armMorphStateP state,
    Uns32          bits,
    vmiReg         flag,
    Bool           select1,
    vmiReg         rd,
    vmiReg         r1,
    Uns64          c2
) {
    setVariable(state, rd, False);
    vmimtCondMoveRRC(bits, flag, select1, rd, r1, c2);
}

//
// rd = (flag==select1) ? r1 : r2
//
void armEmitCondMoveRRR(
    armMorphStateP state,
    Uns32          bits,
    vmiReg         flag,
    Bool           select1,
    vmiReg         rd,
    vmiReg         r1,
    vmiReg         r2
) {
    setVariable(state, rd, False);
    vmimtCondMoveRRR(bits, flag, select1, rd, r1, r2);
}

//
// rd = <unop> rd
//
void armEmitUnopR(
    armMorphStateP state,
    Uns32          bits,
    vmiUnop        op,
    vmiReg         rd,
    vmiFlagsCP     flags
) {
    invalidateDerivedFlags(state, flags);
    setVariable(state, rd, False);
    vmimtUnopR(bits, op, rd, flags);
}

//
// rd = <unop> ra
//
void armEmitUnopRR(
    armMorphStateP state,
    Uns32          bits,
    vmiUnop        op,
    vmiReg         rd,
    vmiReg         ra,
    vmiFlagsCP     flags
) {
    invalidateDerivedFlags(state, flags);
    setVariable(state, rd, (op==vmi_MOV) && isLinkReg(ra));
    vmimtUnopRR(bits, op, rd, ra, flags);
}

//
// rd = <unop> c
//
void armEmitUnopRC(
    armMorphStateP state,
    Uns32          bits,
    vmiUnop        op,
    vmiReg         rd,
    Uns64          c,
    vmiFlagsCP     flags
) {
    invalidateDerivedFlags(state, flags);

    // convert constant so that value can be moved directly
    if(op==vmi_MOV) {
        // no action
    } else if(op==vmi_NOT) {
        op = vmi_MOV;
        c  = ~c;
    } else {
        VMI_ABORT("unexpected unary opcode %u", op);
    }

    // operation is only required if the target is not the program counter or
    // if flags are needed
    if(!setConstant(state, rd, c) || flags) {
        vmimtUnopRC(bits, op, rd, c, flags);
    }
}

//
// rd = rd <binop> ra
//
void armEmitBinopRR(
    armMorphStateP state,
    Uns32          bits,
    vmiBinop       op,
    vmiReg         rd,
    vmiReg         ra,
    vmiFlagsCP     flags
) {
    invalidateDerivedFlags(state, flags);
    setVariable(state, rd, False);
    vmimtBinopRR(bits, op, rd, ra, flags);
}

//
// rd = rd <binop> c
//
void armEmitBinopRC(
    armMorphStateP state,
    Uns32          bits,
    vmiBinop       op,
    vmiReg         rd,
    Uns64          c,
    vmiFlagsCP     flags
) {
    invalidateDerivedFlags(state, flags);
    setVariable(state, rd, False);
    vmimtBinopRC(bits, op, rd, c, flags);
}

//
// rd = ra <binop> rb
//
void armEmitBinopRRR(
    armMorphStateP state,
    Uns32          bits,
    vmiBinop       op,
    vmiReg         rd,
    vmiReg         ra,
    vmiReg         rb,
    vmiFlagsCP     flags
) {
    invalidateDerivedFlags(state, flags);
    setVariable(state, rd, False);
    vmimtBinopRRR(bits, op, rd, ra, rb, flags);
}

//
// rd = ra <binop> c
//
void armEmitBinopRRC(
    armMorphStateP state,
    Uns32          bits,
    vmiBinop       op,
    vmiReg         rd,
    vmiReg         ra,
    Uns64          c,
    vmiFlagsCP     flags
) {
    invalidateDerivedFlags(state, flags);
    setVariable(state, rd, False);
    vmimtBinopRRC(bits, op, rd, ra, c, flags);
}

//
// Generate shift mask prefix (sets mask to 255)
//
void armEmitSetShiftMask(void) {
    vmimtSetShiftMask(255);
}

//
// rdh:rdl = ra*rb
//
void armEmitMulopRRR(
    armMorphStateP state,
    Uns32          bits,
    vmiBinop       op,
    vmiReg         rdH,
    vmiReg         rdL,
    vmiReg         ra,
    vmiReg         rb,
    vmiFlagsCP     flags
) {
    invalidateDerivedFlags(state, flags);
    setVariable(state, rdL, False);
    setVariable(state, rdH, False);
    vmimtMulopRRR(bits, op, rdH, rdL, ra, rb, flags);
}


////////////////////////////////////////////////////////////////////////////////
// FLOATING POINT OPCODES
////////////////////////////////////////////////////////////////////////////////

//
// Return the floating point mode for the current instruction
//
static armFPMode getFPMode(armMorphStateP state) {
    armFPMode mode = 0;
    switch (state->attrs->iType) {
    case ARM_TY_SIMD: mode = ARM_FPM_SIMD; break;
    case ARM_TY_VFP:  mode = ARM_FPM_VFP;  break;
    default: VMI_ABORT("Unknown instruction type setting SIMD/VFP mode");
    }
    return mode;
}

//
// Emit code to switch floating point mode
//
static void switchFPMode(armFPMode fpMode) {

    vmimtArgProcessor();

    if(fpMode==ARM_FPM_SIMD) {
        vmimtCall((vmiCallFn)armSetSIMDControlWord);
    } else {
        vmimtCall((vmiCallFn)armSetVFPControlWord);
    }
}

//
// Perform actions before a floating point operation
//
static void startFPOperation(armMorphStateP state) {

    armP      arm    = state->arm;
    armFPMode fpMode = getFPMode(state);

    if(arm->fpModeMT==ARM_FPM_NONE) {

        // if this is the first floating point operation in this code block then
        // validate that the processor is currently in the correct floating
        // point mode and set it if not
        vmiLabelP noSetMode = armEmitNewLabel();

        // skip mode switch unless floating point mode has changed
        vmimtCompareRCJumpLabel(8, vmi_COND_Z, ARM_FP_MODE, fpMode, noSetMode);

        // switch processor mode
        switchFPMode(fpMode);

        // here if no mode switch required
        armEmitInsertLabel(noSetMode);

    } else if(fpMode!=arm->fpModeMT) {

        // if this is not the first floating point operation in this code block
        // then switch mode unconditionally if required floating point mode
        // differs from the current code block mode
        switchFPMode(fpMode);
    }

    // floating point mode in the remainder of this code block is now known
    arm->fpModeMT = fpMode;
}

//
// Perform actions after a floating point operation
//
static void endFPOperation(armMorphStateP state, Bool setSticky) {

	if(setSticky) {
		// merge sticky flags
		vmimtBinopRR(8, vmi_OR, ARM_FP_STICKY, ARM_FP_FLAGS, 0);
	}
}

//
// fd = fa <fp ternnop> fb
//
void armEmitFTernopSimdRRRR(
    armMorphStateP state,
    vmiFType       type,
    Uns32          num,
    vmiFTernop     op,
    vmiReg         fd,
    vmiReg         fa,
    vmiReg         fb,
    vmiReg         fc,
    Bool           roundInt
) {
    // do prologue actions
    startFPOperation(state);

    // do the ternop
    vmimtFTernopSimdRRRR(type, num, op, fd, fa, fb, fc, ARM_FP_FLAGS, roundInt);

    // do epilogue actions
    endFPOperation(state, True);
}

//
// fd = fa <fp binop> fb
//
void armEmitFBinopSimdRRR(
    armMorphStateP state,
    vmiFType       type,
    Uns32          num,
    vmiFBinop      op,
    vmiReg         fd,
    vmiReg         fa,
    vmiReg         fb
) {
    // do prologue actions
    startFPOperation(state);

    // do the binop
    vmimtFBinopSimdRRR(type, num, op, fd, fa, fb, ARM_FP_FLAGS);

    // do epilogue actions
    endFPOperation(state, True);
}

//
// fd = <fp unop> fa
//
void armEmitFUnopSimdRR(
    armMorphStateP state,
    vmiFType       type,
    Uns32          num,
    vmiFUnop       op,
    vmiReg         fd,
    vmiReg         fa
) {
    // do prologue actions
    startFPOperation(state);

    // do the unop
    vmimtFUnopSimdRR(type, num, op, fd, fa, ARM_FP_FLAGS);

    // do epilogue actions
    endFPOperation(state, True);
}

//
// fd = <fp convert> fa
//
void armEmitFConvertRR(
    armMorphStateP state,
    vmiFType       destType,
    vmiReg         fd,
    vmiFType       srcType,
    vmiReg         fa,
    vmiFPRC        round
) {
    // conversions from integer values to longer values (integer or floating
    // point) never produce exceptions or require rounding
    Bool  srcIsFP       = !VMI_FTYPE_IS_INT(srcType);
    Uns32 srcBits       = VMI_FTYPE_BITS(srcType);
    Uns32 destBits      = VMI_FTYPE_BITS(destType);
    Bool  exceptOrRound = (srcIsFP || (srcBits>=destBits));

    // do prologue actions if exceptions or rounding required
    if(exceptOrRound) {
        startFPOperation(state);
    }

    // do the conversion
    vmimtFConvertRR(destType, fd, srcType, fa, round, ARM_FP_FLAGS);

    // do epilogue actions if exceptions were possible
    if(exceptOrRound) {
        endFPOperation(state, True);
    }
}

//
// compare fa to fb, setting relation and flags
//
void armEmitFCompareRR(
    armMorphStateP state,
    vmiFType type,
    vmiReg   relation,
    vmiReg   fa,
    vmiReg   fb,
    Bool     allowQNaN,
    Bool     setSticky
) {
    // do prologue actions
    startFPOperation(state);

    // do the compare
    vmimtFCompareRR(type, relation, fa, fb, ARM_FP_FLAGS, allowQNaN);

    // do epilogue actions
    endFPOperation(state, setSticky);
}

//
// Get VFP FPSCR LEN and STRIDE fields
// Gets actual values to use, not values in fields
// Return false if invalid value found in register
//
Bool armGetVFPLenStride(
    armMorphStateP state,
    Uns32          ebytes,
    Uns32         *lenP,
    Uns32         *strideP
) {

    // emit blockmask check to make sure len and stride are valid
    vmimtValidateBlockMask(ARM_BM_STRIDE | ARM_BM_LEN);

    Uns32 stride = SDFP_FIELD(state->arm, FPSCR, Stride);
    Uns32 len    = SDFP_FIELD(state->arm, FPSCR, Len);

    // Value in Stride field = b'00' maps to 1, b'11' maps to 2
    if (stride == 0) {
        *strideP = 1;
    } else if (stride == 3) {
        *strideP = 2;
    } else {
        // Only 00 or 11 are supported
        return False;
    }

    // Use value in LEN field + 1 for the number of regs
    *lenP = len + 1;

    // Attempt to execute vector instruction when short vectors
    // are not implemented = undefined instruction
    if (len != 0 && SDFP_FIELD(state->arm, MVFR0, ShortVectors) == 0) {
        return False;
    }

    // Unpredictable when len and stride are larger than the bank size
    if ((*lenP) * (*strideP) * ebytes > 32) {
        return False;
    }

    return True;

}


////////////////////////////////////////////////////////////////////////////////
// LOAD AND STORE OPCODES
////////////////////////////////////////////////////////////////////////////////

//
// Return processor endianness (and add blockMask check to validate it if
// required)
//
static memEndian getEndian(armMorphStateP state, Uns32 bits) {

    armP arm = state->arm;

    // validate endianness for memory operations wider than a byte
    if(arm->checkEndian && (bits>8)) {
        vmimtValidateBlockMask(ARM_BM_BIG_ENDIAN);
    }

    // return current endianness
    return armGetEndian((vmiProcessorP)arm, False);
}

//
// Indicate whether instruction requires alignment checking (and add blockMask
// check to validate it if required)
//
static Bool getAlign(armMorphStateP state, Uns32 bits) {

    armP arm = state->arm;

    // validate alignment checking for memory operations wider than a byte
    if(arm->checkUnaligned && (bits>8)) {
        vmimtValidateBlockMask(ARM_BM_UNALIGNED);
    }

    // alignment checking required if ua is not ARM_UA_UNALIGNED
    return !(DO_UNALIGNED(arm) && (state->info.ua==ARM_UA_UNALIGNED));
}

//
// mem[ra+offset] = rb  (when ra!=VMI_NOREG)
// mem[offset]    = rb  (when ra==VMI_NOREG)
//
void armEmitStoreRRO(
    armMorphStateP state,
    Uns32          bits,
    Uns32          offset,
    vmiReg         ra,
    vmiReg         rb
) {
    memEndian endian = getEndian(state, bits);
    Bool      align  = getAlign(state, bits);

    // emit single store
    vmimtStoreRRO(bits, offset, ra, rb, endian, align);
}

//
// mem[ra+offset] = rbH:rbL  (when ra!=VMI_NOREG)
// mem[offset]    = rbH:rbL  (when ra==VMI_NOREG)
//
void armEmitStoreRRRO(
    armMorphStateP state,
    Uns32          bits,
    Uns32          offset,
    vmiReg         ra,
    vmiReg         rbL,
    vmiReg         rbH
) {
    Uns32     regBits = ARM_GPR_BITS;
    armP      arm     = state->arm;
    memEndian endian  = getEndian(state, bits);
    Bool      align   = getAlign(state, bits);

    // generate exception for misaligned or inaccessible address if required
    if(!arm->configInfo.align64as32) {
        vmimtTryStoreRC(bits, offset, ra, align);
    }

    // emit two word stores
    vmimtStoreRRO(regBits, offset,   ra, rbL, endian, align);
    vmimtStoreRRO(regBits, offset+4, ra, rbH, endian, align);
}

//
// rd = mem[ra+offset]  (when ra!=VMI_NOREG)
// rd = mem[offset]     (when ra==VMI_NOREG)
//
void armEmitLoadRRO(
    armMorphStateP state,
    Uns32          destBits,
    Uns32          memBits,
    Uns32          offset,
    vmiReg         rd,
    vmiReg         ra,
    Bool           signExtend,
    Bool           isReturn
) {
    memEndian endian = getEndian(state, memBits);
    Bool      align  = getAlign(state, memBits);

    // emit single load
    vmimtLoadRRO(destBits, memBits, offset, rd, ra, endian, signExtend, align);
    setVariable(state, rd, isReturn);
}

//
// rdH:rdL = mem[ra+offset]  (when ra!=VMI_NOREG)
// rdH:rdL = mem[offset]     (when ra==VMI_NOREG)
//
void armEmitLoadRRRO(
    armMorphStateP state,
    Uns32          bits,
    Uns32          offset,
    vmiReg         rdL,
    vmiReg         rdH,
    vmiReg         ra,
    vmiReg         rt,
    Bool           signExtend,
    Bool           isReturn
) {
    Uns32     regBits = ARM_GPR_BITS;
    armP      arm     = state->arm;
    memEndian endian  = getEndian(state, bits);
    Bool      align   = getAlign(state, bits);

    // generate exception for misaligned or inaccessible address if required
    if(!arm->configInfo.align64as32) {
        vmimtTryLoadRC(bits, offset, ra, align);
    }

    // emit two word loads (the first into a temporary in case the second fails)
    vmimtLoadRRO(regBits, regBits, offset,   rt,  ra, endian, False, align);
    vmimtLoadRRO(regBits, regBits, offset+4, rdH, ra, endian, False, align);
    vmimtMoveRR(regBits, rdL, rt);
    setVariable(state, rdL, isReturn);
    setVariable(state, rdH, isReturn);
}

//
// tryload mem[ra+offset]  (when ra!=VMI_NOREG)
// tryload mem[offset]     (when ra==VMI_NOREG)
//
void armEmitTryLoadRC(
    armMorphStateP state,
    Uns32          bits,
    Addr           offset,
    vmiReg         ra
) {
    vmimtTryLoadRC(bits, offset, ra, getAlign(state, bits));
}

//
// trystore mem[ra+offset]  (when ra!=VMI_NOREG)
// trystore mem[offset]     (when ra==VMI_NOREG)
//
void armEmitTryStoreRC(
    armMorphStateP state,
    Uns32          bits,
    Addr           offset,
    vmiReg         ra
) {
    vmimtTryStoreRC(bits, offset, ra, getAlign(state, bits));
}


////////////////////////////////////////////////////////////////////////////////
// COMPARE OPERATIONS
////////////////////////////////////////////////////////////////////////////////

//
// flag = ra <cond> rb
//
void armEmitCompareRR(
    armMorphStateP state,
    Uns32          bits,
    vmiCondition   cond,
    vmiReg         ra,
    vmiReg         rb,
    vmiReg         flag
) {
    vmimtCompareRR(bits, cond, ra, rb, flag);
}

//
// flag = ra <cond> c
//
void armEmitCompareRC(
    armMorphStateP state,
    Uns32          bits,
    vmiCondition   cond,
    vmiReg         ra,
    Uns64          c,
    vmiReg         flag
) {
    vmimtCompareRC(bits, cond, ra, c, flag);
}


////////////////////////////////////////////////////////////////////////////////
// INTER-INSTRUCTION CONDITIONAL AND UNCONDITIONAL JUMPS
////////////////////////////////////////////////////////////////////////////////

//
// Emit code to clear ITSTATE if required
//
static void emitClearITState(armMorphStateP state) {
    if(state->arm->itStateMT) {
        armEmitMoveRC(state, 8, ARM_IT_STATE, 0);
    }
}

//
// Set address mask to mask off the bottom bit of the target address
//
static void emitAddressMask(void) {
    vmimtSetAddressMask(~1);
}

//
// Perform an unconditional direct jump.
//
void armEmitUncondJump(
    armMorphStateP state,
    armJumpInfoP   ji
) {
    emitAddressMask();
    emitClearITState(state);
    vmimtUncondJump(
        ji->linkPC,
        state->info.t,
        ji->linkReg,
        ji->hint
    );
}

//
// Perform an unconditional indirect jump.
//
void armEmitUncondJumpReg(
    armMorphStateP state,
    armJumpInfoP   ji,
    vmiReg         toReg
) {
    emitAddressMask();
    emitClearITState(state);
    vmimtUncondJumpReg(
        ji->linkPC,
        toReg,
        ji->linkReg,
        ji->hint
    );
}

//
// Perform a conditional direct jump if the condition flag is non-zero
// (jumpIfTrue) or zero (not jumpIfTrue).
//
void armEmitCondJump(
    armMorphStateP state,
    armJumpInfoP   ji,
    vmiReg         flag,
    Bool           jumpIfTrue
) {
    emitAddressMask();
    emitClearITState(state);
    vmimtCondJump(
        flag,
        jumpIfTrue,
        ji->linkPC,
        state->info.t,
        ji->linkReg,
        ji->hint
    );
}

//
// Jump to the label if the register masked with the mask is non-zero (if
// jumpIfNonZero) or zero (if !jumpIfNonZero)
//
static void emitJumpLabelOnMask(
    armMorphStateP state,
    Uns32          bits,
    vmiLabelP      label,
    vmiReg         reg,
    Uns32          mask,
    Bool           jumpIfNonZero
) {
    vmiCondition cond = jumpIfNonZero ? vmi_COND_NZ : vmi_COND_Z;
    vmimtTestRCJumpLabel(bits, cond, reg, mask, label);
}

//
// Emit code to interwork if required, depending on LSB of target address
//
void armEmitInterworkLSB(armMorphStateP state, vmiReg ra) {

    armP arm         = state->arm;
    Bool inThumbMode = IN_THUMB_MODE(arm);

    // this code is Thumb/ThumbEE state dependent
    if(inThumbMode && arm->checkThumbEE) {
        armEmitValidateBlockMask(ARM_BM_JAZ_EE);
    }

    // ThumbEE does not support interworking based on LSB
    if(!IN_THUMB_EE_MODE(arm)) {

        vmiLabelP noSwitch = armEmitNewLabel();

        // skip mode switch unless mode has changed
        emitJumpLabelOnMask(state, ARM_GPR_BITS, noSwitch, ra, 1, inThumbMode);

        // switch processor mode
        armEmitInterwork(state);

        // here if no mode switch required
        armEmitInsertLabel(noSwitch);
    }
}

//
// Check for interwork depending on setting of bottom bit of target address
//
static void emitCheckSetMode(armMorphStateP state) {

    // switch mode if LSB of PC implies a different mode
    if(state->setMode) {
        armEmitInterworkLSB(state, ARM_PC);
    }

    // mask off LSB of target address
    emitAddressMask();
}

//
// Perform an implicit unconditional direct jump if required
//
void armEmitImplicitUncondJump(armMorphStateP state) {

    switch(state->pcSet) {

        case ASPC_NA:
            // no action unless PC was set by the instruction
            break;

        case ASPC_R15:
            // indirect jump to current value in R15
            emitCheckSetMode(state);
            emitClearITState(state);
            vmimtUncondJumpReg(0, ARM_PC, VMI_NOREG, vmi_JH_NONE);
            break;

        case ASPC_R15_RET:
            // return to current value in R15
            emitCheckSetMode(state);
            emitClearITState(state);
            vmimtUncondJumpReg(0, ARM_PC, VMI_NOREG, vmi_JH_RETURN);
            break;

        case ASPC_IMM:
            // direct jump to immediate address
            emitClearITState(state);
            vmimtUncondJump(0, state->pcImmediate, VMI_NOREG, vmi_JH_NONE);
            break;
    }
}


////////////////////////////////////////////////////////////////////////////////
// INTRA-INSTRUCTION CONDITIONAL AND UNCONDITIONAL JUMPS
////////////////////////////////////////////////////////////////////////////////

//
// Create and return a new label
//
vmiLabelP armEmitNewLabel(void) {
    return vmimtNewLabel();
}

//
// Insert a label previously created by vmimtNewLabel at the current location.
//
void armEmitInsertLabel(vmiLabelP label) {
    vmimtInsertLabel(label);
}

//
// Perform an unconditional jump to the passed local label.
//
void armEmitUncondJumpLabel(vmiLabelP toLabel) {
    vmimtUncondJumpLabel(toLabel);
}

//
// Perform a conditional jump if the condition flag is non-zero (jumpIfTrue)
// or zero (not jumpIfTrue). The target location is the passed local label.
//
void armEmitCondJumpLabel(vmiReg flag, Bool jumpIfTrue, vmiLabelP toLabel) {
    vmimtCondJumpLabel(flag, jumpIfTrue, toLabel);
}

//
// Perform a conditional jump if the function result is True (jumpIfTrue)
// or zero (not jumpIfTrue). The target location is the passed local label.
//
void armEmitCondJumpLabelFunctionResult(Bool jumpIfTrue, vmiLabelP toLabel) {
    vmimtCondJumpLabelFunctionResult(jumpIfTrue, toLabel);
}

//
// Test the register value by performing bitwise AND with the passed constant
// value, and jump to 'toLabel' if condition 'cond' is satisfied.
//
void armEmitTestRCJumpLabel(
    Uns32        bits,
    vmiCondition cond,
    vmiReg       r,
    Uns64        c,
    vmiLabelP    toLabel
) {
    vmimtTestRCJumpLabel(bits, cond, r, c, toLabel);
}

//
// Compare the register value by performing subtraction of the passed constant
// value, and jump to 'toLabel' if condition 'cond' is satisfied.
//
void armEmitCompareRCJumpLabel(
    Uns32        bits,
    vmiCondition cond,
    vmiReg       r,
    Uns64        c,
    vmiLabelP    toLabel
) {
    vmimtCompareRCJumpLabel(bits, cond, r, c, toLabel);
}


////////////////////////////////////////////////////////////////////////////////
// CALLBACK FUNCTION INTERFACE
////////////////////////////////////////////////////////////////////////////////

//
// Add processor argument to the stack frame
//
void armEmitArgProcessor(armMorphStateP state) {
    vmimtArgProcessor();
}

//
// Add Uns32 argument to the stack frame
//
void armEmitArgUns32(armMorphStateP state, Uns32 arg) {
    vmimtArgUns32(arg);
}

//
// Add register argument to the stack frame
//
void armEmitArgReg(armMorphStateP state, Uns32 bits, vmiReg r) {
    vmimtArgReg(bits, r);
}

//
// Add program counter argument to the stack frame
//
void armEmitArgSimPC(armMorphStateP state, Uns32 bits) {
    vmimtArgSimPC(bits);
}

//
// Make a call with all current stack frame arguments
//
void armEmitCall(armMorphStateP state, vmiCallFn arg) {
    vmimtCall(arg);
}

//
// As above but generate a function result (placed in rd)
//
void armEmitCallResult(
    armMorphStateP state,
    vmiCallFn      arg,
    Uns32          bits,
    vmiReg         rd
) {
    setVariable(state, rd, False);
    vmimtCallResult(arg, bits, rd);
}


////////////////////////////////////////////////////////////////////////////////
// SIMULATOR CONTROL
////////////////////////////////////////////////////////////////////////////////

//
// Halt the current processor for the passed reason
//
void armEmitHalt(armDisableReason reason) {
    vmimtMoveRC(8, ARM_DISABLE, reason);
    vmimtHalt();
}

//
// Stop simulation of the current processor
//
void armEmitExit(void) {
    vmimtExit();
}

//
// Terminate the current block
//
void armEmitEndBlock(void) {
    vmimtEndBlock();
}

//
// Emit code to validate the current block mode
//
void armEmitValidateBlockMask(armBlockMask blockMask) {
    vmimtValidateBlockMask(blockMask);
}

void armEmitBrArch(void) {

    // Write the PC value to shared memory
    vmimtMoveRSimPC(32, ARM_REG(2));
    vmimtBinopRC(32, vmi_ADD, ARM_REG(2), 4, 0);

    vmimtMoveRC(32, ARM_REG(3), MURAC_PC_ADDRESS);
    vmimtStoreRRO(32, 0, ARM_REG(3), ARM_REG(2), MEM_ENDIAN_LITTLE, True);

    // Write the AA instruction block size to shared memory
    vmimtMoveRC(32, ARM_REG(3), MURAC_PC_ADDRESS + 4);
    vmimtStoreRRO(32, 0, ARM_REG(3), ARM_REG(1), MEM_ENDIAN_LITTLE, True);

    // Write the stack poitner passed to AA to shared memory
    vmimtMoveRC(32, ARM_REG(3), MURAC_PC_ADDRESS + 8);
    vmimtStoreRRO(32, 0, ARM_REG(3), ARM_REG(0), MEM_ENDIAN_LITTLE, True);

    // Halt the processor
    vmimtMoveRC(8, ARM_DISABLE, AD_WFI);
    vmimtHalt();
        
    // Trigger the interrupt*/
    vmimtArgProcessor();
    vmimtArgReg(32, ARM_REG(1));
    vmimtCall((vmiCallFn)vmic_branchAuxiliaryArchitecture);

    vmimtEndBlock();
}


