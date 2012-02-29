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
#include "armConfig.h"
#include "armCP.h"
#include "armDecode.h"
#include "armEmit.h"
#include "armExceptions.h"
#include "armFunctions.h"
#include "armMessage.h"
#include "armMorph.h"
#include "armMorphFunctions.h"
#include "armRegisters.h"
#include "armStructure.h"
#include "armSIMDVFP.h"
#include "armSIMDVFPRegisters.h"
#include "armUtils.h"
#include "armVariant.h"
#include "armVM.h"
#include "armFPConstants.h"

//
// Prefix for messages from this module
//
#define CPU_PREFIX "ARM_MORPH"


////////////////////////////////////////////////////////////////////////////////
// EXCEPTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Generic exception callback
//
#define EXCEPTION_FN(_NAME) void _NAME(armP arm, Uns32 thisPC)
typedef EXCEPTION_FN((*exceptionFn));

//
// Emit call to exception function
//
static void emitExceptionCall(armMorphStateP state, exceptionFn cb) {

    Uns32 bits = ARM_GPR_BITS;

    // emit call to exception routine
    armEmitArgProcessor(state);
    armEmitArgSimPC(state, bits);
    armEmitCall(state, (vmiCallFn)cb);

    // terminate the current block
    armEmitEndBlock();
}


////////////////////////////////////////////////////////////////////////////////
// UNIMPLEMENTED/UNDEFINED INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Called for an unimplemented instruction
//
static void ignoreSWI(armP arm, Uns32 thisPC) {

    vmiMessage("W", CPU_PREFIX"_BII",
        SRCREF_FMT "SWI instruction ignored in nopSVC compatability mode",
        SRCREF_ARGS(arm, thisPC)
    );
}


static void unimplemented(armP arm, Uns32 thisPC) {

    vmiMessage("F", CPU_PREFIX"_UII",
        SRCREF_FMT "unimplemented instruction",
        SRCREF_ARGS(arm, thisPC)
    );
}

//
// Default morpher callback for unimplemented instructions
//
static void emitUnimplemented(armMorphStateP state) {
    armEmitArgProcessor(state);
    armEmitArgSimPC(state, ARM_GPR_BITS);
    armEmitCall(state, (vmiCallFn)unimplemented);
    armEmitExit();
}

//
// Called for an undefined instruction
//
static void undefined(armP arm, Uns32 thisPC) {

    vmiMessage("E", CPU_PREFIX"_UDI",
        SRCREF_FMT "undefined instruction - enable simulated exceptions "
        "to continue simulation if this behavior is desired",
        SRCREF_ARGS(arm, thisPC)
    );
}

//
// Emit code for an undefined instruction
//
static void emitUndefined(armMorphStateP state, Bool setDEX) {
    if(state->arm->simEx) {
        Uns32 bits = ARM_GPR_BITS;

        // emit call to exception routine
        armEmitArgProcessor(state);
        armEmitArgSimPC(state, bits);
        armEmitArgUns32(state, setDEX);
        armEmitCall(state, (vmiCallFn)armUndefined);

        // terminate the current block
        armEmitEndBlock();
    } else {
        armEmitArgProcessor(state);
        armEmitArgSimPC(state, ARM_GPR_BITS);
        armEmitCall(state, (vmiCallFn)undefined);
        armEmitExit();
    }
}

//
// Is the instruction a VFP instruction?
//
static inline Bool isVFPInstr(armMorphStateP state) {
    return (state->attrs->iType == ARM_TY_VFP);
}

//
// Called for a VFP instruction when no VFP implemented on the variant
//
static void VFPNotImplemented(armP arm, Uns32 thisPC) {

    // Only issue message once
    if (!arm->warnVFP) {

        arm->warnVFP = 1;

        vmiMessage("W", CPU_PREFIX"_NVFP",
            SRCREF_FMT "VFP not implemented on this variant",
            SRCREF_ARGS(arm, thisPC)
        );
    }
}

//
// Is the instruction a SIMD instruction?
//
static inline Bool isSIMDInstr(armMorphStateP state) {
    return (state->attrs->iType == ARM_TY_SIMD);
}

//
// Called for a AdvSIMD instruction when no AdvSIMD implemented on the variant
//
static void SIMDNotImplemented(armP arm, Uns32 thisPC) {

    // Only issue message once
    if (!arm->warnAdvSIMD) {

        arm->warnAdvSIMD = 1;

        vmiMessage("W", CPU_PREFIX"_NSIMD",
            SRCREF_FMT "Advanced SIMD not implemented on this variant",
            SRCREF_ARGS(arm, thisPC)
        );
    }
}

//
// Called for an instruction that isn't supported on this variant
//
static void notVariantMessage(armP arm, Uns32 thisPC) {

    vmiMessage("W", CPU_PREFIX"_NSV",
        SRCREF_FMT "not supported on this variant",
        SRCREF_ARGS(arm, thisPC)
    );
}

//
// Morpher callback for instructions that are not valid on this processor
// variant
//
static ARM_MORPH_FN(emitNotVariant) {

    vmiCallFn messageFn = 0;

    if (isVFPInstr(state)) {
        messageFn = (vmiCallFn) VFPNotImplemented;
    } else if (isSIMDInstr(state)) {
        messageFn = (vmiCallFn) SIMDNotImplemented;
    } else if (state->arm->verbose) {
        messageFn = (vmiCallFn) notVariantMessage;
    }

    if (messageFn) {
        armEmitArgProcessor(state);
        armEmitArgSimPC(state, ARM_GPR_BITS);
        armEmitCall(state, messageFn);
    }

    // take UndefinedInstruction exception
    emitUndefined(state, False);
}

//
// Called for an instruction that isn't supported by the ISAR
//
static void notISARMessage(armP arm, Uns32 thisPC) {

    vmiMessage("W", CPU_PREFIX"_NSI",
        SRCREF_FMT "not supported by the ISAR",
        SRCREF_ARGS(arm, thisPC)
    );
}

//
// Morpher callback for instructions that are not valid according to ISAR
//
static ARM_MORPH_FN(emitNotISAR) {

    if (state->arm->verbose) {
        armEmitArgProcessor(state);
        armEmitArgSimPC(state, ARM_GPR_BITS);
        armEmitCall(state, (vmiCallFn) notISARMessage);
    }

    // take UndefinedInstruction exception
    emitUndefined(state, False);
}

//
// Called for an undefined coprocessor instruction
//
static void undefinedCPMessage(armP arm, Uns32 thisPC) {

    vmiMessage("W", CPU_PREFIX"_UCA",
        SRCREF_FMT "unsupported/disabled coprocessor instruction",
        SRCREF_ARGS(arm, thisPC)
    );
}

//
// Morpher callback for undefined coprocessor instructions
//
static ARM_MORPH_FN(emitUndefinedCP) {

    // generate assertion
    if(state->arm->verbose) {
        armEmitArgProcessor(state);
        armEmitArgSimPC(state, ARM_GPR_BITS);
        armEmitCall(state, (vmiCallFn)undefinedCPMessage);
    }

    // take UndefinedInstruction exception
    emitUndefined(state, False);
}


////////////////////////////////////////////////////////////////////////////////
// FLAGS
////////////////////////////////////////////////////////////////////////////////

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

const static vmiFlags flagsCOut = {
    ARM_CF_CONST,
    {
        [vmi_CF] = ARM_CF_CONST,
        [vmi_PF] = VMI_NOFLAG_CONST,
        [vmi_ZF] = VMI_NOFLAG_CONST,
        [vmi_SF] = VMI_NOFLAG_CONST,
        [vmi_OF] = VMI_NOFLAG_CONST
    }
};

// Macro accessors for flags
#define FLAGS_CIN  &flagsCIn
#define FLAGS_COUT &flagsCOut

//
// Return condition flags associated with an instruction, or null if no flags
// are applicable
//
static vmiFlagsCP getFlagsOrNull(armMorphStateP state) {
    return state->info.f ? state->attrs->flagsRW : 0;
}

//
// Return condition flags associated with an instruction, or FLAGS_CIN if no
// flags are applicable
//
static vmiFlagsCP getFlagsOrCIn(armMorphStateP state, vmiReg rd) {

    armP arm = state->arm;

    if(!state->info.f) {
        return state->attrs->flagsR;
    } else if(!VMI_REG_EQUAL(rd, ARM_PC)) {
        return state->attrs->flagsRW;
    } else if(CP_USE_CPUID(arm) && !ARM_ISAR(2, PSR_AR_instrs)) {
        emitNotVariant(state);
        return state->attrs->flagsR;
    } else {
        state->loadCPSR = True;
        return state->attrs->flagsR;
    }
}

//
// If the instruction sets flags from the shifter output, return FLAGS_COUT
//
static vmiFlagsCP getShifterCOut(armMorphStateP state, vmiReg rd) {
    if(!state->info.f) {
        return FLAGS_CIN;
    } else if(!state->attrs->shiftCOut) {
        return FLAGS_CIN;
    } else if(!VMI_REG_EQUAL(rd, ARM_PC)) {
        return FLAGS_COUT;
    } else {
        return FLAGS_CIN;
    }
}

//
// Do the flags specify that the shifter sets the carry out?
//
inline static Bool shifterSetsCOut(vmiFlagsCP flags) {
    return (flags==FLAGS_COUT);
}


////////////////////////////////////////////////////////////////////////////////
// REGISTER ACCESS UTILITIES
////////////////////////////////////////////////////////////////////////////////

//
// GPR Register access macros
//
#define GET_RD(_S, _R) getRD(_S, _S->info._R)
#define GET_RS(_S, _R) getRS(_S, _S->info._R)

// SIMD/VFP register access macros
#define GET_VFP_REG(_S, _R, _OFF, _ES) getVFPReg(_S, _S->info._R+_OFF, _ES)
#define GET_SIMD_REG(_S, _R, _OFF)           getSimdElement(_S, (_S)->info._R+(_OFF), 0,    ARM_SIMD_REG_BYTES)
#define GET_SIMD_EL(_S, _R, _OFF, _IDX, _ES) getSimdElement(_S, (_S)->info._R+(_OFF), _IDX, _ES)

//
// Return vmiReg for GPR with the passed regNum
// NOTE: PC is a temporary, others are true registers
//
static vmiReg getGPR(armMorphStateP state, Uns32 regNum) {
    return regNum==ARM_REG_PC ? ARM_PC : ARM_REG(regNum);
}

//
// Return vmiReg for target GPR with the passed regNum
//
static vmiReg getRD(armMorphStateP state, Uns32 regNum) {
    return getGPR(state, regNum);
}

//
// Return vmiReg for source GPR with the passed regNum
//
static vmiReg getRS(armMorphStateP state, Uns32 regNum) {

    vmiReg r = getGPR(state, regNum);

    // ensure PC is created on demand if required
    if(VMI_REG_EQUAL(r, ARM_PC)) {
        armEmitGetPC(state);
    }

    return r;
}

//
// Return vmiReg for SIMD/VFP register, addressed as a Byte, Half, Word or Double
//
static vmiReg getSimdReg(armMorphStateP state, Uns32 regNum, Uns32 ebytes) {

    armP   arm           = state->arm;
    vmiReg reg           = VMI_NOREG;
    Uns32  simdRegisters = SDFP_FIELD(arm, MVFR0, A_SIMD_Registers);
    Uns32  numRegs       = (simdRegisters == 1) ? ARM_VFP16_REG_NUM : ARM_SIMD_REG_NUM;

    // get the indicated register
    switch (ebytes) {
        case 1:  reg = ARM_BREG(regNum); break;
        case 2:  reg = ARM_HREG(regNum); break;
        case 4:  reg = ARM_WREG(regNum); break;
        case 8:  reg = ARM_DREG(regNum); break;
        default: VMI_ABORT("Invalid ebytes %d", ebytes);
    }

    if(
       (!ADVSIMD_PRESENT(arm) && !VFP_PRESENT(arm))    ||
       simdRegisters == 0                              ||
       (regNum*ebytes) >= (numRegs*ARM_SIMD_REG_BYTES)
    ) {

        // The indicated SIMD Register is not present
        emitUndefined(state, False);

    }

    return reg;
}

//
// Return vmiReg for VFP register, addressed as a Single or Double precision
//
static vmiReg getVFPReg(armMorphStateP state, Uns32 regNum, Uns32 ebytes) {

    VMI_ASSERT(ebytes==2 || ebytes==4 || ebytes == 8, "Invalid VFP ebytes %d", ebytes);
    return getSimdReg(state, regNum, ebytes);
}

//
// Return vmiReg for source or target SIMD element of ebytes bytes defined by
// Double word register index regNum and and element number index
//
static vmiReg getSimdElement(armMorphStateP state, Uns32 regNum, Uns32 index, Uns32 ebytes) {

    // Sanity check that index is within the range of a quadword register
    VMI_ASSERT(index < (16/ebytes), "Index %d out of range for ebytes %d", index, ebytes);

    return getSimdReg(state, (regNum*(ARM_SIMD_REG_BYTES/ebytes))+index, ebytes);
}

//
// Return vmiReg object for low half of 32-bit register
//
inline static vmiReg getR32Lo(vmiReg reg32) {
    return reg32;
}

//
// Return vmiReg object for high half of 32-bit register
//
inline static vmiReg getR32Hi(vmiReg reg32) {
    return VMI_REG_DELTA(reg32, sizeof(Uns16));
}

//
// Return vmiReg object for low half of 64-bit register
//
inline static vmiReg getR64Lo(vmiReg reg64) {
    return reg64;
}

//
// Return vmiReg object for high half of 64-bit register
//
inline static vmiReg getR64Hi(vmiReg reg64) {
    return VMI_REG_DELTA(reg64, sizeof(Uns32));
}

//
// Return vmiReg object for source register 'r' in the passed mode
//
static vmiReg getBankedRSMode(armMorphStateP state, armCPSRMode mode, Uns32 r) {

    armP        arm   = state->arm;
    armCPSRMode mode1 = GET_MODE(arm);
    armCPSRMode mode2 = armGetCPSRMode(mode);

    // get the normal storage location for the requested register in both
    // requested and current modes (note that this emits code to validate block
    // masks)
    vmiReg rs1 = armGetBankedRegMode(mode1, r);
    vmiReg rs2 = armGetBankedRegMode(mode2, r);

    if(mode1==mode2) {
        // requested mode is the current mode - value is in the current bank
        rs2 = getRS(state, r);
    } else if(VMI_REG_EQUAL(rs1, rs2)) {
        // no action if same register is used in current and requested mode
    } else if(VMI_ISNOREG(rs2)) {
        // if the requested mode register is not banked but the current mode
        // register is, the current value for the requested mode is actually
        // stored in the banked slot at the moment
        rs2 = rs1;
    }

    // return either the banked location or the base register
    return VMI_ISNOREG(rs2) ? getRS(state, r) : rs2;
}


////////////////////////////////////////////////////////////////////////////////
// INSTRUCTION SKIPPING UTILITIES
////////////////////////////////////////////////////////////////////////////////

//
// Save the skip label on the info structure
//
inline static void setSkipLabel(armMorphStateP state, vmiLabelP label) {
    state->skipLabel = label;
}

//
// Return the skip label on the info structure
//
inline static vmiLabelP getSkipLabel(armMorphStateP state) {
    return state->skipLabel;
}

//
// Insert a label
//
inline static void emitLabel(vmiLabelP label) {

    if(label) {
        armEmitInsertLabel(label);
    }
}


////////////////////////////////////////////////////////////////////////////////
// MISCELLANEOUS UTILITIES
////////////////////////////////////////////////////////////////////////////////

//
// Return the version of the instruction set implemented by the processor
//
inline static Uns32 getInstructionVersion(armP arm) {
    return ARM_INSTRUCTION_VERSION(arm->configInfo.arch);
}

//
// Return the constant value adjusted so that when added to the PC the result
// is always word aligned
//
inline static Uns32 alignConstWithPC(armMorphStateP state, Uns32 c) {
    return c - (state->info.thisPC & 2);
}

//
// return true if the vmiBinop op is a saturating op
//
static Bool saturatingBinop(vmiBinop op) {

    switch (op) {
        case vmi_ADDSQ:
        case vmi_SUBSQ:
        case vmi_RSUBSQ:
        case vmi_ADDUQ:
        case vmi_SUBUQ:
        case vmi_RSUBUQ:
        case vmi_SHLSQ:
        case vmi_SHLUQ:
            return True;
        default:
            break;
    }

    return False;
}

//
// return true if the vmiUnop op is a saturating op
//
static Bool saturatingUnop(vmiUnop op) {

    switch (op) {
        case vmi_NEGSQ:
        case vmi_ABSSQ:
            return True;
        default:
            break;
    }

    return False;
}

//
// return true if the vmiBinop op is a signed op
//
static Bool signedBinop(vmiBinop op) {

    switch (op) {
        case vmi_IMUL:
        case vmi_IDIV:
        case vmi_ADDSQ:
        case vmi_SUBSQ:
        case vmi_RSUBSQ:
        case vmi_ADDSH:
        case vmi_SUBSH:
        case vmi_RSUBSH:
        case vmi_ADDSHR:
        case vmi_SUBSHR:
        case vmi_RSUBSHR:
        case vmi_SAR:
        case vmi_SHLSQ:
        case vmi_SARR:
            return True;
        default:
            break;
    }

    return False;
}

////////////////////////////////////////////////////////////////////////////////
// TEMPORARIES
////////////////////////////////////////////////////////////////////////////////

//
// Get the current base of the temporary stack
//
inline static Uns32 getTempBase(armMorphStateP state) {
    return state->tempIdx;
}

//
// Set the base of the temporary stack
//
inline static void setTempBase(armMorphStateP state, Uns32 tempBase) {
    VMI_ASSERT(tempBase<ARM_TEMP_NUM, "Invalid tempbase %d", tempBase);
    state->tempIdx = tempBase;
}

//
// Get the current active temporary
//
inline static vmiReg getTemp(armMorphStateP state) {
    VMI_ASSERT(state->tempIdx<ARM_TEMP_NUM, "%s: no more temporaries", FUNC_NAME);
    return ARM_TEMP(state->tempIdx);
}

//
// Allocate a new 32-bit temporary
//
inline static vmiReg newTemp32(armMorphStateP state) {
    vmiReg temp = getTemp(state);
    state->tempIdx++;
    return temp;
}

//
// Free a 32-bit temporary
//
inline static void freeTemp32(armMorphStateP state) {
    VMI_ASSERT(state->tempIdx, "%s: temporary overflow", FUNC_NAME);
    state->tempIdx--;
}

//
// Allocate a new 64-bit temporary
//
inline static vmiReg newTemp64(armMorphStateP state) {
    vmiReg temp = newTemp32(state); newTemp32(state);
    return temp;
}

//
// Free a 64-bit temporary
//
inline static void freeTemp64(armMorphStateP state) {
    freeTemp32(state); freeTemp32(state);
}

//
// Allocate a new 128-bit temporary
//
inline static vmiReg newTemp128(armMorphStateP state) {
    vmiReg temp = newTemp64(state); newTemp64(state);
    return temp;
}

//
// Free a 128-bit temporary
//
inline static void freeTemp128(armMorphStateP state) {
    freeTemp64(state); freeTemp64(state);
}

//
// Allocate a new 256-bit temporary
//
inline static vmiReg newTemp256(armMorphStateP state) {
    vmiReg temp = newTemp128(state); newTemp128(state);
    return temp;
}

//
// Free a 256-bit temporary
//
inline static void freeTemp256(armMorphStateP state) {
    freeTemp128(state); freeTemp128(state);
}


////////////////////////////////////////////////////////////////////////////////
// TEMPORARY FLAGS
////////////////////////////////////////////////////////////////////////////////

//
// Return vmiFlags generating sign and overflow in temporary
//
static vmiFlags getSFOFFlags(vmiReg tf) {

    vmiFlags flags = VMI_NOFLAGS;

    // define overflow and sign flags
    vmiReg of = tf;
    vmiReg sf = VMI_REG_DELTA(of, 1);

    // define vmiFlags structure using the overflow and sign flags
    flags.f[vmi_SF] = sf;
    flags.f[vmi_OF] = of;

    // return vmiFlags structure
    return flags;
}

//
// Return vmiFlags generating overflow in temporary
//
static vmiFlags getOFFlags(vmiReg tf) {

    vmiFlags flags = VMI_NOFLAGS;

    // define vmiFlags structure using the overflow flag
    flags.f[vmi_OF] = tf;

    // return vmiFlags structure
    return flags;
}

//
// Return vmiFlags generating zero flag in temporary
//
static vmiFlags getZFFlags(vmiReg tf) {

    vmiFlags flags  = VMI_NOFLAGS;

    // define vmiFlags structure using the carry flag
    flags.f[vmi_ZF] = tf;

    // return vmiFlags structure
    return flags;
}

//
// Return vmiFlags generating and using carry in temporary
//
static vmiFlags getCFFlags(vmiReg tf) {

    vmiFlags flags = VMI_NOFLAGS;

    // define vmiFlags structure using the carry flag
    flags.cin       = tf;
    flags.f[vmi_CF] = tf;

    // return vmiFlags structure
    return flags;
}

////////////////////////////////////////////////////////////////////////////////
// CONDITIONAL INSTRUCTION EXECUTION
////////////////////////////////////////////////////////////////////////////////

//
// For conditions, this enumeration describes the circumstances under which the
// condition is satisfied
//
typedef enum armCondOpE {
    ACO_ALWAYS = 0,     // condition always True
    ACO_FALSE  = 1,     // condition satisfied if flag unset
    ACO_TRUE   = 2,     // condition satisfied if flag set
} armCondOp;

//
// For conditions, this structure describes a flag and a value for a match
//
typedef struct armCondS {
    vmiReg    flag;
    armCondOp op;
} armCond, *armCondP;

//
// Emit code to prepare a conditional operation and return an armCond structure
// giving the offset of a flag to compare against
//
static armCond emitPrepareCondition(armMorphStateP state) {

    const static armCond condTable[ARM_C_LAST] = {
        [ARM_C_EQ] = {ARM_ZF_CONST, ACO_TRUE  },    // ZF==1
        [ARM_C_NE] = {ARM_ZF_CONST, ACO_FALSE },    // ZF==0
        [ARM_C_CS] = {ARM_CF_CONST, ACO_TRUE  },    // CF==1
        [ARM_C_CC] = {ARM_CF_CONST, ACO_FALSE },    // CF==0
        [ARM_C_MI] = {ARM_NF_CONST, ACO_TRUE  },    // NF==1
        [ARM_C_PL] = {ARM_NF_CONST, ACO_FALSE },    // NF==0
        [ARM_C_VS] = {ARM_VF_CONST, ACO_TRUE  },    // VF==1
        [ARM_C_VC] = {ARM_VF_CONST, ACO_FALSE },    // VF==0
        [ARM_C_HI] = {ARM_HI_CONST, ACO_TRUE  },    // (CF==1) && (ZF==0)
        [ARM_C_LS] = {ARM_HI_CONST, ACO_FALSE },    // (CF==0) || (ZF==1)
        [ARM_C_GE] = {ARM_LT_CONST, ACO_FALSE },    // NF==VF
        [ARM_C_LT] = {ARM_LT_CONST, ACO_TRUE  },    // NF!=VF
        [ARM_C_GT] = {ARM_LE_CONST, ACO_FALSE },    // (ZF==0) && (NF==VF)
        [ARM_C_LE] = {ARM_LE_CONST, ACO_TRUE  },    // (ZF==1) || (NF!=VF)
        [ARM_C_AL] = {VMI_NOREG,    ACO_ALWAYS},    // always
        [ARM_C_NV] = {VMI_NOREG,    ACO_ALWAYS}     // always (historically never)
   };

    // get the table entry corresponding to the instruction condition
    armCond cond = condTable[state->info.cond];
    vmiReg  tf   = cond.flag;
    armP    arm  = state->arm;

    switch(state->info.cond) {

        case ARM_C_AL:
        case ARM_C_NV:
             // unconditional execution
             break;

        case ARM_C_EQ:
        case ARM_C_NE:
        case ARM_C_CS:
        case ARM_C_CC:
        case ARM_C_MI:
        case ARM_C_PL:
        case ARM_C_VS:
        case ARM_C_VC:
            // basic flags, always valid
            break;

        case ARM_C_GT:      // (ZF==0) && (NF==VF)
        case ARM_C_LE:      // (ZF==1) || (NF!=VF)
            // derived LE flag
            if(!arm->validLE) {
                arm->validLE = True;
                armEmitBinopRRR(state, 8, vmi_XOR, tf, ARM_NF, ARM_VF, 0);
                armEmitBinopRR(state, 8, vmi_OR, tf, ARM_ZF, 0);
            }
            break;

        case ARM_C_GE:      // NF==VF
        case ARM_C_LT:      // NF!=VF
            // derived LT flag
            if(!arm->validLT) {
                arm->validLT = True;
                armEmitBinopRRR(state, 8, vmi_XOR, tf, ARM_NF, ARM_VF, 0);
            }
            break;

        case ARM_C_HI:      // (CF==1) && (ZF==0)
        case ARM_C_LS:      // (CF==0) || (ZF==1)
            // derived HI flag
            if(!arm->validHI) {
                arm->validHI = True;
                armEmitBinopRRR(state, 8, vmi_ANDN, tf, ARM_CF, ARM_ZF, 0);
            }
            break;

        default:
            // not reached
            VMI_ABORT("%s: unimplemented condition", FUNC_NAME);
    }

    // return the condition description
    return cond;
}

//
// Create code to jump to a new label if the instruction is conditional
//
static vmiLabelP emitStartSkip(armMorphStateP state) {

    armCond   cond   = emitPrepareCondition(state);
    vmiLabelP doSkip = 0;

    if(cond.op!=ACO_ALWAYS) {
        doSkip = armEmitNewLabel();
        armEmitCondJumpLabel(cond.flag, cond.op==ACO_FALSE, doSkip);
    }

    return doSkip;
}

//
// Force all derived flag values to be regenerated
//
static void resetDerivedFlags(armMorphStateP state) {

    armP arm = state->arm;

    arm->validHI = False;
    arm->validLT = False;
    arm->validLE = False;
}


////////////////////////////////////////////////////////////////////////////////
// ARGUMENT GENERATION
////////////////////////////////////////////////////////////////////////////////

//
// Map from armShiftOp to vmiBinop
//
static vmiBinop mapShiftOp(armShiftOp so) {
    switch(so) {
        case ARM_SO_LSL: return vmi_SHL;
        case ARM_SO_LSR: return vmi_SHR;
        case ARM_SO_ASR: return vmi_SAR;
        case ARM_SO_ROR: return vmi_ROR;
        default: VMI_ABORT("%s: unimplemented case", FUNC_NAME); return 0;
    }
}

//
// Generate register argument by shifting register 'ra' by constant, placing
// result in register 't'
//
static void getShiftedRC(
    armMorphStateP state,
    vmiReg         t,
    vmiReg         ra,
    vmiFlagsCP     shiftCOut
) {
    Uns32    shift = state->info.c;
    vmiBinop op    = mapShiftOp(state->info.so);

    armEmitBinopRRC(state, ARM_GPR_BITS, op, t, ra, shift, shiftCOut);
}

//
// Generate register argument by shifting register 'ra' by register 'rs',
// placing result in register 't'
//
static void getShiftedRR(
    armMorphStateP state,
    vmiReg         t,
    vmiReg         ra,
    vmiReg         rs,
    vmiFlagsCP     shiftCOut
) {
    vmiBinop op = mapShiftOp(state->info.so);

    armEmitSetShiftMask();
    armEmitBinopRRR(state, ARM_GPR_BITS, op, t, ra, rs, shiftCOut);
}

//
// Generate register argument by rotate right of register 'ra' through carry
//
static void getExtendedR(
    armMorphStateP state,
    vmiReg         t,
    vmiReg         ra,
    vmiFlagsCP     shiftCOut
) {
    armEmitBinopRRC(state, ARM_GPR_BITS, vmi_RCR, t, ra, 1, shiftCOut);
}


////////////////////////////////////////////////////////////////////////////////
// UNOPS
////////////////////////////////////////////////////////////////////////////////

#define vmiFlagsCPU vmiFlagsCP __attribute__ ((unused))

//
// Macro for emission of declarations for generic unop
//
#define EMIT_UNOP_DECLS(_S)                         \
    vmiReg      rd        = GET_RD(_S, r1);         \
    vmiUnop     op        = _S->attrs->unop;        \
    vmiFlagsCP  opFlags   = getFlagsOrCIn(_S, rd);  \
    vmiFlagsCPU shiftCOut = getShifterCOut(_S, rd)

//
// Macro for emission of call implementing generic unop
//
#define EMIT_UNOP_CALL(_S, _F, _S1)                 \
    _F(_S, ARM_GPR_BITS, op, rd, _S1, opFlags)

//
// Macro for emission of generic unop with unshifted argument
//
#define EMIT_UNOP_NO_SHIFT(_S, _F, _S1)             \
    EMIT_UNOP_DECLS(_S);                            \
    EMIT_UNOP_CALL(_S, _F, _S1)

//
// Macro for emission of generic unop with shifted argument
//
#define EMIT_UNOP_SHIFT(_S, _T)                     \
    EMIT_UNOP_DECLS(_S);                            \
    _T;                                             \
    EMIT_UNOP_CALL(_S, armEmitUnopRR, rd);

//
// Unop with immediate
//
ARM_MORPH_FN(armEmitUnopI) {

    Uns32 c = state->info.c;

    // emit register/constant unop with unshifted argument
    EMIT_UNOP_NO_SHIFT(state, armEmitUnopRC, c);

    // set carry from bit 31 of rotated constant if constant rotation was
    // non-zero
    if(shifterSetsCOut(shiftCOut) && state->info.crotate) {
        armEmitMoveRC(state, 8, ARM_CF, (c & 0x80000000) ? 1 : 0);
    }
}

//
// Unop with register
//
ARM_MORPH_FN(armEmitUnopReg) {
    EMIT_UNOP_NO_SHIFT(state, armEmitUnopRR, GET_RS(state, r2));
}

//
// Unop with register shifted by immediate addressing mode
//
ARM_MORPH_FN(armEmitUnopRSI) {
    EMIT_UNOP_SHIFT(
        state,
        getShiftedRC(state, rd, GET_RS(state, r2), shiftCOut)
    );
}

//
// Unop with register shifted by register addressing mode
//
ARM_MORPH_FN(armEmitUnopRSR) {
    EMIT_UNOP_SHIFT(
        state,
        getShiftedRR(state, rd, GET_RS(state, r2), GET_RS(state, r3), shiftCOut)
    );
}

//
// Unop with register shifted by register addressing mode (Thumb variant)
//
ARM_MORPH_FN(armEmitUnopRSRT) {
    EMIT_UNOP_SHIFT(
        state,
        getShiftedRR(state, rd, GET_RS(state, r1), GET_RS(state, r2), shiftCOut)
    );
}

//
// Unop with rotate right with extend addressing mode
//
ARM_MORPH_FN(armEmitUnopRX) {
    EMIT_UNOP_SHIFT(
        state,
        getExtendedR(state, rd, GET_RS(state, r2), shiftCOut)
    );
}


////////////////////////////////////////////////////////////////////////////////
// BINOPS
////////////////////////////////////////////////////////////////////////////////

//
// Macro for emission of declarations for generic binop
//
#define EMIT_BINOP_DECLS(_S, _S1)                   \
    vmiReg      rd        = GET_RD(_S, r1);         \
    vmiReg      rs1       = GET_RS(_S, _S1);        \
    vmiBinop    op        = _S->attrs->binop;       \
    vmiFlagsCP  opFlags   = getFlagsOrCIn(_S, rd);  \
    vmiFlagsCPU shiftCOut = getShifterCOut(_S, rd)

//
// Macro for emission of call implementing generic binop
//
#define EMIT_BINOP_CALL(_S, _F, _S2)                \
    _F(_S, ARM_GPR_BITS, op, rd, rs1, _S2, opFlags)

//
// Macro for emission of generic binop with unshifted argument
//
#define EMIT_BINOP_NO_SHIFT(_S, _F, _S1, _S2)       \
    EMIT_BINOP_DECLS(_S, _S1);                      \
    EMIT_BINOP_CALL(_S, _F, _S2)

//
// Macro for emission of generic binop with shifted argument
//
#define EMIT_BINOP_SHIFT(_S, _S1, _RS2, _T)         \
    EMIT_BINOP_DECLS(_S, _S1);                      \
    vmiReg _RS2 = newTemp32(_S);                    \
    _T;                                             \
    EMIT_BINOP_CALL(_S, armEmitBinopRRR, _RS2);     \
    freeTemp32(_S)

//
// Binop with immediate
//
ARM_MORPH_FN(armEmitBinopI) {

    Uns32 c = state->info.c;

    // emit register/constant binop with unshifted argument
    EMIT_BINOP_NO_SHIFT(state, armEmitBinopRRC, r2, c);

    // set carry from bit 31 of rotated constant if constant rotation was
    // non-zero
    if(shifterSetsCOut(shiftCOut) && state->info.crotate) {
        armEmitMoveRC(state, 8, ARM_CF, (c & 0x80000000) ? 1 : 0);
    }
}

//
// Binop with three registers
//
ARM_MORPH_FN(armEmitBinopR) {
    EMIT_BINOP_NO_SHIFT(state, armEmitBinopRRR, r2, GET_RS(state, r3));
}

//
// Binop with two registers (Thumb variant)
//
ARM_MORPH_FN(armEmitBinopRT) {
    EMIT_BINOP_NO_SHIFT(state, armEmitBinopRRR, r1, GET_RS(state, r2));
}

//
// Binop with register and immediate (Thumb variant)
//
ARM_MORPH_FN(armEmitBinopIT) {
    EMIT_BINOP_NO_SHIFT(state, armEmitBinopRRC, r1, state->info.c);
}

//
// Binop with register, program counter and immediate (ADR)
//
ARM_MORPH_FN(armEmitBinopADR) {
    Uns32 c = state->attrs->negate ? -state->info.c : state->info.c;
    EMIT_BINOP_NO_SHIFT(state, armEmitBinopRRC, r2, alignConstWithPC(state, c));
}

//
// Binop with register shifted by immediate addressing mode
//
ARM_MORPH_FN(armEmitBinopRSI) {
    EMIT_BINOP_SHIFT(
        state, r2, rs2,
        getShiftedRC(state, rs2, GET_RS(state, r3), shiftCOut)
    );
}

//
// Binop with register shifted by register addressing mode
//
ARM_MORPH_FN(armEmitBinopRSR) {
    EMIT_BINOP_SHIFT(
        state, r2, rs2,
        getShiftedRR(state, rs2, GET_RS(state, r3), GET_RS(state, r4), shiftCOut)
    );
}

//
// Binop with rotate right with extend addressing mode
//
ARM_MORPH_FN(armEmitBinopRX) {
    EMIT_BINOP_SHIFT(
        state, r2, rs2,
        getExtendedR(state, rs2, GET_RS(state, r3), shiftCOut)
    );
}


////////////////////////////////////////////////////////////////////////////////
// CMPOPS
////////////////////////////////////////////////////////////////////////////////

//
// Macro for emission of declarations for generic cmpop
//
#define EMIT_CMPOP_DECLS(_S)                        \
    vmiReg      rd        = VMI_NOREG;              \
    vmiReg      rs1       = GET_RS(_S, r1);         \
    vmiBinop    op        = _S->attrs->binop;       \
    vmiFlagsCP  opFlags   = getFlagsOrCIn(_S, rd);  \
    vmiFlagsCPU shiftCOut = getShifterCOut(_S, rd)

