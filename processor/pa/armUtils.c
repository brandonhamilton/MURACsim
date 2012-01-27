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
#include "vmi/vmiRt.h"
#include "vmi/vmiMessage.h"

// model header files
#include "armDecode.h"
#include "armExceptions.h"
#include "armFunctions.h"
#include "armStructure.h"
#include "armSIMDVFPRegisters.h"
#include "armUtils.h"
#include "armVM.h"

//
// Return the version of the instruction set implemented by the processor
//
inline static Uns32 getInstructionVersion(armP arm) {
    return ARM_INSTRUCTION_VERSION(arm->configInfo.arch);
}

//
// Does this processor use the legacy endian model?
//
inline static Bool useLegacyEndianModel(armP arm) {
    return (getInstructionVersion(arm)<=5);
}

//
// This returns the endianness of the ARM processor
//
VMI_ENDIAN_FN(armGetEndian) {

    armP arm = (armP)processor;
    Bool isBigEndian;

    if(useLegacyEndianModel(arm)) {
        // legacy BE-32 endian mode in ARMv4 and ARMv5
        isBigEndian = CP_FIELD_DEFAULT(arm, SCTLR, B);
    } else if(isFetch) {
        // ARMv6 and ARMv7 instruction fetch
        isBigEndian = CP_FIELD_DEFAULT(arm, SCTLR, IE);
    } else {
        // ARMv6 and ARMv7 data access
        isBigEndian = arm->CPSR.fields.E;
    }

    return isBigEndian ? MEM_ENDIAN_BIG : MEM_ENDIAN_LITTLE;
}

//
// Set the initial endianness for the model
//
void armSetInitialEndian(armP arm, Bool isBigEndian) {

    if(useLegacyEndianModel(arm)) {
        // legacy BE-32 endian mode in ARMv4 and ARMv5
        CP_FIELD_DEFAULT(arm, SCTLR, B) = isBigEndian;
    } else {
        // ARMv6 and ARMv7 endian mode - initialize the instruction endianess,
        // the exception endianness and the data endianness to the passed value
        CP_FIELD_DEFAULT(arm, SCTLR, IE) = isBigEndian;
        CP_FIELD_DEFAULT(arm, SCTLR, EE) = isBigEndian;
        arm->CPSR.fields.E               = isBigEndian;
    }
}

//
// Return the next instruction address after 'thisPC'.
//
VMI_NEXT_PC_FN(armNextInstruction) {
    return (Uns32)(thisPC + armGetInstructionSize((armP)processor, thisPC));
}

//
// Return the name of a GPR
//
const char *armGetGPRName(armP arm, Uns32 index) {

    static const char *gprNames[] = {
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
        "r8", "r9", "sl", "fp", "ip", "sp", "lr", "pc"
    };

    return gprNames[index];
}

//
// Return the name of a CPR
//
const char *armGetCPRName(armP arm, Uns32 index) {

    static const char *cprNames[] = {
        "cr0",  "cr1",  "cr2",  "cr3",  "cr4",  "cr5",  "cr6",  "cr7",
        "cr8",  "cr9",  "cr10", "cr11", "cr12", "cr13", "cr14", "cr15"
    };

    return cprNames[index];
}

//
// If register 'base' is in the GPR group, set 'gprIndex' to its index number
// and return True; otherwise, return False
//
Bool armGetGPRIndex(armMorphStateP state, vmiReg base, Uns32 *gprIndex) {

    // get bounds of the register bank
    const static vmiReg r0 = ARM_REG_CONST(0);
    Uns32 first = VMI_REG_INDEX(r0);
    Uns32 last  = first + (ARM_GPR_NUM*ARM_GPR_BYTES);

    // get base register index
    Uns32 this = VMI_REG_INDEX(base);

    // if this index lies in the GPR range, return the GPR number to which it
    // maps
    if((first<=this) && (this<last)) {
        *gprIndex = (this-first)/ARM_GPR_BYTES;
        return True;
    } else {
        return False;
    }
}