//
// Macro for emission of call implementing generic cmpop
//
#define EMIT_CMPOP_CALL(_S, _F, _S2)                \
    _F(_S, ARM_GPR_BITS, op, rd, rs1, _S2, opFlags)

//
// Macro for emission of generic cmpop with unshifted argument
//
#define EMIT_CMPOP_NO_SHIFT(_S, _F, _S2)            \
    EMIT_CMPOP_DECLS(_S);                           \
    EMIT_CMPOP_CALL(_S, _F, _S2)

//
// Macro for emission of generic cmpop with shifted argument
//
#define EMIT_CMPOP_SHIFT(_S, _RS2, _T)              \
    EMIT_CMPOP_DECLS(_S);                           \
    vmiReg _RS2 = newTemp32(_S);                    \
    _T;                                             \
    EMIT_CMPOP_CALL(_S, armEmitBinopRRR, _RS2);     \
    freeTemp32(_S)

//
// Cmpop with immediate
//
ARM_MORPH_FN(armEmitCmpopI) {

    Uns32 c = state->info.c;

    // emit register/constant cmpop with unshifted argument
    EMIT_CMPOP_NO_SHIFT(state, armEmitBinopRRC, c);

    // set carry from bit 31 of rotated constant if constant rotation was
    // non-zero
    if(shifterSetsCOut(shiftCOut) && state->info.crotate) {
        armEmitMoveRC(state, 8, ARM_CF, (c & 0x80000000) ? 1 : 0);
    }
}

//
// Cmpop with register
//
ARM_MORPH_FN(armEmitCmpopR) {
    EMIT_CMPOP_NO_SHIFT(state, armEmitBinopRRR, GET_RS(state, r2));
}

//
// Cmpop with register shifted by immediate addressing mode
//
ARM_MORPH_FN(armEmitCmpopRSI) {
    EMIT_CMPOP_SHIFT(
        state, rs2,
        getShiftedRC(state, rs2, GET_RS(state, r2), shiftCOut)
    );
}

//
// Cmpop with register shifted by register addressing mode
//
ARM_MORPH_FN(armEmitCmpopRSR) {
    EMIT_CMPOP_SHIFT(
        state, rs2,
        getShiftedRR(state, rs2, GET_RS(state, r2), GET_RS(state, r3), shiftCOut)
    );
}

//
// Cmpop with rotate right with extend addressing mode
//
ARM_MORPH_FN(armEmitCmpopRX) {
    EMIT_CMPOP_SHIFT(
        state, rs2,
        getExtendedR(state, rs2, GET_RS(state, r2), shiftCOut)
    );
}


////////////////////////////////////////////////////////////////////////////////
// MULTIPLY AND DIVIDE INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// MUL instruction
//
ARM_MORPH_FN(armEmitMUL) {

    vmiFlagsCP flags = getFlagsOrNull(state);
    vmiReg     rd    = GET_RD(state, r1);
    vmiReg     rm    = GET_RS(state, r2);
    vmiReg     rs    = GET_RS(state, r3);

    armEmitBinopRRR(state, ARM_GPR_BITS, vmi_IMUL, rd, rm, rs, flags);
}

//
// Emit code for SDIV and UDIV instructions
//
ARM_MORPH_FN(armEmitDIV) {

    vmiBinop op = state->attrs->binop;
    vmiReg   rd = GET_RD(state, r1);
    vmiReg   rn = GET_RS(state, r2);
    vmiReg   rm = GET_RS(state, r3);

    // record the target of the divide instruction (in case of exception)
    armEmitMoveRC(state, 8, ARM_DIVIDE_TARGET, state->info.r1);
    armEmitBinopRRR(state, ARM_GPR_BITS, op, rd, rn, rm, 0);
}

//
// Emit code for MLA or MLS instructions
//
static void emitMLAMLS(armMorphStateP state, vmiBinop op) {

    vmiFlagsCP flags = getFlagsOrNull(state);
    vmiReg     rd    = GET_RD(state, r1);
    vmiReg     rm    = GET_RS(state, r2);
    vmiReg     rs    = GET_RS(state, r3);
    vmiReg     rn    = GET_RS(state, r4);
    vmiReg     t     = VMI_REG_EQUAL(rd, rn) ? getTemp(state) : rd;

    armEmitBinopRRR(state, ARM_GPR_BITS, vmi_IMUL, t, rm, rs, 0);
    armEmitBinopRRR(state, ARM_GPR_BITS, op, rd, t, rn, flags);
}

//
// MLA instruction
//
ARM_MORPH_FN(armEmitMLA) {
    emitMLAMLS(state, vmi_ADD);
}

//
// MLS instruction
//
ARM_MORPH_FN(armEmitMLS) {
    emitMLAMLS(state, vmi_RSUB);
}

//
// [SU]MULL instructions
//
ARM_MORPH_FN(armEmitMULL) {

    vmiBinop   op    = state->attrs->binop;
    vmiFlagsCP flags = getFlagsOrNull(state);
    vmiReg     rdlo  = GET_RD(state, r1);
    vmiReg     rdhi  = GET_RD(state, r2);
    vmiReg     rm    = GET_RS(state, r3);
    vmiReg     rs    = GET_RS(state, r4);

    armEmitMulopRRR(state, ARM_GPR_BITS, op, rdhi, rdlo, rm, rs, flags);
}

//
// [SU]MLAL instructions
//
ARM_MORPH_FN(armEmitMLAL) {

    Uns32      bits   = ARM_GPR_BITS;
    vmiBinop   op     = state->attrs->binop;
    vmiFlagsCP flags  = getFlagsOrNull(state);
    vmiReg     rdlo   = GET_RS(state, r1);
    vmiReg     rdhi   = GET_RS(state, r2);
    vmiReg     rm     = GET_RS(state, r3);
    vmiReg     rs     = GET_RS(state, r4);
    vmiReg     t      = newTemp64(state);
    vmiReg     t164lo = getR64Lo(t);
    vmiReg     t164hi = getR64Hi(t);

    // perform initial multiply, result in temporary t164lo/t164hi
    armEmitMulopRRR(state, bits, op, t164hi, t164lo, rm, rs, 0);

    if(VMI_REG_EQUAL(rdhi, getR64Hi(rdlo))) {

        // rdlo/rdhi are an adjacent pair
        armEmitBinopRR(state, bits*2, vmi_ADD, rdlo, t164lo, flags);

    } else {

        // rdlo/rdhi are not an adjacent pair
        vmiReg t      = newTemp64(state);
        vmiReg t264lo = getR64Lo(t);
        vmiReg t264hi = getR64Hi(t);

        // move rdlo/rdhi into adjacent pair
        armEmitMoveRR(state, bits, t264hi, rdhi);
        armEmitMoveRR(state, bits, t264lo, rdlo);

        // perform addition using adjacent pair
        armEmitBinopRR(state, bits*2, vmi_ADD, t264lo, t164lo, flags);

        // move to rdlo/rdhi from adjacent pair
        armEmitMoveRR(state, bits, rdhi, t264hi);
        armEmitMoveRR(state, bits, rdlo, t264lo);

        freeTemp64(state);
    }

    freeTemp64(state);
}


////////////////////////////////////////////////////////////////////////////////
// BRANCH INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Morph-time check whether the processor is in ThumbEE mode (may require run
// time validation)
//
static Bool inThumbEEModeMT(armP arm) {

    // validate Thumb/ThumbEE mode if required
    if(IN_THUMB_MODE(arm) && arm->checkThumbEE) {
        armEmitValidateBlockMask(ARM_BM_JAZ_EE);
    }

    // return True if in ThumbEE mode
    return IN_THUMB_EE_MODE(arm);
}

//
// Confirm at morph-time that the processor is in ThumbEE mode
//
static void requireThumbEEModeMT(armP arm) {

    Bool ok = inThumbEEModeMT(arm);

    VMI_ASSERT(ok, "ThumbEE mode required");
}

//
// Return link address from state
//
inline static Uns32 getStateLinkPC(armMorphStateP state, Uns32 linkPC) {
    return linkPC | (IN_THUMB_MODE(state->arm) ? 1 : 0);
}

//
// Fill armJumpInfo structure with information about a jump to a target address
//
static void seedJumpInfo(
    armJumpInfoP   ji,
    armMorphStateP state,
    Bool           isLink,
    Bool           isReturn,
    Bool           isRelative
) {
    ji->linkReg = isLink ? ARM_LR : VMI_NOREG;
    ji->hint    = isRelative ? vmi_JH_RELATIVE : vmi_JH_NONE;
    ji->hint   |= isReturn ? vmi_JH_RETURN : isLink ? vmi_JH_CALL : vmi_JH_NONE;
    ji->linkPC  = isLink ? getStateLinkPC(state, state->nextPC) : 0;
}

//
// Emit an explicit unconditional jump to target address
//
static void emitUncondJumpC(armMorphStateP state, Bool isLink, Bool switchMode) {

    // switch processor mode if required
    if(switchMode) {
        armEmitInterwork(state);
    }

    // get information about the jump
    armJumpInfo ji;
    seedJumpInfo(&ji, state, isLink, False, True);

    // do the jump
    armEmitUncondJump(state, &ji);
}

//
// Emit an explicit conditional jump to target address
//
static void emitCondJumpC(
    armMorphStateP state,
    vmiReg         tf,
    Bool           jumpIfTrue,
    Bool           isLink
) {
    // get information about the jump
    armJumpInfo ji;
    seedJumpInfo(&ji, state, isLink, False, True);

    // do the jump
    armEmitCondJump(state, &ji, tf, jumpIfTrue);
}

//
// Macro defining the body of a (possibly conditional) jump to target
//
#define COND_OR_UNCOND_BRANCH_BODY(_UNCOND_CB, _COND_CB)                    \
                                                                            \
    armCond cond   = emitPrepareCondition(state);                           \
    Bool    isLink = state->attrs->isLink;                                  \
                                                                            \
    if(cond.op==ACO_ALWAYS) {                                               \
        /* unconditional jump */                                            \
        _UNCOND_CB(state, isLink, False);                                   \
    } else {                                                                \
        /* conditional jump */                                              \
        _COND_CB(state, cond.flag, cond.op==ACO_TRUE, isLink);              \
    }                                                                       \

//
// Emit conditional branch to constant address, possibly with link
//
ARM_MORPH_FN(armEmitBranchC) {
    COND_OR_UNCOND_BRANCH_BODY(emitUncondJumpC, emitCondJumpC);
}

//
// Emit conditional branch to register address, possibly with link
//
ARM_MORPH_FN(armEmitBranchR) {

    vmiReg ra     = GET_RS(state, r1);
    Bool   isLink = state->attrs->isLink;

    // switch mode if LSB of target address implies a different mode
    armEmitInterworkLSB(state, ra);

    // get information about the jump
    armJumpInfo ji;
    seedJumpInfo(&ji, state, isLink, state->info.r1==ARM_REG_LR, False);

    // do the jump
    armEmitUncondJumpReg(state, &ji, ra);
}

//
// Emit code for BLX instruction (undefined in ThumbEE mode)
//
ARM_MORPH_FN(armEmitBLX) {

    if(inThumbEEModeMT(state->arm)) {
        emitUndefined(state, False);
    } else {
        emitUncondJumpC(state, state->attrs->isLink, True);
    }
}

//
// Emit code for MURAC BAA instruction
//
ARM_MORPH_FN(armEmitBAA) {
    // emit the BrArch instruction
    armEmitBrArch();
}

////////////////////////////////////////////////////////////////////////////////
// HALFWORD INSTRUCTIONS (IMPLEMENT AS BRANCHES)
////////////////////////////////////////////////////////////////////////////////

//
// Emit code for Thumb half-instruction BL_H10
//
ARM_MORPH_FN(armEmitBL_H10) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg lr   = getRD(state, ARM_REG_LR);
    vmiReg pc   = getRS(state, ARM_REG_PC);

    armEmitBinopRRC(state, bits, vmi_ADD, lr, pc, state->info.c, 0);
}

//
// Emit code for Thumb half-instruction BL_H11
//
ARM_MORPH_FN(armEmitBL_H11) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg lr   = getRD(state, ARM_REG_LR);
    vmiReg t    = getTemp(state);

    // derive target address
    armEmitBinopRRC(state, bits, vmi_ADD, t, lr, state->info.c, 0);

    // get information about the jump
    armJumpInfo ji;
    seedJumpInfo(&ji, state, True, False, True);

    // do the jump
    armEmitUncondJumpReg(state, &ji, t);
}

//
// Emit code for Thumb half-instruction BL_H01
//
ARM_MORPH_FN(armEmitBL_H01) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg lr   = getRD(state, ARM_REG_LR);
    vmiReg t    = getTemp(state);

    // derive target address
    armEmitBinopRRC(state, bits, vmi_ADD, t, lr, state->info.c, 0);
    armEmitBinopRC(state, bits, vmi_AND, t, ~3, 0);

    // switch processor mode
    armEmitInterwork(state);

    // get information about the jump
    armJumpInfo ji;
    seedJumpInfo(&ji, state, True, False, True);

    // do the jump
    armEmitUncondJumpReg(state, &ji, t);
}


////////////////////////////////////////////////////////////////////////////////
// THUMBEE INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Emit code to perform ThumbEE null check if required
//
static void emitNullCheckIfThumbEE(armMorphStateP state, vmiReg rb) {

    armP  arm      = state->arm;
    Bool  rbIsPC   = VMI_REG_EQUAL(rb, ARM_PC);
    Uns32 pageMask = MIN_PAGE_SIZE-1;

    if(rbIsPC && ((state->info.thisPC+ARM_PC_DELTA(arm)) & pageMask)) {

        // no possibility that the PC could be null unless it is ARM_PC_DELTA
        // bytes from the end of a page

    } else if(inThumbEEModeMT(arm)) {

        // get index of the base register and convert to a mask
        Uns32 teeBase;
        Bool  ok        = armGetGPRIndex(state, rb, &teeBase);
        Uns32 teeNZMask = (1<<teeBase);

        // sanity check the base registe was mapped successfully
        VMI_ASSERT(ok, "base register is not a GPR");

        // validate that this register has not already been zero-checked
        if(!(teeNZMask & arm->teeNZMask)) {

            // null check code must be inserted in ThumbEE mode (and omitted in
            // Thumb mode)
            Uns32     bits     = ARM_GPR_BITS;
            vmiLabelP noBranch = armEmitNewLabel();

            // add to the mask of registers that have already been checked
            // (unless this is the PC, which changes every instruction, or this
            // is a conditional instruction)
            if(!rbIsPC && (state->info.cond==ARM_C_AL)) {
                arm->teeNZMask |= teeNZMask;
            }

            // skip the check handler call if base is non-zero
            armEmitCompareRCJumpLabel(bits, vmi_COND_NE, rb, 0, noBranch);

            // branch to null handler
            armEmitArgProcessor(state);
            armEmitCall(state, (vmiCallFn)armCheckHandlerNull);

            // here if there was no branch
            emitLabel(noBranch);
        }
    }
}

//
// Emit code for a handler branch
//
static void emitHB(armMorphStateP state) {

    Uns32  bits    = ARM_GPR_BITS;
    vmiReg t       = getTemp(state);
    vmiReg TEEHBR  = ARM_CP_REG(CP_ID(TEEHBR));
    Bool   isLink  = state->attrs->isLink;
    Uns32  handler = state->info.handler*32;

    // calculate target address
    armEmitBinopRRC(state, bits, vmi_ADD, t, TEEHBR, handler, 0);

    // get information about the jump
    armJumpInfo ji;
    seedJumpInfo(&ji, state, isLink, False, True);

    // do the jump
    armEmitUncondJumpReg(state, &ji, t);
}

//
// Emit code for undefined ThumbEE instruction
//
ARM_MORPH_FN(armEmitUND_EE) {

    // add Thumb/ThumbEE check if required
    requireThumbEEModeMT(state->arm);

    emitUndefined(state, False);
}

//
// Emit code for HB/HBL instructions
//
ARM_MORPH_FN(armEmitHB) {

    // add Thumb/ThumbEE check if required
    requireThumbEEModeMT(state->arm);

    // do the handler branch
    emitHB(state);
}

//
// Emit code for HBP/HBLP instructions
//
ARM_MORPH_FN(armEmitHBP) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg r8   = getRD(state, ARM_REG_R8);

    // add Thumb/ThumbEE check if required
    requireThumbEEModeMT(state->arm);

    // move constant to R8
    armEmitMoveRC(state, bits, r8, state->info.c);

    // do the handler branch
    emitHB(state);
}

//
// Emit code for CHKA instructions
//
ARM_MORPH_FN(armEmitCHKA) {

    Uns32     bits     = ARM_GPR_BITS;
    vmiReg    rn       = GET_RS(state, r1);
    vmiReg    rm       = GET_RS(state, r2);
    vmiReg    tf       = getTemp(state);
    vmiLabelP noBranch = armEmitNewLabel();

    // add Thumb/ThumbEE check if required
    requireThumbEEModeMT(state->arm);

    // do the comparison
    armEmitCompareRR(state, bits, vmi_COND_BE, rn, rm, tf);

    // skip the check handler call if condition is False
    armEmitCondJumpLabel(tf, False, noBranch);

    // branch to array handler
    armEmitArgProcessor(state);
    armEmitCall(state, (vmiCallFn)armCheckHandlerArray);

    // here if there was no branch
    emitLabel(noBranch);
}

//
// Emit code for ENTERX/LEAVEX instructions
//
static void emitENTERLEAVEX(armMorphStateP state, vmiCondition cond) {

    vmiLabelP noSwitch = armEmitNewLabel();

    // skip the mode switch if the blockMask is already in the correct state
    armEmitTestRCJumpLabel(32, cond, ARM_BLOCK_MASK, ARM_BM_JAZ_EE, noSwitch);

    // switch mode
    armEmitEnterLeaveX(state);

    // here if there was no mode switch
    emitLabel(noSwitch);
}

//
// Emit code for ThumbEE ENTERX instruction
//
ARM_MORPH_FN(armEmitENTERX) {
    emitENTERLEAVEX(state, vmi_COND_NZ);
}

//
// Emit code for ThumbEE LEAVEX instruction
//
ARM_MORPH_FN(armEmitLEAVEX) {
    emitENTERLEAVEX(state, vmi_COND_Z);
}


////////////////////////////////////////////////////////////////////////////////
// MISCELLANEOUS INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Emit call to perform CLZ
//
ARM_MORPH_FN(armEmitCLZ) {

    vmiReg rd = GET_RD(state, r1);
    vmiReg rm = GET_RS(state, r2);

    armEmitUnopRR(state, ARM_GPR_BITS, vmi_CLZ, rd, rm, 0);
}

//
// Emit call to perform BKPT
//
ARM_MORPH_FN(armEmitBKPT) {
    emitExceptionCall(state, armBKPT);
}

//
// Emit call to perform SWI
//
ARM_MORPH_FN(armEmitSWI) {
    if(state->arm->compatMode==COMPAT_CODE_SOURCERY) {
        armEmitArgProcessor(state);
        armEmitArgSimPC(state, ARM_GPR_BITS);
        armEmitCall(state, (vmiCallFn)ignoreSWI);
    } else {
        emitExceptionCall(state, armSWI);
    }
}


////////////////////////////////////////////////////////////////////////////////
// SPECIAL PURPOSE REGISTER ACCESS INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Callback function for MSR value
//
#define PSR_VALUE_FN(_NAME) void _NAME(armMorphStateP state)
typedef PSR_VALUE_FN((*PSRValueFn));

//
// Callback function to update CPSR/SPSR
//
#define PSR_WRITE_FN(_NAME) void _NAME(armP arm, Uns32 value, Uns32 mask)
typedef PSR_WRITE_FN((PSRWriteFn));

//
// Emit code to load rd with contents of CPSR
//
static void emitReadCPSR(armMorphStateP state, vmiReg rd, Uns32 readMask) {

    Uns32 bits = ARM_GPR_BITS;

    // get the raw CPSR value
    armEmitArgProcessor(state);
    armEmitCallResult(state, (vmiCallFn)armReadCPSR, bits, rd);

    // mask it if required
    if(readMask != -1) {
        armEmitBinopRC(state, bits, vmi_AND, rd, readMask, 0);
    }
}

//
// Emit code to save value to CPSR/SPSR
//
static void emitWritePSR(
    armMorphStateP state,
    PSRValueFn     valueCB,
    PSRWriteFn     writeCB,
    Uns32          writeMask
) {
    // emit call to update PSR
    armEmitArgProcessor(state);
    valueCB(state);
    armEmitArgUns32(state, writeMask);
    armEmitCall(state, (vmiCallFn)writeCB);

    // if CPSR flags are updated by this write then derived flag values may no
    // longer be relied upon
    if((writeCB==armWriteCPSR) && (writeMask&CPSR_FLAGS)) {
        resetDerivedFlags(state);
    }
}

//
// Emit code for MRS using CPSR
//
ARM_MORPH_FN(armEmitMRSC) {
    emitReadCPSR(state, GET_RD(state, r1), armGetReadMaskCPSR(state->arm));
}

//
// If the processor is not in user mode, validate that SPSR is either CPSR
// (supervisor mode) or a true register (all other non-user modes)
//
static void validateSPSR(armP arm) {
    if(!IN_USER_MODE(arm)) {
        armEmitValidateBlockMask(ARM_BM_R13_R14_BASE);
    }
}

//
// Emit code for MRS using SPSR
//
ARM_MORPH_FN(armEmitMRSS) {

    armP   arm  = state->arm;
    Uns32  bits = ARM_GPR_BITS;
    vmiReg rd   = GET_RD(state, r1);

    // validate that SPSR is either a true register or mapped to CPSR
    validateSPSR(arm);

    // get SPSR register (or CPSR in modes with no SPSR)
    if(IN_BASE_MODE(arm)) {
        emitReadCPSR(state, rd, armGetReadMaskCPSR(arm));
    } else {
        armEmitMoveRR(state, bits, rd, ARM_SPSR);
    }
}

//
// Given a field mask, return a mask of bits that can be written using that
// field mask
//
static Uns32 getPSRWriteMask(armMorphStateP state, Uns32 fieldMask) {

    Uns32 writeMask = 0;

    // get writable bits implied by instruction
    if(fieldMask & ARM_SR_C) {writeMask |= 0x000000ff;}
    if(fieldMask & ARM_SR_X) {writeMask |= 0x0000ff00;}
    if(fieldMask & ARM_SR_S) {writeMask |= 0x00ff0000;}
    if(fieldMask & ARM_SR_F) {writeMask |= 0xff000000;}

    return writeMask;
}

//
// Emit immediate value argument for MSR
//
static PSR_VALUE_FN(emitMSRPSRIArg) {
    armEmitArgUns32(state, state->info.c);
}

//
// Emit register value argument for MSR
//
static PSR_VALUE_FN(emitMSRPSRRArg) {
    armEmitArgReg(state, ARM_GPR_BITS, GET_RS(state, r2));
}

//
// Emit code for MSR, internal routine
//
static void emitMSRPSRInt(
    armMorphStateP state,
    PSRValueFn     valueCB,
    PSRWriteFn     writeCB,
    Uns32          writeMask,
    Bool           endBlock
) {
    // mask writable bits with selected bits
    writeMask &= getPSRWriteMask(state, state->info.r1);

    // bo action unless some bits are selected for update
    if(writeMask) {

        // update PSR register
        emitWritePSR(state, valueCB, writeCB, writeMask);

        // terminate the current code block if write could have caused mode
        // switch or switched load/store endianess
        if(endBlock && (writeMask&(CPSR_MODE|CPSR_ENDIAN))) {
            armEmitEndBlock();
        }
    }
}

//
// Emit code for MSR using CPSR, immediate value
//
ARM_MORPH_FN(armEmitMSRCI) {
    Uns32 writeMask = armGetWriteMaskCPSR(state->arm, False);
    emitMSRPSRInt(state, emitMSRPSRIArg, armWriteCPSR, writeMask, True);
}

//
// Emit code for MSR using CPSR, register value
//
ARM_MORPH_FN(armEmitMSRCR) {
    Uns32 writeMask = armGetWriteMaskCPSR(state->arm, False);
    emitMSRPSRInt(state, emitMSRPSRRArg, armWriteCPSR, writeMask, True);
}

//
// Emit code for MSR using SPSR, immediate value
//
ARM_MORPH_FN(armEmitMSRSI) {

    armP arm = state->arm;

    // validate that SPSR is either a true register or mapped to CPSR
    validateSPSR(arm);

    if(!IN_BASE_MODE(arm)) {
        Uns32 writeMask = armGetWriteMaskSPSR(arm);
        emitMSRPSRInt(state, emitMSRPSRIArg, armWriteSPSR, writeMask, False);
    }
}

//
// Emit code for MSR using SPSR, register value
//
ARM_MORPH_FN(armEmitMSRSR) {

    armP arm = state->arm;

    // validate that SPSR is either a true register or mapped to CPSR
    validateSPSR(arm);

    if(!IN_BASE_MODE(arm)) {
        Uns32 writeMask = armGetWriteMaskSPSR(arm);
        emitMSRPSRInt(state, emitMSRPSRRArg, armWriteSPSR, writeMask, False);
    }
}

//
// Emit immediate value argument for MSR
//
static PSR_VALUE_FN(emitLoadCPSRFromSPSRArg) {
    armEmitArgReg(state, ARM_GPR_BITS, ARM_SPSR);
}

//
// Emit code to load CPSR from SPSR of current state
//
static void emitLoadCPSRFromSPSR(armMorphStateP state) {

    armP arm = state->arm;

    // action only required if in a processor mode with valid SPSR
    if(!IN_BASE_MODE(arm)) {
        Uns32 writeMask = armGetWriteMaskCPSR(arm, True);
        emitWritePSR(state, emitLoadCPSRFromSPSRArg, armWriteCPSR, writeMask);
    }
}


////////////////////////////////////////////////////////////////////////////////
// HINT INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Emit NOP
//
ARM_MORPH_FN(armEmitNOP) {
    // no action
}

//
// Emit code to wait for an event
//
ARM_MORPH_FN(armEmitWFE) {

    vmiLabelP noWait = armEmitNewLabel();
    vmiLabelP done   = armEmitNewLabel();

    // don't stop if the event is already registered
    armEmitCompareRCJumpLabel(8, vmi_COND_Z, ARM_EVENT, 1, noWait);

    // halt the processor at the end of this instruction
    armEmitHalt(AD_WFE);

    // jump to the instruction end
    armEmitUncondJumpLabel(done);

    // here if event is already registered
    emitLabel(noWait);

    // clear down event register
    armEmitMoveRC(state, 8, ARM_EVENT, 0);

    // here on completion
    emitLabel(done);
}

//
// Emit code to wait for an interrupt
//
ARM_MORPH_FN(armEmitWFI) {
    armEmitHalt(AD_WFI);
}

//
// Emit code to send an event
//
ARM_MORPH_FN(armEmitSEV) {

    // action depends on whether this is a multiprocessor
    if(!state->arm->parent) {

        // uniprocessor - set the event register on this processor only
        armEmitMoveRC(state, 8, ARM_EVENT, 1);

    } else {

        // multiprocessor - signal event on all processors in the cluster
        armEmitArgProcessor(state);
        armEmitCall(state, (vmiCallFn)armDoSEV);
    }
}


////////////////////////////////////////////////////////////////////////////////
// BANKED REGISTERS
////////////////////////////////////////////////////////////////////////////////

//
// Macro returning base register (and validating block mode)
//
#define GET_BASE_REG(_BM) \
    armEmitValidateBlockMask(_BM);  \
    return VMI_NOREG

//
// Macro returning banked register (and validating block mode)
//
#define GET_BANKED_REG(_BM, _N, _SET) \
    armEmitValidateBlockMask(_BM);  \
    return ARM_BANK_REG(_N, _SET)

//
// If the register index specifies a banked register, return a vmiReg structure
// for the banked register; otherwise, return VMI_NOREG. Also, validate the
// current block mode for registers r8 to r14.
//
vmiReg armGetBankedRegMode(armCPSRMode mode, Uns32 r) {

    switch(mode) {

        case ARM_CPSR_FIQ:
            switch(r) {
                case 8:  GET_BANKED_REG(ARM_BM_R8_R12_FIQ,       8,  fiq);
                case 9:  GET_BANKED_REG(ARM_BM_R8_R12_FIQ,       9,  fiq);
                case 10: GET_BANKED_REG(ARM_BM_R8_R12_FIQ,       10, fiq);
                case 11: GET_BANKED_REG(ARM_BM_R8_R12_FIQ,       11, fiq);
                case 12: GET_BANKED_REG(ARM_BM_R8_R12_FIQ,       12, fiq);
                case 13: GET_BANKED_REG(ARM_BM_R13_R14_SPSR_FIQ, 13, fiq);
                case 14: GET_BANKED_REG(ARM_BM_R13_R14_SPSR_FIQ, 14, fiq);
                default: break;
            }
            break;

        case ARM_CPSR_IRQ:
            switch(r) {
                case 8:  GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 9:  GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 10: GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 11: GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 12: GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 13: GET_BANKED_REG(ARM_BM_R13_R14_SPSR_IRQ, 13, irq);
                case 14: GET_BANKED_REG(ARM_BM_R13_R14_SPSR_IRQ, 14, irq);
                default: break;
            }
            break;

        case ARM_CPSR_SUPERVISOR:
            switch(r) {
                case 8:  GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 9:  GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 10: GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 11: GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 12: GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 13: GET_BANKED_REG(ARM_BM_R13_R14_SPSR_SVC, 13, svc);
                case 14: GET_BANKED_REG(ARM_BM_R13_R14_SPSR_SVC, 14, svc);
                default: break;
            }
            break;

        case ARM_CPSR_ABORT:
            switch(r) {
                case 8:  GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 9:  GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 10: GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 11: GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 12: GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 13: GET_BANKED_REG(ARM_BM_R13_R14_SPSR_ABT, 13, abt);
                case 14: GET_BANKED_REG(ARM_BM_R13_R14_SPSR_ABT, 14, abt);
                default: break;
            }
            break;

        case ARM_CPSR_UNDEFINED:
            switch(r) {
                case 8:  GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 9:  GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 10: GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 11: GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 12: GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 13: GET_BANKED_REG(ARM_BM_R13_R14_SPSR_UND, 13, und);
                case 14: GET_BANKED_REG(ARM_BM_R13_R14_SPSR_ABT, 14, und);
                default: break;
            }
            break;

        default:
            switch(r) {
                case 8:  GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 9:  GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 10: GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 11: GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 12: GET_BASE_REG(ARM_BM_R8_R12_BASE);
                case 13: GET_BASE_REG(ARM_BM_R13_R14_BASE);
                case 14: GET_BASE_REG(ARM_BM_R13_R14_BASE);
                default: break;
            }
            break;
    }

    return VMI_NOREG;
}

//
// If 'userRegs' is True and the register index specifies a banked register,
// return a vmiReg structure for the banked register; otherwise, return
// VMI_NOREG.
//
static vmiReg getBankedReg(armMorphStateP state, Uns32 r, Bool userRegs) {
    if(userRegs) {
        return armGetBankedRegMode(GET_MODE(state->arm), r);
    } else {
        return VMI_NOREG;
    }
}

//
// Get target register, possibly from bank
//
static vmiReg getBankedRD(armMorphStateP state, Uns32 r, Bool userRegs) {
    vmiReg rd = getBankedReg(state, r, userRegs);
    return VMI_ISNOREG(rd) ? getRD(state, r) : rd;
}

//
// Get source register, possibly from bank
//
static vmiReg getBankedRS(armMorphStateP state, Uns32 r, Bool userRegs) {
    vmiReg rs = getBankedReg(state, r, userRegs);
    return VMI_ISNOREG(rs) ? getRS(state, r) : rs;
}


////////////////////////////////////////////////////////////////////////////////
// LOAD AND STORE INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// This indicates that the register being loaded is also the base in an LDM
//
#define LDM_BASE_REG -1

//
// If this is an STM with the base register in the list and also writeback, is
// the value written the *final* value of the base register?
//
#define STM_WB_BASE_FINAL True

//
// Callback function for load/store
//
#define LOAD_STORE_FN(_NAME) void _NAME( \
    armMorphStateP state,   \
    vmiReg         base,    \
    Int32          offset   \
)
typedef LOAD_STORE_FN((*loadStoreFn));

//
// Callback function for one register of a load/store multiple
//
#define LOAD_STORE_M_FN(_NAME) void _NAME( \
    armMorphStateP state,           \
    vmiReg         base,            \
    Int32          offset,          \
    Uns32          r,               \
    Bool           isWBNotFirst,    \
    Int32          frameDelta,      \
    Bool           userRegs         \
)
typedef LOAD_STORE_M_FN((*loadStoreMFn));

//
// Should increment/decrement be performed before access?
//
inline static Bool doBefore(armMorphStateP state) {
    armIncDec incDec = state->info.incDec;
    return ((incDec==ARM_ID_IB) || (incDec==ARM_ID_DA));
}

//
// Does load/store multiple increment?
//
inline static Bool doIncrement(armMorphStateP state) {
    return state->info.incDec & ARM_ID_I;
}

//
// Return the step to apply before load/store multiple
//
inline static Int32 getStepBefore(armMorphStateP state, Uns32 bytes) {
    return doBefore(state) ? bytes : 0;
}

//
// Return the step to apply after load/store multiple
//
inline static Int32 getStepAfter(armMorphStateP state, Uns32 bytes) {
    return doBefore(state) ? 0 : bytes;
}

//
// Return frame size for a load/store multiple
//
static Uns32 getFrameSize(armMorphStateP state) {

    Uns32 rList = state->info.rList;
    Uns32 mask;
    Uns32 r;
    Int32 size = 0;

    for(r=0, mask=1; r<ARM_GPR_NUM; r++, mask<<=1) {
        if(rList&mask) {
            size += ARM_GPR_BYTES;
        }
    }

    return size;
}

//
// Is the base register overwritten by a load multiple?
//
static Bool baseIsLoaded(armMorphStateP state, Bool isLoad, Bool userRegs) {

    Uns32 r      = state->info.r1;
    Uns32 rmMask = (1<<r);

    return (
        // operation must be an LDM
        isLoad &&
        // base register must be in loaded list
        (state->info.rList & rmMask) &&
        // loaded register must be in the current bank
        VMI_ISNOREG(getBankedReg(state, r, userRegs))
    );
}

//
// Is the base register not the highest (first) register in the list?
//
inline static Bool baseIsNotFirst(Uns32 rList, Uns32 rBase) {
    return rList & ((1<<rBase)-1);
}

//
// Perform iteration for a load/store multiple instruction, calling the passed
// callback for each access
//
static void emitLoadStoreMultiple(
    armMorphStateP state,
    loadStoreMFn   cb,
    Bool           isLoad,
    Bool           userRegs
) {
    Uns32  bits       = ARM_GPR_BITS;
    Uns32  rList      = state->info.rList;
    Uns32  rBase      = state->info.r1;
    Bool   baseLoaded = baseIsLoaded(state, isLoad, userRegs);
    vmiReg base       = GET_RS(state, r1);
    Bool   increment  = doIncrement(state);
    Int32  stepBefore = getStepBefore(state, ARM_GPR_BYTES);
    Int32  stepAfter  = getStepAfter (state, ARM_GPR_BYTES);
    Uns32  frameSize  = getFrameSize(state);
    Int32  offset     = increment ? 0 : -frameSize;
    Int32  frameDelta = increment ? frameSize : -frameSize;
    Bool   isNotFirst = state->info.wb && baseIsNotFirst(rList, rBase);
    Uns32  mask;
    Uns32  r;

    // base register must be null-checked in ThumbEE mode
    emitNullCheckIfThumbEE(state, base);

    // load or store registers
    for(r=0, mask=1; r<ARM_GPR_NUM; r++, mask<<=1) {
        if(rList&mask) {
            Bool isWBNotFirst = isNotFirst && (r==rBase);
            Uns32 rt = (baseLoaded && (r==rBase)) ? LDM_BASE_REG : r;
            offset += stepBefore;
            cb(state, base, offset, rt, isWBNotFirst, frameDelta, userRegs);
            offset += stepAfter;
        }
    }

    // perform base register update if required
    if(baseLoaded) {
        vmiReg rn = GET_RD(state, r1);
        armEmitMoveRR(state, bits, rn, getTemp(state));
    } else if(state->info.wb) {
        vmiReg rn = GET_RD(state, r1);
        armEmitBinopRRC(state, bits, vmi_ADD, rn, base, frameDelta, 0);
    }
}