#define SWAP_REG(_R1, _R2)  {Uns32 _V = _R1; _R1= _R2; _R2 = _V;}
#define SWAP_GPR(_N, _SET)  SWAP_REG(arm->regs[_N], arm->bank.R##_N##_##_SET)
#define SWAP_SPSR(_SET)     SWAP_REG(arm->SPSR.reg, arm->bank.SPSR_##_SET.reg)

//
// Switch banked registers on switch to the passed mode
//
static void switchRegs(armP arm, armCPSRMode mode) {

    switch(mode) {

        case ARM_CPSR_USER:
            // no action - already in base mode
            break;

        case ARM_CPSR_FIQ:
            SWAP_GPR(8,  fiq);
            SWAP_GPR(9,  fiq);
            SWAP_GPR(10, fiq);
            SWAP_GPR(11, fiq);
            SWAP_GPR(12, fiq);
            SWAP_GPR(13, fiq);
            SWAP_GPR(14, fiq);
            SWAP_SPSR(fiq);
            break;

        case ARM_CPSR_IRQ:
            SWAP_GPR(13, irq);
            SWAP_GPR(14, irq);
            SWAP_SPSR(irq);
            break;

        case ARM_CPSR_SUPERVISOR:
            SWAP_GPR(13, svc);
            SWAP_GPR(14, svc);
            SWAP_SPSR(svc);
            break;

        case ARM_CPSR_ABORT:
            SWAP_GPR(13, abt);
            SWAP_GPR(14, abt);
            SWAP_SPSR(abt);
            break;

        case ARM_CPSR_UNDEFINED:
            SWAP_GPR(13, und);
            SWAP_GPR(14, und);
            SWAP_SPSR(und);
            break;

        case ARM_CPSR_SYSTEM:
            // no action - already in base mode
            break;

        default:
            break;
    }
}

//
// Switch banked registers on switch between the passed modes
//
void armSwitchRegs(armP arm, armCPSRMode oldCPSRMode, armCPSRMode newCPSRMode) {
    switchRegs(arm, oldCPSRMode);
    switchRegs(arm, newCPSRMode);
}

#define ARM_MODE_INFO(_D) [ARM_CPSR_##_D] = {name:#_D, code:ARM_CPSR_##_D}

//
// Table of processor mode descriptions
//
static const vmiModeInfo modes[ARM_CPSR_LAST] = {
    ARM_MODE_INFO(USER),
    ARM_MODE_INFO(FIQ),
    ARM_MODE_INFO(IRQ),
    ARM_MODE_INFO(SUPERVISOR),
    ARM_MODE_INFO(ABORT),
    ARM_MODE_INFO(UNDEFINED),
    ARM_MODE_INFO(SYSTEM),
};

//
// Return effective processor mode for the passed effective mode (translates
// invalid modes to user mode)
//
inline armCPSRMode armGetCPSRMode(armCPSRMode raw) {
    return modes[raw].name ? raw : ARM_CPSR_USER;
}

//
// Processor mode iterator
//
VMI_MODE_INFO_FN(armModeInfo) {

    vmiModeInfoCP end = modes+ARM_CPSR_LAST;
    vmiModeInfoCP this;

    // on the first call, start with the first member of the table
    if(!prev) {
        prev = modes-1;
    }

    // search for the next member with seeded name
    for(this=prev+1; this!=end; this++) {
        if(this->name) {
            return this;
        }
    }

    // no more modes
    return 0;
}

//
// Return the current processor mode
//
VMI_GET_MODE_FN(armGetMode) {
    armP arm = (armP)processor;
    return &modes[armGetCPSRMode(arm->CPSR.fields.mode)];
}

//
// Update processor block mask
//
void armSetBlockMask(armP arm) {

    // preserve T and J bits from the current blockMask
    armBlockMask blockMask = arm->blockMask & (ARM_BM_THUMB|ARM_BM_JAZ_EE);

    // get blockmask component selecting register bank
    switch(arm->CPSR.fields.mode) {
        case ARM_CPSR_FIQ:        blockMask |= ARM_BM_FIQ;        break;
        case ARM_CPSR_IRQ:        blockMask |= ARM_BM_IRQ;        break;
        case ARM_CPSR_SUPERVISOR: blockMask |= ARM_BM_SUPERVISOR; break;
        case ARM_CPSR_ABORT:      blockMask |= ARM_BM_ABORT;      break;
        case ARM_CPSR_UNDEFINED:  blockMask |= ARM_BM_UNDEFINED;  break;
        case ARM_CPSR_SYSTEM:     blockMask |= ARM_BM_SYSTEM;     break;
        default:                  blockMask |= ARM_BM_USER;       break;
    }

    // include component for endianness
    if(arm->CPSR.fields.E) {
        blockMask |= ARM_BM_BIG_ENDIAN;
    }

    // include component for alignment checking
    if(DO_UNALIGNED(arm)) {
        blockMask |= ARM_BM_UNALIGNED;
    }

    // include component for interwork checking
    if(CP_FIELD(arm, SCTLR, L4)) {
        blockMask |= ARM_BM_L4;
    }

    // include component for FPU-enable checking
    if(SDFP_FIELD(arm, FPEXC, EN)) {
        blockMask |= ARM_BM_SDFP_EN;
    }

    // include component SWP/SWP enable checking
    if(!CP_MP_IMPLEMENTED(arm) || CP_FIELD(arm, SCTLR, SW)) {
        blockMask |= ARM_BM_SWP;
    }

    // include component for cp10 (and cp11) enable checking
    // NOTE: arm documentation specifies behavior is undefined if cp10 enable
    // differs from cp11 enable, so only cp10 enable is checked here
    Uns32 cp10Enable = CP_FIELD(arm, CPACR, cp10);
    if(blockMask&ARM_BM_USER) {
        if(cp10Enable&2) {blockMask |= ARM_BM_CP10;}
    } else {
        if(cp10Enable&1) {blockMask |= ARM_BM_CP10;}
    }

    // include component for CPACR.ASEDIS
    if(CP_FIELD(arm, CPACR, ASEDIS)) {
        blockMask |= ARM_BM_ASEDIS;
    }

    // include component for CPACR.D32DIS
    if(CP_FIELD(arm, CPACR, D32DIS)) {
        blockMask |= ARM_BM_D32DIS;
    }

    // include FPSCR Stride field bits
    blockMask |= (SDFP_FIELD(arm, FPSCR, Stride) * ARM_BM_STRIDE0);

    // include FPSCR Len field bits
    blockMask |= (SDFP_FIELD(arm, FPSCR, Len) * ARM_BM_LEN0);

    // save modified blockMask on the processor
    arm->blockMask = blockMask;

    // use the new blockMask
    vmirtSetBlockMask((vmiProcessorP)arm, blockMask);
}

//
// Update processor mode when CPSR has been written
//
static void writeCPSRMode(armP arm, armCPSRMode oldCPSRMode) {

    // switch banked registers if required
    armSwitchRegs(arm, oldCPSRMode, arm->CPSR.fields.mode);

    // update block mask
    armSetBlockMask(arm);

    // switch mode if required
    armSwitchMode(arm);
}

//
// Update processor endianness when CPSR has been written
//
static void writeCPSREndian(armP arm) {

    // if this is the first time that endianness has been switched, discard the
    // current code dictionaries (they need to be regenerated with endianness
    // checking enabled in the block mask)
    if(!arm->checkEndian) {
        vmirtFlushAllDicts((vmiProcessorP)arm);
        arm->checkEndian = True;
    }

    // update block mask
    armSetBlockMask(arm);
}

//
// Set up read and write masks for PSRs based on configuration options
//
void armSetPSRMasks(armP arm) {

    armArchitecture arch = arm->configInfo.arch;

    // CPSR/APSR read masks - IT, J and T bits are RAZ
    arm->rMaskCPSR = ~CPSR_EXEC;
    arm->rMaskAPSR = arm->rMaskCPSR;

    // SPSR write mask contains all bits except reserved field 23:0
    arm->wMaskSPSR = 0xff0fffff;

    // CPSR write mask initially identical to SPSR write mask
    arm->wMaskCPSR = arm->wMaskSPSR;

    // CPSR.E, CPSR.GE and CPSR.A are only writeable in ARMv6 and later
    if(ARM_INSTRUCTION_VERSION(arch)<6) {
        arm->wMaskCPSR &= ~(CPSR_ENDIAN|CPSR_GE30|CPSR_ABORTE);
    }

    // if-then state is only writable if Thumb2 is present
    if(!ARM_SUPPORT(arch, ARM_VT2)) {
        arm->wMaskCPSR &= ~CPSR_IT;
    }

    // APSR write mask excludes lower half of word and execution state bits
    arm->wMaskAPSR = arm->wMaskCPSR & ~(0x0000ffff|CPSR_EXEC);
}