//
// Macro to emit one register load for LDM
//
#define EMIT_LDM_REG(_S, _BASE, _OFFSET, _R, _USERREGS)  {                           \
                                                                                     \
    Uns32  bits = ARM_GPR_BITS;                                                      \
    vmiReg rd   = (_R==LDM_BASE_REG) ? getTemp(_S) : getBankedRD(_S, _R, _USERREGS); \
                                                                                     \
    armEmitLoadRRO(_S, bits, bits, _OFFSET, rd, _BASE, False, True);                 \
}

//
// Emit one register load for LDM
//
static LOAD_STORE_M_FN(emitLDMCB) {
    EMIT_LDM_REG(state, base, offset, r, userRegs);
}

//
// If the source register is the PC, adjust to a 12-byte offset if required
//
static vmiReg emitPCDelta(armMorphStateP state, Uns32 r, vmiReg rs) {

    if((r==ARM_REG_PC) && state->arm->configInfo.STRoffsetPC12) {
        vmiReg t = getTemp(state);
        armEmitBinopRRC(state, ARM_GPR_BITS, vmi_ADD, t, rs, 4, 0);
        return t;
    } else {
        return rs;
    }
}

//
// Emit one register store for STM
//
static LOAD_STORE_M_FN(emitSTMCB) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rs   = getBankedRS(state, r, userRegs);

    // write the final value of the base register if it is in the register list
    if(STM_WB_BASE_FINAL && isWBNotFirst) {
        rs = getTemp(state);
        armEmitBinopRRC(state, bits, vmi_ADD, rs, base, frameDelta, 0);
    }

    // if source register is PC, allow for store offset adjustment
    rs = emitPCDelta(state, r, rs);

    // do the store
    armEmitStoreRRO(state, bits, offset, base, rs);
}

//
// Emit code for LDM (1)
//
ARM_MORPH_FN(armEmitLDM1) {
    emitLoadStoreMultiple(state, emitLDMCB, True, False);
}

//
// Emit code for LDM (2)
//
ARM_MORPH_FN(armEmitLDM2) {
    emitLoadStoreMultiple(state, emitLDMCB, True, True);
}

//
// Emit code for LDM (3)
//
ARM_MORPH_FN(armEmitLDM3) {
    emitLoadStoreMultiple(state, emitLDMCB, True, False);
    state->loadCPSR = True;
}

//
// Emit code for STM (1)
//
ARM_MORPH_FN(armEmitSTM1) {
    emitLoadStoreMultiple(state, emitSTMCB, False, False);
}

//
// Emit code for STM (2)
//
ARM_MORPH_FN(armEmitSTM2) {
    emitLoadStoreMultiple(state, emitSTMCB, False, True);
}

//
// Should the LDR instruction do writeback?
//
inline static Bool doLDRWriteBack(armMorphStateP state) {
    return (state->info.wb && (state->info.r1!=state->info.r2));
}

//
// Is translation required? (LDRT, STRT)
//
inline static Bool doTranslate(armMorphStateP state) {
    return (state->info.tl && IN_PRIV_MMU_MPU_MODE(state->arm));
}

//
// If this is an LDRT/STRT instruction and we are currently in privileged mode
// with TLB enabled, emit code to switch to the user mode data memDomain
//
static void emitTranslateOn(armMorphStateP state) {
    if(doTranslate(state)) {
        armEmitArgProcessor(state);
        armEmitCall(state, (vmiCallFn)armVMSetUserPrivilegedModeDataDomain);
    }
}

//
// If this is after a LDRT/STRT instruction and we are currently in privileged
// mode with TLB enabled, emit code to switch back to the privileged mode data
// memDomain. Note that this code is not executed if the prior access causes an
// exception; in this case, mode is restored in armDataAbort.
//
static void emitTranslateOff(armMorphStateP state) {
    if(doTranslate(state)) {
        armEmitArgProcessor(state);
        armEmitCall(state, (vmiCallFn)armVMRestoreNormalDataDomain);
    }
}

//
// Emit code for LDR variant
//
static LOAD_STORE_FN(emitLDR) {

    Uns32  memBits = state->info.sz*8;
    Bool   isLong  = (memBits==64);
    vmiReg rd      = GET_RD(state, r1);
    Bool   xs      = state->info.xs;

    // start translation (LDRT, STRT)
    emitTranslateOn(state);

    // emit load
    if(isLong) {

        vmiReg rdH = GET_RD(state, r4);
        vmiReg rt  = getTemp(state);

        armEmitLoadRRRO(state, memBits, offset, rd, rdH, base, rt, xs, False);

    } else {

        armEmitLoadRRO(state, ARM_GPR_BITS, memBits, offset, rd, base, xs, False);
    }

    // end translation (LDRT, STRT)
    emitTranslateOff(state);
}

//
// Emit code for STR variant
//
static LOAD_STORE_FN(emitSTR) {

    Uns32  memBits = state->info.sz*8;
    Bool   isLong  = (memBits==64);
    Uns32  r1      = state->info.r1;
    vmiReg rs      = getRS(state, r1);

    // if source register is PC, allow for store offset adjustment
    rs = emitPCDelta(state, r1, rs);

    // start translation (LDRT, STRT)
    emitTranslateOn(state);

    // emit store
    if(isLong) {

        Uns32  r4  = state->info.r4;
        vmiReg rsH = getRS(state, r4);

        armEmitStoreRRRO(state, memBits, offset, base, rs, rsH);

    } else {

        armEmitStoreRRO(state, memBits, offset, base, rs);
    }

    // end translation (LDRT, STRT)
    emitTranslateOff(state);
}

//
// Emit code for LDR/STR with immediate offset
//
static void emitLDRSTRI(armMorphStateP state, loadStoreFn cb, Bool align) {

    Uns32  bits      = ARM_GPR_BITS;
    vmiReg base      = GET_RS(state, r2);
    Int32  offset    = state->info.c;
    Int32  memOffset = state->info.pi ? 0 : offset;

    // align the constant if the base register is the program counter (has
    // effect for PC-relative Thumb load instructions only)
    if(align && VMI_REG_EQUAL(base, ARM_PC)) {
        memOffset = alignConstWithPC(state, memOffset);
    }

    // emit register load or store
    cb(state, base, memOffset);

    // do writeback if required
    if(doLDRWriteBack(state)) {
        vmiReg ra = GET_RD(state, r2);
        armEmitBinopRC(state, bits, vmi_ADD, ra, offset, 0);
    }
}

//
// Emit code for LDR/STR variant
//
static void emitLDRSTRInt(armMorphStateP state, vmiReg offset, loadStoreFn cb) {

    Uns32    bits   = ARM_GPR_BITS;
    vmiReg   base   = GET_RS(state, r2);
    vmiReg   t      = newTemp32(state);
    vmiReg   memReg = state->info.pi ? base : t;
    vmiBinop op     = state->info.u ? vmi_ADD : vmi_SUB;

    // calculate incremented address
    armEmitBinopRRR(state, bits, op, t, base, offset, 0);

    // emit register load
    cb(state, memReg, 0);

    // do writeback if required
    if(doLDRWriteBack(state)) {
        vmiReg ra = GET_RD(state, r2);
        armEmitMoveRR(state, bits, ra, t);
    }

    freeTemp32(state);
}

//
// Emit code for LDR/STR with register offset
//
static void emitLDRSTRR(armMorphStateP state, loadStoreFn cb) {

    vmiReg rm = GET_RS(state, r3);

    emitLDRSTRInt(state, rm, cb);
}

//
// Emit code for LDR/STR with scaled register offset (LSL, LSR, ASR, ROR)
//
static void emitLDRSTRRSI(armMorphStateP state, loadStoreFn cb) {

    vmiReg t  = newTemp32(state);
    vmiReg rm = GET_RS(state, r3);

    getShiftedRC(state, t, rm, FLAGS_CIN);

    emitLDRSTRInt(state, t, cb);

    freeTemp32(state);
}

//
// Emit code for LDR/STR with immediate offset (RRX)
//
static void emitLDRSTRRX(armMorphStateP state, loadStoreFn cb) {

    vmiReg t  = newTemp32(state);
    vmiReg rm = GET_RS(state, r3);

    getExtendedR(state, t, rm, FLAGS_CIN);

    emitLDRSTRInt(state, t, cb);

    freeTemp32(state);
}

//
// Emit code for LDR with immediate offset
//
ARM_MORPH_FN(armEmitLDRI) {
    emitNullCheckIfThumbEE(state, GET_RS(state, r2));
    emitLDRSTRI(state, emitLDR, True);
}

//
// Emit code for LDR with register offset
//
ARM_MORPH_FN(armEmitLDRR) {
    emitNullCheckIfThumbEE(state, GET_RS(state, r2));
    emitLDRSTRR(state, emitLDR);
}

//
// Emit code for LDR with scaled register offset (LSL, LSR, ASR, ROR)
//
ARM_MORPH_FN(armEmitLDRRSI) {
    emitNullCheckIfThumbEE(state, GET_RS(state, r2));
    emitLDRSTRRSI(state, emitLDR);
}

//
// Emit code for LDR with immediate offset (RRX)
//
ARM_MORPH_FN(armEmitLDRRX) {
    emitNullCheckIfThumbEE(state, GET_RS(state, r2));
    emitLDRSTRRX(state, emitLDR);
}

//
// Emit code for STR with immediate offset
//
ARM_MORPH_FN(armEmitSTRI) {
    emitNullCheckIfThumbEE(state, GET_RS(state, r2));
    emitLDRSTRI(state, emitSTR, False);
}

//
// Emit code for STR with register offset
//
ARM_MORPH_FN(armEmitSTRR) {
    emitNullCheckIfThumbEE(state, GET_RS(state, r2));
    emitLDRSTRR(state, emitSTR);
}

//
// Emit code for STR with scaled register offset (LSL, LSR, ASR, ROR)
//
ARM_MORPH_FN(armEmitSTRRSI) {
    emitNullCheckIfThumbEE(state, GET_RS(state, r2));
    emitLDRSTRRSI(state, emitSTR);
}

//
// Emit code for STR with immediate offset (RRX)
//
ARM_MORPH_FN(armEmitSTRRX) {
    emitNullCheckIfThumbEE(state, GET_RS(state, r2));
    emitLDRSTRRX(state, emitSTR);
}


////////////////////////////////////////////////////////////////////////////////
// SEMAPHORE INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Emit code for SWP and SWPB
// NOTE: enabled by SCTLR.SW if multprocessing extensions are present
//
ARM_MORPH_FN(armEmitSWP) {

    armP arm           = state->arm;
    Bool mpImplemented = CP_MP_IMPLEMENTED(arm);
    Bool enabled       = !mpImplemented || CP_FIELD(arm, SCTLR, SW);

    // this code is dependent of the value of SCTLR.SW if the processor supports
    // multiprocessing extensions
    if(arm->checkEnableSWP) {
        armEmitValidateBlockMask(ARM_BM_SWP);
    }

    if(enabled) {

        // SWP/SWPB enabled
        Uns32  regBits = ARM_GPR_BITS;
        Uns32  memBits = state->info.sz*8;
        vmiReg rd      = GET_RD(state, r1);
        vmiReg rm      = GET_RS(state, r2);
        vmiReg rn      = GET_RS(state, r3);
        vmiReg t       = getTemp(state);

        // use a temporary to avoid side effects if the store fails
        armEmitLoadRRO (state, regBits, memBits, 0, t, rn, False, False);
        armEmitStoreRRO(state, memBits, 0, rn, rm);
        armEmitMoveRR(state, regBits, rd, t);

    } else {

        // SWP/SWPB disabled
        emitUndefinedCP(state);
    }
}


////////////////////////////////////////////////////////////////////////////////
// DSP INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Minimum and maximum values for clamping
//
#define ARM_MIN 0x80000000
#define ARM_MAX 0x7fffffff

//
// Emit code for to perform rd = rs1 op rs2, clamping result and setting Q
// flag if overflow
//
static void emitOpSetQClamp(
    armMorphStateP state,
    vmiBinop       op,
    vmiReg         rd,
    vmiReg         rs1,
    vmiReg         rs2
) {
    Uns32     bits       = ARM_GPR_BITS;
    vmiFlags  flags      = getSFOFFlags(getTemp(state));
    vmiLabelP noOverflow = armEmitNewLabel();

    // do the operation, setting flags
    armEmitBinopRRR(state, bits, op, rd, rs1, rs2, &flags);

    // skip clamping and Q flag update unless there was an overflow
    armEmitCondJumpLabel(flags.f[vmi_OF], False, noOverflow);

    // clamp depending on sign of result
    armEmitCondMoveRCC(state, bits, flags.f[vmi_SF], True, rd, ARM_MAX, ARM_MIN);

    // set the sticky Q flag
    armEmitMoveRC(state, 8, ARM_QF, 1);

    // here if the operation didn't overflow
    emitLabel(noOverflow);
}

//
// Emit code to perform rd = rs1 op rs2, setting Q flag if overflow
//
static void emitOpSetQ(
    armMorphStateP state,
    Uns32          bits,
    vmiBinop       op,
    vmiReg         rd,
    vmiReg         rs1,
    vmiReg         rs2
) {
    vmiReg   tf    = getTemp(state);
    vmiFlags flags = getOFFlags(tf);

    // do the operation, setting flags
    armEmitBinopRRR(state, bits, op, rd, rs1, rs2, &flags);

    // set the sticky Q flag of there was overflow
    armEmitBinopRR(state, 8, vmi_OR, ARM_QF, tf, 0);
}

//
// Emit code for 16 x 16 = 32 multiply
//
static void emitMul1632(
    armMorphStateP state,
    vmiReg         rd,
    vmiReg         rs1,
    vmiReg         rs2
) {
    vmiReg rdlo = getR32Lo(rd);
    vmiReg rdhi = getR32Hi(rd);

    // do 16 x 16 = 32 multiply
    armEmitMulopRRR(state, ARM_GPR_BITS/2, vmi_IMUL, rdhi, rdlo, rs1, rs2, 0);
}

//
// Emit code for QADD/QSUB
//
static void emitQADDSUB(armMorphStateP state, vmiBinop op) {

    vmiReg rd  = GET_RD(state, r1);
    vmiReg rs1 = GET_RS(state, r2);
    vmiReg rs2 = GET_RS(state, r3);

    emitOpSetQClamp(state, op, rd, rs1, rs2);
}

//
// Emit code for QDADD/QDSUB
//
static void emitQDADDSUB(armMorphStateP state, vmiBinop op) {

    vmiReg rd  = GET_RD(state, r1);
    vmiReg rs1 = GET_RS(state, r2);
    vmiReg rs2 = GET_RS(state, r3);
    vmiReg t   = newTemp32(state);

    emitOpSetQClamp(state, vmi_ADD, t, rs2, rs2);
    emitOpSetQClamp(state, op, rd, rs1, t);

    freeTemp32(state);
}

//
// Emit code for QADD
//
ARM_MORPH_FN(armEmitQADD) {
    emitQADDSUB(state, vmi_ADD);
}

//
// Emit code for QSUB
//
ARM_MORPH_FN(armEmitQSUB) {
    emitQADDSUB(state, vmi_SUB);
}

//
// Emit code for QDADD
//
ARM_MORPH_FN(armEmitQDADD) {
    emitQDADDSUB(state, vmi_ADD);
}

//
// Emit code for QDSUB
//
ARM_MORPH_FN(armEmitQDSUB) {
    emitQDADDSUB(state, vmi_SUB);
}

//
// Emit code for SMLA<x><y>
//
static void emitSMLA(armMorphStateP state, Uns32 xDelta, Uns32 yDelta) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rd   = GET_RD(state, r1);
    vmiReg rs1  = VMI_REG_DELTA(GET_RS(state, r2), xDelta);
    vmiReg rs2  = VMI_REG_DELTA(GET_RS(state, r3), yDelta);
    vmiReg rs3  = GET_RS(state, r4);
    vmiReg t    = newTemp32(state);

    // do 16 x 16 = 32 multiply
    emitMul1632(state, t, rs1, rs2);

    // do the accumulate, setting Q flag on overflow
    emitOpSetQ(state, bits, vmi_ADD, rd, rs3, t);

    // free multiply result temporary
    freeTemp32(state);
}

//
// Emit code for SMLABB
//
ARM_MORPH_FN(armEmitSMLABB) {
    emitSMLA(state, 0, 0);
}

//
// Emit code for SMLABT
//
ARM_MORPH_FN(armEmitSMLABT) {
    emitSMLA(state, 0, 2);
}

//
// Emit code for SMLATB
//
ARM_MORPH_FN(armEmitSMLATB) {
    emitSMLA(state, 2, 0);
}

//
// Emit code for SMLATT
//
ARM_MORPH_FN(armEmitSMLATT) {
    emitSMLA(state, 2, 2);
}

//
// Emit code for SMLAL<x><y>
//
static void emitSMLAL(armMorphStateP state, Uns32 xDelta, Uns32 yDelta) {

    Uns32    bits  = ARM_GPR_BITS;
    vmiReg   rdlo  = GET_RD(state, r1);
    vmiReg   rdhi  = GET_RD(state, r2);
    vmiReg   rs1   = VMI_REG_DELTA(GET_RS(state, r3), xDelta);
    vmiReg   rs2   = VMI_REG_DELTA(GET_RS(state, r4), yDelta);
    vmiReg   t     = newTemp64(state);
    vmiReg   tlo32 = getR64Lo(t);
    vmiReg   thi32 = getR64Hi(t);
    vmiFlags flags = getCFFlags(getTemp(state));

    // do 16 x 16 = 32 multiply and extend to 64 bits
    emitMul1632(state, t, rs1, rs2);
    armEmitMoveExtendRR(state, bits*2, t, bits, tlo32, True);

    // do the accumulate
    armEmitBinopRR(state, bits, vmi_ADD, rdlo, tlo32, &flags);
    armEmitBinopRR(state, bits, vmi_ADC, rdhi, thi32, &flags);

    // free multiply result temporary
    freeTemp64(state);
}

//
// Emit code for SMLABB
//
ARM_MORPH_FN(armEmitSMLALBB) {
    emitSMLAL(state, 0, 0);
}

//
// Emit code for SMLABT
//
ARM_MORPH_FN(armEmitSMLALBT) {
    emitSMLAL(state, 0, 2);
}

//
// Emit code for SMLATB
//
ARM_MORPH_FN(armEmitSMLALTB) {
    emitSMLAL(state, 2, 0);
}

//
// Emit code for SMLATT
//
ARM_MORPH_FN(armEmitSMLALTT) {
    emitSMLAL(state, 2, 2);
}

//
// Emit code for SMLAW<y>
//
static void emitSMLAW(armMorphStateP state, Uns32 yDelta) {

    Uns32  bits  = ARM_GPR_BITS;
    vmiReg rd    = GET_RD(state, r1);
    vmiReg rs1   = GET_RS(state, r2);
    vmiReg rs2   = VMI_REG_DELTA(GET_RS(state, r3), yDelta);
    vmiReg rs3   = GET_RS(state, r4);
    vmiReg t     = newTemp64(state);
    vmiReg tlo32 = getR64Lo(t);
    vmiReg thi32 = getR64Hi(t);
    vmiReg t48   = VMI_REG_DELTA(t, 2);

    // sign extend rs2 and place result in temporary
    armEmitMoveExtendRR(state, bits, t, bits/2, rs2, True);

    // do 32 x 32 = 64 multiply
    armEmitMulopRRR(state, bits, vmi_IMUL, thi32, tlo32, t, rs1, 0);

    // do the accumulate, setting Q flag on overflow
    emitOpSetQ(state, bits, vmi_ADD, rd, t48, rs3);

    // free multiply result temporary
    freeTemp64(state);
}

//
// Emit code for SMLAWB
//
ARM_MORPH_FN(armEmitSMLAWB) {
    emitSMLAW(state, 0);
}

//
// Emit code for SMLAWT
//
ARM_MORPH_FN(armEmitSMLAWT) {
    emitSMLAW(state, 2);
}

//
// Emit code for emitSMUL<x><y>
//
static void emitSMUL(armMorphStateP state, Uns32 xDelta, Uns32 yDelta) {

    vmiReg rd  = GET_RD(state, r1);
    vmiReg rs1 = VMI_REG_DELTA(GET_RS(state, r2), xDelta);
    vmiReg rs2 = VMI_REG_DELTA(GET_RS(state, r3), yDelta);

    emitMul1632(state, rd, rs1, rs2);
}

//
// Emit code for SMULBB
//
ARM_MORPH_FN(armEmitSMULBB) {
    emitSMUL(state, 0, 0);
}

//
// Emit code for SMULBT
//
ARM_MORPH_FN(armEmitSMULBT) {
    emitSMUL(state, 0, 2);
}

//
// Emit code for SMULTB
//
ARM_MORPH_FN(armEmitSMULTB) {
    emitSMUL(state, 2, 0);
}

//
// Emit code for SMULTT
//
ARM_MORPH_FN(armEmitSMULTT) {
    emitSMUL(state, 2, 2);
}

//
// Emit code for SMULW<y>
//
static void emitSMULW(armMorphStateP state, Uns32 yDelta) {

    Uns32  bits  = ARM_GPR_BITS;
    vmiReg rd    = GET_RD(state, r1);
    vmiReg rs1   = GET_RS(state, r2);
    vmiReg rs2   = VMI_REG_DELTA(GET_RS(state, r3), yDelta);
    vmiReg t     = newTemp64(state);
    vmiReg tlo32 = getR64Lo(t);
    vmiReg thi32 = getR64Hi(t);
    vmiReg t48   = VMI_REG_DELTA(t, 2);

    // sign extend rs2 and place result in temporary
    armEmitMoveExtendRR(state, bits, t, bits/2, rs2, True);

    // do 32 x 32 = 64 multiply
    armEmitMulopRRR(state, bits, vmi_IMUL, thi32, tlo32, t, rs1, 0);

    // assign the result
    armEmitMoveRR(state, bits, rd, t48);

    // free multiply result temporary
    freeTemp64(state);
}

//
// Emit code for SMULWB
//
ARM_MORPH_FN(armEmitSMULWB) {
    emitSMULW(state, 0);
}

//
// Emit code for SMULWT
//
ARM_MORPH_FN(armEmitSMULWT) {
    emitSMULW(state, 2);
}


////////////////////////////////////////////////////////////////////////////////
// COPROCESSOR INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Emit code for CDP
//
ARM_MORPH_FN(armEmitCDP) {

    emitUndefinedCP(state);
}

//
// Emit code for LDC
//
ARM_MORPH_FN(armEmitLDC) {

    if(True) {
        emitUndefinedCP(state);
    } else {
        emitNullCheckIfThumbEE(state, GET_RS(state, r2));
    }
}

//
// Emit code for MRC
//
ARM_MORPH_FN(armEmitMRC) {

    if(!armEmitCpRead(state, GET_RD(state, r1))) {
        emitUndefinedCP(state);
    }
}

//
// Emit code for MCR
//
ARM_MORPH_FN(armEmitMCR) {

    if(!armEmitCpWrite(state, GET_RS(state, r1))) {
        emitUndefinedCP(state);
    }
}

//
// Emit code for MRRC
//
ARM_MORPH_FN(armEmitMRRC) {

    if(!armEmitCpReadPair(state, GET_RD(state, r1), GET_RD(state, r2))) {
        emitUndefinedCP(state);
    }
}

//
// Emit code for MCRR
//
ARM_MORPH_FN(armEmitMCRR) {

    if(!armEmitCpWritePair(state, GET_RS(state, r1), GET_RS(state, r2))) {
        emitUndefinedCP(state);
    }
}

//
// Emit code for STC
//
ARM_MORPH_FN(armEmitSTC) {

    if(True) {
        emitUndefinedCP(state);
    } else {
        emitNullCheckIfThumbEE(state, GET_RS(state, r2));
    }
}


////////////////////////////////////////////////////////////////////////////////
// CPSR UPDATE UTILITIES
////////////////////////////////////////////////////////////////////////////////

//
// Emit code to perform any special actions required when certain bits of the
// CPSR may have been updated
//
static void emitWriteCPSRActions(armMorphStateP state, Uns32 writeMask) {

    // if CPSR flags are updated by this write then derived flag values may no
    // longer be relied upon
    if(writeMask&CPSR_FLAGS) {
        resetDerivedFlags(state);
    }

    // terminate the current code block if mode or endian switch is possible
    if(writeMask&(CPSR_MODE|CPSR_ENDIAN)) {
        armEmitEndBlock();
    }
}

//
// Emit code to write CPSR register from temporary register value
//
static void emitWriteCPSRReg(armMorphStateP state, vmiReg t, Uns32 writeMask) {

    // emit call to update CPSR
    armEmitArgProcessor(state);
    armEmitArgReg(state, ARM_GPR_BITS, t);
    armEmitArgUns32(state, writeMask);
    armEmitCall(state, (vmiCallFn)armWriteCPSR);

    // do any special actions required after CPSR update
    emitWriteCPSRActions(state, writeMask);
}

//
// Emit code to write CPSR register using value and mask
//
static void emitWriteCPSRValue(armMorphStateP state, Uns32 v, Uns32 writeMask) {

    // emit call to update CPSR
    armEmitArgProcessor(state);
    armEmitArgUns32(state, v);
    armEmitArgUns32(state, writeMask);
    armEmitCall(state, (vmiCallFn)armWriteCPSR);

    // do any special actions required after CPSR update
    emitWriteCPSRActions(state, writeMask);
}


////////////////////////////////////////////////////////////////////////////////
// MOVE INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// MOVW instruction
//
ARM_MORPH_FN(armEmitMOVW) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rd   = GET_RD(state, r1);

    armEmitMoveRC(state, bits, rd, state->info.c);
}

//
// MOVT instruction
//
ARM_MORPH_FN(armEmitMOVT) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rd   = GET_RD(state, r1);

    if(!VMI_REG_EQUAL(rd, ARM_PC)) {

        vmiReg rdH = VMI_REG_DELTA(rd, 2);

        armEmitMoveRC(state, bits/2, rdH, state->info.c);

    } else {

        vmiReg rs   = getRS(state, ARM_REG_PC);
        vmiReg tmp  = newTemp32(state);
        vmiReg tmpH = VMI_REG_DELTA(tmp, 2);

        armEmitMoveRR(state, bits, tmp, rs);
        armEmitMoveRC(state, bits/2, tmpH, state->info.c);
        armEmitMoveRR(state, bits, rd, tmp);

        freeTemp32(state);
    }
}


////////////////////////////////////////////////////////////////////////////////
// MULTIPLY INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// [SU]MAAL instructions
//
ARM_MORPH_FN(armEmitMAAL) {

    Uns32    bits  = ARM_GPR_BITS;
    vmiBinop op    = state->attrs->binop;
    vmiReg   rdlo  = GET_RS(state, r1);
    vmiReg   rdhi  = GET_RS(state, r2);
    vmiReg   rm    = GET_RS(state, r3);
    vmiReg   rs    = GET_RS(state, r4);
    vmiReg   t     = newTemp64(state);
    vmiReg   t64lo = getR64Lo(t);
    vmiReg   t64hi = getR64Hi(t);
    vmiFlags tf    = getCFFlags(getTemp(state));

    // perform initial multiply, result in temporary t64lo/t64hi
    armEmitMulopRRR(state, bits, op, t64hi, t64lo, rm, rs, 0);

    // add in rdhi
    armEmitBinopRR(state, bits, vmi_ADD, t64lo, rdhi, &tf);
    armEmitBinopRC(state, bits, vmi_ADC, t64hi, 0,    &tf);

    // add in rdlo
    armEmitBinopRR(state, bits, vmi_ADD, t64lo, rdlo, &tf);
    armEmitBinopRC(state, bits, vmi_ADC, t64hi, 0,    &tf);

    // move result to rdlo/rdhi
    armEmitMoveRR(state, bits, rdhi, t64hi);
    armEmitMoveRR(state, bits, rdlo, t64lo);

    freeTemp64(state);
}


////////////////////////////////////////////////////////////////////////////////
// SYNCHRONIZATION INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Generate exclusive access tag address ra+offset in register rtag
//
static void generateEATag(
    armMorphStateP state,
    Uns32          offset,
    vmiReg         rtag,
    vmiReg         ra
) {
    Uns32 bits = ARM_GPR_BITS;
    Uns32 mask = state->arm->exclusiveTagMask;

    armEmitBinopRRC(state, bits, vmi_ADD, rtag, ra, offset, 0);
    armEmitBinopRC(state, bits, vmi_AND, rtag, mask, 0);
}

//
// Emit code to start exclusive access to address ra+offset
//
inline static void startEA(armMorphStateP state, Uns32 offset, vmiReg ra) {
    generateEATag(state, offset, ARM_EA_TAG, ra);
}

//
// Validate the exclusive access and jump to label 'done' if it is invalid,
// setting rd to 1
//
static vmiLabelP validateEA(
    armMorphStateP state,
    Uns32          offset,
    vmiReg         ra,
    vmiReg         rd
) {
    Uns32     bits    = ARM_GPR_BITS;
    Uns32     memBits = state->info.sz*8;
    vmiLabelP done    = armEmitNewLabel();
    vmiLabelP ok      = armEmitNewLabel();
    vmiReg    t       = getTemp(state);

    // generate any store exception prior to exclusive access tag check
    armEmitTryStoreRC(state, memBits, offset, ra);

    // generate exclusive access tag for this address
    generateEATag(state, offset, t, ra);

    // do load and store tags match?
    armEmitCompareRR(state, bits, vmi_COND_EQ, ARM_EA_TAG, t, t);

    // commit store if tags match
    armEmitCondJumpLabel(t, True, ok);

    // indicate store failed
    armEmitMoveRC(state, bits, rd, 1);

    // jump to instruction end
    armEmitUncondJumpLabel(done);

    // here to commit store
    armEmitInsertLabel(ok);

    return done;
}

//
// Do actions required to terminate exclusive access
//
static void clearEA(armMorphStateP state) {

    // exclusiveTag becomes ARM_NO_TAG to indicate no active access
    armEmitMoveRC(state, ARM_GPR_BITS, ARM_EA_TAG, ARM_NO_TAG);
}

//
// Do actions required to complete exclusive access
//
static void endEA(armMorphStateP state, vmiReg rd, vmiLabelP done) {

    // indicate store succeeded
    armEmitMoveRC(state, ARM_GPR_BITS, rd, 0);

    // insert target label for aborted stores
    armEmitInsertLabel(done);

    // terminate exclusive access
    clearEA(state);
}

//
// Emit code for LDREX*
//
ARM_MORPH_FN(armEmitLDREX) {

    vmiReg rn = GET_RS(state, r2);

    // base register must be null-checked in ThumbEE mode
    emitNullCheckIfThumbEE(state, rn);

    // indicate LDREX is now active at address r2+offset
    startEA(state, state->info.c, rn);

    // emit load
    emitLDRSTRI(state, emitLDR, True);
}

//
// Emit code for STREX*
//
ARM_MORPH_FN(armEmitSTREX) {

    vmiReg rn = GET_RS(state, r3);
    vmiReg rd = GET_RD(state, r1);

    // base register must be null-checked in ThumbEE mode
    emitNullCheckIfThumbEE(state, rn);

    // validate STREX attempt at address r3+offset
    vmiLabelP done = validateEA(state, state->info.c, rn, rd);

    // move down rt and rn so that they are in the required positions for a
    // normal store
    state->info.r1 = state->info.r2;
    state->info.r2 = state->info.r3;

    // emit store
    emitLDRSTRI(state, emitSTR, False);

    // complete STREX attempt
    endEA(state, rd, done);
}

//
// Emit code for CLREX
//
ARM_MORPH_FN(armEmitCLREX) {
    clearEA(state);
}


////////////////////////////////////////////////////////////////////////////////
// EXCEPTION INSTRUCTONS
////////////////////////////////////////////////////////////////////////////////

//
// Emit code for SRS
//
ARM_MORPH_FN(armEmitSRS) {

    #define SRS_NUM_REGS 2

    armP        arm        = state->arm;
    Uns32       bits       = ARM_GPR_BITS;
    armCPSRMode mode       = state->info.c;
    vmiReg      base       = getBankedRSMode(state, mode, state->info.r1);
    Bool        increment  = doIncrement(state);
    Int32       stepBefore = getStepBefore(state, ARM_GPR_BYTES);
    Int32       stepAfter  = getStepAfter (state, ARM_GPR_BYTES);
    Uns32       frameSize  = SRS_NUM_REGS * ARM_GPR_BYTES;
    Int32       offset     = increment ? 0 : -frameSize;
    Int32       frameDelta = increment ? frameSize : -frameSize;
    Uns32       i;

    // get a list of the registers to save
    vmiReg saveRegs[SRS_NUM_REGS] = {ARM_LR, ARM_SPSR};

    // validate that SPSR is either a true register or mapped to CPSR
    validateSPSR(arm);

    // get CPSR in temporary for modes with no SPSR, and ensure that only the
    // current stack pointer can ever be used (documentation says that this is
    // unpredictable, but update of privileged register state from User or
    // System modes sounds wrong)
    if(IN_BASE_MODE(arm)) {
        vmiReg t = getTemp(state);
        emitReadCPSR(state, t, -1);
        saveRegs[1] = t;
        base = GET_RS(state, r1);
    }

    // write the required register values
    for(i=0; i<SRS_NUM_REGS; i++) {
        offset += stepBefore;
        armEmitStoreRRO(state, bits, offset, base, saveRegs[i]);
        offset += stepAfter;
    }

    // perform base register writeback if required
    if(state->info.wb) {
        armEmitBinopRC(state, bits, vmi_ADD, base, frameDelta, 0);
    }

    #undef SRS_NUM_REGS
}

//
// Emit code for RFE
//
ARM_MORPH_FN(armEmitRFE) {

    #define RFE_NUM_REGS 2

    Uns32  bits       = ARM_GPR_BITS;
    vmiReg base       = GET_RS(state, r1);
    Bool   increment  = doIncrement(state);
    Int32  stepBefore = getStepBefore(state, ARM_GPR_BYTES);
    Int32  stepAfter  = getStepAfter (state, ARM_GPR_BYTES);
    Uns32  frameSize  = RFE_NUM_REGS * ARM_GPR_BYTES;
    Int32  offset     = increment ? 0 : -frameSize;
    Int32  frameDelta = increment ? frameSize : -frameSize;
    vmiReg t1         = newTemp32(state);
    vmiReg t2         = getTemp(state);
    Uns32  i;

    // get a list of the registers to save
    vmiReg loadRegs[RFE_NUM_REGS] = {t1, t2};

    // read the required register values into temporaries
    for(i=0; i<RFE_NUM_REGS; i++) {
        offset += stepBefore;
        armEmitLoadRRO(state, bits, bits, offset, loadRegs[i], base, False, False);
        offset += stepAfter;
    }

    // perform base register writeback if required
    if(state->info.wb) {
        armEmitBinopRC(state, bits, vmi_ADD, base, frameDelta, 0);
    }

    // emit call to update CPSR
    emitWriteCPSRReg(state, t2, armGetWriteMaskCPSR(state->arm, True));

    // do jump to target address
    armEmitMoveRR(state, bits, ARM_PC, t1);

    // free allocated temporary
    freeTemp32(state);

    #undef RFE_NUM_REGS
}