//
// Return CPSR/APSR read mask for the current mode
//
Uns32 armGetReadMaskCPSR(armP arm) {
    return IN_USER_MODE(arm) ? arm->rMaskAPSR : arm->rMaskCPSR;
}

//
// Return CPSR/APSR write mask for the current mode
//
Uns32 armGetWriteMaskCPSR(armP arm, Bool affectExecState) {

    Uns32 writeMask = IN_USER_MODE(arm) ? arm->wMaskAPSR : arm->wMaskCPSR;

    // exclude execution state bits if required
    if(!affectExecState) {
        writeMask &= ~CPSR_EXEC;
    }

    return writeMask;
}

//
// Return SPSR write mask
//
Uns32 armGetWriteMaskSPSR(armP arm) {
    return arm->wMaskSPSR;
}

//
// Read CPSR register
//
Uns32 armReadCPSR(armP arm) {

    // seed T and J bits from blockMask
    arm->CPSR.fields.T = IN_THUMB_MODE(arm);
    arm->CPSR.fields.J = IN_JAZ_EE_MODE(arm);

    // seed flag fields from flag structure
    arm->CPSR.fields.Z = arm->aflags.ZF;
    arm->CPSR.fields.N = arm->aflags.NF;
    arm->CPSR.fields.C = arm->aflags.CF;
    arm->CPSR.fields.V = arm->aflags.VF;
    arm->CPSR.fields.Q = arm->oflags.QF;

    // seed if-then state fields
    arm->CPSR.fields.IT10 = arm->itStateRT & 0x3;
    arm->CPSR.fields.IT72 = arm->itStateRT >> 2;

    // return derived value
    return arm->CPSR.reg;
}

//
// Write CPSR register
//
void armWriteCPSR(armP arm, Uns32 value, Uns32 mask) {

    // save current processor mode and endianness
    armCPSRMode oldCPSRMode = arm->CPSR.fields.mode;
    Bool        oldCPSRE    = arm->CPSR.fields.E;

    // set new register value (writable bits only)
    arm->CPSR.reg = (value & mask) | (arm->CPSR.reg & ~mask);

    // ensure final mode in CPSR is always valid
    arm->CPSR.fields.mode = armGetCPSRMode(arm->CPSR.fields.mode);

    // get new processor mode and endianness
    armCPSRMode newCPSRMode = arm->CPSR.fields.mode;
    Bool        newCPSRT    = arm->CPSR.fields.T;
    Bool        newCPSRJ    = arm->CPSR.fields.J;
    Bool        newCPSRE    = arm->CPSR.fields.E;

    // update arithmetic flag fields if required
    if(mask & CPSR_FLAGS) {
        arm->aflags.ZF = arm->CPSR.fields.Z;
        arm->aflags.NF = arm->CPSR.fields.N;
        arm->aflags.CF = arm->CPSR.fields.C;
        arm->aflags.VF = arm->CPSR.fields.V;
        arm->oflags.QF = arm->CPSR.fields.Q;
    }

    // update if-then state if required
    if(mask & CPSR_IT) {
        arm->itStateRT = (arm->CPSR.fields.IT10 | (arm->CPSR.fields.IT72<<2));
    }

    // update processor mode if required
    if(
        (oldCPSRMode!=newCPSRMode) ||
        (IN_THUMB_MODE(arm)!=newCPSRT) ||
        (IN_JAZ_EE_MODE(arm)!=newCPSRJ)
    ) {
        // reset blockMask with correct T & J bits
        arm->blockMask = 0;
        if(newCPSRT) {arm->blockMask |= ARM_BM_THUMB; }
        if(newCPSRJ) {arm->blockMask |= ARM_BM_JAZ_EE;}

        // update mode (and possibly switch register set)
        writeCPSRMode(arm, oldCPSRMode);
    }

    // take any pending interrupts that have just been enabled
    if((mask & CPSR_IE) && armInterruptPending(arm)) {
        vmirtDoSynchronousInterrupt((vmiProcessorP)arm);
    }

    // switch endianness if required
    if((oldCPSRE != newCPSRE) && !useLegacyEndianModel(arm)) {
        writeCPSREndian(arm);
    }
}