////////////////////////////////////////////////////////////////////////////////
// MISCELLANEOUS INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Emit code for SETEND
//
ARM_MORPH_FN(armEmitSETEND) {

    Uns32 writeMask = CPSR_ENDIAN;
    Uns32 value     = state->info.c ? writeMask : 0;

    // emit call to update CPSR
    emitWriteCPSRValue(state, value, writeMask);
}

//
// Emit code for CPS
//
ARM_MORPH_FN(armEmitCPS) {

    armP arm = state->arm;

    if(!IN_USER_MODE(arm)) {

        armFlagAffect faff      = state->info.faff;
        Uns32         flagMask  = 0;
        Uns32         writeMask = 0;
        Uns32         value     = 0;

        // allow for possible mode switch
        if(state->info.ma) {
            writeMask |= CPSR_MODE;
            value     |= state->info.c;
        }

        // get mask of flag bits affected by the instruction
        flagMask |= (faff&ARM_FAFF_A) ? CPSR_ABORTE : 0;
        flagMask |= (faff&ARM_FAFF_F) ? CPSR_FIQE   : 0;
        flagMask |= (faff&ARM_FAFF_I) ? CPSR_IRQE   : 0;

        // allow for possible flag bit update
        switch(state->info.fact) {
            case ARM_FACT_ID: value     |= flagMask; // fallthru
            case ARM_FACT_IE: writeMask |= flagMask; // fallthru
            default: break;
        }

        // emit call to update CPSR
        emitWriteCPSRValue(state, value, writeMask);
    }
}


////////////////////////////////////////////////////////////////////////////////
// BRANCH INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Emit code for CBZ/CBNZ
//
ARM_MORPH_FN(armEmitCBZ) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rn   = GET_RS(state, r1);
    vmiReg tf   = getTemp(state);

    // do the comparison
    armEmitCompareRC(state, bits, vmi_COND_Z, rn, 0, tf);

    // get information about the jump
    armJumpInfo ji;
    seedJumpInfo(&ji, state, False, False, True);

    // do the jump
    armEmitCondJump(state, &ji, tf, state->attrs->jumpIfTrue);
}

//
// Emit code for TBB/TBH
//
ARM_MORPH_FN(armEmitTB) {

    Uns32  bits    = ARM_GPR_BITS;
    Uns32  sz      = state->info.sz;
    Uns32  memBits = sz*8;
    vmiReg rn      = GET_RS(state, r1);
    vmiReg rm      = GET_RS(state, r2);
    vmiReg t1      = getTemp(state);
    Uns32  shift   = 0;

    // base register must be null-checked in ThumbEE mode
    emitNullCheckIfThumbEE(state, GET_RS(state, r1));

    // convert from size to shift
    while(sz>1) {
        sz >>= 1;
        shift++;
    }

    // get offset, shifted if required
    armEmitBinopRRC(state, bits, vmi_SHL, t1, rm, shift, 0);

    // compose full address
    armEmitBinopRR(state, bits, vmi_ADD, t1, rn, 0);

    // load zero-extended offset from table and double it
    armEmitLoadRRO(state, bits, memBits, 0, t1, t1, False, False);
    armEmitBinopRR(state, bits, vmi_ADD, t1, t1, 0);

    // add to effective PC to get target address
    armEmitBinopRR(state, bits, vmi_ADD, t1, getRS(state, ARM_REG_PC), 0);

    // get information about the jump
    armJumpInfo ji;
    seedJumpInfo(&ji, state, False, False, True);

    // do the jump
    armEmitUncondJumpReg(state, &ji, t1);
}


////////////////////////////////////////////////////////////////////////////////
// BASIC MEDIA INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Common routine for USAD8 and USADA8
//
static void emitUSAD8Int(armMorphStateP state, vmiReg ra) {

    Uns32  bits     = ARM_GPR_BITS;
    Uns32  partBits = 8;
    vmiReg rd       = GET_RD(state, r1);
    vmiReg rn       = GET_RS(state, r2);
    vmiReg rm       = GET_RS(state, r3);
    vmiReg t1       = newTemp32(state);
    vmiReg t2       = newTemp32(state);
    vmiReg t3       = getTemp(state);
    Uns32  i;

    // clear result accumulators
    armEmitMoveRR(state, bits, t1, ra);

    for(i=0; i<(bits/partBits); i++) {

        // get zero-extended arguments in temporaries
        armEmitMoveExtendRR(state, bits, t2, partBits, rn, False);
        armEmitMoveExtendRR(state, bits, t3, partBits, rm, False);

        // perform subtraction
        armEmitBinopRR(state, bits, vmi_SUB, t2, t3, 0);

        // get absolute result
        armEmitUnopRR(state, bits, vmi_ABS, t2, t2, 0);

        // update accumulated result
        armEmitBinopRR(state, bits, vmi_ADD, t1, t2, 0);

        // step to next register pair
        rn = VMI_REG_DELTA(rn, 1);
        rm = VMI_REG_DELTA(rm, 1);
    }

    // assign result register
    armEmitMoveRR(state, bits, rd, t1);

    // free allocated temporaries
    freeTemp32(state);
    freeTemp32(state);
}

//
// Common routine for SBFX and UBFX
//
static void emitBFXInt(armMorphStateP state, vmiBinop op) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rd   = GET_RD(state, r1);
    vmiReg rn   = GET_RS(state, r2);
    Uns32  lsb  = state->info.c;
    Uns32  msb  = lsb + state->info.w - 1;

    if(msb<ARM_GPR_BITS) {

        Uns32 leftShift  = bits - msb - 1;
        Uns32 rightShift = leftShift + lsb;

        armEmitBinopRRC(state, bits, vmi_SHL, rd, rn, leftShift,  0);
        armEmitBinopRRC(state, bits, op,      rd, rd, rightShift, 0);
    }
}

//
// Emit code for USAD8
//
ARM_MORPH_FN(armEmitUSAD8) {
    emitUSAD8Int(state, VMI_NOREG);
}

//
// Emit code for USADA8
//
ARM_MORPH_FN(armEmitUSADA8) {
    emitUSAD8Int(state, GET_RS(state, r4));
}

//
// Emit code for SBFX
//
ARM_MORPH_FN(armEmitSBFX) {
    emitBFXInt(state, vmi_SAR);
}

//
// Emit code for BFC
//
ARM_MORPH_FN(armEmitBFC) {

    Uns32  bits  = ARM_GPR_BITS;
    vmiReg rd    = GET_RS(state, r1);
    Uns32  lsb   = state->info.c;
    Int32  width = state->info.w;

    if(width<=0) {

        // no action

    } else if(width==bits) {

        // optimize to a move
        armEmitMoveRC(state, bits, rd, 0);

    } else {

        // general case
        Uns32 mask1 = ((1<<width)-1);
        Uns32 mask2 = ~(mask1<<lsb);

        armEmitBinopRC(state, bits, vmi_AND, rd, mask2, 0);
    }
}

//
// Emit code for BFI
//
ARM_MORPH_FN(armEmitBFI) {

    Uns32  bits  = ARM_GPR_BITS;
    vmiReg rd    = GET_RS(state, r1);
    vmiReg rs    = GET_RS(state, r2);
    Uns32  lsb   = state->info.c;
    Int32  width = state->info.w;

    if(width<=0) {

        // no action

    } else if(width==bits) {

        // optimize to a move
        armEmitMoveRR(state, bits, rd, rs);

    } else {

        // general case
        vmiReg t     = getTemp(state);
        Uns32  mask1 = ((1<<width)-1);
        Uns32  mask2 = ~(mask1<<lsb);

        armEmitBinopRRC(state, bits, vmi_AND, t, rs, mask1, 0);
        armEmitBinopRC(state, bits, vmi_SHL, t, lsb, 0);
        armEmitBinopRC(state, bits, vmi_AND, rd, mask2, 0);
        armEmitBinopRR(state, bits, vmi_OR, rd, t, 0);
    }
}

//
// Emit code for UBFX
//
ARM_MORPH_FN(armEmitUBFX) {
    emitBFXInt(state, vmi_SHR);
}


////////////////////////////////////////////////////////////////////////////////
// PARALLEL ADD/SUBTRACT INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Emit code to clear GE flags in the CPSR if required
//
static void emitClearGEFlags(armMorphStateP state) {

    if(state->attrs->setGE) {
        armEmitBinopRC(state, ARM_GPR_BITS, vmi_AND, ARM_CPSR, ~CPSR_GE30, 0);
    }
}

//
// Emit code to perform part of a parallel operation
//
static void emitParallelBinopInt(
    armMorphStateP state,
    Uns32          partBits,
    vmiBinop       op,
    vmiReg         rd,
    vmiReg         rn,
    vmiReg         rm,
    Uns32          delta1,
    Uns32          delta2,
    Uns32          apsrMask
) {
    Bool     setGE   = state->attrs->setGE;
    Bool     sextend = state->attrs->sextend;
    Bool     halve   = state->attrs->halve;
    vmiFlags flags   = VMI_NOFLAGS;
    vmiReg   tf      = newTemp32(state);
    vmiReg   rnTmp;
    vmiReg   rmTmp;
    vmiReg   rdTmp;
    Uns32    opBits;

    // set up flags to generate CPSR greater-or-equal bits if required
    if(!(setGE || halve)) {
        // no action
    } else if(sextend) {
        flags.f[vmi_SF] = tf;
    } else {
        flags.f[vmi_CF] = flags.cin = tf;
    }

    // get shift to appropriate part of each register
    rd = VMI_REG_DELTA(rd, delta1);
    rn = VMI_REG_DELTA(rn, delta1);
    rm = VMI_REG_DELTA(rm, delta2);

    // argument format depends on whether sign extension is required
    if(sextend) {

        // sign extension required: perform operation on extended arguments
        opBits = ARM_GPR_BITS;
        rdTmp  = newTemp32(state);
        rnTmp  = rdTmp;
        rmTmp  = getTemp(state);

        // sign extend arguments into temporaries
        armEmitMoveExtendRR(state, opBits, rnTmp, partBits, rn, True);
        armEmitMoveExtendRR(state, opBits, rmTmp, partBits, rm, True);

    } else {

        // use original unextended arguments
        opBits = partBits;
        rdTmp  = rd;
        rnTmp  = rn;
        rmTmp  = rm;
    }

    // do the operation
    armEmitBinopRRR(state, opBits, op, rdTmp, rnTmp, rmTmp, &flags);

    // update CPSR greater-or-equal bits if required
    if(setGE) {

        vmiLabelP done = armEmitNewLabel();

        // skip flag update if required
        armEmitCondJumpLabel(tf, (sextend || (op!=vmi_ADD)), done);

        // include apsrMask in CPSR register
        armEmitBinopRC(state, ARM_GPR_BITS, vmi_OR, ARM_CPSR, apsrMask, 0);

        // jump to here if flag update is not required
        armEmitInsertLabel(done);
    }

    // halve results if required
    if(!halve) {
        // no action
    } else if(sextend) {
        armEmitBinopRC(state, opBits, vmi_SHR, rdTmp, 1, 0);
    } else {
        armEmitBinopRC(state, opBits, vmi_RCR, rdTmp, 1, &flags);
    }

    // write back temporary if required
    if(sextend) {
        armEmitMoveRR(state, partBits, rd, rdTmp);
        freeTemp32(state);
    }

    // free temporary flag
    freeTemp32(state);
}

//
// Emit code for parallel add/subtract of 8-bit data
//
ARM_MORPH_FN(armEmitParallelBinop8) {

    Uns32    bits = ARM_GPR_BITS/4;
    vmiReg   rd   = GET_RD(state, r1);
    vmiReg   rn   = GET_RS(state, r2);
    vmiReg   rm   = GET_RS(state, r3);
    vmiBinop op   = state->attrs->binop;

    // clear GE flags
    emitClearGEFlags(state);

    // perform the parallel operations
    emitParallelBinopInt(state, bits, op, rd, rn, rm, 0, 0, CPSR_GE0);
    emitParallelBinopInt(state, bits, op, rd, rn, rm, 1, 1, CPSR_GE1);
    emitParallelBinopInt(state, bits, op, rd, rn, rm, 2, 2, CPSR_GE2);
    emitParallelBinopInt(state, bits, op, rd, rn, rm, 3, 3, CPSR_GE3);
}

//
// Emit code for parallel add/subtract of 16-bit data
//
ARM_MORPH_FN(armEmitParallelBinop16) {

    Uns32    bits = ARM_GPR_BITS/2;
    vmiReg   rd   = GET_RD(state, r1);
    vmiReg   rn   = GET_RS(state, r2);
    vmiReg   rm   = GET_RS(state, r3);
    Bool     exch = state->attrs->exchange;
    vmiBinop op1  = state->attrs->binop;
    vmiBinop op2  = state->attrs->binop2;
    vmiReg   tmp  = newTemp32(state);

    // save rm in temporary if exchange required and it is clobbered
    if(exch && VMI_REG_EQUAL(rd, rm)) {
        armEmitMoveRR(state, ARM_GPR_BITS, tmp, rm);
        rm = tmp;
    }

    // clear GE flags
    emitClearGEFlags(state);

    // perform the parallel operations
    emitParallelBinopInt(state, bits, op1, rd, rn, rm, 0, exch?2:0, CPSR_GE10);
    emitParallelBinopInt(state, bits, op2, rd, rn, rm, 2, exch?0:2, CPSR_GE32);

    // free temporary
    freeTemp32(state);
}


////////////////////////////////////////////////////////////////////////////////
// PACKING, UNPACKING, SATURATION AND REVERSAL INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Emit code for SSAT/SSAT16
//
static void emitSSAT(
    armMorphStateP state,
    vmiBinop       so,
    Uns32          bits,
    vmiReg         rd,
    vmiReg         rn
) {
    vmiReg    t1    = newTemp32(state);
    vmiReg    tf    = getTemp(state);
    Uns32     shift = state->info.c;
    Uns32     width = state->info.w;
    vmiFlags  flags = getZFFlags(tf);
    vmiLabelP done  = armEmitNewLabel();

    // create bitmasks
    Uns32 signMask = 1<<(width-1);
    Uns32 maxMask  = signMask-1;
    Uns32 propMask = ~maxMask;

    // mask propMask to the operand width
    if(bits!=32) {
        propMask &= (1<<bits)-1;
    }

    // get shifted argument
    armEmitBinopRRC(state, bits, so, rd, rn, shift, 0);

    // mask with propMask
    armEmitBinopRRC(state, bits, vmi_AND, t1, rd, propMask, &flags);

    // no action if the top bits are all zero
    armEmitCondJumpLabel(tf, True, done);

    // no action if the top bits are all one
    armEmitCompareRCJumpLabel(bits, vmi_COND_EQ, t1, propMask, done);

    // saturation required: set the sticky Q flag
    armEmitMoveRC(state, 8, ARM_QF, 1);

    // is the argument signed?
    armEmitCompareRC(state, bits, vmi_COND_S, rd, 0, tf);

    // saturate result based on original sign
    armEmitCondMoveRCC(state, bits, tf, True, rd, propMask, maxMask);

    // jump to here when done
    armEmitInsertLabel(done);

    // free temporaries
    freeTemp32(state);
}

//
// Emit code for USAT/USAT16
//
static void emitUSAT(
    armMorphStateP state,
    vmiBinop       so,
    Uns32          bits,
    vmiReg         rd,
    vmiReg         rn
) {
    vmiReg    tf    = getTemp(state);
    Uns32     shift = state->info.c;
    Uns32     width = state->info.w;
    vmiLabelP done  = armEmitNewLabel();

    // create bitmasks
    Uns32 maxMask  = (1<<width)-1;
    Uns32 propMask = ~maxMask;

    // mask propMask to the operand width
    if(bits!=32) {
        propMask &= (1<<bits)-1;
    }

    // get shifted argument
    armEmitBinopRRC(state, bits, so, rd, rn, shift, 0);

    // no action if the top bits are all zero
    armEmitTestRCJumpLabel(bits, vmi_COND_Z, rd, propMask, done);

    // saturation required: set the sticky Q flag
    armEmitMoveRC(state, 8, ARM_QF, 1);

    // is the argument signed?
    armEmitCompareRC(state, bits, vmi_COND_S, rd, 0, tf);

    // saturate result based on original sign
    armEmitCondMoveRCC(state, bits, tf, True, rd, 0, maxMask);

    // jump to here when done
    armEmitInsertLabel(done);
}

//
// These specify byte offsets equivalent to ROR values in SXTAH etc
//
const static Uns32 rorDelta1[] = {0, 1, 2, 3};
const static Uns32 rorDelta2[] = {2, 3, 0, 1};

//
// Extract byte into temporary with sign extension
//
static void emitExtByte(
    armMorphStateP state,
    vmiReg         rm,
    vmiReg         t1,
    Bool           sextend
) {
    Uns32 bits   = ARM_GPR_BITS;
    Uns32 dIndex = state->info.c/8;
    Uns32 delta1 = rorDelta1[dIndex];

    // do the extension
    armEmitMoveExtendRR(state, bits, t1, 8, VMI_REG_DELTA(rm,delta1), sextend);
}

//
// Extract word into temporary with sign extension
//
static void emitExtWord(
    armMorphStateP state,
    vmiReg         rm,
    vmiReg         t1,
    Bool           sextend
) {
    Uns32 bits   = ARM_GPR_BITS;
    Uns32 dIndex = state->info.c/8;
    Uns32 delta1 = rorDelta1[dIndex];

    // if the word straddles the register boundary, handle it specially
    if(delta1==3) {
        armEmitBinopRRC(state, bits, vmi_ROR, t1, rm, 24, 0);
        rm     = t1;
        delta1 = 0;
    }

    // do the extension
    armEmitMoveExtendRR(state, bits, t1, 16, VMI_REG_DELTA(rm,delta1), sextend);
}

//
// Extract byte pair into temporaries with sign extension
//
static void emitExtBytePair(
    armMorphStateP state,
    vmiReg         rm,
    vmiReg         t1,
    vmiReg         t2,
    Bool           sextend
) {
    Uns32 bits   = 16;
    Uns32 dIndex = state->info.c/8;
    Uns32 delta1 = rorDelta1[dIndex];
    Uns32 delta2 = rorDelta2[dIndex];

    // do the extension
    armEmitMoveExtendRR(state, bits, t1, 8, VMI_REG_DELTA(rm,delta1), sextend);
    armEmitMoveExtendRR(state, bits, t2, 8, VMI_REG_DELTA(rm,delta2), sextend);
}

//
// Emit code for SXTAB/UXTAB
//
static void emitXTAB(armMorphStateP state, Bool sextend) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rd   = GET_RD(state, r1);
    vmiReg rn   = GET_RS(state, r2);
    vmiReg rm   = GET_RS(state, r3);
    vmiReg t1   = getTemp(state);

    // extract extended byte into temporary
    emitExtByte(state, rm, t1, sextend);

    // create result from 32-bit addition
    armEmitBinopRRR(state, bits, vmi_ADD, rd, rn, t1, 0);
}

//
// Emit code for SXTAH/UXTAH
//
static void emitXTAH(armMorphStateP state, Bool sextend) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rd   = GET_RD(state, r1);
    vmiReg rn   = GET_RS(state, r2);
    vmiReg rm   = GET_RS(state, r3);
    vmiReg t1   = getTemp(state);

    // extract extended word into temporary
    emitExtWord(state, rm, t1, sextend);

    // create result from 32-bit addition
    armEmitBinopRRR(state, bits, vmi_ADD, rd, rn, t1, 0);
}

//
// Emit code for SXTAB16/UXTAB16
//
static void emitXTAB16(armMorphStateP state, Bool sextend) {

    Uns32  bits = 16;
    vmiReg rdL  = GET_RD(state, r1);
    vmiReg rnL  = GET_RS(state, r2);
    vmiReg rdH  = VMI_REG_DELTA(rdL, 2);
    vmiReg rnH  = VMI_REG_DELTA(rnL, 2);
    vmiReg rm   = GET_RS(state, r3);
    vmiReg t1   = newTemp32(state);
    vmiReg t2   = getTemp(state);

    // extract extended bytes into temporaries
    emitExtBytePair(state, rm, t1, t2, sextend);

    // create result from two 16-bit additions
    armEmitBinopRRR(state, bits, vmi_ADD, rdL, rnL, t1, 0);
    armEmitBinopRRR(state, bits, vmi_ADD, rdH, rnH, t2, 0);

    // free temporary
    freeTemp32(state);
}

//
// Emit code for SXTB16/UXTB16
//
static void emitXTB16(armMorphStateP state, Bool sextend) {

    Uns32  bits = 16;
    vmiReg rdL  = GET_RD(state, r1);
    vmiReg rdH  = VMI_REG_DELTA(rdL, 2);
    vmiReg rm   = GET_RS(state, r2);
    vmiReg t1   = newTemp32(state);
    vmiReg t2   = getTemp(state);

    // extract extended bytes into temporaries
    emitExtBytePair(state, rm, t1, t2, sextend);

    // create result from two 16-bit moves
    armEmitMoveRR(state, bits, rdL, t1);
    armEmitMoveRR(state, bits, rdH, t2);

    // free temporary
    freeTemp32(state);
}

//
// Emit code for SXTB/UXTB
//
static void emitXTB(armMorphStateP state, Bool sextend) {

    vmiReg rd = GET_RD(state, r1);
    vmiReg rm = GET_RS(state, r2);

    emitExtByte(state, rm, rd, sextend);
}

//
// Emit code for SXTH/UXTH
//
static void emitXTH(armMorphStateP state, Bool sextend) {

    vmiReg rd = GET_RD(state, r1);
    vmiReg rm = GET_RS(state, r2);

    emitExtWord(state, rm, rd, sextend);
}

//
// Emit code for PKHBT
//
ARM_MORPH_FN(armEmitPKHBT) {

    Uns32    bits  = ARM_GPR_BITS;
    vmiReg   rd    = GET_RD(state, r1);
    vmiReg   rn    = GET_RS(state, r2);
    vmiReg   rm    = GET_RS(state, r3);
    vmiReg   t1    = newTemp32(state);
    vmiReg   t2    = getTemp(state);
    Uns32    shift = state->info.c;
    vmiBinop so    = mapShiftOp(state->info.so);

    // create lower half of the result
    armEmitBinopRRC(state, bits, vmi_AND, t1, rn, 0xffff, 0);

    // create upper half of the result
    armEmitBinopRRC(state, bits, so, t2, rm, shift, 0);

    // mask upper half of the result if required
    if(shift<16) {
        armEmitBinopRC(state, bits, vmi_AND, t2, 0xffff0000, 0);
    }

    // create combined result
    armEmitBinopRRR(state, bits, vmi_OR, rd, t1, t2, 0);

    // free temporary
    freeTemp32(state);
}

//
// Emit code for PKHTB
//
ARM_MORPH_FN(armEmitPKHTB) {

    Uns32    bits  = ARM_GPR_BITS;
    vmiReg   rd    = GET_RD(state, r1);
    vmiReg   rn    = GET_RS(state, r2);
    vmiReg   rm    = GET_RS(state, r3);
    vmiReg   t1    = newTemp32(state);
    vmiReg   t2    = getTemp(state);
    Uns32    shift = state->info.c;
    vmiBinop so    = mapShiftOp(state->info.so);

    // create upper half of the result
    armEmitBinopRRC(state, bits, vmi_AND, t1, rn, 0xffff0000, 0);

    // create lower half of the result
    armEmitBinopRRC(state, bits, so, t2, rm, shift, 0);
    armEmitBinopRC(state, bits, vmi_AND, t2, 0xffff, 0);

    // create combined result
    armEmitBinopRRR(state, bits, vmi_OR, rd, t1, t2, 0);

    // free temporary
    freeTemp32(state);
}

//
// Emit code for SSAT
//
ARM_MORPH_FN(armEmitSSAT) {

    Uns32    bits = ARM_GPR_BITS;
    vmiBinop so   = mapShiftOp(state->info.so);
    vmiReg   rd   = GET_RD(state, r1);
    vmiReg   rn   = GET_RS(state, r2);

    emitSSAT(state, so, bits, rd, rn);
}

//
// Emit code for SSAT16
//
ARM_MORPH_FN(armEmitSSAT16) {

    Uns32    bits = ARM_GPR_BITS/2;
    vmiBinop so   = vmi_SHR;
    vmiReg   rd   = GET_RD(state, r1);
    vmiReg   rn   = GET_RS(state, r2);

    emitSSAT(state, so, bits, VMI_REG_DELTA(rd, 0), VMI_REG_DELTA(rn, 0));
    emitSSAT(state, so, bits, VMI_REG_DELTA(rd, 2), VMI_REG_DELTA(rn, 2));
}

//
// Emit code for USAT
//
ARM_MORPH_FN(armEmitUSAT) {

    Uns32    bits = ARM_GPR_BITS;
    vmiBinop so   = mapShiftOp(state->info.so);
    vmiReg   rd   = GET_RD(state, r1);
    vmiReg   rn   = GET_RS(state, r2);

    emitUSAT(state, so, bits, rd, rn);
}

//
// Emit code for USAT16
//
ARM_MORPH_FN(armEmitUSAT16) {

    Uns32    bits = ARM_GPR_BITS/2;
    vmiBinop so   = vmi_SHR;
    vmiReg   rd   = GET_RD(state, r1);
    vmiReg   rn   = GET_RS(state, r2);

    emitUSAT(state, so, bits, VMI_REG_DELTA(rd, 0), VMI_REG_DELTA(rn, 0));
    emitUSAT(state, so, bits, VMI_REG_DELTA(rd, 2), VMI_REG_DELTA(rn, 2));
}

//
// Emit code for SXTAB
//
ARM_MORPH_FN(armEmitSXTAB) {
    emitXTAB(state, True);
}

//
// Emit code for UXTAB
//
ARM_MORPH_FN(armEmitUXTAB) {
    emitXTAB(state, False);
}

//
// Emit code for SXTAB16
//
ARM_MORPH_FN(armEmitSXTAB16) {
    emitXTAB16(state, True);
}

//
// Emit code for UXTAB16
//
ARM_MORPH_FN(armEmitUXTAB16) {
    emitXTAB16(state, False);
}

//
// Emit code for SXTAH
//
ARM_MORPH_FN(armEmitSXTAH) {
    emitXTAH(state, True);
}

//
// Emit code for UXTAH
//
ARM_MORPH_FN(armEmitUXTAH) {
    emitXTAH(state, False);
}

//
// Emit code for SXTB
//
ARM_MORPH_FN(armEmitSXTB) {
    emitXTB(state, True);
}

//
// Emit code for UXTB
//
ARM_MORPH_FN(armEmitUXTB) {
    emitXTB(state, False);
}

//
// Emit code for SXTB16
//
ARM_MORPH_FN(armEmitSXTB16) {
    emitXTB16(state, True);
}

//
// Emit code for UXTB16
//
ARM_MORPH_FN(armEmitUXTB16) {
    emitXTB16(state, False);
}

//
// Emit code for SXTH
//
ARM_MORPH_FN(armEmitSXTH) {
    emitXTH(state, True);
}

//
// Emit code for UXTH
//
ARM_MORPH_FN(armEmitUXTH) {
    emitXTH(state, False);
}

//
// Emit code for SEL
//
ARM_MORPH_FN(armEmitSEL) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rd   = GET_RD(state, r1);
    vmiReg rn   = GET_RS(state, r2);
    vmiReg rm   = GET_RS(state, r3);
    vmiReg t1  = newTemp32(state);
    vmiReg t2  = getTemp(state);

    // select GE bits from CPSR
    armEmitBinopRRC(state, bits, vmi_SHR, t1, ARM_CPSR, 16, 0);
    armEmitBinopRC (state, bits, vmi_AND, t1, 0xf, 0);

    // convert from 0..15 to bitmask
    armEmitBinopRC(state, bits, vmi_IMUL, t1, 0x00204081, 0);
    armEmitBinopRC(state, bits, vmi_AND,  t1, 0x01010101, 0);
    armEmitBinopRC(state, bits, vmi_IMUL, t1, 0xff,       0);

    // get components from each source
    armEmitBinopRRR(state, bits, vmi_AND,  t2, rn, t1, 0);
    armEmitBinopRRR(state, bits, vmi_ANDN, t1, rm, t1, 0);

    // assign result register
    armEmitBinopRRR(state, bits, vmi_OR, rd, t1, t2, 0);

    // free temporary
    freeTemp32(state);
}


//
// Emit code for REV
//
ARM_MORPH_FN(armEmitREV) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rd   = GET_RD(state, r1);
    vmiReg rm   = GET_RS(state, r2);

    armEmitUnopRR(state, bits, vmi_SWP, rd, rm, 0);
}

//
// Emit code for REV16
//
ARM_MORPH_FN(armEmitREV16) {

    Uns32  bits = 16;
    vmiReg rdL  = GET_RD(state, r1);
    vmiReg rmL  = GET_RS(state, r2);
    vmiReg rdH  = VMI_REG_DELTA(rdL, 2);
    vmiReg rmH  = VMI_REG_DELTA(rmL, 2);

    armEmitUnopRR(state, bits, vmi_SWP, rdL, rmL, 0);
    armEmitUnopRR(state, bits, vmi_SWP, rdH, rmH, 0);
}

//
// Emit code for REVSH
//
ARM_MORPH_FN(armEmitREVSH) {

    Uns32  bits = 16;
    vmiReg rd   = GET_RD(state, r1);
    vmiReg rm   = GET_RS(state, r2);

    armEmitUnopRR(state, bits, vmi_SWP, rd, rm, 0);
    armEmitMoveExtendRR(state, ARM_GPR_BITS, rd, bits, rd, True);
}

//
// This array will hold bit-reversed byte values for RBIT
//
static Uns8 rbit8[256];

//
// Return bit-reversed value
//
static Uns32 doRBIT(Uns32 value) {

    union {Uns32 u32; Uns8 u8[4];} u1 = {value};
    union {Uns32 u32; Uns8 u8[4];} u2;

    // generate reversed result a byte at a time using the lookup table
    u2.u8[0] = rbit8[u1.u8[3]];
    u2.u8[1] = rbit8[u1.u8[2]];
    u2.u8[2] = rbit8[u1.u8[1]];
    u2.u8[3] = rbit8[u1.u8[0]];

    // return the reversed result
    return u2.u32;
}

//
// Emit code for RBIT
//
ARM_MORPH_FN(armEmitRBIT) {

    Uns32  bits = ARM_GPR_BITS;
    vmiReg rd   = GET_RD(state, r1);
    vmiReg rm   = GET_RS(state, r2);

    static Bool init;

    // set up rbit8 table if required
    if(!init) {

        Uns32 i;

        for(i=0; i<256; i++) {

            Uns8  byte   = i;
            Uns8  result = 0;
            Uns32 j;

            for(j=0; j<8; j++) {
                result = (result<<1) | (byte&1);
                byte >>= 1;
            }

            rbit8[i] = result;
        }

        init = True;
    }

    // emit embedded call to perform operation
    armEmitArgReg(state, bits, rm);
    armEmitCallResult(state, (vmiCallFn)doRBIT, bits, rd);
}


////////////////////////////////////////////////////////////////////////////////
// SIGNED MULTIPLY INSTRUCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Emit code to perform dual multiply, with results in t1 and t2
//
static void emitDualMultiply(
    armMorphStateP state,
    vmiReg         rnL,
    vmiReg         rmL,
    vmiReg         t1,
    vmiReg         t2
) {
    Uns32  bits = ARM_GPR_BITS;
    vmiReg rnH  = VMI_REG_DELTA(rnL, 2);
    vmiReg rmH  = VMI_REG_DELTA(rmL, 2);
    vmiReg t3   = getTemp(state);

    // exchange arguments if required
    if(state->attrs->exchange) {
        vmiReg tmp = rmL; rmL = rmH; rmH = tmp;
    }

    // do first multiply
    armEmitMoveExtendRR(state, bits, t1, 16, rnL, True);
    armEmitMoveExtendRR(state, bits, t3, 16, rmL, True);
    armEmitBinopRR(state, bits, vmi_IMUL, t1, t3, 0);

    // do second multiply
    armEmitMoveExtendRR(state, bits, t2, 16, rnH, True);
    armEmitMoveExtendRR(state, bits, t3, 16, rmH, True);
    armEmitBinopRR(state, bits, vmi_IMUL, t2, t3, 0);
}

//
// Emit code for SMLAD/SMLSD
//
ARM_MORPH_FN(armEmitSMLXD) {

    Uns32    bits = ARM_GPR_BITS;
    vmiBinop op   = state->attrs->binop;
    vmiReg   rd   = GET_RD(state, r1);
    vmiReg   rnL  = GET_RS(state, r2);
    vmiReg   rmL  = GET_RS(state, r3);
    vmiReg   ra   = GET_RS(state, r4);
    vmiReg   t1   = newTemp32(state);
    vmiReg   t2   = newTemp32(state);

    // do dual multiply, results in t1 and t2
    emitDualMultiply(state, rnL, rmL, t1, t2);

    // combine results, setting CPSR.Q if overflow (ADD only, special case of
    // 0x8000*0x8000 + 0x8000*0x8000)
    if(op==vmi_ADD) {
        emitOpSetQ(state, bits, op, t1, t1, t2);
    } else {
        armEmitBinopRR(state, bits, op, t1, t2, 0);
    }

    // accumulate, setting CPSR.Q if overflow
    emitOpSetQ(state, bits, vmi_ADD, rd, ra, t1);

    // free temporaries
    freeTemp32(state);
    freeTemp32(state);
}

//
// Emit code for SMUAD/SMUSD
//
ARM_MORPH_FN(armEmitSMUXD) {

    Uns32    bits = ARM_GPR_BITS;
    vmiBinop op   = state->attrs->binop;
    vmiReg   rd   = GET_RD(state, r1);
    vmiReg   rnL  = GET_RS(state, r2);
    vmiReg   rmL  = GET_RS(state, r3);
    vmiReg   t1   = newTemp32(state);
    vmiReg   t2   = newTemp32(state);

    // do dual multiply, results in t1 and t2
    emitDualMultiply(state, rnL, rmL, t1, t2);

    // generate result, setting CPSR.Q if overflow (ADD only)
    if(op==vmi_ADD) {
        emitOpSetQ(state, bits, op, rd, t1, t2);
    } else {
        armEmitBinopRRR(state, bits, op, rd, t1, t2, 0);
    }

    // free temporaries
    freeTemp32(state);
    freeTemp32(state);
}

//
// Emit code for SMLALD/SMLSLD
//
ARM_MORPH_FN(armEmitSMLXLD) {

    Uns32    bits  = ARM_GPR_BITS;
    vmiBinop op    = state->attrs->binop;
    vmiReg   rdLo  = GET_RD(state, r1);
    vmiReg   rdHi  = GET_RD(state, r2);
    vmiReg   rnL   = GET_RS(state, r3);
    vmiReg   rmL   = GET_RS(state, r4);
    vmiReg   t1    = newTemp64(state);
    vmiReg   t2    = newTemp32(state);
    vmiReg   tf    = t2;
    vmiFlags flags = getCFFlags(tf);

    // do dual multiply, results in t1 and t2
    emitDualMultiply(state, rnL, rmL, t1, t2);

    // extend results to 64 bits
    armEmitMoveExtendRR(state, 64, t1, bits, t1, True);
    armEmitMoveExtendRR(state, 64, t2, bits, t2, True);

    // add/subtract extended results
    armEmitBinopRR(state, 64, op, t1, t2, 0);

    // add total to accumulator
    armEmitBinopRR(state, bits, vmi_ADD, rdLo, VMI_REG_DELTA(t1, 0), &flags);
    armEmitBinopRR(state, bits, vmi_ADC, rdHi, VMI_REG_DELTA(t1, 4), &flags);

    // free temporaries
    freeTemp32(state);
    freeTemp64(state);
}