//
// Read SPSR register
//
Uns32 armReadSPSR(armP arm) {
    return arm->SPSR.reg;
}

//
// Write SPSR register
//
void armWriteSPSR(armP arm, Uns32 value, Uns32 mask) {
    arm->SPSR.reg = (value & mask) | (arm->SPSR.reg & ~mask);
}

//
// Read PC register
//
Uns32 armReadPC(armP arm) {
    return vmirtGetPC((vmiProcessorP)arm);
}

//
// Write PC register
//
void armWritePC(armP arm, Uns32 value) {
    Uns32 snap = IN_THUMB_MODE(arm) ? 1 : 3;
    vmirtSetPC((vmiProcessorP)arm, value & ~snap);
}

//
// Switch processor mode if required
//
void armSwitchMode(armP arm) {

    armMode mode = 0;

    // derive dictionary mode
    if(IN_USER_MODE(arm))    {mode |= ARM_MODE_U;      }
    if(MMU_MPU_ENABLED(arm)) {mode |= ARM_MODE_MMU_MPU;}

    if(mode != arm->mode) {

        // switch mode if required
        vmirtSetMode((vmiProcessorP)arm, mode);
        arm->mode = mode;

        // memory mappings may need to be updated if FCSE is enabled
        if(arm->configInfo.fcsePresent) {
            armVMSetPIDOrModeFCSE(arm);
        }
    }
}

//
// Force the processor temporarily into the passed mode
//
void armForceMode(armP arm, armCPSRMode mode) {
    arm->CPSR.fields.mode = mode;
    armSwitchMode(arm);
}


////////////////////////////////////////////////////////////////////////////////
// PROCESSOR RUN STATE TRANSITION HANDLING
////////////////////////////////////////////////////////////////////////////////

//
// If this memory access callback is triggered, abort any active load linked
//
static VMI_MEM_WATCH_FN(abortEA) {
    armAbortExclusiveAccess((armP)userData);
}

//
// Install or remove the exclusive access monitor callback
//
static void updateExclusiveAccessCallback(armP arm, Bool install) {

    memDomainP domain  = vmirtGetProcessorDataDomain((vmiProcessorP)arm);
    Uns32      simLow  = arm->exclusiveTag;
    Uns32      simHigh = simLow + ~arm->exclusiveTagMask;

    // install or remove a watchpoint on the current exclusive access address
    if(install) {
        vmirtAddWriteCallback(domain, simLow, simHigh, abortEA, arm);
    } else {
        vmirtRemoveWriteCallback(domain, simLow, simHigh, abortEA, arm);
    }
}

//
// Abort any active exclusive access
//
void armAbortExclusiveAccess(armP arm) {

    if(arm->exclusiveTag != ARM_NO_TAG) {

        // remove callback on exclusive access monitor region
        updateExclusiveAccessCallback(arm, False);

        // clear exclusive tag (AFTER updateExclusiveAccessCallback)
        arm->exclusiveTag = ARM_NO_TAG;
    }
}

//
// This is called on simulator context switch (when this processor is either
// about to start or about to stop simulation)
//
VMI_IASSWITCH_FN(armContextSwitchCB) {

    armP arm = (armP)processor;

    // establish a watchpoint on a pending exclusive address to detect if that
    // address is written elsewhere.
    if(arm->exclusiveTag != ARM_NO_TAG) {
        updateExclusiveAccessCallback(arm, state==RS_SUSPEND);
    }

    // if a processor is about to be run, make state consistent with any changes
    // that may have occurred while the processor was suspended
    if(state==RS_RUN) {
        armSetBlockMask(arm);
    }
}