//
// Emit code for SMMLX
//
ARM_MORPH_FN(armEmitSMMLX) {

    Uns32    bits = ARM_GPR_BITS;
    vmiBinop op   = state->attrs->binop;
    vmiReg   rd   = GET_RD(state, r1);
    vmiReg   rn   = GET_RS(state, r2);
    vmiReg   rm   = GET_RS(state, r3);
    vmiReg   t1L  = newTemp64(state);
    vmiReg   t1H  = VMI_REG_DELTA(t1L, 4);

    // do the multiply
    armEmitMulopRRR(state, bits, vmi_IMUL, t1H, t1L, rn, rm, 0);

    // accumulate if required
    if(state->attrs->accumulate) {

        vmiReg t2L = getTemp(state);
        vmiReg t2H = VMI_REG_DELTA(t2L, 4);

        armEmitMoveRC(state, bits, t2L, 0);
        armEmitMoveRR(state, bits, t2H, GET_RS(state, r4));

        armEmitBinopRR(state, 64, op, t1L, t2L, 0);
    }

    // round if required
    if(state->attrs->round) {
        armEmitBinopRC(state, 64, vmi_ADD, t1L, 0x80000000, 0);
    }

    // assign to result
    armEmitMoveRR(state, bits, rd, t1H);

    // free temporary
    freeTemp64(state);
}


////////////////////////////////////////////////////////////////////////////////
// SIMD/VFP Utility functions
////////////////////////////////////////////////////////////////////////////////

//
// Emit undefined instruction if FPU is disabled
//
#define VFP_DISABLED(_S) emitUndefined(_S, False); return False;

//
// Get floating point operation type for ARM integer type from size of operand in bytes
//
static vmiFType bytesToIType(Uns32 ebytes, Bool isSigned) {
    switch(ebytes) {
        case 2:
            return isSigned ? vmi_FT_16_INT : vmi_FT_16_UNS;
        case 4:
            return isSigned ? vmi_FT_32_INT : vmi_FT_32_UNS;
        default:
            VMI_ABORT("%s: unimplemented size for floating point type: %d bytes", FUNC_NAME, ebytes);
            return 0;    // Not reached
    }
}

//
// Get floating point operation type for ARM floating point type from size of operand in bytes
//
static vmiFType bytesToFType(Uns32 ebytes) {
    switch(ebytes) {
        case 4:
            return vmi_FT_32_IEEE_754;
        case 8:
            return vmi_FT_64_IEEE_754;
        default:
            VMI_ABORT("%s: unimplemented size for floating point type: %d bytes", FUNC_NAME, ebytes);
            return 0;    // Not reached
    }
}

//
// Get floating point operation type for ARM instruction
//
static vmiFType getFType(armMorphStateP state) {
    return bytesToFType(state->attrs->ebytes);
}

//
// This is the ARM CheckAdvSIMDOrVFPEnabled primitive
//
static Bool checkAdvSIMDOrVFPEnabled(
    armMorphStateP state,
    Bool           include_fpexc_check,
    Bool           advsimd
) {
    armP  arm        = state->arm;

    // Check if AdvSIMD or VFP is present
    if ((advsimd && !ADVSIMD_PRESENT(arm)) || (!advsimd && !VFP_PRESENT(arm))) {
        VFP_DISABLED(state);
    }

    Bool  inUserMode = IN_USER_MODE(arm);
    Uns32 cp10Enable = CP_FIELD(arm, CPACR, cp10);

    // this code block is dependent on the enable state of coprocessor 10
    armEmitValidateBlockMask(ARM_BM_CP10);

    // check CPAR for permission to use cp10 (and cp11)
    if(inUserMode && !(cp10Enable&2)) {
        VFP_DISABLED(state);
    } else if(!inUserMode && !(cp10Enable&1)) {
        VFP_DISABLED(state);
    }

    // if the advanced SIMD extension is specified, check whether it is enabled
    if(advsimd) {

        // this code block is dependent on the enable state of the advanced
        // SIMD extension
        armEmitValidateBlockMask(ARM_BM_ASEDIS);

        if(CP_FIELD(arm, CPACR, ASEDIS)) {
            VFP_DISABLED(state);
        }
    }

    // if required, check FPEXC enabled bit
    if(include_fpexc_check) {

        // this code block is dependent on the enable state of the advanced
        // SIMD extension
        armEmitValidateBlockMask(ARM_BM_SDFP_EN);

        if(!SDFP_FIELD(arm, FPEXC, EN)) {
            VFP_DISABLED(state);
        }
    }

    // extension is enabled
    return True;
}

//
// This is the ARM CheckSIMDEnabled primitive
//
static Bool checkAdvSIMDEnabled(armMorphStateP state) {

    VMI_ASSERT(isSIMDInstr(state), "Wrong instruction type for SIMD instruction");
    return checkAdvSIMDOrVFPEnabled(state, True, True);

}

//
// This is the ARM CheckVFPEnabled primitive
//
static Bool checkVFPEnabled(armMorphStateP state, Bool include_fpexc_check) {

    VMI_ASSERT(isVFPInstr(state), "Wrong instruction type for VFP instruction");
    return checkAdvSIMDOrVFPEnabled(state, include_fpexc_check, False);

}

//
// Load the Flt64 register with the floating point constant value of 2^n
//
static vmiReg getFPConstPower2Flt64(armMorphStateP state, Uns32 n) {

    Uns64  twoN = (1ULL << n);
    vmiReg r    = newTemp64(state);

    union {Flt64 f64; Uns64 u64;} u = {f64:twoN};

    armEmitMoveRC(state, 64, r, u.u64);

    return r;
}

//
// Load the Flt80 register with the floating point constant value of 2^n
//
static vmiReg getFPConstPower2Flt80(armMorphStateP state, Uns32 n) {

    Uns64  twoN = (1ULL << n);
    vmiReg r    = newTemp128(state);

    union {Flt80 f80; Uns64 u64; Uns16 u16[8];} u = {f80:twoN};

    armEmitMoveRC(state, 64, r, u.u64);
    armEmitMoveRC(state, 16, VMI_REG_DELTA(r, 8), u.u16[4]);

    return r;
}

//
// This is FPNeg from psuedo-code
// Negate the value in the register by toggling the sign bit
//
static void FPNeg(armMorphStateP state, vmiReg dest, vmiReg src, Uns32 size) {

    Uns64 signBit = 1ULL << (size-1);

    armEmitBinopRRC(state, size, vmi_XOR, dest, src, signBit, 0);
}

//
// This is FixedToFP from psuedo-code
// Note: fpscr_controlled selection is done automatically in armEmit... call
//
static void fixedToFP(
    armMorphStateP state,
    vmiReg         result,
    Uns32          resultBytes,
    vmiReg         operand,
    Uns32          operandBytes,
    Uns32          fracBits,
    Bool           isSigned,
    Bool           roundToNearest
) {
    vmiFPRC  round       = roundToNearest ? vmi_FPR_NEAREST : vmi_FPR_CURRENT;
    vmiFType operandType = bytesToIType(operandBytes, isSigned);

    VMI_ASSERT(operandBytes==4 || operandBytes==2, "operand size in bytes %d must be 2 or 4", operandBytes);
    VMI_ASSERT(fracBits <= operandBytes*8, "fracBits = %d must be <= %d", fracBits, operandBytes*8);

    if (fracBits) {

        // Fixed point to floating point - must scale by fracBits
        Bool   singlePrec = (resultBytes == 4);
        vmiReg t          = newTemp64(state);
        vmiReg power2     = getFPConstPower2Flt64(state, fracBits);

        // Convert integer to double precision floating point (rounding mode unused here)
        armEmitFConvertRR(state, vmi_FT_64_IEEE_754, t, operandType, operand, vmi_FPR_CURRENT);

        if (singlePrec) {

            // Get result / 2^fracBits (always exact, so no rounding)
            armEmitFBinopSimdRRR(state, vmi_FT_64_IEEE_754, 1, vmi_FDIV, t, t, power2);

            // Convert to single precision (may be rounded)
            armEmitFConvertRR(state, vmi_FT_32_IEEE_754, result, vmi_FT_64_IEEE_754, t, round);

        } else {

            // result = value / 2^fracBits (always exact, so no rounding)
            armEmitFBinopSimdRRR(state, vmi_FT_64_IEEE_754, 1, vmi_FDIV, result, t, power2);
        }

    } else {

        // Just convert integer to target floating point type (may be rounded)
        vmiFType resultType = bytesToFType(resultBytes);

        armEmitFConvertRR(state, resultType, result, operandType, operand, round);
    }
}

//
// This is FPToFixed from psuedo-code
// Note: fpscr_controlled selection is done automatically in armEmit... call
//
static void FPToFixed (
    armMorphStateP state,
    vmiReg         result,
    Uns32          resultBytes,
    vmiReg         operand,
    Uns32          operandBytes,
    Uns32          fracBits,
    Bool           isSigned,
    Bool           roundTowardsZero
) {
    vmiFPRC  round  = roundTowardsZero ? vmi_FPR_ZERO : vmi_FPR_CURRENT;
    vmiFType opType = bytesToFType(operandBytes);

    VMI_ASSERT(operandBytes==4 || operandBytes==8, "operand size in bytes %d must be 8 or 4", operandBytes);
    VMI_ASSERT(resultBytes==4 || resultBytes==2, "result size in bytes %d must be 2 or 4", resultBytes);
    VMI_ASSERT(fracBits <= resultBytes*8, "fracBits = %d must be <= %d", fracBits, resultBytes*8);

    if (fracBits) {

        // Scale by fracBits
        vmiReg power2 = getFPConstPower2Flt80(state, fracBits);
        vmiReg t      = newTemp128(state);

        // Convert operand to 80 bit fp value (rounding mode unused here)
        armEmitFConvertRR(state, vmi_FT_80_X87, t, opType, operand, vmi_FPR_CURRENT);

        // t = t * 2^fracBits (always bigger, so no rounding)
        armEmitFBinopSimdRRR(state, vmi_FT_80_X87, 1, vmi_FMUL, t, t, power2);

        // Use temporary as operand source
        opType  = vmi_FT_80_X87;
        operand = t;
    }

    // convert to required type
    vmiFType resultType = bytesToIType(resultBytes, isSigned);
    armEmitFConvertRR(state, resultType, result, opType, operand, round);
}

//
// Validate the processor is not in user mode
//
static Bool validateNotUserMode(armMorphStateP state) {
    if (IN_USER_MODE(state->arm)) {
        emitUndefined(state, False);
        return False;
    } else {
        return True;
    }
}

//
// Set FPSCR flags according to the vmiFPRelation value
//
static void setFPSCRFlags(armP arm, vmiFPRelation relation) {

    if (relation == vmi_FPRL_UNORDERED) {
        arm->sdfpAFlags.ZF = arm->sdfpAFlags.NF = 0;
        arm->sdfpAFlags.CF = arm->sdfpAFlags.VF = 1;
    } else if (relation == vmi_FPRL_EQUAL) {
        arm->sdfpAFlags.NF = arm->sdfpAFlags.VF = 0;
        arm->sdfpAFlags.ZF = arm->sdfpAFlags.CF = 1;
    } else if (relation == vmi_FPRL_LESS) {
        arm->sdfpAFlags.ZF = arm->sdfpAFlags.CF= arm->sdfpAFlags.VF = 0;
        arm->sdfpAFlags.NF = 1;
    } else if (relation == vmi_FPRL_GREATER) {
        arm->sdfpAFlags.ZF = arm->sdfpAFlags.NF= arm->sdfpAFlags.VF = 0;
        arm->sdfpAFlags.CF = 1;
    } else {
        VMI_ABORT("unsupported fp relation result 0x%x", relation);
    }

}

//
// Assign CPSR VCMPflags from FPSCR flags
//
static void getFPSCRFlags(armP arm) {

    arm->aflags = arm->sdfpAFlags;

}

////////////////////////////////////////////////////////////////////////////////
// SIMD/VFP VMOV, etc instructions
////////////////////////////////////////////////////////////////////////////////

//
// Emit code for VMRS
//
ARM_MORPH_FN(armEmitVMRS) {

    Uns32 bits = ARM_GPR_BITS;

    if(state->info.r2==ARM_VESR_FPSCR) {

        // FPSCR is accessible in user mode if FPEXC.EN is set
        if(!checkVFPEnabled(state, True)) {

            // no action

        } else if(state->info.r1==ARM_REG_PC) {

            // assign CPSR flags from FPSCR flags
            armEmitArgProcessor(state);
            armEmitCall(state, (vmiCallFn)getFPSCRFlags);

            // terminate the code block (derived flags are invalid)
            armEmitEndBlock();

        } else {

            vmiReg rd = GET_RD(state, r1);

            armEmitArgProcessor(state);
            armEmitCallResult(state, (vmiCallFn)armReadFPSCR, bits, rd);
        }

    } else if(validateNotUserMode(state)) {

        // non-FPSCR registers are privileged-only and not affected by FPEXC.EN
        vmiReg rd = GET_RD(state, r1);

        if(!checkVFPEnabled(state, False)) {

            // no action

        } else switch(state->info.r2) {
            case ARM_VESR_FPSID:
                armEmitMoveRR(state, bits, rd, ARM_SDFP_REG(SDFP_ID(FPSID)));
                break;
            case ARM_VESR_MVFR1:
                armEmitMoveRR(state, bits, rd, ARM_SDFP_REG(SDFP_ID(MVFR1)));
                break;
            case ARM_VESR_MVFR0:
                armEmitMoveRR(state, bits, rd, ARM_SDFP_REG(SDFP_ID(MVFR0)));
                break;
            case ARM_VESR_FPEXC:
                armEmitMoveRR(state, bits, rd, ARM_SDFP_REG(SDFP_ID(FPEXC)));
                break;
            default:
                emitUndefined(state, False);
                break;
        }
    }
}

//
// Emit code for VMSR
//
ARM_MORPH_FN(armEmitVMSR) {

    Uns32 bits = ARM_GPR_BITS;

    if(state->info.r1==ARM_VESR_FPSCR) {

        // FPSCR is accessible in user mode if FPEXC.EN is set
        if(!checkVFPEnabled(state, True)) {
            // no action
        } else {
            armEmitArgProcessor(state);
            armEmitArgReg(state, bits, GET_RS(state, r2));
            armEmitCall(state, (vmiCallFn)armWriteFPSCR);
        }

    } else if(validateNotUserMode(state)) {

        // non-FPSCR registers are privileged-only and not affected by FPEXC.EN
        if(!checkVFPEnabled(state, False)) {
            // no action
        } else switch(state->info.r1) {
            case ARM_VESR_FPSID:
                // no action (SerializeVFP)
                break;
            case ARM_VESR_FPEXC:
                armEmitArgProcessor(state);
                armEmitArgReg(state, bits, GET_RS(state, r2));
                armEmitCall(state, (vmiCallFn)armWriteFPEXC);
                break;
            default:
                emitUndefined(state, False);
                break;
        }
    }

    // terminate the code block (block masks or floating point mode may have
    // changed)
    armEmitEndBlock();
}

//
// Emit code for VMOV.F32 Sd, Sm or VMOV.F64 Dd, Dm
//
ARM_MORPH_FN(armEmitVMOVR_VFP) {

    if(checkVFPEnabled(state, True)) {

        Uns32  ebytes = state->attrs->ebytes;
        Uns32  esize  = ebytes*8;
        vmiReg rd     = GET_VFP_REG(state, r1, 0, ebytes);
        vmiReg rs     = GET_VFP_REG(state, r2, 0, ebytes);

        armEmitMoveRR(state, esize, rd, rs);
    }
}

//
// Emit code for VMOV.F32 Sd, # or VMOV.F64 Dd, #
//
ARM_MORPH_FN(armEmitVMOVI_VFP) {

    if(checkVFPEnabled(state, True)) {

        Uns32  ebytes = state->attrs->ebytes;
        Uns32  esize  = ebytes*8;
        vmiReg rd     = GET_VFP_REG(state, r1, 0, ebytes);
        Uns64  mi     = state->info.sdfpMI.u64;

        armEmitMoveRC(state, esize, rd, mi);
    }
}

//
// Emit code for VMOV RT, SN
//
ARM_MORPH_FN(armEmitVMOVRS) {

    if(checkVFPEnabled(state, True)) {

        vmiReg rd = GET_RD(state, r1);
        vmiReg rs = GET_VFP_REG(state, r2, 0, 4);

        armEmitMoveRR(state, ARM_GPR_BITS, rd, rs);
    }
}

//
// Emit code for VMOV SN, RT
//
ARM_MORPH_FN(armEmitVMOVSR) {

    if(checkVFPEnabled(state, True)) {

        vmiReg rd = GET_VFP_REG(state, r1, 0, 4);
        vmiReg rs = GET_RS(state, r2);

        armEmitMoveRR(state, ARM_GPR_BITS, rd, rs);
    }
}

//
// Emit code for VMOV RT, RT2, DN
//
ARM_MORPH_FN(armEmitVMOVRRD) {

    if(checkVFPEnabled(state, True)) {

        vmiReg rdL = GET_RD(state, r1);
        vmiReg rdH = GET_RD(state, r2);
        vmiReg rsL = GET_VFP_REG(state, r3, 0, 8);
        vmiReg rsH = getR64Hi(rsL);

        armEmitMoveRR(state, ARM_GPR_BITS, rdL, rsL);
        armEmitMoveRR(state, ARM_GPR_BITS, rdH, rsH);
    }
}

//
// Emit code for VMOV DN, RT, RT2
//
ARM_MORPH_FN(armEmitVMOVDRR) {

    if(checkVFPEnabled(state, True)) {

        vmiReg rdL = GET_VFP_REG(state, r1, 0, 8);
        vmiReg rdH = getR64Hi(rdL);
        vmiReg rsL = GET_RS(state, r2);
        vmiReg rsH = GET_RS(state, r3);

        armEmitMoveRR(state, ARM_GPR_BITS, rdL, rsL);
        armEmitMoveRR(state, ARM_GPR_BITS, rdH, rsH);
    }
}

//
// Emit code for VMOV RT, RT2, SM, SM1
//
ARM_MORPH_FN(armEmitVMOVRRSS) {

    if(checkVFPEnabled(state, True)) {

        vmiReg rdL = GET_RD(state, r1);
        vmiReg rdH = GET_RD(state, r2);
        vmiReg rsL = GET_VFP_REG(state, r3,   0,   4);
        vmiReg rsH = GET_VFP_REG(state, r3+1, 0, 4);

        armEmitMoveRR(state, ARM_GPR_BITS, rdL, rsL);
        armEmitMoveRR(state, ARM_GPR_BITS, rdH, rsH);
    }
}

//
// Emit code for VMOV SM, SM1, RT, RT2
//
ARM_MORPH_FN(armEmitVMOVSSRR) {

    if(checkVFPEnabled(state, True)) {

        vmiReg rdL = GET_VFP_REG(state, r1,   0,   4);
        vmiReg rdH = GET_VFP_REG(state, r1+1, 0, 4);
        vmiReg rsL = GET_RS(state, r2);
        vmiReg rsH = GET_RS(state, r3);

        armEmitMoveRR(state, ARM_GPR_BITS, rdL, rsL);
        armEmitMoveRR(state, ARM_GPR_BITS, rdH, rsH);
    }
}

//
// Emit code for VMOV DD[x], RT
//
ARM_MORPH_FN(armEmitVMOVZR) {

    Bool isAdvSIMD = (state->attrs->iType == ARM_TY_SIMD);

    if(checkAdvSIMDOrVFPEnabled(state, True, isAdvSIMD)) {

        Uns32  index  = state->info.index;
        Uns32  ebytes = state->attrs->ebytes;
        Uns32  bits   = ebytes*8;
        vmiReg rd     = GET_SIMD_EL(state, r1, 0, index, ebytes);
        vmiReg rs     = GET_RS(state, r2);

        armEmitMoveRR(state, bits, rd, rs);
    }
}

//
// Emit code for VMOV RT, DD[x]
//
ARM_MORPH_FN(armEmitVMOVRZ) {

    Bool isAdvSIMD = (state->attrs->iType == ARM_TY_SIMD);

    if(checkAdvSIMDOrVFPEnabled(state, True, isAdvSIMD)) {

        Uns32  index  = state->info.index;
        Uns32  ebytes = state->attrs->ebytes;
        Uns32  bits   = ebytes*8;
        vmiReg rd     = GET_RS(state, r1);
        vmiReg rs     = GET_SIMD_EL(state, r2, 0, index, ebytes);

        armEmitMoveExtendRR(state, ARM_GPR_BITS, rd, bits, rs, state->attrs->sextend);
    }
}

//
// Emit code for VDUPR QD or DD, RT
//
ARM_MORPH_FN(armEmitVDUPR) {

    if(checkAdvSIMDEnabled(state)) {

        Uns32  regs      = state->attrs->regs;
        Uns32  elements  = state->attrs->elements;
        Uns32  ebytes    = state->attrs->ebytes;
        Uns32  bits      = ebytes*8;
        vmiReg rs        = GET_RS(state, r2);
        vmiReg rd;
        Uns32  r, e;

        for (r = 0; r < regs; r++) {
            for (e = 0; e < elements; e++) {

                rd = GET_SIMD_EL(state, r1, r, e, ebytes);
                armEmitMoveRR(state, bits, rd, rs);

            }
        }
    }
}

//
// Emit code for VEXT Vd, Vn, Vm, #imm
// (V may be Double or Quad registers)
// Concatenate low order bytes from Vm with high order bytes from Vn into Vd
// Immediate constant indicates byte position of lowest order byte in Vn
//
ARM_MORPH_FN(armEmitVEXT) {

    if(checkAdvSIMDEnabled(state)) {

        Uns32  regs     = state->attrs->regs;
        Uns32  position = state->info.c;

        VMI_ASSERT(regs==1 || regs==2,  "Invalid regs %d", regs);
        VMI_ASSERT(position < (regs*ARM_SIMD_REG_BYTES), "Invalid position %d", position);

        vmiReg rd     = GET_SIMD_REG(state, r1, 0);
        vmiReg rn     = GET_SIMD_REG(state, r2, 0);
        vmiReg rm     = GET_SIMD_REG(state, r3, 0);
        vmiReg r1temp = regs == 1 ? newTemp64(state) : newTemp128(state);
        vmiReg r2temp = regs == 1 ? newTemp64(state) : newTemp128(state);
        vmiReg rs     = VMI_REG_DELTA(r1temp, position);

        // Form Vm:Vn
        armEmitMoveRR(state, ARM_SIMD_REG_BITS*regs, r1temp, rn);
        armEmitMoveRR(state, ARM_SIMD_REG_BITS*regs, r2temp, rm);

        // Vd gets portion of Vm:Vn selected by the value of position
        armEmitMoveRR(state, ARM_SIMD_REG_BITS*regs, rd, rs);

        if (regs == 1) {
            freeTemp64(state);  freeTemp64(state);
        } else {
            freeTemp128(state); freeTemp128(state);
        }

    }
}

//
// Called at run time for a VTBL or VTBX instruction
//
static void doVTBL(armP arm, Uns32 rd, Uns32 rn, Uns32 length, Uns64 controlVec, Uns32 is_vtbl) {

    Uns32 i;

    // form result here - don't go directly to dest reg in case regs overlap
    union {
        Uns8  b[8]; // when viewed as bytes
        Uns64 d;    // When viewed as 64 bit doubleword
    } result;


    for (i = 0; i < ARM_SIMD_REG_BYTES; i++) {

        // Get the next index from controlVec
        Uns32 index = controlVec & 0xff;

        // Shift next byte down in controlVec
        controlVec = controlVec >> 8;

        if (index < 8*length) {

            // if index is in range copy that byte from table
            result.b[i] = arm->vregs.b[ARM_SIMD_REGNUM(rn,index,1)];

        } else if (is_vtbl) {

            // for vtbl out of range index writes a zero to destination
            result.b[i] = 0;

        } else {

            // for vtbx out of range index leaves destination unchanged
            result.b[i] = arm->vregs.b[ARM_SIMD_REGNUM(rd,i,1)];

        }

    }

    // Copy result to dest register
    arm->vregs.d[rd] = result.d;
}

//
// Common code for VTBL and VTBX
// (This uses register indirection so must be implemented with a run time function)
//
static void emitVTBL(armMorphStateP state, Bool is_vtbl) {

    if(checkAdvSIMDEnabled(state)) {

        Uns32  nregs = state->info.nregs;

        VMI_ASSERT(nregs>0 && nregs<=4,  "Invalid regs %d", nregs);

        vmiReg rm = GET_SIMD_REG(state, r3, 0);

        // don't need these here but this catches any registers out of range
        (void) GET_SIMD_REG(state, r1, 0);
        (void) GET_SIMD_EL (state, r2, nregs-1, 0, ARM_SIMD_REG_BYTES);

        armEmitArgProcessor(state);
        armEmitArgUns32(state, state->info.r1);         // Destination register
        armEmitArgUns32(state, state->info.r2);         // Base register of table
        armEmitArgUns32(state, nregs);                  // Length of table
        armEmitArgReg(state, ARM_SIMD_REG_BITS, rm);    // Control vector value
        armEmitArgUns32(state, is_vtbl);                // VTBL or VTBX instruction
        armEmitCall(state, (vmiCallFn)doVTBL);

    }
}

//
// Emit code for VTBL  Dd, <Dn reg list>, Dm
//
ARM_MORPH_FN(armEmitVTBL) {
    emitVTBL(state, True);
}

//
// Emit code for VTBX  Dd, <Dn reg list>, Dm
//
ARM_MORPH_FN(armEmitVTBX) {
    emitVTBL(state, False);
}

//
// Emit code for VDUP Vd, <Dm[x]>
// Duplicate the scalar Dm[x] in every element of Vd
// (V may be Double or Quad registers)
//
ARM_MORPH_FN(armEmitVDUPZ) {

    if(checkAdvSIMDEnabled(state)) {

        Uns32  ebytes   = state->attrs->ebytes;
        Uns32  regs     = state->attrs->regs;
        Uns32  elements = state->attrs->elements;
        Uns32  index    = state->info.index;
        vmiReg rm       = GET_SIMD_EL(state, r2, 0, index, ebytes);
        Uns32  r, e;

        // Note: don't need temporary for the scalar because even if it overlaps with
        //       rd the same value will be copied into it
        for (r = 0; r < regs; r++) {
            for (e = 0; e < elements; e++) {
                vmiReg rd = GET_SIMD_EL(state, r1, r, e, ebytes);
                armEmitMoveRR(state, ebytes*8, rd, rm);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// SIMD/VFP VLD and VST
////////////////////////////////////////////////////////////////////////////////

//
// Callback function for V load/store
//
#define V_LOAD_STORE_FN(_NAME) void _NAME( \
    armMorphStateP state,   \
    Uns32          memBits, \
    vmiReg         base,    \
    Int32          offset,  \
    vmiReg         rd       \
)
typedef V_LOAD_STORE_FN((*VLoadStoreFn));


//
// Swap rdH and rd if endian is big
//
static inline void endianSwapRegs(armMorphStateP state, vmiReg *rdH, vmiReg *rd) {

    memEndian endian = armGetEndian((vmiProcessorP)state->arm, False);

    if (endian == MEM_ENDIAN_BIG) {

        vmiReg t;

        t    = *rdH;
        *rdH = *rd;
        *rd  = t;
    }
}

//
// Emit code to load a SIMD/VFP register from memory address [base]+offset
//
static V_LOAD_STORE_FN(emitVLoad)  {

    if (memBits == 2*ARM_GPR_BITS) {

        vmiReg rdH = getR64Hi(rd);

        endianSwapRegs(state, &rdH, &rd);
        armEmitLoadRRRO(state, memBits, offset, rd, rdH, base, getTemp(state), False, False);

    } else if (memBits <= ARM_GPR_BITS) {

        armEmitLoadRRO(state, ARM_GPR_BITS, memBits, offset, rd, base, False, False);

    } else {

        VMI_ABORT("Invalid memBits %d", memBits);

    }
}

//
// Emit code to store a SIMD/VFP register to memory address [base]+offset
// When double word register adjust register depending on endian
//
static V_LOAD_STORE_FN(emitVStore) {

    if (memBits == 2*ARM_GPR_BITS) {

        vmiReg rdH = getR64Hi(rd);

        endianSwapRegs(state, &rdH, &rd);
        armEmitStoreRRRO(state, memBits, offset, base, rd, rdH);

    } else if (memBits <= ARM_GPR_BITS) {

        armEmitStoreRRO(state, memBits, offset, base, rd);

    } else {

        VMI_ABORT("Invalid memBits %d", memBits);

    }
}


//
// Common code for a VLDR or VSTR instruction
//
static void emitVLoadStore(armMorphStateP state, VLoadStoreFn cb, Bool isLoad) {

    if(checkVFPEnabled(state, True)) {

        Uns32  ebytes  = state->attrs->ebytes;
        Uns32  memBits = ebytes*8;
        vmiReg rd      = GET_VFP_REG(state, r1, 0, ebytes);
        vmiReg base    = GET_RS(state, r2);
        Int32  offset  = state->info.c;

        // base register must be null-checked in ThumbEE mode
        emitNullCheckIfThumbEE(state, base);

        if (isLoad) {

            // For loads only: align the constant if the base register is the
            // program counter (has effect for PC-relative Thumb load instructions only)
            if(VMI_REG_EQUAL(base, ARM_PC)) {
                offset = alignConstWithPC(state, offset);
            }

        }

        (*cb) (state, memBits, base, offset, rd);

    }
}

//
// Emit code for VLDR S or D, [RN, #imm]
//
ARM_MORPH_FN(armEmitVLDR) {
    emitVLoadStore(state,  emitVLoad, True);
}

//
// Emit code for VLDR S or D, [RN, #imm]
//
ARM_MORPH_FN(armEmitVSTR) {
    emitVLoadStore(state,  emitVStore, False);
}

//
// Common code for Load or Store Multiple using the passed call back function
//
static void emitVLoadStoreM(armMorphStateP state, VLoadStoreFn cb) {

    if(checkVFPEnabled(state, True)) {

        Uns32  ebytes     = state->attrs->ebytes;
        Uns32  memBits    = ebytes*8;
        vmiReg base       = GET_RS(state, r1);
        Uns32  nregs      = state->info.nregs;
        Bool   increment  = doIncrement(state);
        Uns32  frameSize  = nregs * ebytes;
        Int32  frameDelta = increment ? frameSize : -frameSize;
        Int32  offset     = increment ? 0 : -frameSize;
        Int32  stepBefore = getStepBefore(state, ebytes);
        Int32  stepAfter  = getStepAfter (state, ebytes);
        Uns32  r;

        // base register must be null-checked in ThumbEE mode
        emitNullCheckIfThumbEE(state, base);

        // load or store registers
        for(r=0; r<nregs; r++) {

            vmiReg rd = GET_VFP_REG(state, r2, r, ebytes);

            offset += stepBefore;
            (*cb) (state, memBits, base, offset, rd);
            offset += stepAfter;
        }

        // perform base register update if required
        if(state->info.wb) {

            armEmitBinopRC(state, ARM_GPR_BITS, vmi_ADD, base, frameDelta, 0);

        }
    }
}

//
// Emit code for VLDM RN{!}, <S or D reg list>
//
ARM_MORPH_FN(armEmitVLDM) {
    emitVLoadStoreM(state, emitVLoad);
}

//
// Emit code for VSTM RN{!}, <S or D reg list>
//
ARM_MORPH_FN(armEmitVSTM) {
    emitVLoadStoreM(state, emitVStore);
}

////////////////////////////////////////////////////////////////////////////////
// SIMD Element or structure load/store instructions
////////////////////////////////////////////////////////////////////////////////

//
// Emit code to perform alignment check for SIMD element or structure load/store instructions
// with alignment specified. align is the number of bytes and may be 2, 4, 8, 16 or 32
//
static void emitAlignCheck(armMorphStateP state, vmiReg rb, Uns32 align, Bool isLoad) {

    if (align > 1) {

        Uns32 bits = align*8;

        VMI_ASSERT(align==2||align==4||align==8||align==16||align==32, "Invalid align %d", align);

        if(isLoad) {
            armEmitTryLoadRC(state, bits, 0, rb);
        } else {
            armEmitTryStoreRC(state, bits, 0, rb);
        }
    }
}

//
// Perform writeback for a SIMD VLDn/VSTn instruction
//
static void emitVLDSTNWback(armMorphStateP state, vmiReg base, Uns32 offset) {

    Uns32 m = state->info.r3;

    if(m == 15) {

        // Specifying register 15 indicates no writeback should be performed

    } else if (m == 13) {

        // Specifying register 13 means increment base by number of bytes accessed
        armEmitBinopRC(state, ARM_GPR_BITS, vmi_ADD, base, offset, 0);

    } else {

        // Any other register means increment base by value in that register
        vmiReg rm = GET_RD(state, r3);
        armEmitBinopRR(state, ARM_GPR_BITS, vmi_ADD, base, rm, 0);

    }
}

//
// Common code for VLDn/VSTn (multiple n-element structures or single elements to one lane)
//
static void emitVLoadStoreN(armMorphStateP state, Bool isLoad, Bool singleEl) {

    if(checkAdvSIMDEnabled(state)) {

        Uns32  ebytes     = state->attrs->ebytes;
        Uns32  elements   = state->attrs->elements;
        Uns32  nregs      = state->info.nregs;
        Uns32  index      = state->info.index;
        Uns32  nels       = state->info.nels;
        Uns32  incr       = state->info.incr;
        Uns32  align      = state->info.align;
        vmiReg base       = GET_RS(state, r2);
        Uns32  bits       = ebytes*8;
        Int32  offset     = 0;
        Uns32  r, e, i;

        // base register must be null-checked in ThumbEE mode
        emitNullCheckIfThumbEE(state, base);

        // Check any alignment that was specified
        emitAlignCheck(state, base, align, isLoad);

        // Walk through each element in the registers
        for(r=0; r<nregs; r++) {
            for (e = 0; e < elements; e++) {

                // Walk through each element in the structure
                for (i=0; i < nels; i++) {

                    vmiReg el = GET_SIMD_EL(state, r1, r+(incr*i), singleEl ? index : e, ebytes);

                    // Emit code to load/store this element.
                    // However, In single element mode only do it for the single element
                    if (!singleEl || e == index) {
                        if (isLoad) {
                            armEmitLoadRRO(state, bits, bits, offset, el, base, False, False);
                        } else {
                            armEmitStoreRRO(state, bits, offset, base, el);
                        }
                        offset += ebytes;
                    }
                }
            }
        }

        // Update the base register value if requested
        emitVLDSTNWback(state, base, offset);
    }
}

//
// Emit code for multiple n-element structure VLDn <D list>, [Rn {@align}] {! | ,Rm}
//
ARM_MORPH_FN(armEmitVLDN) {
    emitVLoadStoreN(state, True, False);
}

//
// Emit code for multiple n-element structure VSTn <D list>, [Rn {@align}] {! | ,Rm}
//
ARM_MORPH_FN(armEmitVSTN) {
    emitVLoadStoreN(state, False, False);
}

//
// Emit code for single n-element from one lane VLDn <D[x] list>, [Rn {@align}] {! | ,Rm}
//
ARM_MORPH_FN(armEmitVLDNZ1) {
    emitVLoadStoreN(state, True, True);
}

//
// Emit code for single n-element from one lane VSTn <D[x] list>, [Rn {@align}] {! | ,Rm}
//
ARM_MORPH_FN(armEmitVSTNZ1) {
    emitVLoadStoreN(state, False, True);
}

//
// Emit code for single n-element to all lanes VLDn <D[x] list>, [Rn {@align}] {! | ,Rm}
//
ARM_MORPH_FN(armEmitVLDNZA) {

    if(checkAdvSIMDEnabled(state)) {

        Uns32  ebytes     = state->attrs->ebytes;
        Uns32  bits       = ebytes*8;
        Uns32  elements   = 8 / ebytes;
        Uns32  nregs      = state->info.nregs;
        Uns32  nels       = state->info.nels;
        Uns32  incr       = state->info.incr;
        Uns32  align      = state->info.align;
        vmiReg base       = GET_RS(state, r2);
        Int32  offset     = 0;
        Uns32  r, e, i;

        // base register must be null-checked in ThumbEE mode
        emitNullCheckIfThumbEE(state, base);

        // Check any alignment that was specified
        emitAlignCheck(state, base, align, True);

        // Walk through each element in the structure
        for (i=0; i < nels; i++) {

            // Load the element from memory into every element in the register(s)
            for (e = 0; e < elements; e++) {
                for(r=0; r<nregs; r++) {

                    // Get the register element being loaded
                    vmiReg el = GET_SIMD_EL(state, r1, r+(incr*i), e, ebytes);

                    // Emit code to load this element.
                    armEmitLoadRRO(state, bits, bits, offset, el, base, False, False);
                }
            }

            // Move to next element
            offset += ebytes;
        }

        // Update the base register value if requested
        emitVLDSTNWback(state, base, offset);
    }
}

////////////////////////////////////////////////////////////////////////////////
// SIMD Utilities to dispatch functions per element
////////////////////////////////////////////////////////////////////////////////

//
// Callback function to execute a SIMD op on a single element
//
#define SIMD_EL_OP_FN(_NAME) void _NAME(    \
    armMorphStateP state,                     \
    Uns32          esize,                    \
    Uns32          resultSize,                   \
    Uns32          opSize,                     \
    vmiReg         result,                    \
    vmiReg         r1,                        \
    vmiReg         r2,                        \
    vmiReg         r3,                        \
    void          *userData                    \
)
typedef SIMD_EL_OP_FN((*simdOpFunc));

//
// Types for SIMD operands
//
typedef enum simdOperandTypeE {
    SDOP_NONE,                 // No Operand
    SDOP_ELEMENT,            // Operand is an element in a vector
    SDOP_SCALAR,            // Operand is a scalar Element
} simdOperandType;

// Compute how many SIMD Double word registers are used for an operand
// given the element size, number of regs and number of elements per reg
#define SIMD_NREGS(_S, _R, _E) (((_S)*(_R)*(_E))/ARM_SIMD_REG_BITS)

//
// Determine if temp register must be used for result due to overlapping source and dest operands
//
static Bool simdRegsOverlap(
    armMorphStateP  state,
    Uns32           r1Size,
    Uns32           r2Size,
    Uns32           r3Size,
    simdOperandType r3Type
) {

    armSIMDShape shape = state->attrs->shape;

    if (shape==ASDS_NORMAL && r3Type!=SDOP_SCALAR) {

        // For normal shape instructions that don't use a scalar
        // overlapping registers do not matter
        return False;

    } else {

        Uns32 regs     = state->attrs->regs;
        Uns32 elements = state->attrs->elements;
        Uns32 r1       = state->info.r1;
        Uns32 r2       = state->info.r2;
        Uns32 r3       = state->info.r3;
        Uns32 r1max    = r1 + SIMD_NREGS(r1Size, regs, elements) - 1;
        Uns32 r2max    = r2 + SIMD_NREGS(r2Size, regs, elements) - 1;
        Uns32 r3max    = r3 + (r3Type==SDOP_ELEMENT ? (SIMD_NREGS(r3Size, regs, elements) - 1) : 0);

        if ((r1 > r2max || r1max < r2)  && (r3Type==SDOP_NONE || (r1 > r3max || r1max < r3))) {

            // r1 does not overlap with either r2 or r3 (if specified)
            return False;

        }
    }

    return True;
}

//
// For a pairwise instruction, determine if temp register must be used for result
// due to overlapping source and dest operands
//
static Bool simdPairwiseRegsOverlap(armMorphStateP state) {

    armSIMDShape shape = state->attrs->shape;

    if (shape==ASDS_NORMAL) {

        // Only a normal shaped pairwise instruction can overlap
        Uns32 r1       = state->info.r1;
        Uns32 r2       = state->info.r2;
        Uns32 r3       = state->info.r3;

        // Overlaps only if r1 is the same as r2 or r3
        return r1==r2 || r1==r3;

    }

    return False;

}

//
// Given a SIMD instruction shape and base element size
// get the size of the operation, result and operand sizes in bits
//
static Uns32 getOpSize(armSIMDShape shape, Uns32 esize, Uns32 *r1Size, Uns32 *op1Size, Uns32 *op2Size)
{

    switch (shape) {

    case ASDS_NORMAL:
        *r1Size = *op1Size = esize;
        if (op2Size) *op2Size = esize;
        return (esize);

    case ASDS_LONG:
        VMI_ASSERT(esize <= 32, "Instruction shape incompatible with element size");
        *r1Size  = 2*esize;
        *op1Size = esize;
        if (op2Size) *op2Size = esize;
        return (2*esize);

    case ASDS_WIDE:
        VMI_ASSERT(esize <= 32, "Instruction shape incompatible with element size");
        *r1Size  = 2*esize;
        *op1Size = 2*esize;
        if (op2Size) *op2Size = esize;
        return (2*esize);

    case ASDS_NARROW:
        VMI_ASSERT(esize <= 32, "Instruction shape incompatible with element size");
        *r1Size  = esize;
        *op1Size = 2*esize;
        if (op2Size) *op2Size = 2*esize;
        return (2*esize);

    default:
        VMI_ABORT("Unimplemented case");

    }

    // not reached
    return (0);
}

//
// Zero- or Sign-Extend the operand to twice the existing size in a temp reg
//
static vmiReg extendOperand(armMorphStateP state, Uns32 esize, vmiReg r, Bool sextend) {

    VMI_ASSERT(esize<= 32, "element size %d too large to extend", esize);

    vmiReg t      = newTemp64(state);
    Uns32  opSize = 2*esize;

    armEmitMoveExtendRR(state, opSize, t, esize, r, sextend);

    return t;

}

//
// return the vmiReg for the given element of the specified Q or D simd Regeister:
//
static vmiReg simdElQorD(
    armMorphStateP state,
    Uns32          regNum,
    Uns32          r,
    Uns32          e,
    Uns32          ebytes,
    Bool           Dreg)
{
    vmiReg reg;

    if (Dreg) {
        reg = getSimdElement(state, regNum+r, e, ebytes);
    } else {
        reg = getSimdElement(state, regNum, 2*e, ebytes);
    }

    return reg;
}

//
// Return the vmiReg for the given simd element operand
// If necessary extend it into the given temporary
//
static vmiReg simdElOperand(
        armMorphStateP state,
        Uns32          regNum,
        Uns32          r,
        Uns32          e,
        Uns32          ebytes,
        Uns32          opSize,
        Uns32          regSize,
        Bool           extend
) {

    Uns32  esize   = 8*ebytes;
    Bool   sextend = state->attrs->sextend;
    vmiReg reg;

    // Get the vmiReg for operand element
    reg = simdElQorD(state, regNum, r, e, ebytes, esize==regSize);

    // If register is smaller than the operation size and extend is true
    // then extend it in a temp reg
    if (regSize < opSize && extend) {
        reg = extendOperand(state, regSize, reg, sextend);
    }

    return reg;

}

// Get a VMI_REG for the indicated element of the passed register
#define TEMP_ELEMENT(_T, _R, _E, _RS) VMI_REG_DELTA((_T), (ARM_SIMD_REG_BYTES*(_R))+(((_RS)/8)*(_E)))

//
// Emit code per element for SIMD ops with 2 or 3 regs
// Can have shape Normal, Long, Wide, Narrow
// Call the 'perEl' function for each element
//
static void emitSIMDPerEl(
    armMorphStateP  state,
    simdOpFunc      perEl,
    simdOperandType r3Type,
    Bool            extend,
    void           *userData
) {

    if(checkAdvSIMDEnabled(state)) {

        Uns32        regs     = state->attrs->regs;
        Uns32        ebytes   = state->attrs->ebytes;
        Uns32        elements = state->attrs->elements;
        armSIMDShape shape    = state->attrs->shape;
        Uns32        esize    = ebytes*8;


        Uns32   r1Size, r2Size, r3Size;
        Uns32   opSize   = getOpSize(shape, esize, &r1Size, &r2Size, &r3Size);
        Bool    useR1Tmp = simdRegsOverlap(state, r1Size, r2Size, r3Size, r3Type);
        vmiReg  t1       = useR1Tmp ? newTemp128(state) : VMI_NOREG;
        vmiReg  r3       = VMI_NOREG;
        Uns32   r, e;

        if (r3Type==SDOP_SCALAR) {

            // If using a scalar for r3 get it once here, not in the inner loop
            Uns32 index = state->info.index;
            r3 = simdElOperand(state, state->info.r3, 0, index, ebytes, opSize, r3Size, extend);

        }

        for (r = 0; r < regs; r++) {
            for (e = 0; e < elements; e++) {

                Uns32  tempBase = getTempBase(state);
                vmiReg r1       = simdElQorD   (state, state->info.r1, r, e, ebytes, esize==r1Size);
                vmiReg r2       = simdElOperand(state, state->info.r2, r, e, ebytes, opSize, r2Size, extend);
                vmiReg result   = useR1Tmp ? TEMP_ELEMENT(t1, r, e, r1Size) : r1;

                if (r3Type==SDOP_ELEMENT) {

                    // If r3 is an element, must get it here
                    r3 = simdElOperand(state, state->info.r3, r, e, ebytes, opSize, r3Size, extend);

                }

                // Call the per-element callback function
                (*perEl)(state, esize, r1Size, opSize, result, r1, r2, r3, userData);

                // Restore the tempIdx (frees any temps allocated inside the loop)
                setTempBase(state, tempBase);

            }
        }

        if (useR1Tmp) {

            // Copy result to rd register if we used a temporary
            armEmitMoveRR(state, r1Size*elements*regs, GET_SIMD_EL(state, r1, 0, 0, ebytes), t1);

        }
    }
}

//
// Emit code for SIMD Pairwise instructions
// Can have shape Normal (3 regs), or Long (2 regs)
// Call the 'perEl' function for each element pair
//
static void emitSIMDPairwisePerEl(armMorphStateP state, simdOpFunc perEl, void *userData) {

    if(checkAdvSIMDEnabled(state)) {

        Uns32        regs     = state->attrs->regs;
        Uns32        ebytes   = state->attrs->ebytes;
        Uns32        elements = state->attrs->elements;
        armSIMDShape shape    = state->attrs->shape;
        Uns32        esize    = ebytes*8;
        Uns32        h        = elements/2;

        VMI_ASSERT(shape==ASDS_LONG || shape==ASDS_NORMAL, "Pairwise must be shape normal or long");

        Uns32   r1Size, r2Size, r3Size;
        Uns32   opSize   = getOpSize(shape, esize, &r1Size, &r2Size, &r3Size);
        Bool    useR1Tmp = simdPairwiseRegsOverlap(state);
        vmiReg  t1       = useR1Tmp ? newTemp128(state) : VMI_NOREG;
        Uns32   r, e;

        for (r = 0; r < regs; r++) {
            for (e = 0; e < h; e++) {

                Uns32  tempBase = getTempBase(state);
                vmiReg r1       = simdElQorD   (state, state->info.r1, r, e, r1Size/8, True);
                vmiReg r2       = simdElOperand(state, state->info.r2, r, (2*e),   ebytes, opSize, r2Size, True);
                vmiReg r3       = simdElOperand(state, state->info.r2, r, (2*e)+1, ebytes, opSize, r2Size, True);
                vmiReg result   = useR1Tmp ? TEMP_ELEMENT(t1, r, e, r1Size) : r1;

                // Call the per-element callback function
                (*perEl)(state, esize, r1Size, opSize, result, r1, r2, r3, userData);

                if (shape==ASDS_NORMAL) {

                    setTempBase(state, tempBase);

                    r1     = simdElQorD   (state, state->info.r1, r, e+h, r1Size/8, True);
                    r2     = simdElOperand(state, state->info.r3, r, (2*e),   ebytes, opSize, r3Size, True);
                    r3     = simdElOperand(state, state->info.r3, r, (2*e)+1, ebytes, opSize, r3Size, True);
                    result = useR1Tmp ? TEMP_ELEMENT(t1, r, e+h, r1Size) : r1;

                    (*perEl)(state, esize, r1Size, opSize, result, r1, r2, r3, userData);
                }

                setTempBase(state, tempBase);
            }
        }

        if (useR1Tmp) {

            // Copy result to rd register if we used a temporary
            armEmitMoveRR(state, r1Size*elements*regs, GET_SIMD_EL(state, r1, 0, 0, ebytes), t1);

        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// SIMD Data processing instructions
////////////////////////////////////////////////////////////////////////////////

//
// Per-element code for standard binop
// Supports saturate, round, sextend
//
static SIMD_EL_OP_FN(simdBinopEl) {

    vmiBinop binop      = state->attrs->binop;
    Bool     accumulate = state->attrs->accumulate;
    Bool     useTemp    = resultSize!=opSize || accumulate;
    Bool     saturate   = saturatingBinop(binop);
    vmiReg   tf         = newTemp32(state);
    vmiFlags flags      = signedBinop(binop) ? getOFFlags(tf) : getCFFlags(tf);

    if (!useTemp) {

        // Can use result directly for result of operation
        armEmitBinopRRR(state, opSize, binop, result, r2, r3, saturate ? &flags : 0);

    } else {

        // Must use a temporary for the result
        vmiReg t = newTemp64(state);

        armEmitBinopRRR(state, opSize, binop, t, r2, r3,  saturate ? &flags : 0);

        if (2*resultSize == opSize) {

            // Result is 1/2 the size of the intermediate result
            Bool highhalf = state->attrs->highhalf;
            Bool round    = state->attrs->round;

            if (round) armEmitBinopRC(state, opSize, vmi_ADD, t, 1ULL<<(esize-1), 0);

            // Move the low or high half of the result to rd
            armEmitMoveRR(state, resultSize, result, highhalf ? VMI_REG_DELTA(t, esize/8) : t);

        } else if (accumulate) {

            // add the intermediate value to the destination
            armEmitBinopRRR(state, resultSize, vmi_ADD, result, r1, t, 0);

        } else {

            VMI_ABORT("Unsupported resultSize (%d) and opSize (%d)", resultSize, opSize);

        }
    }

    // if saturating, set the sticky QC flag if there was overflow
    if (saturate) armEmitBinopRR(state, 8, vmi_OR, ARM_QC, tf, 0);
}

//
// Emit code for a standard SIMD binary operation on 3 regs
//
ARM_MORPH_FN(armEmitVBinop) {
    emitSIMDPerEl(state, simdBinopEl, SDOP_ELEMENT, True, 0);
}

//
// Emit code for a standard SIMD binary operation on 2 regs and a scalar
//
ARM_MORPH_FN(armEmitVBinopZ) {
    emitSIMDPerEl(state, simdBinopEl, SDOP_SCALAR, True, 0);
}

//
// Emit code for a pairwise SIMD binary operation on 2 regs and a scalar
//
ARM_MORPH_FN(armEmitVPairOp) {
    emitSIMDPairwisePerEl(state, simdBinopEl, 0);
}

//
// Emit check for Narrowing saturating op:
// If long intResult is out of range for the short result reg then saturate
//
static void emitNarrowSatCheck(
    armMorphStateP state,
    Uns32          opSize,
    vmiReg         intResult,
    Uns32          resultSize,
    vmiReg         result,
    vmiLabelP      done
) {

    VMI_ASSERT(opSize > resultSize, "opSize (%d) must be greater than resultSize (%d)", opSize, resultSize);

    Bool       sextend      = state->attrs->sextend;
    Bool       ussat        = state->attrs->ussat;
    vmiLabelP  notTooBig    = armEmitNewLabel();
    vmiLabelP  notSaturated = armEmitNewLabel();
    Bool       resSigned    = sextend && !ussat;
    Uns64      satMax       = resSigned ? (1Ull<<(resultSize-1))-1 : (1Ull<<resultSize)-1;
    Uns64      satMin       = resSigned ? -1Ull<<(resultSize-1)    : 0;
    vmiReg     cf           = getTemp(state);


    // No saturation if intResult <= max value
    armEmitCompareRC(state, opSize, sextend ? vmi_COND_LE : vmi_COND_BE, intResult, satMax, cf);
    armEmitCondJumpLabel(cf, True, notTooBig);

    //////////////////////////////////////////////////
    // Here if value is too big
    //////////////////////////////////////////////////
    armEmitMoveRC(state, resultSize, result, satMax);
    armEmitBinopRC(state, 8, vmi_OR, ARM_QC, 1, 0);
    armEmitUncondJumpLabel(done);

    //////////////////////////////////////////////////
    // Here when value is not too big
    //////////////////////////////////////////////////
    armEmitInsertLabel(notTooBig);

    if (sextend) {

        // if signed operands then no saturation if intResult >= min value
        armEmitCompareRC(state, opSize, sextend ? vmi_COND_L : vmi_COND_B, intResult, satMin, cf);
        armEmitCondJumpLabel(cf, False, notSaturated);

        //////////////////////////////////////////////////
        // Here if value is too small
        //////////////////////////////////////////////////
        armEmitMoveRC(state, resultSize, result, satMin);
        armEmitBinopRC(state, 8, vmi_OR, ARM_QC, 1, 0);
        armEmitUncondJumpLabel(done);

    }

    //////////////////////////////////////////////////
    // Here if value is not saturated
    //////////////////////////////////////////////////
    armEmitInsertLabel(notSaturated);

}

//
// Per-element code for standard unnop
// Supports signed saturating ops
//
static SIMD_EL_OP_FN(simdUnopEl) {

    vmiUnop  unop        = state->attrs->unop;
    Bool     saturate    = saturatingUnop(unop);
    Bool     satOnNarrow = state->attrs->satOnNarrow;
    vmiReg   tf          = newTemp32(state);
    vmiFlags flags       = getOFFlags(tf);
    vmiLabelP done       = armEmitNewLabel();

    // If saturating on narrow is enabled, emit that check first
    if (satOnNarrow) emitNarrowSatCheck(state, opSize, r2, resultSize, result, done);

    // emit unop instruction
    armEmitUnopRR(state, resultSize, unop, result, r2, saturate ? &flags : 0);

    // if saturating, set the sticky QC flag if there was overflow
    if (saturate) armEmitBinopRR(state, 8, vmi_OR, ARM_QC, tf, 0);

    armEmitInsertLabel(done);

}

//
// Emit code for a standard SIMD unary operation on 2 regs
//
ARM_MORPH_FN(armEmitVUnop) {
    emitSIMDPerEl(state, simdUnopEl, SDOP_NONE, True, 0);
}

//
// Emit code for SIMD integer ops with 1 register and a modified immediate
//
ARM_MORPH_FN(armEmitVOpRI) {

    if(checkAdvSIMDEnabled(state)) {

        vmiBinop binop  = state->attrs->binop;
        Uns32    regs   = state->attrs->regs;
        Bool     negate = state->attrs->negate;
        Uns64    mi     = negate ? ~state->info.sdfpMI.u64 : state->info.sdfpMI.u64;
        Uns32    r;

        for (r = 0; r < regs; r++) {

            vmiReg rd = GET_SIMD_EL(state, r1, r, 0, ARM_SIMD_REG_BYTES);

            if (binop==vmi_BINOP_LAST) {

                // No binop - just a move
                armEmitMoveRC(state, ARM_SIMD_REG_BITS, rd, mi);

            } else {

                armEmitBinopRC(state, ARM_SIMD_REG_BITS, binop, rd, mi, 0);

            }
        }
    }
}

//
// Emit code for SIMD integer ops that can be processed
// with a single unop per register rather than per element
//
ARM_MORPH_FN(armEmitVUnopReg) {

    if(checkAdvSIMDEnabled(state)) {

        vmiUnop  unop     = state->attrs->unop;
        Uns32    regs     = state->attrs->regs;
        Uns32    r;

        for (r = 0; r < regs; r++) {

            vmiReg rd = GET_SIMD_EL(state, r1, r, 0, ARM_SIMD_REG_BYTES);
            vmiReg rm = GET_SIMD_EL(state, r2, r, 0, ARM_SIMD_REG_BYTES);

            armEmitUnopRR(state, ARM_SIMD_REG_BITS, unop, rd, rm, 0);

        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// SIMD bit counting instructions
////////////////////////////////////////////////////////////////////////////////

//
// Per-element code for VCLS
//  Count the number of leading bits that match the sign bit
//  Do not count the sign bit itself
//
static SIMD_EL_OP_FN(simdVClsEl) {

    vmiLabelP positive = armEmitNewLabel();
    vmiLabelP doCount  = armEmitNewLabel();
    vmiReg    t        = newTemp64(state);
    vmiFlags  flags    = getCFFlags(getTemp(state));

    VMI_ASSERT(opSize==resultSize, "opSize (%d) must be same as resultSize(%d)", opSize, resultSize);

    // Shift out the sign bit, setting CF if the sign bit was set
    armEmitBinopRRC(state, opSize, vmi_SHL, t, r2, 1, &flags);

    // Check CF which now equals the sign bit of the value
    armEmitCondJumpLabel(flags.f[vmi_CF], False, positive);

    // Here when negative (sign bit is set) - need to invert all bits
    // (note lsb had 0 shifted in so won't be counted when it gets toggled to 1)
    armEmitUnopR(state, opSize, vmi_NOT, t, 0);
    armEmitUncondJumpLabel(doCount);

    // Here when positive value (sign bit is not set) -
    // Set lsb of shifted result so it won't get counted
    armEmitInsertLabel(positive);
    armEmitBinopRC(state, opSize, vmi_OR, t, 0x01, 0);

    // Count the leading zeros
    armEmitInsertLabel(doCount);
    armEmitUnopRR(state, opSize, vmi_CLZ, result, t, 0);

}

//
// Emit code for a SIMD VCLS operation
//
ARM_MORPH_FN(armEmitVCLS) {
    emitSIMDPerEl(state, simdVClsEl, SDOP_NONE, True, 0);
}

//
// This array will hold bit count values for VCNT
//
static Uns8 vcnt8[256];

//
// Return vcnt value
//
static Uns8 doVCNT(Uns8 value) {

    // return the count of bits for the value
    return vcnt8[value];
}

//
// Per-element code for VCNT
//  Count the number of  bits that are set
//  Use a table lookup
//
static SIMD_EL_OP_FN(simdVCntEl) {

    static Bool init;

    VMI_ASSERT(opSize==8 && resultSize==8, "opSize (%d) and resultSize(%d) must both be 8", opSize, resultSize);

      // set up vcnt8 table on initial use
    if(!init) {

        Uns32 i;

        for(i=0; i<256; i++) {

            Uns8  result = 0;
            Uns8  byte   = (Uns8) i;
            Uns32 j;

            for(j=0; j<8; j++) {
                result += (byte & 0x01);
                byte >>= 1;
            }

            vcnt8[i] = result;
        }

        init = True;
    }

    // emit embedded call to perform operation
    armEmitArgReg(state, opSize, r2);
    armEmitCallResult(state, (vmiCallFn)doVCNT, opSize, result);
}

//
// Emit code for a SIMD VCNT operation
//
ARM_MORPH_FN(armEmitVCNT) {
    emitSIMDPerEl(state, simdVCntEl, SDOP_NONE, True, 0);
}

////////////////////////////////////////////////////////////////////////////////
// SIMD Multiple opcode instructions (absoulte diff, multiply accumulate)
////////////////////////////////////////////////////////////////////////////////

//
// Per-element code for vector absolute diff instrs: VABA/VABL/VABD/VABDL
// Intermediate results must be in double length operands
//
static SIMD_EL_OP_FN(simdAbsDiffEl) {

    vmiBinop  binop      = state->attrs->binop;
    Bool      accumulate = state->attrs->accumulate;
    Bool      sextend    = state->attrs->sextend;
    vmiReg    t          = newTemp64(state);

    if (esize == opSize) {

        // get double sized zero- or sign-extended arguments
        r2 = extendOperand(state, esize, r2, sextend);
        r3 = extendOperand(state, esize, r3, sextend);
        opSize = 2*esize;

    }

    // perform subtraction
    armEmitBinopRRR(state, opSize, vmi_SUB, t, r2, r3, 0);

    // get absolute result
    armEmitUnopR(state, opSize, vmi_ABS, t, 0);

    if (accumulate) {
        armEmitBinopRRR(state, resultSize, binop, result, r1, t, 0);
    } else {
        armEmitMoveRR (state, resultSize, result, t);
    }
}

//
// Emit code for VABA/VABAL/VABD/VABDL Vd, Vn, Vm
//
ARM_MORPH_FN(armEmitVAbsDiff) {
    emitSIMDPerEl(state, simdAbsDiffEl, SDOP_ELEMENT, True, 0);
}

//
// Per-element code for vector Mulitiply Accumlate/Subtract VMLA/VMLS/VMLAL/VMLSL
// Intermediate results must be in double length operands
//
static SIMD_EL_OP_FN(simdMulAccEl) {

    vmiBinop  binop      = state->attrs->binop;
    Bool      sextend    = state->attrs->sextend;
    vmiReg    t          = newTemp64(state);

    if (esize == opSize) {

        // get double sized zero- or sign-extended arguments
        r2 = extendOperand(state, esize, r2, sextend);
        r3 = extendOperand(state, esize, r3, sextend);
        opSize = 2*esize;

    }

    // perform multiply and accumulate
    armEmitBinopRRR(state, opSize, sextend ? vmi_IMUL : vmi_MUL, t, r2, r3, 0);
    armEmitBinopRRR(state, resultSize, binop, result, r1, t, 0);


}

//
// Emit code for Mulitiply Accumlate/Subtract VMLA/VMLS/VMLAL/VMLSL Vd, Vn, Vm
//
ARM_MORPH_FN(armEmitVMulAcc) {
    emitSIMDPerEl(state, simdMulAccEl, SDOP_ELEMENT, True, 0);
}

//
// Emit code for Mulitiply Accumlate/Subtract VMLA/VMLS/VMLAL/VMLSL Vd, Vn, Vm[x]
//
ARM_MORPH_FN(armEmitVMulAccZ) {
    emitSIMDPerEl(state, simdMulAccEl, SDOP_SCALAR, True, 0);
}

////////////////////////////////////////////////////////////////////////////////
// SIMD Saturating Doubling Multiply instructions
////////////////////////////////////////////////////////////////////////////////

//
// Per-element code for SIMD Saturating Doubling Multiply instructions:
//        VQDMLAL, VQDMLSL, VQDMULH, VQDMULL, VQRDMULH
// Intermediate result is twice the esize
// Always saturating ops, may be rounded
// Result may be the low half, high half, entire intermediate result or
//   may be accumulated (using add or subtract with saturation)
// Cumulative saturation flag set if result saturates
//
static SIMD_EL_OP_FN(simdVQDMultiplyEl) {

    vmiBinop binop      = state->attrs->binop;
    Bool     accumulate = state->attrs->accumulate;
    Bool     round      = state->attrs->round;
    Bool     highhalf   = state->attrs->highhalf;
    vmiReg   t          = newTemp64(state);
    vmiReg   tf         = newTemp32(state);
    vmiFlags flags      = getOFFlags(tf);    // Saturation flag for signed operations


    // perform multiplication (always signed) - result is double element size
    if (opSize == esize) {

        // Operands not extended (shape is Normal) Use vmimtMulop to avoid extending them
        armEmitMulopRRR(state, esize, vmi_IMUL, VMI_REG_DELTA(t, esize/8), t, r2, r3, 0);

        // Make all operations double esize
        opSize = 2*esize;

    } else if (opSize == 2*esize) {

        // Operands are extended already (when shape is long)
        armEmitBinopRRR(state, opSize, vmi_IMUL, t, r2, r3, 0);

    } else {

        VMI_ABORT("Unsupported combination: resultSize=%d, opSize=%d", resultSize, opSize);

    }

    // Double the results, with saturation
    armEmitBinopRR(state, opSize, vmi_ADDSQ, t, t, round ? 0 : &flags);

    // Add rounding constant with saturation if indicated
    if (round) armEmitBinopRC(state, opSize, vmi_ADDSQ, t, 1ULL<<(esize-1), &flags);

    // set the sticky QC flag if there was overflow
    armEmitBinopRR(state, 8, vmi_OR, ARM_QC, tf, 0);

    if (accumulate) {

        VMI_ASSERT(binop==vmi_ADDSQ || binop==vmi_SUBSQ, "Saturating op required");
        VMI_ASSERT(resultSize==opSize, "Accumulation requires resultSize (%d) equals opSize (%d)", resultSize, opSize);

        // Add/subtract with saturation and set sticky QC flag if overflows
        armEmitBinopRRR(state, resultSize, binop, result, r1, t, &flags);
        armEmitBinopRR(state, 8, vmi_OR, ARM_QC, tf, 0);

    } else {

        // Adjust t if we only need the high half of the result
        if (highhalf) t = VMI_REG_DELTA(t, esize/8);

        // Move result to destination
        armEmitMoveRR(state, resultSize, result, t);

    }
}

//
// Emit code for a VQD Multiply instruction
//
ARM_MORPH_FN(armEmitVQDMul) {
    emitSIMDPerEl(state, simdVQDMultiplyEl, SDOP_ELEMENT, True, 0);
}

//
// Emit code for a VQD Multiply instruction with Scalar
//
ARM_MORPH_FN(armEmitVQDMulZ) {
    emitSIMDPerEl(state, simdVQDMultiplyEl, SDOP_SCALAR, True, 0);
}

////////////////////////////////////////////////////////////////////////////////
// SIMD Shift instructions
////////////////////////////////////////////////////////////////////////////////

//
// Per-element code for SIMD Vector Shift (register) instructions:
//        VSHL, VQSHL, VRSHL, VQRSHL
//
static SIMD_EL_OP_FN(simdVShiftRegEl) {

    Bool      sextend    = state->attrs->sextend;
//    Bool      saturate   = state->attrs->saturate;
    vmiBinop  op         = state->attrs->binop;
    vmiBinop  op2        = state->attrs->binop2;
    vmiReg    shift      = newTemp32(state);
    vmiLabelP shiftRight = armEmitNewLabel();
    vmiLabelP done       = armEmitNewLabel();


    VMI_ASSERT(opSize==esize && resultSize==esize, "Instruction shape must be normal");

    // Jump to shiftRight if sign bit on shift amount is set
    armEmitTestRCJumpLabel(8, vmi_COND_NZ, r3, 0x80, shiftRight);

    ////////////////////////////////////////////////////////////////
    // Here when Rn > 0: Rd = Rm << Rn[7:0]
    ////////////////////////////////////////////////////////////////

    if (saturatingBinop(op)) {

        // Rd = Rm << Rn[7:0], set sticky QC flag if saturates
        vmiReg   tf    = getTemp(state);
        vmiFlags flags = sextend ? getOFFlags(tf) : getCFFlags(tf);

        armEmitSetShiftMask();
        armEmitBinopRRR(state, esize, op, result, r2, r3, &flags);
        armEmitBinopRR(state, 8, vmi_OR, ARM_QC, tf, 0);

    } else {

        // Rd = Rm << Rn[7:0], no saturation
        armEmitSetShiftMask();
        armEmitBinopRRR(state, esize, op, result, r2, r3, 0);

    }

    // Finished with instruction
    armEmitUncondJumpLabel(done);

    ////////////////////////////////////////////////////////////////
    // Here when Rn < 0: Rd = Rm >> -Rn[7:0]
    ////////////////////////////////////////////////////////////////
    armEmitInsertLabel(shiftRight);

    // Shift amount is -Rn (implemented as 0 - Rn)
    // (Use saturating instruction to handle -128 case)
    armEmitBinopRRC(state, 8, vmi_RSUBSQ, shift, r3, 0, 0);

    //  Do the shift using op2 for right shifts
    armEmitSetShiftMask();
    armEmitBinopRRR(state, esize, op2, result, r2, shift, 0);

    // Here when done
    armEmitInsertLabel(done);

    freeTemp32(state);
}

//
// Emit code for a VSHL (Register) type instruction
//
ARM_MORPH_FN(armEmitVShiftReg) {
    emitSIMDPerEl(state, simdVShiftRegEl, SDOP_ELEMENT, True, 0);
}

//
// Emit check for unsigned saturating op with signed operands
// Any negative operand saturates to 0
//
static void emitUnsSatCheck(
    armMorphStateP state,
    Uns32          opSize,
    vmiReg         operand,
    Uns32          resultSize,
    vmiReg         result,
    vmiLabelP      done
) {
    vmiLabelP noSaturate = armEmitNewLabel();
    Uns64     signMask   = 1Ull << (opSize-1);

    // Test for sign and skip saturation if it is clear
    armEmitTestRCJumpLabel(
        opSize, vmi_COND_Z, operand, signMask, noSaturate
    );

    // Saturate the result to zero and skip other actions
    armEmitMoveRC(state, resultSize, result, 0);
    armEmitBinopRC(state, 8, vmi_OR, ARM_QC, 1, 0);
    armEmitUncondJumpLabel(done);

    // Here if saturate-to-zero is not required
    armEmitInsertLabel(noSaturate);

}

//
// Per-element code for SIMD Vector Shift (immediate) instructions
//
static SIMD_EL_OP_FN(simdVShiftImmEl) {

    Uns32     shift       = state->info.c;
    vmiBinop  op          = state->attrs->binop;
    Bool      accumulate  = state->attrs->accumulate;
    Bool      ussat       = state->attrs->ussat;
    Bool      useTemp     = accumulate || (resultSize < opSize);
    vmiLabelP done        = armEmitNewLabel();

    // Signed operand and unsigned result saturates to zero if the operand is negative
    if (ussat) emitUnsSatCheck(state, opSize, r2, resultSize, result, done);

    // If accumulating or result is a smaller size use a temporary for the shift result
    // Otherwise operation goes right into result
    vmiReg shiftRes = useTemp ? newTemp64(state) : result;

    if (saturatingBinop(op)) {

        // Get flag to indicate saturation, depending on whether it is signed/unsigned
        vmiReg   tf    = getTemp(state);
        vmiFlags flags = signedBinop(op) ? getOFFlags(tf) : getCFFlags(tf);

        // Rd = Rm << Rn[7:0], saturate if overflow/carry
        armEmitSetShiftMask();
        armEmitBinopRRC(state, opSize, op, shiftRes, r2, shift, &flags);
        armEmitBinopRR(state, 8, vmi_OR, ARM_QC, tf, 0);

    } else {

        // Rd = Rm << Rn[7:0], no saturation
        armEmitSetShiftMask();
        armEmitBinopRRC(state, opSize, op, shiftRes, r2, shift, 0);

    }

    if (accumulate) {

        // shiftRes must be added to destination register
        armEmitBinopRRR(state, resultSize, vmi_ADD, result, r1, shiftRes, 0);

    } else if (resultSize < opSize) {

        Bool satOnNarrow = state->attrs->satOnNarrow;

        // Check for saturation on narrow if selected
        if (satOnNarrow) emitNarrowSatCheck(state, opSize, shiftRes, resultSize, result, done);

        // Move low half of shift result to result register
        armEmitMoveRR(state, resultSize, result, shiftRes);

    }

    // Here when done with element
    armEmitInsertLabel(done);

}


//
// Emit code for a Vector shift immediate type instruction
//
ARM_MORPH_FN(armEmitVShiftImm) {
    emitSIMDPerEl(state, simdVShiftImmEl, SDOP_NONE, True, 0);
}

//
// Emit code for a Vector shift immediate max type instruction
//
ARM_MORPH_FN(armEmitVShiftMax) {

    // Set the constant shift amount to the size of the elements
    state->info.c = state->attrs->ebytes*8;

    emitSIMDPerEl(state, simdVShiftImmEl, SDOP_NONE, True, 0);
}

//
// Get mask selecting the un-cleared bits to use on a shift insert instruction
//
static Uns64 shiftInsertMask(armMorphStateP state, vmiBinop op, Uns32 esize, Uns32 shift) {

    Uns64 mask;

    if (op == vmi_SHR) {

        mask = esize==64 ? (-1ULL) : ((1ULL << esize) - 1);
        mask >>= shift;

    } else if (op == vmi_SHL) {

        mask = -1ULL << shift;

    } else {

        VMI_ABORT("Invalid binop");

    }

    return mask;
}

//
// Per-element code for vector shift and insert instructions: VSRI/VSLI
//
static SIMD_EL_OP_FN(simdVShiftInsertEl) {

    Uns32 shift = state->info.c;

    if (shift == esize) {

        // when shift == esize this is effectively a nop

    } else {

        vmiBinop  op = state->attrs->binop;
        vmiReg    t1 = newTemp64(state);
        Uns64     mask = shiftInsertMask(state, op, esize, shift);

        armEmitSetShiftMask();
        armEmitBinopRRC(state, esize, op,       t1,     r2, shift, 0);
        armEmitBinopRRC(state, esize, vmi_ANDN, result, r1, mask,  0);
        armEmitBinopRR (state, esize, vmi_OR,   result, t1,        0);

        freeTemp64(state);

    }
}

//
// Emit code for vector shift and insert instructions: VSRI/VSLI
//
ARM_MORPH_FN(armEmitVShiftIns) {
    emitSIMDPerEl(state, simdVShiftInsertEl, SDOP_NONE, True, 0);
}

////////////////////////////////////////////////////////////////////////////////
// SIMD Convert instructions
////////////////////////////////////////////////////////////////////////////////

//
// Per-element code for SIMD VCVT instructions for Floating point to Integer/Fixed
//
static SIMD_EL_OP_FN(simdVCvtXFEl) {

    Uns32   fracBits   = state->info.c;
    Uns32   sextend    = state->attrs->sextend;

    FPToFixed(state, result, resultSize/8, r2, opSize/8, fracBits, sextend, True);

}

//
// Emit code for SIMD VCVT instructions for Integer/Fixed to Floating point
//
ARM_MORPH_FN(armEmitVCVT_XF_SIMD) {
    emitSIMDPerEl(state, simdVCvtXFEl, SDOP_NONE, False, 0);
}

//
// Per-element code for SIMD VCVT instructions for Integer/Fixed to Floating point
//
static SIMD_EL_OP_FN(simdVCvtFXEl) {

    Uns32   fracBits   = state->info.c;
    Uns32   sextend    = state->attrs->sextend;

    fixedToFP(state, result, resultSize/8, r2, opSize/8, fracBits, sextend, True);

}

//
// Emit code for SIMD VCVT instructions for Integer/Fixed to Floating point
//
ARM_MORPH_FN(armEmitVCVT_FX_SIMD) {
    emitSIMDPerEl(state, simdVCvtFXEl, SDOP_NONE, False, 0);
}

//
// Per-element code for SIMD VCVT instructions from Half to Single precision Floating point
//
static SIMD_EL_OP_FN(simdVCvtFHEl) {

    VMI_ASSERT(resultSize==32 && esize==16, "Invalid sizes");

    armEmitArgProcessor(state);         // argument 1: processor
    armEmitArgReg(state, esize, r2);    // argument 2: value
    armEmitCallResult(state, (vmiCallFn)armFPHalfToSingle, resultSize, result);
}

//
// Emit code for SIMD VCVT instructions from Half to Single precision Floating point
//
ARM_MORPH_FN(armEmitVCVT_FH_SIMD) {
    emitSIMDPerEl(state, simdVCvtFHEl, SDOP_NONE, False, 0);
}

//
// Per-element code for SIMD VCVT instructions from Single to Half precision Floating point
//
static SIMD_EL_OP_FN(simdVCvtHFEl) {

    VMI_ASSERT(resultSize==16 && opSize==32, "Invalid sizes");

    armEmitArgProcessor(state);         // argument 1: processor
    armEmitArgReg(state, opSize, r2);   // argument 2: value
    armEmitCallResult(state, (vmiCallFn)armFPSingleToHalf, resultSize, result);
}

//
// Emit code for SIMD VCVT instructions from Single to Half precision Floating point
//
ARM_MORPH_FN(armEmitVCVT_HF_SIMD) {
    emitSIMDPerEl(state, simdVCvtHFEl, SDOP_NONE, False, 0);
}

////////////////////////////////////////////////////////////////////////////////
// SIMD Compare/Test instructions
////////////////////////////////////////////////////////////////////////////////

// Return all ones of size esize (max 32)
#define ALL_ONES(_SIZE) ((_SIZE)==32 ? 0xffffffff : (1<<(_SIZE))-1)
//
// Per-element code for Vector Compare instructions that set result to 1/0 based on result of r1 & r2
//
static SIMD_EL_OP_FN(simdVTst) {

    Uns32       ones    = ALL_ONES(resultSize);
    vmiFlags    flags   = getZFFlags(getTemp(state));

    VMI_ASSERT(opSize==resultSize, "opSize (%d) must be same as resultSize(%d)", opSize, resultSize);

    // AND together the operands, setting the zero flag
    armEmitBinopRRR(state, opSize, vmi_AND, VMI_NOREG, r2, r3, &flags);

    // Move Ones or Zero to destination depending on the result
    armEmitCondMoveRCC(state, opSize, flags.f[vmi_ZF], True, result, 0, ones);

}

//
// Emit code for Vector Comparison Ops that set result to 0/1 depending on comparison
//
ARM_MORPH_FN(armEmitVTst) {
    emitSIMDPerEl(state, simdVTst, SDOP_ELEMENT, True, 0);
}

//
// Per-element code for Vector Compare instructions that set result to 0/1 depending on comparison
//
static SIMD_EL_OP_FN(simdVCmpSelBool) {

    vmiCondition cond = state->attrs->cond;
    Uns32        ones = ALL_ONES(resultSize);
    vmiReg       cf   = getTemp(state);

    VMI_ASSERT(opSize==resultSize, "opSize (%d) must be same as resultSize(%d)", opSize, resultSize);

    // Do the indicated comparison test on the operands
    armEmitCompareRR(state, opSize, cond, r2, r3, cf);

    // Move Ones or Zero to destination depending on the result
    armEmitCondMoveRCC(state, opSize, cf, True, result, ones, 0);

}

//
// Emit code for Vector Comparison Ops that set result to 0/1 depending on comarison
//
ARM_MORPH_FN(armEmitVCmpBool) {
    emitSIMDPerEl(state, simdVCmpSelBool, SDOP_ELEMENT, True, 0);
}

//
// Per-element code for Vector Compare instructions that set result to 0/1 depending on comparison to 0
//
static SIMD_EL_OP_FN(simdVCmp0) {

    vmiCondition cond = state->attrs->cond;
    Uns32        ones = ALL_ONES(resultSize);
    vmiReg       cf   = getTemp(state);

    VMI_ASSERT(opSize==resultSize, "opSize (%d) must be same as resultSize(%d)", opSize, resultSize);

    // Do the indicated comparison test on the operands
    armEmitCompareRC(state, opSize, cond, r2, 0, cf);

    // Move Ones or Zero to destination depending on the result
    armEmitCondMoveRCC(state, opSize, cf, True, result, ones, 0);

}

//
// Emit code for Vector Comparison Ops that set result to 0/1 depending on comarison to 0
//
ARM_MORPH_FN(armEmitVCmp0) {
    emitSIMDPerEl(state, simdVCmp0, SDOP_NONE, True, 0);
}

//
// Per-element code for Vector Compare instructions select Rn/Rm based on comparison
//
static SIMD_EL_OP_FN(simdVCmpSelReg) {

    vmiCondition cond    = state->attrs->cond;
    vmiReg       cf      = getTemp(state);

    VMI_ASSERT(opSize==resultSize, "opSize (%d) must be same as resultSize(%d)", opSize, resultSize);

    // Do the indicated comparison test on the operands
    armEmitCompareRR(state, opSize, cond, r2, r3, cf);

    // Move Rn or Rm to destination depending on the result
    armEmitCondMoveRRR(state, opSize, cf, True, result, r2, r3);

}

//
// Emit code for Vector Compare instructions that select Rn/Rm based on comparison
//
ARM_MORPH_FN(armEmitVCmpReg) {
    emitSIMDPerEl(state, simdVCmpSelReg, SDOP_ELEMENT, True, 0);
}

//
// Emit code for Pairwise Vector Compare instructions that select Rn/Rm based on comparison
//
ARM_MORPH_FN(armEmitVPCmpReg) {
    emitSIMDPairwisePerEl(state, simdVCmpSelReg, 0);
}

//
// Emit code for SIMD VSWP instruction
// (operates per register rather than per element)
//
ARM_MORPH_FN(armEmitVSWP) {

    if(checkAdvSIMDEnabled(state)) {

        Uns32    regs = state->attrs->regs;
        vmiReg   t    = newTemp64(state);
        Uns32    r;

        for (r = 0; r < regs; r++) {

            vmiReg rd = GET_SIMD_EL(state, r1, r, 0, ARM_SIMD_REG_BYTES);
            vmiReg rm = GET_SIMD_EL(state, r2, r, 0, ARM_SIMD_REG_BYTES);

            armEmitMoveRR(state, ARM_SIMD_REG_BITS, t,  rm);
            armEmitMoveRR(state, ARM_SIMD_REG_BITS, rm, rd);
            armEmitMoveRR(state, ARM_SIMD_REG_BITS, rd, t);

        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// SIMD Zip/Unzip/Transpose instructions
////////////////////////////////////////////////////////////////////////////////

//
// Emit code for SIMD VTRN instruction
// (operates on only half of the elements)
//  overwrites source and dest so must make temp copies of both
//
ARM_MORPH_FN(armEmitVTRN) {

    if(checkAdvSIMDEnabled(state)) {

        Uns32  regs     = state->attrs->regs;
        Uns32  ebytes   = state->attrs->ebytes;
        Uns32  elements = state->attrs->elements;
        Uns32  esize    = ebytes*8;
        vmiReg t1       = newTemp64(state);
        vmiReg t2       = newTemp64(state);
        Uns32  h        = elements/2;
        Bool   sameRegs = state->info.r1 == state->info.r2;
        Uns32  r, e;

        for (r = 0; r < regs; r++) {

            // Get copy of each reg to use as source of move
            armEmitMoveRR(state, ARM_SIMD_REG_BITS, t1, GET_SIMD_REG(state, r1, r));
            if (!sameRegs) armEmitMoveRR(state, ARM_SIMD_REG_BITS, t2, GET_SIMD_REG(state, r2, r));

            for (e = 0; e < h; e++) {

                vmiReg rd = GET_SIMD_EL(state, r1, r, (2*e)+1, ebytes);
                vmiReg rm = GET_SIMD_EL(state, r2, r, (2*e),   ebytes);

                // Swap between registers
                armEmitMoveRR(state, esize, rm, VMI_REG_DELTA(t1, (((2*e)+1)*ebytes)));
                if (!sameRegs) {

                    // Note: sameRegs check needed to match reference simulator behavior, which matches
                    //       behavior of non-overlapping regs with identical values. Psuedocode
                    //       in arch ref manual does not show this - probably a documentation bug.
                    armEmitMoveRR(state, esize, rd, VMI_REG_DELTA(t2, ((2*e)*ebytes)));

                }

            }
        }

    }
}

//
// Emit code for SIMD Zip instructions to interleave/deinterleave vectors
//  overwrites source and dest so must make temp copies of both
//
static void emitSIMDUnzip(armMorphStateP state, Bool unzip) {

    if(checkAdvSIMDEnabled(state)) {

        Uns32 regs     = state->attrs->regs;
        Uns32 ebytes   = state->attrs->ebytes;
        Uns32 elements = state->attrs->elements;
        Uns32 esize    = ebytes*8;

        VMI_ASSERT(regs==1   || regs==2,                "Regs (%d) must be 1 or 2", regs);
        VMI_ASSERT(ebytes==1 || ebytes==2 || ebytes==4, "ebytes (%d) must be 1, 2 or 4", ebytes);

        Uns32  tempBase = getTempBase(state);
        vmiReg t1       = regs == 1 ? newTemp64(state) : newTemp128(state);
        vmiReg t2       = regs == 1 ? newTemp64(state) : newTemp128(state);
        vmiReg t        = t1;
        Uns32  r, e;

        if (unzip) {

            // Copy value to temp regs
            armEmitMoveRR(state, ARM_SIMD_REG_BITS*regs, t1, GET_SIMD_EL(state, r1, 0, 0, ebytes));
            armEmitMoveRR(state, ARM_SIMD_REG_BITS*regs, t2, GET_SIMD_EL(state, r2, 0, 0, ebytes));

        }

        for (r = 0; r < regs; r++) {
            for (e = 0; e < elements; e++) {

                vmiReg r1  = GET_SIMD_EL(state, r1, r, e, ebytes);
                vmiReg r2  = GET_SIMD_EL(state, r2, r, e, ebytes);

                if (unzip) {

                    // Copy from temp to registers
                    armEmitMoveRR(state, esize, r1, t);
                    armEmitMoveRR(state, esize, r2, VMI_REG_DELTA(t, ebytes));

                } else {

                    // Copy from regs to temp
                    armEmitMoveRR(state, esize, t,                        r1);
                    armEmitMoveRR(state, esize, VMI_REG_DELTA(t, ebytes), r2);

                }

                t = VMI_REG_DELTA(t, 2*ebytes);

            }
        }

        if (!unzip) {

            // Copy temp regs to registers
            armEmitMoveRR(state, ARM_SIMD_REG_BITS*regs, GET_SIMD_EL(state, r1, 0, 0, ebytes), t1);
            armEmitMoveRR(state, ARM_SIMD_REG_BITS*regs, GET_SIMD_EL(state, r2, 0, 0, ebytes), t2);

        }

        setTempBase(state, tempBase);
    }
}

//
// Emit code for a VZIP instruction
//
ARM_MORPH_FN(armEmitVZIP) {
    emitSIMDUnzip(state, False);
}

//
// Emit code for a VUZP instruction
//
ARM_MORPH_FN(armEmitVUZP) {
    emitSIMDUnzip(state, True);
}

////////////////////////////////////////////////////////////////////////////////
// SIMD VREV instructions
////////////////////////////////////////////////////////////////////////////////

//
// Emit code for SIMD VREV instructions
// (Does non standard accesses of elements so cannot use PerEl functions)
//
ARM_MORPH_FN(armEmitVREV) {

    if(checkAdvSIMDEnabled(state)) {

        Uns32  regs        = state->attrs->regs;
        Uns32  ebytes      = state->attrs->ebytes;
        Uns32  elements    = state->attrs->elements;
        Uns32  groupSize   = state->attrs->groupSize;
        Bool   sameRegs    = state->info.r1 == state->info.r2;
        Uns32  reverseMask = groupSize-1;
        Uns32  esize       = ebytes*8;
        vmiReg t           = newTemp64(state);
        Uns32  r, e;

        for (r = 0; r < regs; r++) {

            vmiReg rd = sameRegs ? t : GET_SIMD_REG(state, r1, r);

            for (e = 0; e < elements; e++) {

                vmiReg rm = GET_SIMD_EL(state, r2, r, e, ebytes);

                // XOR the element number with reverseMask to get dest element number
                Uns32 destEl = e ^ reverseMask;

                // move element from rm to rd
                armEmitMoveRR(state, esize, VMI_REG_DELTA(rd, (destEl*ebytes)), rm);

            }

            // If using temp for result copy it to the dest register
            if (sameRegs) armEmitMoveRR(state, ARM_SIMD_REG_BITS, GET_SIMD_REG(state, r1, r), t);

        }
    }
}


////////////////////////////////////////////////////////////////////////////////
// SIMD Bit Select instructions
////////////////////////////////////////////////////////////////////////////////

//
// Common code for SIMD Bit select ops VBIF, VBIT and VBSL
//  rd = r1 & r3 | r2 & !r3
// (Operates on entire register - no need to work element by element)
//
static void emitSIMDBSelop (armMorphStateP state, Uns32 ri1, Uns32 ri2, Uns32 ri3) {

    if(checkAdvSIMDEnabled(state)) {

        Uns32  regs     = state->attrs->regs;
        vmiReg t1        = newTemp64(state);
        vmiReg t2        = newTemp64(state);
        Uns32  r;

        for (r = 0; r < regs; r++) {

            vmiReg rd = GET_SIMD_EL(state, r1, r, 0, ARM_SIMD_REG_BYTES);
            vmiReg r1 = getSimdElement(state, ri1+r, 0, ARM_SIMD_REG_BYTES);
            vmiReg r2 = getSimdElement(state, ri2+r, 0, ARM_SIMD_REG_BYTES);
            vmiReg r3 = getSimdElement(state, ri3+r, 0, ARM_SIMD_REG_BYTES);

            armEmitBinopRRR(state, ARM_SIMD_REG_BITS, vmi_AND,  t1, r1, r3, 0);
            armEmitBinopRRR(state, ARM_SIMD_REG_BITS, vmi_ANDN, t2, r2, r3, 0);
            armEmitBinopRRR(state, ARM_SIMD_REG_BITS, vmi_OR,   rd, t1, t2, 0);
        }

        freeTemp64(state); freeTemp64(state);
    }
}

//
// Emit code for VBIF Vd, Vn, Vm
//   Vd = Vd & Vm | Vn & !Vm
//
ARM_MORPH_FN(armEmitVBIF) {
    emitSIMDBSelop(state, state->info.r1, state->info.r2, state->info.r3);
}

//
// Emit code for VBIT Vd, Vn, Vm
//   Vd = Vn & Vm | Vd & !Vm
//
ARM_MORPH_FN(armEmitVBIT) {
    emitSIMDBSelop(state, state->info.r2, state->info.r1, state->info.r3);
}

//
// Emit code for VBSL Vd, Vn, Vm
//   Vd = Vn & Vd | Vm & !Vd
//
ARM_MORPH_FN(armEmitVBSL) {
    emitSIMDBSelop(state, state->info.r2, state->info.r3, state->info.r1);
}


////////////////////////////////////////////////////////////////////////////////
// SIMD Polynomial instructions
////////////////////////////////////////////////////////////////////////////////

//
// Called at run time for a VMUL.P8 or VMULL.P8 instruction
//
static void doVMULP(armP arm, Uns32 rd, Uns32 rn, Uns32 rm, Uns32 nregs, Uns32 isLong) {

    Uns32 r, e, i;
    union {
        Uns8  b[16];  // when viewed as bytes
        Uns16 h[8];   // When viewed as 16 bit halfwords
        Uns64 d[2];   // When viewed as 64 bit doublewords
    } temp = {.d = {0, 0}};

    for (r = 0; r < nregs; r++) {
        for (e = 0; e < ARM_SIMD_REG_BYTES; e++) {

            Uns32 result = 0;
            Uns32 op1    = arm->vregs.b[ARM_SIMD_REGNUM(rn+r, e, 1)];
            Uns32 op2    = arm->vregs.b[ARM_SIMD_REGNUM(rm+r, e, 1)];

            if (!op1 || !op2) {

                // If either operand is zero then result will be zero

            } else {

                for (i = 0; i < 8; i++) {

                    // If bit i of op1 is set then XOR current op2 into result
                    if (op1 & (1<<i)) {
                        result ^= op2;
                    }

                    // Shift op2
                    op2 <<= 1;

                }
            }

            // put result for this element in temp
            if (isLong) {
                temp.h[e] = (Uns16) result;
            } else {
                temp.b[(ARM_SIMD_REG_BYTES*r)+e] = (Uns8) result;
            }
        }
    }

    // Move temp to result
    arm->vregs.d[rd] = temp.d[0];
    if (nregs==2 || isLong) {
        arm->vregs.d[rd+1] = temp.d[1];
    }

}


//
// Common code for SIMD Polynomial multiply VMUL.P8 and VMULL.P8
// Operands may only be 1 byte
// Operations done in double element size
// Result may be same size as element or double the element size (L version)
// Implemented with run time call - morph code would be very complicated so
//    doing simple version for now - could be optimized if needed
//
static void emitVMULP(armMorphStateP state, Bool isLong) {

    if(checkAdvSIMDEnabled(state)) {

        Uns32 regs = state->attrs->regs;

        VMI_ASSERT(regs==1 || regs==2,  "Invalid regs %d", regs);

        // don't need these here but this catches any registers out of range
        (void) GET_SIMD_EL(state, r1, regs-1, 0, ARM_SIMD_REG_BYTES);
        (void) GET_SIMD_EL(state, r2, regs-1, 0, ARM_SIMD_REG_BYTES);
        (void) GET_SIMD_EL(state, r3, regs-1, 0, ARM_SIMD_REG_BYTES);

        armEmitArgProcessor(state);
        armEmitArgUns32(state, state->info.r1);        // Destination register
        armEmitArgUns32(state, state->info.r2);        // First operand register
        armEmitArgUns32(state, state->info.r3);        // Second operand register
        armEmitArgUns32(state, regs);                // 1 for D version, 2 for Q version
        armEmitArgUns32(state, isLong);                // VMUL.P8 or VMULL.P8 instruction
        armEmitCall(state, (vmiCallFn)doVMULP);

    }
}

//
// Emit code for VMULL.P8  Qd, Dn, Dm
//
ARM_MORPH_FN(armEmitVMULL_P) {
    emitVMULP(state, True);
}

//
// Emit code for VMUL.P8  Vd, Vn, Vm
//
ARM_MORPH_FN(armEmitVMUL_P) {
    emitVMULP(state, False);
}

////////////////////////////////////////////////////////////////////////////////
// SIMD Floating Point instructions
////////////////////////////////////////////////////////////////////////////////

//
// Emit code for SIMD floating point binop, with 3 regs same size
// SIMD floating point is always single precision, with 1 to 4 elements
//
ARM_MORPH_FN(armEmitVBinop_F) {

    if(checkAdvSIMDEnabled(state)) {

        Uns32     regs     = state->attrs->regs;
        Uns32     elements = state->attrs->elements;
        Uns32     nels     = elements*regs;
        vmiFBinop op       = state->attrs->fbinop;
        vmiReg    r1       = GET_SIMD_EL(state, r1, 0, 0, 4);
        vmiReg    r2       = GET_SIMD_EL(state, r2, 0, 0, 4);
        vmiReg    r3       = GET_SIMD_EL(state, r3, 0, 0, 4);

        armEmitFBinopSimdRRR(state, vmi_FT_32_IEEE_754, nels, op, r1, r2, r3);
    }
}

//
// Per-element code for SIMD floating point binop. Used when instruction doesn't
// operate on pairs of elements in the standard way (e.g. pairwise or scalar)
//
static SIMD_EL_OP_FN(simdBinopEl_F) {

    VMI_ASSERT(opSize==32 && resultSize==32, "opSize (%d) and resultSize(%d) must be 32", opSize, resultSize);

    vmiFBinop op = state->attrs->fbinop;

    armEmitFBinopSimdRRR(state, vmi_FT_32_IEEE_754, 1, op, result, r2, r3);

}

//
// Emit code for SIMD floating point Binop with a scalar
//
ARM_MORPH_FN(armEmitVBinopZ_F) {
    emitSIMDPerEl(state, simdBinopEl_F, SDOP_SCALAR, True, 0);
}

//
// Emit code for SIMD floating point Pairwise op
//
ARM_MORPH_FN(armEmitVPairOp_F) {
    emitSIMDPairwisePerEl(state, simdBinopEl_F, 0);
}

//
// Emit code for SIMD floating point ternary op, with 3 regs same size
// SIMD floating point is always single precision, with 1 to 4 elements
//
ARM_MORPH_FN(armEmitVTernop_F) {

    if(checkAdvSIMDEnabled(state)) {

        Uns32      regs     = state->attrs->regs;
        Uns32      elements = state->attrs->elements;
        Uns32      nels     = elements*regs;
        vmiFTernop op       = state->attrs->fternop;
        vmiReg     r1       = GET_SIMD_EL(state, r1, 0, 0, 4);
        vmiReg     r2       = GET_SIMD_EL(state, r2, 0, 0, 4);
        vmiReg     r3       = GET_SIMD_EL(state, r3, 0, 0, 4);

        armEmitFTernopSimdRRRR(state, vmi_FT_32_IEEE_754, nels, op, r1, r2, r3, r1, True);
    }
}

//
// Per-element code for SIMD floating point ternop. Used when instruction doesn't
// operate on pairs of elements in the standard way (e.g. pairwise or scalar)
//
static SIMD_EL_OP_FN(simdTernopEl_F) {

    VMI_ASSERT(opSize==32 && resultSize==32, "opSize (%d) and resultSize(%d) must be 32", opSize, resultSize);

    vmiFTernop op = state->attrs->fternop;

    armEmitFTernopSimdRRRR(state, vmi_FT_32_IEEE_754, 1, op, result, r2, r3, r1, True);
}

//
// Emit code for SIMD floating point Ternop with a scalar
//
ARM_MORPH_FN(armEmitVTernopZ_F) {
    emitSIMDPerEl(state, simdTernopEl_F, SDOP_SCALAR, True, 0);
}

//
// Per-element code for SIMD floating point VABS instruction
//
static SIMD_EL_OP_FN(simdAbsEl_F) {

    VMI_ASSERT(opSize==32 && resultSize==32, "opSize (%d) and resultSize(%d) must be 32", opSize, resultSize);

    // Take abs value by simply clearing the sign bit
    armEmitBinopRRC(state, resultSize, vmi_ANDN, result, r2, 0x80000000, 0);
}

//
// Emit code for SIMD floating point VABS instruction
//
ARM_MORPH_FN(armEmitVABS_F) {
    emitSIMDPerEl(state, simdAbsEl_F, SDOP_NONE, True, 0);
}

//
// Per-element code for SIMD floating point VNEG instruction
//
static SIMD_EL_OP_FN(simdNegEl_F) {

    VMI_ASSERT(opSize==32 && resultSize==32, "opSize (%d) and resultSize(%d) must be 32", opSize, resultSize);

    // Take negative of value by simply toggling the sign bit
    FPNeg(state, result, r2, 32);

}

//
// Emit code for SIMD floating point VNEG instruction
//
ARM_MORPH_FN(armEmitVNEG_F) {
    emitSIMDPerEl(state, simdNegEl_F, SDOP_NONE, True, 0);
}

//
// Per-element code for SIMD floating point VABD instruction
//
static SIMD_EL_OP_FN(simdAbsDiffEl_F) {

    VMI_ASSERT(opSize==32 && resultSize==32, "opSize (%d) and resultSize(%d) must be 32", opSize, resultSize);

    // Perform subtraction, checking for floating point exceptions
    armEmitFBinopSimdRRR(state, vmi_FT_32_IEEE_754, 1, vmi_FSUB, result, r2, r3);

    // Take abs value by simply clearing the sign bit
    armEmitBinopRC(state, resultSize, vmi_ANDN, result, 0x80000000, 0);
}

//
// Emit code for SIMD floating point VABD instruction
//
ARM_MORPH_FN(armEmitVABD_F) {
    emitSIMDPerEl(state, simdAbsDiffEl_F, SDOP_ELEMENT, True, 0);
}

////////////////////////////////////////////////////////////////////////////////
// SIMD Floating point Compare/Test instructions
////////////////////////////////////////////////////////////////////////////////

//
// Per-element code for floating point Vector Compare instructions
// that set result to 0/1 depending on comparison
//
static SIMD_EL_OP_FN(simdVCmpSelBool_F) {

    vmiFPRelation cond      = state->attrs->fpRelation;
    Bool          allowQNaN = state->attrs->allowQNaN;
    Uns32         ones      = ALL_ONES(resultSize);
    vmiReg        relation  = getTemp(state);
    vmiFlags      flags     = getZFFlags(VMI_REG_DELTA(relation, 1));

    VMI_ASSERT(opSize==32 && resultSize==32, "opSize (%d) and resultSize(%d) must be 32", opSize, resultSize);

    // Compare the floating point operands, getting vmiFPRelation result in the register relation
    armEmitFCompareRR(state, vmi_FT_32_IEEE_754, relation, r2, r3, allowQNaN, True);

    // Move zeros or ones to result depending on whether any cond bits are set in relation
    armEmitBinopRRC(state, 8, vmi_AND, VMI_NOREG, relation, cond, &flags);
    armEmitCondMoveRCC(state, resultSize, flags.f[vmi_ZF], False, result, ones, 0);
}

//
// Emit code for floating point Vector Comparison instructions
// that set result to 0/1 depending on comparison
//
ARM_MORPH_FN(armEmitVCmpBool_F) {
    emitSIMDPerEl(state, simdVCmpSelBool_F, SDOP_ELEMENT, True, 0);
}

//
// Per-element code for floating point Vector Absolute Compare instructions that
// set result to 0/1 depending on comparison
//
static SIMD_EL_OP_FN(simdVAbsCmpSelBool_F) {

    vmiReg t2 = newTemp32(state);
    vmiReg t3 = newTemp32(state);

    VMI_ASSERT(opSize==32, "opSize (%d) must be 32", opSize);

    // get absolute value of the 32 bit operands (clear the sign bit)
    armEmitBinopRRC(state, 32, vmi_AND, t2, r2, 0x7fffffff, 0);
    armEmitBinopRRC(state, 32, vmi_AND, t3, r3, 0x7fffffff, 0);

    // Perform standard compare with temporary values
    simdVCmpSelBool_F(state, esize, resultSize, opSize, result, r1, t2, t3, userData);
}

//
// Emit code for Vector Absolute Comparison Instructions
// that set result to 0/1 depending on comparison
//
ARM_MORPH_FN(armEmitVAbsCmp_F) {
    emitSIMDPerEl(state, simdVAbsCmpSelBool_F, SDOP_ELEMENT, True, 0);
}

//
// Per-element code for floating point Vector Compare immediate #0 instructions
//
static SIMD_EL_OP_FN(simdVCmp0SelBool_F) {

    vmiReg t = newTemp32(state);

    VMI_ASSERT(opSize==32, "opSize (%d) must be 32", opSize);

    // set a temp reg to 0
    armEmitMoveRC(state, 32, t, 0);

    // Perform standard compare with temporary 0
    simdVCmpSelBool_F(state, esize, resultSize, opSize, result, r1, r2, t, userData);
}

//
// Emit code for Vector Comparison with immediate #0 Instructions
// that set result to 0/1 depending on comparison
//
ARM_MORPH_FN(armEmitVCmp0_F) {
    emitSIMDPerEl(state, simdVCmp0SelBool_F, SDOP_NONE, True, 0);
}

//
// Per-element code for Vector Compare instructions
// that select Rn/Rm based on comparison
//
static SIMD_EL_OP_FN(simdVCmpSelReg_F) {

    vmiFPRelation cond       = state->attrs->fpRelation;
    Bool          allowQNaN  = state->attrs->allowQNaN;
    vmiReg        t          = newTemp32(state);
    vmiReg        relation   = newTemp32(state);
    vmiFlags      flags      = getZFFlags(VMI_REG_DELTA(relation, 1));
    vmiLabelP     checkZeros = armEmitNewLabel();
    vmiLabelP     setResult  = armEmitNewLabel();
    vmiLabelP     done       = armEmitNewLabel();

    VMI_ASSERT(opSize==32 && resultSize==32, "opSize (%d) and resultSize(%d) must be 32", opSize, resultSize);

    // Compare the floating point operands (sets invalidOp and input denormal sticky flags as needed)
    armEmitFCompareRR(state, vmi_FT_32_IEEE_754, relation, r2, r3, allowQNaN, True);

    //////////////////////// NAN CHECK ////////////////////////////////////////

    // Check if relation is unordered, indicating a NaN input
    armEmitTestRCJumpLabel(8, vmi_COND_Z, relation, vmi_FPRL_UNORDERED, checkZeros);

    // Here when there was a NaN input - set output to default NaN
    armEmitMoveRC(state, resultSize, result, ARM_QNAN_DEFAULT_32);
    armEmitUncondJumpLabel(done);

    //////////////////////// +/- 0 CHECK /////////////////////////////////

    // Check for both operands = 0 (including denormals that are flushed to 0)
    armEmitInsertLabel(checkZeros);
    armEmitTestRCJumpLabel(32, vmi_COND_NZ, r2, 0x7f800000, setResult);
    armEmitTestRCJumpLabel(32, vmi_COND_NZ, r3, 0x7f800000, setResult);

    // Here when both operands are 0:
    //   If sign bits are different (comparing +0 to -0) then
    //   result depends on whether this is a min or max check
    armEmitBinopRRR(state, 32, vmi_XOR, t, r2, r3, 0);
    armEmitBinopRC (state, 32, vmi_AND, t, 0x80000000, &flags);
    armEmitCondJumpLabel(flags.f[vmi_ZF], True, setResult);
    armEmitMoveRC(state, 32, result, cond == vmi_FPRL_LESS ? 0x80000000 : 0);
    armEmitUncondJumpLabel(done);

    ///////////////////////// SET RESULT ///////////////////////////////////////

    // set the result based on the comparison
    armEmitInsertLabel(setResult);
    armEmitBinopRRC(state, 8, vmi_AND, VMI_NOREG, relation, cond, &flags);
    armEmitCondMoveRRR(state, resultSize, flags.f[vmi_ZF], False, result, r2, r3);

    ///////////////////////// FLUSH TO ZERO CHECK ////////////////////////////

    // Flush result to zero if its exponent = 0
    armEmitTestRCJumpLabel(32, vmi_COND_NZ, result, 0x7f800000, done);
    armEmitBinopRC(state, 32, vmi_AND, result, 0x80000000, 0);

    ////////////////////////////////////////////////////////////////////////////

    // Here when done
    armEmitInsertLabel(done);
}

//
// Emit code for Vector Compare instructions that select Rn/Rm based on comparison
//
ARM_MORPH_FN(armEmitVCmpReg_F) {
    emitSIMDPerEl(state, simdVCmpSelReg_F, SDOP_ELEMENT, True, 0);
}

//
// Emit code for Pairwise Vector Compare instructions that select Rn/Rm based on comparison
//
ARM_MORPH_FN(armEmitVPCmpReg_F) {
    emitSIMDPairwisePerEl(state, simdVCmpSelReg_F, 0);
}


////////////////////////////////////////////////////////////////////////////////
// SIMD Step and Estimate Instructions
////////////////////////////////////////////////////////////////////////////////

// 32 bit Floating point representations of 1.5, 2.0 and 3.0
#define FP32_1POINT5 (0x3fc00000)
#define FP32_TWO     (0x40000000)
#define FP32_THREE   (0x40400000)

//
// Per-element code for SIMD VRECPE function
// Emit a run time call the function passed in userData
//
static SIMD_EL_OP_FN(simdVRecpeEl) {

    VMI_ASSERT(opSize==32 && resultSize==32, "opSize (%d) and resultSize(%d) must be 32", opSize, resultSize);

    armEmitArgProcessor(state);
    armEmitArgReg(state, 32, r2);
    armEmitCallResult(state, (vmiCallFn)userData, 32, result);
}

//
// Emit code for the floating point VRECPE instruction
//
ARM_MORPH_FN(armEmitVRECPE_F) {
    emitSIMDPerEl(state, simdVRecpeEl, SDOP_NONE, True, armFPRecipEstimate);
}

//
// Emit code for the unsigned VRECPE instruction
//
ARM_MORPH_FN(armEmitVRECPE_U) {
    emitSIMDPerEl(state, simdVRecpeEl, SDOP_NONE, True, armFPUnsignedRecipEstimate);
}

//
// Emit code for the floating point VRSQRTE instruction
//
ARM_MORPH_FN(armEmitVRSQRTE_F) {
    emitSIMDPerEl(state, simdVRecpeEl, SDOP_NONE, True, armFPRSqrtEstimate);
}

//
// Emit code for the unsigned VRSQRTE instruction
//
ARM_MORPH_FN(armEmitVRSQRTE_U) {
    emitSIMDPerEl(state, simdVRecpeEl, SDOP_NONE, True, armFPUnsignedRsqrtEstimate);
}

//
// Check if the regs op1 and op2 contain a single precision 0 and an infinity:
//  If found move value to result and jump to done, otherwise jump to label
//  Note: the denormal sticky flag will be set if check is true and an input is a denormal
//
static void emitInfAndZeroCheck(
    armMorphStateP state,
    vmiReg         op1,
    vmiReg         op2,
    vmiReg         result,
    Uns32          value,
    vmiLabelP      done
) {
    vmiLabelP doOperation = armEmitNewLabel();

    armEmitArgProcessor(state);
    armEmitArgReg(state, 32, op1);
    armEmitArgReg(state, 32, op2);
    armEmitCall(state, (vmiCallFn)armFPInfinityAndZero);
    armEmitCondJumpLabelFunctionResult(False, doOperation);

    // Here when input values are a zero and an infinity - result is the passed
    // value (2.0 or 1.5)
    armEmitMoveRC(state, 32, result, value);
    armEmitUncondJumpLabel(done);

    armEmitInsertLabel(doOperation);
}

//
// Per-element code for SIMD VRECPS
//
static SIMD_EL_OP_FN(simdVrecpsEl) {

    VMI_ASSERT(opSize==32 && resultSize==32, "opSize (%d) and resultSize(%d) must be 32", opSize, resultSize);

    vmiReg    t    = newTemp32(state);
    vmiLabelP done = armEmitNewLabel();

    emitInfAndZeroCheck(state, r2, r3, result, FP32_TWO, done);

    // result = 2 - r2*r3
    armEmitFBinopSimdRRR(state, vmi_FT_32_IEEE_754, 1, vmi_FMUL, t, r2, r3);
    armEmitMoveRC(state, 32, result, FP32_TWO);
    armEmitFBinopSimdRRR(state, vmi_FT_32_IEEE_754, 1, vmi_FSUB, result, result, t);

    armEmitInsertLabel(done);
}

//
// Emit code for the floating point VRECPS instruction
//
ARM_MORPH_FN(armEmitVRECPS_F) {
    emitSIMDPerEl(state, simdVrecpsEl, SDOP_ELEMENT, True, 0);
}

//
// Per-element code for SIMD VRSQRTS
//
static SIMD_EL_OP_FN(simdVrsqrtsEL) {

    VMI_ASSERT(opSize==32 && resultSize==32, "opSize (%d) and resultSize(%d) must be 32", opSize, resultSize);

    vmiReg    t    = newTemp32(state);
    vmiLabelP done = armEmitNewLabel();

    emitInfAndZeroCheck(state, r2, r3, result, FP32_1POINT5, done);

    // result = (3.0 - r2*r3) / 2.0
    armEmitFBinopSimdRRR(state, vmi_FT_32_IEEE_754, 1, vmi_FMUL, t, r2, r3);
    armEmitMoveRC(state, 32, result, FP32_THREE);
    armEmitFBinopSimdRRR(state, vmi_FT_32_IEEE_754, 1, vmi_FSUB, result, result, t);
    armEmitMoveRC(state, 32, t, FP32_TWO);
    armEmitFBinopSimdRRR(state, vmi_FT_32_IEEE_754, 1, vmi_FDIV, result, result, t);

    armEmitInsertLabel(done);
}

//
// Emit code for the floating point VRSQRTS instruction
//
ARM_MORPH_FN(armEmitVRSQRTS_F) {
    emitSIMDPerEl(state, simdVrsqrtsEL, SDOP_ELEMENT, True, 0);
}


////////////////////////////////////////////////////////////////////////////////
// VFP Data-Processing Instructions
////////////////////////////////////////////////////////////////////////////////

//
// Callback function to execute a VFP op on a single element
//
#define VFP_EL_OP_FN(_NAME) void _NAME(    \
    armMorphStateP state,                  \
    Uns32          ebytes,                 \
    vmiReg         result,                 \
    vmiReg         r1,                     \
    vmiReg         r2,                     \
    vmiReg         r3                      \
)
typedef VFP_EL_OP_FN((*vfpOpFunc));

//
// Is the indicated VFP register in a vector or scalar bank
//
static Bool vfpVectorBank(Uns32 r, Bool singlePrec)
{
    if (singlePrec) {

        // Registers 0-7 are scalars in single precision
        return r > 7;

    } else {

        // Registers 0-3 and 16-19 are scalars in double precision
        return (r > 3 && r < 16) || r > 19;

    }
}

//
// Return True if the VFP registers overlap
// Consider them overlapping if they use the same bank
//
static Bool vfpOverlap(
    Uns32 r1,
    Bool  r1Vector,
    Uns32 r2,
    Bool  usesR3,
    Uns32 r3,
    Bool  singlePrec
) {

    if (r1==r2 || (usesR3 && r1==r3)) {

        // If identical then they overlap
        return True;

    } else if (r1Vector) {

        // If not identical they can only overlap if they are vectors

        Uns32 bankMask = singlePrec ? 0x18 : 0x1c;
        Uns32 bank1    = r1 & bankMask;
        Uns32 bank2    = r2 & bankMask;
        Uns32 bank3    = r3 & bankMask;

        if (bank1==bank2 || (usesR3 && bank1==bank3)) {

            // Using vectors and r1 is in same bank as r2 and or r3 - assume they overlap
            // (It is possible they don't but we'll just keep it simplesince it will just
            //  add a little overhead and not affect results)
            return True;

        }
    }

    return False;

}

//
// Return the vmiReg for  the reg if vector is not true (reg is a scalar) or
// element e of the vector if vector is true. Vectors are implemented in banks
// of 18 registers if  singlePrec or 4 registers if double precision.
// The vectors wrap around with in each bank (no vector crosses between banks)
//
//
static vmiReg vfpRegEl(
    armMorphStateP state,
    Uns32          r,
    Uns32          e,
    Uns32          stride,
    Bool           vector,
    Bool           singlePrec
) {

    VMI_ASSERT (r < 32, "Reg number %d out of range", r);

    if (vector) {

        // Vector element - get reg r + (e*stride), rolling over within the bank
        Uns32 bankIdxMask = singlePrec ? 0x7 : 0x3;
        Uns32 bankIdx  = (r + (e*stride));

        r = (r & ~bankIdxMask) | (bankIdx & bankIdxMask);
    }

    return getVFPReg(state, r, singlePrec ? 4 : 8);

}

//
// Emit code per element for VFP vectors
// Vector ops specified by FPSCR LEN and STRIDE fields if dest is in a vector bank
// Call the 'perEl' function for each element in the vector
//
static void emitVFPPerEl(
    armMorphStateP  state,
    vfpOpFunc       perEl,
    Bool            usesR3
) {

    if(checkVFPEnabled(state, True)) {

        Uns32 ebytes   = state->attrs->ebytes;

        VMI_ASSERT(ebytes==4 || ebytes==8, "Invalid ebytes");

        Uns32 r1         = state->info.r1;
        Uns32 r2         = state->info.r2;
        Uns32 r3         = state->info.r3;
        Bool  singlePrec = (ebytes == 4);
        Bool  r1Vector   = vfpVectorBank(r1, singlePrec);
        Uns32 len        = 1;
        Uns32 stride     = 1;

        if (r1Vector && !armGetVFPLenStride(state, ebytes, &len, &stride)) {

            // Invalid value for Len/Stride
            emitUndefined(state, True);

        } else {

            Bool   overlaps  = vfpOverlap(r1, r1Vector, r2, usesR3, r3, singlePrec);
            Bool   r2Vector  = r1Vector && (usesR3 || vfpVectorBank(r2, singlePrec));
            Bool   r3Vector  = r1Vector && vfpVectorBank(r3, singlePrec);
            vmiReg temp      = overlaps ? newTemp256(state) : VMI_NOREG;
            Uns32  e;

            // Call perEl function for each element in the vector
            // (Note: all elements or just last element may be scalars)
            for (e = 0; e < len; e++) {

                vmiReg rd     = vfpRegEl(state, r1, e, stride, r1Vector, singlePrec);
                vmiReg rn     = vfpRegEl(state, r2, e, stride, r2Vector, singlePrec);
                vmiReg rm     = usesR3 ? vfpRegEl(state, r3, e, stride, r3Vector, singlePrec) : VMI_NOREG;
                vmiReg result = overlaps ? VMI_REG_DELTA(temp, (e*ebytes)) : rd;

                // Scalar operation if R1 is in a scalar bank or FPSCR.LEN is 1
                (*perEl)(state, ebytes, result, rd, rn, rm);

            }

            if (overlaps) {

                // Move values in temporary to the destination
                for (e = 0; e < len; e++) {

                    vmiReg r = vfpRegEl(state, r1, e, stride, r1Vector, singlePrec);
                    vmiReg t = VMI_REG_DELTA(temp, (e*ebytes));

                    armEmitMoveRR(state, ebytes*8, r, t);

                }
            }
        }
    }
}

//
// element code for VFP binop
//
VFP_EL_OP_FN(emitVFPBinopEL) {

    vmiFBinop op     = state->attrs->fbinop;
    Bool      negate = state->attrs->negate;

    armEmitFBinopSimdRRR(state, bytesToFType(ebytes), 1, op, result, r2, r3);

    // If negate attribute is selected negate the result
    if (negate) FPNeg(state, result, result, ebytes*8);

}

//
// Emit code for VFP floating point <binop>, with 3 regs same size
// Operands are either single or double precision according to ebytes
//
ARM_MORPH_FN(armEmitVFPBinop) {
    emitVFPPerEl(state, emitVFPBinopEL, True);
}

//
// element code for VFP unop
//
VFP_EL_OP_FN(emitVFPUnopEL) {

    vmiFUnop op = state->attrs->funop;

    armEmitFUnopSimdRR(state, bytesToFType(ebytes), 1, op, result, r2);

}

//
// Emit code for VFP floating point <Unop>, with 2 regs same size
// Operands are either single or double precision according to ebytes
//
ARM_MORPH_FN(armEmitVFPUnop) {
    emitVFPPerEl(state, emitVFPUnopEL, False);
}

//
// element code for VFP VABS
//
VFP_EL_OP_FN(emitVFPVabsEL) {

    Uns32 esize = ebytes*8;
    Uns64 mask  = 1ULL << (esize-1);

    // Take abs value by simply clearing the sign bit
    armEmitBinopRRC(state, esize, vmi_ANDN, result, r2, mask, 0);

}

//
// Emit code for VFP VABS Sd, Sm or VABS Dd, Dm
//
ARM_MORPH_FN(armEmitVABS_VFP) {
    emitVFPPerEl(state, emitVFPVabsEL, False);
}

//
// element code for VFP VNEG
//
VFP_EL_OP_FN(emitVFPVnegEL) {

    Uns32 esize = ebytes*8;

    // Take negative value by simply toggling the sign bit
    FPNeg(state, result, r2, esize);

}

//
// Emit code for VFP VNEG Sd, Sm or VNEG Dd, Dm
//
ARM_MORPH_FN(armEmitVNEG_VFP) {
    emitVFPPerEl(state, emitVFPVnegEL, False);
}

//
// element code for VFP multiplyAccumulate instructions
//
VFP_EL_OP_FN(emitVFPVMultAccEL) {

    vmiFBinop op      = state->attrs->fbinop;
    Bool      negate  = state->attrs->negate;
    Uns32     esize   = ebytes*8;
    vmiFType  type    = bytesToFType(ebytes);
    vmiReg    product = newTemp64(state);
    vmiReg    acc     = r1;

    VMI_ASSERT(op==vmi_FSUB || op==vmi_FADD, "Invalid fbinop");

    // Perform Multiply to temporary
    armEmitFBinopSimdRRR(state, type, 1, vmi_FMUL, product, r2, r3);

    if (negate) {

        // Negate the accumulation value before adding/subtracting
        acc = newTemp64(state);
        FPNeg(state, acc, r1, esize);

    }

    // To implement subtraction, negate the value being subtracted and use vmi_FADD
    if (op == vmi_FSUB) FPNeg(state, product, product, esize);

    // Add the (possibly negated) product to/from the accumlate value (+/- rd)
    armEmitFBinopSimdRRR(state, type, 1, vmi_FADD, result, acc, product);
}

//
// Emit code for VFP floating point Multiply (negate) accumulate/subtract instruction
//
ARM_MORPH_FN(armEmitVMulAcc_VFP) {
    emitVFPPerEl(state, emitVFPVMultAccEL, True);
}

////////////////////////////////////////////////////////////////////////////////
// VFP VCMP Instructions
////////////////////////////////////////////////////////////////////////////////

//
// Common code to execute VFP VCMP instructions
//
static void emitVCmpVFP(armMorphStateP state, vmiReg rd, vmiReg rm) {

    Bool   allowQNaN = state->attrs->allowQNaN;
    vmiReg relation  = getTemp(state);

    // Compare the floating point operands, getting vmiFPRelation result in the register relation
    armEmitFCompareRR(state, getFType(state), relation, rd, rm, allowQNaN, True);

    // Set the FPSCR N, Z, C, V flags according to the result
    armEmitArgProcessor(state);
    armEmitArgReg(state, 8, relation);
    armEmitCall(state, (vmiCallFn) setFPSCRFlags);

}

//
// Emit code for VFP VCMP/VCMPE instruction: VCMP{E} Dd, Dm or VCMP{E} Sd, Sm
// Note: VCMP instruction does not use short vectors
//
ARM_MORPH_FN(armEmitVCMP_VFP) {

    Uns32  ebytes = state->attrs->ebytes;
    vmiReg rd     = GET_VFP_REG(state, r1, 0, ebytes);
    vmiReg rm     = GET_VFP_REG(state, r2, 0, ebytes);

    emitVCmpVFP(state, rd, rm);

}

//
// Emit code for VFP VCMP/VCMPE immediate 0.0 instruction: VCMP{E} Dd, #0.0 or VCMP{E} Sd, #0.0
// Note: VCMP instruction does not use short vectors
//
ARM_MORPH_FN(armEmitVCMP0_VFP) {

    Uns32  ebytes = state->attrs->ebytes;
    vmiReg rd     = GET_VFP_REG(state, r1, 0, ebytes);
    vmiReg rm     = newTemp32(state);

    armEmitMoveRC(state, ebytes*8, rm, 0);
    emitVCmpVFP(state, rd, rm);

}


////////////////////////////////////////////////////////////////////////////////
// VFP VCVT Instructions
////////////////////////////////////////////////////////////////////////////////

//
// Emit code for VFP Single/Double precision conversion: VCVT Sd, Dm or VCVT Dd, Sm
// Note: VCVT instruction does not use short vectors
//
ARM_MORPH_FN(armEmitVCVT_SD_VFP) {

    if(checkVFPEnabled(state, True)) {

        Uns32  r1Bytes = state->attrs->ebytes;
        Uns32  r2Bytes = state->attrs->srcBytes;
        vmiReg rd      = GET_VFP_REG(state, r1, 0, r1Bytes);
        vmiReg rm      = GET_VFP_REG(state, r2, 0, r2Bytes);

        // Convert from source type to dest type
        armEmitFConvertRR(
            state,
            bytesToFType(r1Bytes),
            rd,
            bytesToFType(r2Bytes),
            rm,
            vmi_FPR_CURRENT
        );
    }
}

//
// Emit code for VFP Half to Single precision conversion: VCVTT.F32.F16 Sd, Sm or VCVTB.F32.F16 Sd, Sm
// Note: VCVT instruction does not use short vectors
//
ARM_MORPH_FN(armEmitVCVT_SH_VFP) {

    if(checkVFPEnabled(state, True)) {

        Uns32  r1Bytes = state->attrs->ebytes;
        Uns32  r2Bytes = state->attrs->srcBytes;

        VMI_ASSERT(r1Bytes==4 && r2Bytes==2, "Invalid sizes");

        Uns32  top = state->attrs->highhalf ? 1 : 0;
        vmiReg rd  = GET_VFP_REG(state, r1, 0, r1Bytes);
        vmiReg rm  = getVFPReg(state, (state->info.r2*2)+top, r2Bytes);

        armEmitArgProcessor(state);         // argument 1: processor
        armEmitArgReg(state, r2Bytes*8, rm);// argument 2: value
        armEmitCallResult(state, (vmiCallFn)armFPHalfToSingle, r1Bytes*8, rd);
    }
}

//
// Emit code for VFP  Single to Half precision conversion: VCVTT.F16.F32 Sd, Sm or VCVTB.F16.F32 Sd, Sm
// Note: VCVT instruction does not use short vectors
//
ARM_MORPH_FN(armEmitVCVT_HS_VFP) {

    if(checkVFPEnabled(state, True)) {

        Uns32  r1Bytes = state->attrs->ebytes;
        Uns32  r2Bytes = state->attrs->srcBytes;

        VMI_ASSERT(r1Bytes==2 && r2Bytes==4, "Invalid sizes");

        Uns32  top = state->attrs->highhalf ? 1 : 0;
        vmiReg rd  = getVFPReg(state, (state->info.r1*2)+top, r1Bytes);
        vmiReg rm  = GET_VFP_REG(state, r2, 0, r2Bytes);

        armEmitArgProcessor(state);         // argument 1: processor
        armEmitArgReg(state, r2Bytes*8, rm);// argument 2: value
        armEmitCallResult(state, (vmiCallFn)armFPSingleToHalf, r1Bytes*8, rd);
    }
}

//
// Emit code for VFP Floating point to Fixed point conversion: VCVT
// Note: r1 always equals r2 and the size of the register is defined by srcBytes
// Note: VCVT instruction does not use short vectors
//
ARM_MORPH_FN(armEmitVCVT_XF_VFP) {

    if(checkVFPEnabled(state, True)) {

        VMI_ASSERT(state->info.r1== state->info.r2, "r1 must be same as r2");

        Uns32   fracBits   = state->info.c;
        Uns32   sextend    = state->attrs->sextend;
        vmiFPRC roundFPSCR = state->attrs->roundFPSCR;
        Uns32   rdBytes    = state->attrs->ebytes;
        Uns32   rmBytes    = state->attrs->srcBytes;
        vmiReg  rd         = GET_VFP_REG(state, r1, 0, rmBytes);

        FPToFixed(state, rd, rdBytes, rd, rmBytes, fracBits, sextend, !roundFPSCR);

        if (rmBytes != rdBytes) {
            // sign extend to srcBytes
            armEmitMoveExtendRR(state, rmBytes*8, rd, rdBytes*8, rd, sextend);
        }
    }
}

//
// Emit code for VFP Floating point to Integer conversion: VCVT
// Note: VCVT instruction does not use short vectors
//
ARM_MORPH_FN(armEmitVCVT_IF_VFP) {

    if(checkVFPEnabled(state, True)) {

        Uns32   sextend    = state->attrs->sextend;
        vmiFPRC roundFPSCR = state->attrs->roundFPSCR;
        Uns32   rdBytes    = state->attrs->ebytes;
        Uns32   rmBytes    = state->attrs->srcBytes;
        vmiReg  rd         = GET_VFP_REG(state, r1, 0, rdBytes);
        vmiReg  rm         = GET_VFP_REG(state, r2, 0, rmBytes);

        FPToFixed(state, rd, rdBytes, rm, rmBytes, 0, sextend, !roundFPSCR);
    }
}

//
// Emit code for VFP Fixed point to Floating point conversion: VCVT
// Note: r1 always equals r2 and the size of the register is defined by srcBytes
// Note: VCVT instruction does not use short vectors
//
ARM_MORPH_FN(armEmitVCVT_FX_VFP) {

    if(checkVFPEnabled(state, True)) {

        VMI_ASSERT(state->info.r1== state->info.r2, "r1 must be same as r2");

        Uns32   fracBits   = state->info.c;
        Uns32   sextend    = state->attrs->sextend;
        vmiFPRC roundFPSCR = state->attrs->roundFPSCR;
        Uns32   rdBytes    = state->attrs->ebytes;
        Uns32   rmBytes    = state->attrs->srcBytes;
        vmiReg  rd         = GET_VFP_REG(state, r1, 0, rdBytes);

        fixedToFP(state, rd, rdBytes, rd, rmBytes, fracBits, sextend, !roundFPSCR);
    }
}

//
// Emit code for VFP Integer to Floating point conversion: VCVT
// Note: VCVT instruction does not use short vectors
//
ARM_MORPH_FN(armEmitVCVT_FI_VFP) {

    if(checkVFPEnabled(state, True)) {

        Uns32   sextend    = state->attrs->sextend;
        vmiFPRC roundFPSCR = state->attrs->roundFPSCR;
        Uns32   rdBytes    = state->attrs->ebytes;
        Uns32   rmBytes    = state->attrs->srcBytes;
        vmiReg  rd         = GET_VFP_REG(state, r1, 0, rdBytes);
        vmiReg  rm         = GET_VFP_REG(state, r2, 0, rmBytes);

        fixedToFP(state, rd, rdBytes, rm, rmBytes, 0, sextend, !roundFPSCR);
    }
}


////////////////////////////////////////////////////////////////////////////////
// DEPRECATED DISASSEMBLY MODE
////////////////////////////////////////////////////////////////////////////////

//
// Disassembly mode: SWI 99/9999 terminates the test
//
#define ARM_SWI_EXIT_CODE_THUMB  99
#define ARM_SWI_EXIT_CODE_NORMAL 9999

//
// Should morphing be disabled? (disassembly test mode only)
//
static Bool disableMorph(armMorphStateP state) {

    armP arm = state->arm;

    if(!ARM_DISASSEMBLE(arm)) {
        return False;
    } else if(state->info.type!=ARM_IT_SWI) {
        return True;
    } else if(
        ( IN_THUMB_MODE(arm) && (state->info.c==ARM_SWI_EXIT_CODE_THUMB)) ||
        (!IN_THUMB_MODE(arm) && (state->info.c==ARM_SWI_EXIT_CODE_NORMAL))
    ) {
        armEmitExit();
        return True;
    } else {
        return True;
    }
}


////////////////////////////////////////////////////////////////////////////////
// MORPHER MAIN ROUTINES
////////////////////////////////////////////////////////////////////////////////

//
// Advance to the new IT state
//
static Uns8 itAdvance(Uns8 oldState) {
    Uns8 newState = (oldState & 0xe0) | ((oldState<<1) & 0x1f);
    return (newState & 0xf) ? newState : 0;
}

//
// Emit code to update ITSTATE if required when the instruction completes (note
// that jumps and branches take special action to reset ITSTATE elsewhere)
//
static void emitUpdateITState(armMorphStateP state) {

    armP arm = state->arm;

    if(state->info.it) {
        arm->itStateMT = state->info.it;
        armEmitMoveRC(state, 8, ARM_IT_STATE, arm->itStateMT);
    } else if(arm->itStateMT) {
        arm->itStateMT = itAdvance(arm->itStateMT);
        armEmitMoveRC(state, 8, ARM_IT_STATE, arm->itStateMT);
    }
}

//
// Default morpher callback for implemented instructions
//
static void emitImplemented(armMorphStateP state) {

    // start code to skip this instruction conditionally unless it can be
    // emitted as a conditional jump
    setSkipLabel(state, !state->attrs->condJump ? emitStartSkip(state) : 0);

    // generate instruction code
    state->attrs->morphCB(state);

    // load CPSR from SPSR if required
    if(state->loadCPSR) {
        emitLoadCPSRFromSPSR(state);
    }

    // generate implicit jump to next instruction if required
    armEmitImplicitUncondJump(state);

    // insert conditional instruction skip target label
    emitLabel(getSkipLabel(state));

    // update ITSTATE if required when the instruction completes
    emitUpdateITState(state);
}

//
// Check whether the VFP floating-point type is not supported at the minimum required level
//
static Bool supportVFP(armMorphStateP state, armSDFPType dt, Uns32 minLevel) {

    armP arm = state->arm;
    Bool ok = False;

    VMI_ASSERT (state->attrs->iType==ARM_TY_VFP, "Called with non-VFP instruction");

    if (!VFP_PRESENT(arm)) {
        ok = False;
    } else if (dt == ARM_SDFPT_F32) {
        ok = ARM_MVFR(0, SinglePrecision) >= minLevel;
    } else if (dt == ARM_SDFPT_F64) {
        ok = ARM_MVFR(0, DoublePrecision) >= minLevel;
    } else {
        ok = True;
    }

    return ok;
}

//
// Determine whether a feature is supported by ISAR
//
static Bool supportedByISAR(armP arm, armMorphStateP state) {

    if(!state->info.isar) {

        // No ISAR restrictions on instruction
        return True;

    } else if (!CP_USE_CPUID(arm)) {

        // Processor doesen't implement CPUID scheme
        return True;

    } else {

        // Check if ISAR values meet requirements for this instruction to be supported
        switch(state->info.isar) {
            case ARM_ISAR_DIV:
                return ARM_ISAR(0, Divide_instrs);
            case ARM_ISAR_BKPT:
                return ARM_ISAR(0, Debug_instrs);
            case ARM_ISAR_CBZ:
                return ARM_ISAR(0, CmpBranch_instrs);
            case ARM_ISAR_BFC:
                return ARM_ISAR(0, BitField_instrs);
            case ARM_ISAR_CLZ:
                return ARM_ISAR(0, BitCount_instrs);
            case ARM_ISAR_SWP:
                return ARM_ISAR(0, Swap_instrs);
            case ARM_ISAR_BXJ:
                return ARM_ISAR(1, Jazelle_instrs);
            case ARM_ISAR_BX:
                return ARM_ISAR(1, Interwork_instrs)>0;
            case ARM_ISAR_BLX:
                return ARM_ISAR(1, Interwork_instrs)>1;
            case ARM_ISAR_BAA:
                return ARM_ISAR(1, Murac_instrs);
            case ARM_ISAR_MOVT:
                return ARM_ISAR(1, Immediate_instrs);
            case ARM_ISAR_IT:
                return ARM_ISAR(1, IfThen_instrs);
            case ARM_ISAR_SXTB:
                return ARM_ISAR(1, Extend_instrs)>0;
            case ARM_ISAR_SXTAB:
                return ARM_ISAR(1, Extend_instrs)>1;
            case ARM_ISAR_SXTB16:
                return (ARM_ISAR(1, Extend_instrs)>1) && (ARM_ISAR(3, SIMD_instrs)>2);
            case ARM_ISAR_SRS:
                return ARM_ISAR(1, Except_AR_instrs);
            case ARM_ISAR_LDM_UR:
                return ARM_ISAR(1, Except_instrs);
            case ARM_ISAR_SETEND:
                return ARM_ISAR(1, Endian_instrs);
            case ARM_ISAR_REV:
                return ARM_ISAR(2, Reversal_instrs)>0;
            case ARM_ISAR_RBIT:
                return ARM_ISAR(2, Reversal_instrs)>1;
            case ARM_ISAR_MRS_AR:
                return ARM_ISAR(2, PSR_AR_instrs);
            case ARM_ISAR_UMULL:
                return ARM_ISAR(2, MultU_instrs)>0;
            case ARM_ISAR_UMAAL:
                return ARM_ISAR(2, MultU_instrs)>1;
            case ARM_ISAR_SMULL:
                return ARM_ISAR(2, MultS_instrs)>0;
            case ARM_ISAR_SMLABB:
                return ARM_ISAR(2, MultS_instrs)>1;
            case ARM_ISAR_SMLAD:
                return ARM_ISAR(2, MultS_instrs)>2;
            case ARM_ISAR_MLA:
                return ARM_ISAR(2, Mult_instrs)>0;
            case ARM_ISAR_MLS:
                return ARM_ISAR(2, Mult_instrs)>1;
            case ARM_ISAR_PLD:
                return ARM_ISAR(2, MemHint_instrs)>0;
            case ARM_ISAR_PLI:
                return ARM_ISAR(2, MemHint_instrs)>2;
            case ARM_ISAR_PLDW:
                return ARM_ISAR(2, MemHint_instrs)>3;
            case ARM_ISAR_LDRD:
                return ARM_ISAR(2, LoadStore_instrs);
            case ARM_ISAR_THUMBEE:
                return ARM_ISAR(3, T2ExeEnvExtn_instrs);
            case ARM_ISAR_NOP:
                return ARM_ISAR(3, TrueNOP_instrs);
            case ARM_ISAR_MOVLL:
                return ARM_ISAR(3, ThumbCopy_instrs);
            case ARM_ISAR_TBB:
                return ARM_ISAR(3, TabBranch_instrs);
            case ARM_ISAR_LDREX:
                return ARM_ISAR(3, SynchPrim_instrs)>0;
            case ARM_ISAR_CLREX:
                return (ARM_ISAR(3, SynchPrim_instrs)>1) || (ARM_ISAR(4, SynchPrim_instrs_frac)==3);
            case ARM_ISAR_LDREXD:
                return ARM_ISAR(3, SynchPrim_instrs)>1;
            case ARM_ISAR_SVC:
                return ARM_ISAR(3, SVC_instrs);
            case ARM_ISAR_SSAT:
                return ARM_ISAR(3, SIMD_instrs)>0;
            case ARM_ISAR_PKHBT:
                return ARM_ISAR(3, SIMD_instrs)>2;
            case ARM_ISAR_QADD:
                return ARM_ISAR(3, Saturate_instrs);
            case ARM_ISAR_MRS_M:
                return ARM_ISAR(4, PSR_M_instrs);
            case ARM_ISAR_DMB:
                return ARM_ISAR(4, Barrier_instrs);
            case ARM_ISAR_LDRBT:
                return ARM_ISAR(4, Unpriv_instrs)>0;
            case ARM_ISAR_LDRHT:
                return ARM_ISAR(4, Unpriv_instrs)>1;
            case ARM_ISAR_VMRS:
                return (ADVSIMD_PRESENT(arm) || VFP_PRESENT(arm)) && ARM_MVFR(0, A_SIMD_Registers);
            case ARM_ISAR_VFPV2:
                return supportVFP(state, state->info.dt1, 1);
            case ARM_ISAR_VFPV3:
                return supportVFP(state, state->info.dt1, 2);
            case ARM_ISAR_VFPSQRT:
                return ARM_MVFR(0, SquareRoot) && supportVFP(state, state->info.dt1, 1);
            case ARM_ISAR_VFPDIV:
                return ARM_MVFR(0, Divide) && supportVFP(state, state->info.dt1, 1);
            case ARM_ISAR_VFPCVT2:
                return supportVFP(state, state->info.dt1, 1) && supportVFP(state, state->info.dt2, 1);
            case ARM_ISAR_VFPCVT3:
                return supportVFP(state, state->info.dt1, 2) && supportVFP(state, state->info.dt2, 2);
            case ARM_ISAR_VFPHP:
                return VFP_PRESENT(arm) && ARM_MVFR(1, VFP_HalfPrecision);
            case ARM_ISAR_SIMDHP:
                return ADVSIMD_PRESENT(arm) && ARM_MVFR(1, A_SIMD_HalfPrecision);
            case ARM_ISAR_SIMDSP:
                return ADVSIMD_PRESENT(arm) && ARM_MVFR(1, A_SIMD_SinglePrecision);
            case ARM_ISAR_SIMDINT:
                return ADVSIMD_PRESENT(arm) && ARM_MVFR(1, A_SIMD_Integer);
            case ARM_ISAR_SIMDLDST:
                return ADVSIMD_PRESENT(arm) && ARM_MVFR(1, A_SIMD_LoadStore);
            default:
                VMI_ABORT("%s: unimplemented case", FUNC_NAME);
                return False;
        }
    }
}

//
// Return a boolean indicating whether the processor supports the required
// architecture
//
static Bool supportedOnVariant(armP arm, armMorphStateP state) {

    armArchitecture configVariant   = arm->configInfo.arch;
    armArchitecture requiredVariant = state->info.support;

    if(ARM_INSTRUCTION_VERSION(requiredVariant) > getInstructionVersion(arm)) {
        return False;
    } else if(!ARM_SUPPORT(configVariant, requiredVariant & ~ARM_MASK_VERSION)) {
        return False;
    } else {
        return True;
    }
}

//
// Create code for the ARM instruction at the simulated address referenced
// by 'thisPC'.
//
VMI_MORPH_FN(armMorphInstruction) {

    armP          arm = (armP)processor;
    armMorphState state;

    // seed morph-time ITSTATE and floating point mode if this is the first
    // instruction in a code block
    if(firstInBlock) {
        arm->itStateMT = arm->itStateRT;
        arm->fpModeMT  = ARM_FPM_NONE;
    }

    // get instruction and instruction type
    armDecode(arm, thisPC, &state.info);

    // get morpher attributes for the decoded instruction and initialize other
    // state fields
    state.attrs       = &armMorphTable[state.info.type];
    state.arm         = arm;
    state.nextPC      = state.info.thisPC + state.info.bytes;
    state.skipLabel   = 0;
    state.tempIdx     = 0;
    state.pcFetched   = False;
    state.pcSet       = ASPC_NA;
    state.pcImmediate = 0;
    state.loadCPSR    = False;
    state.setMode     = False;

    // if this is the first instruction in the block, mark all derived flags as
    // invalid and clear teeNZMask (bitmask of registers that have already been
    // zero-checked in ThumbEE mode)
    if(firstInBlock) {
        resetDerivedFlags(&state);
        arm->teeNZMask = 0;
    }

    // add interwork check if required
    if(arm->checkInterwork) {
        armEmitValidateBlockMask(ARM_BM_THUMB);
    }

    if(disableMorph(&state)) {
        // no action if in disassembly mode
    } else if(!supportedOnVariant(arm, &state)) {
        // instruction not supported on this variant
        emitNotVariant(&state);
    } else if(!supportedByISAR(arm, &state)) {
        // instruction not supported according to the ISAR
        emitNotISAR(&state);
    } else if(state.attrs->morphCB) {
        // translate the instruction
        emitImplemented(&state);
    } else if(state.info.type==ARM_IT_LAST) {
        // take UndefinedInstruction exception
        emitUndefined(&state, False);
    } else {
        // here if no morph callback specified
        emitUnimplemented(&state);
    }
}

//
// Snap instruction fetch addresses to the appropriate boundary
//
VMI_FETCH_SNAP_FN(armFetchSnap) {

    armP  arm  = (armP)processor;
    Uns32 snap = IN_THUMB_MODE(arm) ? 1 : 3;

    return thisPC & ~snap;
}


