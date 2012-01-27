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

// standard header files
#include <stdio.h>
#include <string.h>

// Imperas header files
#include "hostapi/impAlloc.h"

// VMI header files
#include "vmi/vmiDecode.h"
#include "vmi/vmiMessage.h"
#include "vmi/vmiRt.h"
#include "vmi/vmiView.h"
#include "vmi/vmiDoc.h"

// model header files
#include "armCP.h"
#include "armCPRegisters.h"
#include "armDecodeTypes.h"
#include "armEmit.h"
#include "armExceptions.h"
#include "armMessage.h"
#include "armMorph.h"
#include "armMPCore.h"
#include "armStructure.h"
#include "armUtils.h"
#include "armVM.h"

//
// Prefix for messages from this module
//
#define CPU_PREFIX "ARM_CP"


////////////////////////////////////////////////////////////////////////////////
// UTILITIES
////////////////////////////////////////////////////////////////////////////////

//
// Return the version of the instruction set implemented by the processor
//
inline static Uns32 getInstructionVersion(armP arm) {
    return ARM_INSTRUCTION_VERSION(arm->configInfo.arch);
}

//
// Return current program counter
//
inline static Uns32 getPC(armP arm) {
    return vmirtGetPC((vmiProcessorP)arm);
}

//
// Return a value composed of the bits of 'newValue' selected by 'mask' and the
// bits of 'oldValue' selected by '~mask'. Issue an assertion if 'newValue'
// specifies updated bits that aren't selected by 'mask'.
//
static Uns32 getMaskedValue(
    armP        arm,
    const char *regName,
    Uns32       newValue,
    Uns32       oldValue,
    Uns32       mask
) {
    Uns32 result  = (newValue & mask) | (oldValue & ~mask);
    Uns32 discard = (newValue ^ oldValue) & ~mask;

    // assert if modified bits were discarded
    if(discard) {

        Uns32 thisPC = getPC(arm);

        vmiMessage("a", CPU_PREFIX"_MBD",
            SRCREF_FMT "coprocessor 15 register %s: "
            "modified bits at positions 0x%08x were ignored",
            SRCREF_ARGS(arm, thisPC),
            regName,
            discard
        );
    }

    // return the final result
    return result;
}

//
// Macro to get old and new register values, allowing for writable bits
//
#define GET_MASKED_VALUES(_N) \
    Uns32 oldValue = CP_REG_UNS32(arm, _N); \
    newValue = getMaskedValue(arm, #_N, newValue, oldValue, CP_WRITE_MASK_##_N)

//
// For TLB invalidation routines, return the appropriate privilege to select
// instruction or data TLBs, or both (if unified operation)
//
static memPriv crmToPriv(Uns32 crm) {

    static const memPriv map[] = {
        [0] = MEM_PRIV_NONE,
        [1] = MEM_PRIV_X,
        [2] = MEM_PRIV_RW,
        [3] = MEM_PRIV_RWX
    };

    return map[crm&3];
}

//
// Restart the passed processor
//
static void restartProcessor(armP arm, armDisableReason reason) {

    if(arm->disable & reason) {

        arm->disable &= ~reason;

        if(!arm->disable) {
            vmirtRestartNext((vmiProcessorP)arm);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
// READ AND WRITE CALLBACKS
////////////////////////////////////////////////////////////////////////////////

//
// Type passed to read and write callbacks to identify coprocessor register
//
typedef union armCpRegDescU {
    Uns32 value;
    struct {
        Uns32 cpNum :  4;
        Uns32 crn   :  4;
        Uns32 crm   :  4;
        Uns32 op1   :  4;
        Uns32 op2   :  4;
        Uns32 _u1   : 12;
    } desc;
} armCpRegDesc, *armCpRegDescP;

//
// Coprocessor read callback type
//
#define ARM_CP_READFN(_NAME) Uns64 _NAME( \
    armP         arm,       \
    armCpRegDesc regDesc    \
)
typedef ARM_CP_READFN((*armCpReadFn));

//
// Coprocessor write callback type
//
#define ARM_CP_WRITEFN(_NAME) void _NAME( \
    armP         arm,       \
    armCpRegDesc regDesc,   \
    Uns64        newValue   \
)
typedef ARM_CP_WRITEFN((*armCpWriteFn));

//
// Macro to detect changed fields
//
#define CP_FIELD_CHANGED(_A, _R, _F) (CP_FIELD(_A, _R, _F) != oldValueS._F)

//
// Does the regDesc describe a data access?
//
inline static Bool isData(armCpRegDesc regDesc) {
    return !(regDesc.desc.op2&1);
}

//
// Return the currently-selected region number
//
inline static Uns32 regionNum(armP arm) {
    return CP_REG_UNS32(arm, RGNR);
}

//
// This type is used to dispatch coprocessor writes to all processors in a
// cluster
//
typedef struct writeCpInfoS {
    armCpWriteFn writeCB;   // write callback
    armCpRegDesc regDesc;   // register description
    Uns64        newValue;  // value being written
} writeCpInfo, *writeCpInfoP;

//
// Is the processor a leaf processor?
//
inline static Bool isLeaf(armP arm) {
    return !vmirtGetSMPChild((vmiProcessorP)arm);
}

//
// Return the parent of the passed processor
//
inline static armP getParent(armP arm) {
    return (armP)vmirtGetSMPParent((vmiProcessorP)arm);
}

//
// Get inner-shareable root for cache/TLB maintenance broadcast for the passed
// processor (container of all processors with the same inner-shareable
// attribute when broadcast is enabled)
//
static armP getInnerShareableBroadcastRoot(armP arm) {

    armP root = getParent(arm);

    if(!root) {

        //  no action

    } else if(!CP_FIELD(arm, ACTLR.postV7, SMP)) {

        // sender must have ACTLR.SMP set
        vmiMessage("W", CPU_PREFIX"_TLBMBD2",
            SRCREF_FMT "TLB maintenance broadcast disabled by ACTLR.SMP=0",
            SRCREF_ARGS(arm, getPC(arm))
        );

        root = NULL;

    } else if(!CP_FIELD(arm, ACTLR.postV7, FW)) {

        // sender must have ACTLR.FW set
        vmiMessage("W", CPU_PREFIX"_TLBMBD1",
            SRCREF_FMT "TLB maintenance broadcast disabled by ACTLR.FW=0",
            SRCREF_ARGS(arm, getPC(arm))
        );

        root = NULL;
    }

    return root;
}

//
// Callback to dispatch coprocessor write to one processor in a cluster
//
static VMI_SMP_ITER_FN(dispatchCpWriteCB) {

    armP         arm  = (armP)processor;
    writeCpInfoP info = userData;

    if(!isLeaf(arm)) {

        // no action

    } else if(!CP_FIELD(arm, ACTLR.postV7, SMP)) {

        // receiver must have ACTLR.SMP set
        vmiMessage("W", CPU_PREFIX"_TLBMBD3",
            NO_SRCREF_FMT "TLB maintenance broadcast ignored by ACTLR.SMP=0",
            NO_SRCREF_ARGS(arm)
        );

    } else {

        // wake up the processor if required
        restartProcessor(arm, AD_WFE|AD_WFI);

        // do the required broadcast action
        info->writeCB(arm, info->regDesc, info->newValue);
    }
}

//
// Dispatch coprocessor writes to all processors in an inner-shareable cluster
//
static void dispatchCpWriteInnerShareable(
    armP         arm,
    armCpWriteFn writeCB,
    armCpRegDesc regDesc,
    Uns64        newValue
) {
    armP        root = getInnerShareableBroadcastRoot(arm);
    writeCpInfo info = {writeCB, regDesc, newValue};

    if(!root) {
        writeCB(arm, regDesc, newValue);
    } else {
        vmirtIterAllDescendants((vmiProcessorP)root, dispatchCpWriteCB, &info);
    }
}

//
// Used to indicate coprocessor register accesses which have already been warned
// about
//
typedef struct armCPWarningS {
    armCPWarningP next;     // next in list
    armCpRegDesc  desc;     // register description
    Bool          isRead;   // is this a read access?
} armCPWarning;

//
// Common routine for management of invalid access warnings
//
static void ignoreCpReadWrite(armP arm, armCpRegDesc regDesc, Bool isRead) {

    armCPWarningP warn;

    // don't warn if this has already been warned about
    for(warn=arm->cpWarn; warn; warn=warn->next) {
        if((warn->desc.value=regDesc.value) && (isRead==warn->isRead)) {
            return;
        }
    }

    // allocate new warning object
    warn = STYPE_ALLOC(armCPWarning);
    warn->next   = arm->cpWarn;
    warn->desc   = regDesc;
    warn->isRead = isRead;
    arm->cpWarn  = warn;

    // emit warning
    vmiMessage("a", CPU_PREFIX"_CRA",
        SRCREF_FMT "%s p%u, %u, <Rd>, c%u, c%u, %u ignored",
        SRCREF_ARGS(arm, getPC(arm)),
        isRead ? "mrc" : "mcr",
        regDesc.desc.cpNum,
        regDesc.desc.op1,
        regDesc.desc.crn,
        regDesc.desc.crm,
        regDesc.desc.op2
    );
}

//
// Dummy function to ignore a coprocessor register read
//
static ARM_CP_READFN(ignoreCpRead) {
    ignoreCpReadWrite(arm, regDesc, True);
    return 0;
}

//
// Dummy function to ignore a coprocessor register write
//
static ARM_CP_WRITEFN(ignoreCpWrite) {
    ignoreCpReadWrite(arm, regDesc, False);
}

//
// Reset all coprocessor invalid access warnings
//
void armCpResetWarnings(armP arm) {

    armCPWarningP warn;

    while((warn=arm->cpWarn)) {
        arm->cpWarn = warn->next;
        STYPE_FREE(warn);
    }
}

//
// Report an illegal DMA register access
//
static void dmaAccessNotAllowed(armP arm, const char *regName) {

    Uns32 thisPC = getPC(arm);

    vmiMessage("W", CPU_PREFIX"_DMAD",
        SRCREF_FMT "User access to %s is disabled",
        SRCREF_ARGS(arm, thisPC),
        regName
    );

    armUndefined(arm, thisPC, False);
}

//
// Is user-mode DMA enabled for the current DMA channel?
//
static Bool allowCurrentChannelUserModeDMA(armP arm) {

    Uns32 mask = 1<<CP_REG_UNS32(arm, DMAChannel);

    return CP_REG_UNS32(arm, DMAUserAccessibility) & mask;
}

//
// Validate a DMA access to the current channel
//
static Bool dmaValidateChannel(armP arm, const char *regName) {

    if(!IN_USER_MODE(arm) || allowCurrentChannelUserModeDMA(arm)) {
        return True;
    } else {
        dmaAccessNotAllowed(arm, regName);
        return False;
    }
}

//
// Report an illegal TEEHBRA register access and return True if legal
//
static Bool hbrAccessAllowed(armP arm) {

    if(IN_USER_MODE(arm) && CP_FIELD(arm, TEECR, XED)) {

        Uns32 thisPC = getPC(arm);

        vmiMessage("W", CPU_PREFIX"_HBRD",
            SRCREF_FMT "User access to TEEHBR is disabled by TEECR.XED",
            SRCREF_ARGS(arm, thisPC)
        );

        armUndefined(arm, thisPC, False);

        return False;

    } else {

        return True;
    }
}

//
// Write TEEHBR register value
//
static ARM_CP_WRITEFN(writeCp14TEEHBR) {

    if(hbrAccessAllowed(arm)) {

        // get original register value and writable mask
        Uns32 oldValueU = CP_REG_UNS32(arm, TEEHBR);
        Uns32 mask      = CP_WRITE_MASK_TEEHBR;

        // set the new register value, allowing for writable bits
        CP_REG_UNS32(arm, TEEHBR) = getMaskedValue(
            arm, "TEEHBR", newValue, oldValueU, mask
        );
    }
}

//
// Read TEEHBR register value
//
static ARM_CP_READFN(readCp14TEEHBR) {
    return hbrAccessAllowed(arm) ? CP_REG_UNS32(arm, TEEHBR) : 0;
}

//
// Update processor state when Thumb interworking bit changes
//
static void writeCp15L4(armP arm) {

    // if this is the first time that interworking mode has been switched,
    // discard the current code dictionaries (they need to be regenerated with
    // interwork mode checking enabled in the block mask)
    if(!arm->checkL4) {
        vmirtFlushAllDicts((vmiProcessorP)arm);
        arm->checkL4 = True;
    }

    // update block mask
    armSetBlockMask(arm);
}

//
// Update processor state when unaligned accesses have been enabled/disabled
//
static void writeCp15Unaligned(armP arm) {

    // if this is the first time that unaligned accesses have been enabled or
    // disabled, discard the current code dictionaries (they need to be
    // regenerated with unaligned access checking enabled in the block mask)
    if(!arm->checkUnaligned) {
        vmirtFlushAllDicts((vmiProcessorP)arm);
        arm->checkUnaligned = True;
    }

    // update block mask
    armSetBlockMask(arm);
}

//
// Update processor state when Thumb interworking bit changes
//
static void writeCp15SW(armP arm) {

    // if this is the first time that SWP/SWPB instructions have been enabled or
    // disabled, discard the current code dictionaries (they need to be
    // regenerated with SWP/SWPB instruction enable checking enabled in the
    // block mask)
    if(!arm->checkEnableSWP) {
        vmirtFlushAllDicts((vmiProcessorP)arm);
        arm->checkEnableSWP = True;
    }

    // update block mask
    armSetBlockMask(arm);
}

//
// Write SCTLR register value
//
static ARM_CP_WRITEFN(writeCp15SCTLR) {

    Bool oldDoAligned = DO_UNALIGNED(arm);

    // get original register value
    CP_REG_STRUCT_DECL(SCTLR) oldValueS = CP_REG_STRUCT(arm, SCTLR);

    if(!newValue) {

        // special case: write with zero resets the register
        CP_REG_STRUCT(arm, SCTLR) = CP_REG_STRUCT_DEFAULT(arm, SCTLR);

    } else {

        // get original register value and writable mask
        Uns32 oldValueU = CP_REG_UNS32(arm, SCTLR);
        Uns32 mask      = CP_MASK_UNS32(arm, SCTLR);

        // set the new register value, allowing for writable bits
        CP_REG_UNS32(arm, SCTLR) = getMaskedValue(
            arm, "SCTLR", newValue, oldValueU, mask
        );
    }

    // special action is required when MMU is enabled or disabled
    // NOTE: TLB contents are preserved when MMU is disabled, so there is no
    // need to flush the TLB here. This also applies to legacy S and R bits.
    // See ARM documentation for more details.
    if(CP_FIELD_CHANGED(arm, SCTLR, M)) {

        // switch to MMU-enabled mode
        armSwitchMode(arm);

        // trigger the programmer's view MMU enable event
        vmirtTriggerViewEvent(arm->mmuEnableEvent);
    }

    // TODO: switching endianess is not supported for the legacy BE-32 endian
    // mode in ARMv4 and ARMv5
    if(CP_FIELD_CHANGED(arm, SCTLR, B)) {
        vmiMessage("P", CPU_PREFIX"_EUU",
            SRCREF_FMT "legacy BE-32 endian switch not supported",
            SRCREF_ARGS(arm, getPC(arm))
        );
    }

    // do actions required when legacy Thumb interworking bit changes state
    if(CP_FIELD_CHANGED(arm, SCTLR, L4)) {
        writeCp15L4(arm);
    }

    // do actions required when unaligned access mode changes state
    if(oldDoAligned != DO_UNALIGNED(arm)) {
        writeCp15Unaligned(arm);
    }

    // do actions required when SWP/SWPB enable bit changes state
    if(CP_FIELD_CHANGED(arm, SCTLR, SW)) {
        writeCp15SW(arm);
    }
}

//
// Write CPACR register value
//
static ARM_CP_WRITEFN(writeCp15CPACR) {

    // get original register value and writable mask
    Uns32 oldValueU = CP_REG_UNS32(arm, CPACR);
    Uns32 mask      = CP_MASK_UNS32(arm, CPACR);

    // set the new register value, allowing for writable bits
    CP_REG_UNS32(arm, CPACR) = getMaskedValue(
        arm, "CPACR", newValue, oldValueU, mask
    );

    // update blockMask to reflect enabled or disabled coprocessors
    armSetBlockMask(arm);
}

//
// ARMv6: halt, waiting for interrupts
// ARMv7: NOP
//
static ARM_CP_WRITEFN(writeCp15WFI) {

    if(getInstructionVersion(arm)<7) {
        arm->disable = AD_WFI;
        vmirtHalt((vmiProcessorP)arm);
    }
}

//
// Test and clean operation (see ARM926 Technical Reference Manual)
//
static ARM_CP_READFN(readCp15TestCleanDCache) {

    armCPSR CPSR = {{reg:armReadCPSR(arm)}};

    CPSR.fields.Z = 1;

    return CPSR.reg;
}

//
// Handle DCache clean operations (only allowed if ACTLR.RA is clear)
//
static ARM_CP_WRITEFN(writeCp15CleanDCache) {

    if(CP_FIELD(arm, ACTLR.preV7, RA)) {

        Uns32 thisPC = getPC(arm);

        vmiMessage("W", CPU_PREFIX"_CDCD",
            SRCREF_FMT "Clean entire data cache disabled by ACTLR.RA",
            SRCREF_ARGS(arm, thisPC)
        );

        armUndefined(arm, thisPC, False);
    }
}

//
// Handle cache range operations (only allowed if ACTLR.RV is clear)
//
static ARM_CP_WRITEFN(writeCp15RangeOpCache) {

    if(CP_FIELD(arm, ACTLR.preV7, RV)) {

        Uns32 thisPC = getPC(arm);

        vmiMessage("W", CPU_PREFIX"_CDCD",
            SRCREF_FMT "Clean entire data cache disabled by ACTLR.RV",
            SRCREF_ARGS(arm, thisPC)
        );

        armUndefined(arm, thisPC, False);
    }
}

//
// Perform VA to PA mapping
//
static ARM_CP_WRITEFN(writeCp15V2P) {

    memPriv priv   = (regDesc.desc.op2 & 1) ? MEM_PRIV_W : MEM_PRIV_R;
    Bool    isUser = (regDesc.desc.op2 & 2) && True;

    armVMTranslateVAtoPA(arm, newValue, isUser, priv);
}

//
// Invalidate entire TLB
//
static ARM_CP_WRITEFN(writeCp15TLBIALL) {

    armVMInvalidate(arm, crmToPriv(regDesc.desc.crm));
}

//
// Invalidate entire TLB on all CPUs in inner-shareable cluster
//
static ARM_CP_WRITEFN(writeCp15TLBIALLIS) {

    dispatchCpWriteInnerShareable(arm, writeCp15TLBIALL, regDesc, newValue);
}

//
// Invalidate TLB entry by matching MVA (and ASID, if ARMv6 or later)
//
static ARM_CP_WRITEFN(writeCp15TLBIMVA) {

    Bool  withASID = (getInstructionVersion(arm)>=6);
    Uns32 MVA      = withASID ? newValue & CP_TLBInvalidateTLB_MVA  : newValue;
    Uns32 ASID     = withASID ? newValue & CP_TLBInvalidateTLB_ASID : 0;

    armVMInvalidateEntryMVA(arm, MVA, ASID, crmToPriv(regDesc.desc.crm));
}

//
// Invalidate TLB entry by matching MVA (and ASID, if ARMv6 or later) on all
// CPUs in inner-shareable cluster
//
static ARM_CP_WRITEFN(writeCp15TLBIMVAIS) {

    dispatchCpWriteInnerShareable(arm, writeCp15TLBIMVA, regDesc, newValue);
}

//
// Invalidate TLB entry by matching ASID
//
static ARM_CP_WRITEFN(writeCp15TLBIASID) {

    Uns32 ASID = newValue & CP_TLBInvalidateTLB_ASID;

    armVMInvalidateEntryASID(arm, ASID, crmToPriv(regDesc.desc.crm));
}

//
// Invalidate TLB entry by matching ASID on all CPUs in inner-shareable cluster
//
static ARM_CP_WRITEFN(writeCp15TLBIASIDIS) {

    dispatchCpWriteInnerShareable(arm, writeCp15TLBIASID, regDesc, newValue);
}

//
// Invalidate TLB entry by matching MVA, ignoring ASID
//
static ARM_CP_WRITEFN(writeCp15TLBIMVAA) {

    Uns32 MVA = newValue & CP_TLBInvalidateTLB_MVA;

    armVMInvalidateEntryMVAA(arm, MVA, crmToPriv(regDesc.desc.crm));
}

//
// Invalidate TLB entry by matching MVA, ignoring ASID, on all CPUs in
// inner-shareable cluster
//
static ARM_CP_WRITEFN(writeCp15TLBIMVAAIS) {

    dispatchCpWriteInnerShareable(arm, writeCp15TLBIMVAA, regDesc, newValue);
}

//
// Write data/unified lockdown register - masking depends on number of lockdown
// entries available
//
static ARM_CP_WRITEFN(writeCp15TLBLR) {

    armVMWriteLockdown(arm, isData(regDesc), newValue);
}

//
// Write data/unified lockdown register - masking depends on number of lockdown
// entries available
//
static ARM_CP_READFN(readCp15TLBLR) {

    return armVMReadLockdown(arm, isData(regDesc));
}

//
// Write TLB lockdown VA register
//
static ARM_CP_WRITEFN(writeCp15TLBLDVA) {

    armVMWriteLockdownVA(arm, isData(regDesc), newValue);
}

//
// Read TLB lockdown VA register
//
static ARM_CP_READFN(readCp15TLBLDVA) {

    return armVMReadLockdownVA(arm, isData(regDesc));
}

//
// Write TLB lockdown PA register
//
static ARM_CP_WRITEFN(writeCp15TLBLDPA) {

    armVMWriteLockdownPA(arm, isData(regDesc), newValue);
}

//
// Read TLB lockdown PA register
//
static ARM_CP_READFN(readCp15TLBLDPA) {

    return armVMReadLockdownPA(arm, isData(regDesc));
}

//
// Write TLB lockdown attributes register
//
static ARM_CP_WRITEFN(writeCp15TLBLDATTR) {

    armVMWriteLockdownAttr(arm, isData(regDesc), newValue);
}

//
// Read TLB lockdown attributes register
//
static ARM_CP_READFN(readCp15TLBLDATTR) {

    return armVMReadLockdownAttr(arm, isData(regDesc));
}

//
// Data TCM Region write
//
static ARM_CP_WRITEFN(writeCp15DTCMRR) {

    // get old and new values, allowing for writable bits
    GET_MASKED_VALUES(DTCMRR);

    if(oldValue!=newValue) {
        armVMUpdateDTCMRegion(arm, newValue);
    }
}

//
// Instruction TCM Region write
//
static ARM_CP_WRITEFN(writeCp15ITCMRR) {

    // get old and new values, allowing for writable bits
    GET_MASKED_VALUES(ITCMRR);

    if(oldValue!=newValue) {
        armVMUpdateITCMRegion(arm, newValue);
    }
}

//
// Write PRRR register value
//
static ARM_CP_WRITEFN(writeCp15PRRR) {

    // get old and new values, allowing for writable bits
    GET_MASKED_VALUES(PRRR);

    // update raw value
    CP_REG_UNS32(arm, PRRR) = newValue;

    // clear down PRRR.NOS unless outer shareability is implemented
    if(
        CP_USE_CPUID(arm) &&
        (!ARM_MMFR(0, ShareabilityLevels) || (ARM_MMFR(0, OuterShareability)!=1))
    ) {
        CP_FIELD(arm, PRRR, NOS) = 0;
    }
}

//
// Write DACR register value
//
static ARM_CP_WRITEFN(writeCp15DACR) {

    // set the new register value
    CP_REG_UNS32(arm, DACR) = newValue;

    // notify VM module
    armVMSetDomainAccessControl(arm);
}

//
// Write AccessPermissions register value (ARMv4/ARMv5 only)
//
static ARM_CP_WRITEFN(writeCp15APR) {
    armVMWriteAccessARMv5(arm, isData(regDesc), newValue);
}

//
// Read AccessPermissions register value (ARMv4/ARMv5 only)
//
static ARM_CP_READFN(readCp15APR) {
    return armVMReadAccessARMv5(arm, isData(regDesc));
}

//
// Write extended AccessPermissions register value (ARMv4/ARMv5 only)
//
static ARM_CP_WRITEFN(writeCp15EAPR) {
    armVMWriteAccessExtARMv5(arm, isData(regDesc), newValue);
}

//
// Read extended AccessPermissions register value (ARMv4/ARMv5 only)
//
static ARM_CP_READFN(readCp15EAPR) {
    return armVMReadAccessExtARMv5(arm, isData(regDesc));
}

//
// Write ProtectionAreaControl register value (ARMv4/ARMv5 only)
//
static ARM_CP_WRITEFN(writeCp15MRR) {
    armVMWriteRegionARMv5(arm, regDesc.desc.crm, isData(regDesc), newValue);
}

//
// Read ProtectionAreaControl register value (ARMv4/ARMv5 only)
//
static ARM_CP_READFN(readCp15MRR) {
    return armVMReadRegionARMv5(arm, regDesc.desc.crm, isData(regDesc));
}

//
// Write RegionBase register value (ARMv6 and later)
//
static ARM_CP_WRITEFN(writeCp15RBAR) {
    armVMWriteRegionBase(arm, regionNum(arm), isData(regDesc), newValue);
}

//
// Read RegionBase register value (ARMv6 and later)
//
static ARM_CP_READFN(readCp15RBAR) {
    return armVMReadRegionBase(arm, regionNum(arm), isData(regDesc));
}

//
// Write RegionSizeE register value (ARMv6 and later)
//
static ARM_CP_WRITEFN(writeCp15RSR) {
    armVMWriteRegionSizeE(arm, regionNum(arm), isData(regDesc), newValue);
}

//
// Read RegionSizeE register value (ARMv6 and later)
//
static ARM_CP_READFN(readCp15RSR) {
    return armVMReadRegionSizeE(arm, regionNum(arm), isData(regDesc));
}

//
// Write RegionAccess register value (ARMv6 and later)
//
static ARM_CP_WRITEFN(writeCp15RACR) {
    armVMWriteRegionAccess(arm, regionNum(arm), isData(regDesc), newValue);
}

//
// Read RegionAccess register value (ARMv6 and later)
//
static ARM_CP_READFN(readCp15RACR) {
    return armVMReadRegionAccess(arm, regionNum(arm), isData(regDesc));
}

//
// Write FCSEIDR register value
//
static ARM_CP_WRITEFN(writeCp15FCSEIDR) {

    // this register is present even if FCSE is not implemented, but is
    // RAZ/WI in this case
    if(FCSE_PRESENT(arm)) {

        // get old and new values, allowing for writable bits
        GET_MASKED_VALUES(FCSEIDR);

        // memory mappings must be updated if PID has changed
        if(oldValue!=newValue) {
        	CP_REG_UNS32(arm, FCSEIDR) = newValue;
        	armVMSetPIDOrModeFCSE(arm);
        }
    }
}

//
// Write CSSELR register value - update only allowed if the selected cache type
// is present
//
static ARM_CP_WRITEFN(writeCp15CSSELR) {

    // get old level for restoration if update fails
    Uns32 oldLevel = CP_FIELD(arm, CSSELR, Level);

    // get old and new values, allowing for writable bits
    GET_MASKED_VALUES(CSSELR);

    // assume update will be successful
    CP_REG_UNS32(arm, CSSELR) = newValue;

    // get new cache select and level
    Bool  InD      = CP_FIELD(arm, CSSELR, InD);
    Uns32 newLevel = CP_FIELD(arm, CSSELR, Level);
    Bool  ok;

    if(newLevel>=ARM_NUM_CCSIDR) {

        // invalid Level index
        ok = False;

    } else switch((CP_REG_UNS32(arm, CLIDR) >> (newLevel*3)) & 0x7) {

        // valid Level index
        case 1:  ok = !InD;  break;
        case 2:  ok =  InD;  break;
        case 3:  ok = True;  break;
        case 4:  ok = True;  break;
        default: ok = False; break;
    }

    // restore original value if required
    if(!ok) {
        CP_FIELD(arm, CSSELR, Level) = oldLevel;
    }
}

//
// Read CCSIDR register value (ARMv7 and later)
//
static ARM_CP_READFN(readCp15CCSIDR) {

    Bool  InD   = CP_FIELD(arm, CSSELR, InD);
    Uns32 Level = CP_FIELD(arm, CSSELR, Level);

    union {CP_REG_DECL(CCSIDR); Uns32 u32;} u = {
        CP_REG_STRUCT_DEFAULT(arm, CCSIDR)[InD][Level]
    };

    return u.u32;
}

//
// Write DMAChannel register value
//
static ARM_CP_WRITEFN(writeCp15DMAChannel) {

    if(IN_USER_MODE(arm) && !CP_REG_UNS32(arm, DMAUserAccessibility)) {
        dmaAccessNotAllowed(arm, "DMAChannel");
    } else {
        CP_REG_UNS32(arm, DMAChannel) = newValue & CP_WRITE_MASK_DMAChannel;
    }
}

//
// Read DMAChannel register value
//
static ARM_CP_READFN(readCp15DMAChannel) {
    if(IN_USER_MODE(arm) && !CP_REG_UNS32(arm, DMAUserAccessibility)) {
        dmaAccessNotAllowed(arm, "DMAChannel");
        return 0;
    } else {
        return CP_REG_UNS32(arm, DMAChannel);
    }
}

//
// Write DMAStop register value
//
static ARM_CP_WRITEFN(writeCp15DMAStop) {
    if(dmaValidateChannel(arm, "DMAStop")) {
        armVMWriteDMAStop(arm);
    }
}

//
// Write DMAStart register value
//
static ARM_CP_WRITEFN(writeCp15DMAStart) {
    if(dmaValidateChannel(arm, "DMAStart")) {
        armVMWriteDMAStart(arm);
    }
}

//
// Write DMAClear register value
//
static ARM_CP_WRITEFN(writeCp15DMAClear) {
    if(dmaValidateChannel(arm, "DMAClear")) {
        armVMWriteDMAClear(arm);
    }
}

//
// Write DMAControl register value
//
static ARM_CP_WRITEFN(writeCp15DMAControl) {
    if(dmaValidateChannel(arm, "DMAControl")) {
        armVMWriteDMAControl(arm, newValue);
    }
}

//
// Read DMAControl register value
//
static ARM_CP_READFN(readCp15DMAControl) {
    if(dmaValidateChannel(arm, "DMAControl")) {
        return armVMReadDMAControl(arm);
    } else {
        return 0;
    }
}

//
// Write DMAInternalStart register value
//
static ARM_CP_WRITEFN(writeCp15DMAInternalStart) {
    if(dmaValidateChannel(arm, "DMAInternalStart")) {
        armVMWriteDMAInternalStart(arm, newValue);
    }
}

//
// Read DMAInternalStart register value
//
static ARM_CP_READFN(readCp15DMAInternalStart) {
    if(dmaValidateChannel(arm, "DMAInternalStart")) {
        return armVMReadDMAInternalStart(arm);
    } else {
        return 0;
    }
}

//
// Write DMAExternalStart register value
//
static ARM_CP_WRITEFN(writeCp15DMAExternalStart) {
    if(dmaValidateChannel(arm, "DMAExternalStart")) {
        armVMWriteDMAExternalStart(arm, newValue);
    }
}

//
// Read DMAExternalStart register value
//
static ARM_CP_READFN(readCp15DMAExternalStart) {
    if(dmaValidateChannel(arm, "DMAExternalStart")) {
        return armVMReadDMAExternalStart(arm);
    } else {
        return 0;
    }
}

//
// Write DMAInternalEnd register value
//
static ARM_CP_WRITEFN(writeCp15DMAInternalEnd) {
    if(dmaValidateChannel(arm, "DMAInternalEnd")) {
        armVMWriteDMAInternalEnd(arm, newValue);
    }
}

//
// Read DMAInternalEnd register value
//
static ARM_CP_READFN(readCp15DMAInternalEnd) {
    if(dmaValidateChannel(arm, "DMAInternalEnd")) {
        return armVMReadDMAInternalEnd(arm);
    } else {
        return 0;
    }
}

//
// Read DMAStatus register value
//
static ARM_CP_READFN(readCp15DMAStatus) {
    if(dmaValidateChannel(arm, "DMAStatus")) {
        return armVMReadDMAStatus(arm);
    } else {
        return 0;
    }
}

//
// Write DMAContextID register value
//
static ARM_CP_WRITEFN(writeCp15DMAContextID) {
    if(dmaValidateChannel(arm, "DMAContextID")) {
        armVMWriteDMAContextID(arm, newValue);
    }
}

//
// Read DMAContextID register value
//
static ARM_CP_READFN(readCp15DMAContextID) {
    if(dmaValidateChannel(arm, "DMAContextID")) {
        return armVMReadDMAContextID(arm);
    } else {
        return 0;
    }
}

//
// Write CONTEXTIDR register value
//
static ARM_CP_WRITEFN(writeCp15CONTEXTIDR) {

    // get old and new values, allowing for writable bits
    GET_MASKED_VALUES(CONTEXTIDR);

    // memory mappings must be updated if ASID has changed
    if(oldValue!=newValue) {
        CP_REG_UNS32(arm, CONTEXTIDR) = newValue;
        armVMSetASID(arm);
    }
}


////////////////////////////////////////////////////////////////////////////////
// COPROCESSOR REGISTER ACCESS
////////////////////////////////////////////////////////////////////////////////

//
// Type used to return information enabling a coprocessor register to be read
//
typedef struct armCPReadInfoS {
    vmiReg       rs;        // if the register can be read by a simple access
    armCpReadFn  cb;        // if the read requires a callback
    armCpRegDesc regDesc;   // regDesc for read callback
} armCPReadInfo, *armCPReadInfoP;

//
// Type used to return information enabling a coprocessor register to be written
//
typedef struct armCPWriteInfoS {
    vmiReg       rd;        // if the register can be written by a simple access
    armCpWriteFn cb;        // if the read requires a callback
    armCpRegDesc regDesc;   // regDesc for write callback
    Uns64        writeMask; // mask of writable bits in this register
    Bool         endBlock;  // whether write requires code block end
} armCPWriteInfo, *armCPWriteInfoP;

//
// Type enumerating the units for which coprocessor registers are present
//
typedef enum armUnitE {
    AU_ALL,         // always present
    AU_MP,          // present for cores with MP extensions only
    AU_MMU,         // present for cores with TLB only
    AU_MMUMP,       // present for cores with TLB and MP extensions only
    AU_MPU,         // present for cores with MPU only
    AU_MPUX,        // present for cores with MPU & extended AP only
    AU_MPUS,        // present for cores with non-unified MPUs only
    AU_TLD,         // present for cores with TLB & lockdown only
    AU_TLDS,        // present for cores with non-unified TLBs & lockdown only
    AU_ALIGN,       // present for cores with alignment unit only
    AU_DTCM,        // present for cores with data TCM unit only
    AU_ITCM,        // present for cores with instruction TCM unit only
    AU_IFAR,        // present for cores with IFAR register only
    AU_PCR,         // present for cores with PCR register only
    AU_NEONB,       // present for cores with NEONB register only
    AU_CBAR,        // present for cores with CBAR register only
    AU_TLBLD,       // present for cores with Cp15 TLB lockdown registers only
    AU_TLBHR,       // present for cores with Cp15 TLB hitmap register only
    AU_DMA,         // present for cores with DMA unit only
    AU_TEE,         // present for cores with ThumbEE only
    AU_JAZ,         // present for cores with Jazelle only
} armUnit;

//
// Type enumerting passes
//
typedef enum regPriorityE {
    RP_HI,          // match in high priority pass
    RP_LO           // match in low priority pass
} regPriority;

//
// Type enumerating classing of access to register
//
typedef enum accessActionE {

    AA_READ  = 0x0,
    AA_WRITE = 0x1,
    AA_PRIV  = 0x0,
    AA_USER  = 0x2,

    AA_PRIV_READ  = AA_READ  | AA_PRIV, // privileged mode read
    AA_PRIV_WRITE = AA_WRITE | AA_PRIV, // privileged mode write
    AA_USER_READ  = AA_READ  | AA_USER, // user mode read
    AA_USER_WRITE = AA_WRITE | AA_USER, // user mode write

    AA_LAST

} accessAction;

//
// Descr strings for accessAction values
//
static const char *accessActionRW[2][2] = {
        {"--", "-w"},
        {"r-", "rw"},
};

//
// This structure records information about each coprocessor register
//
typedef struct cpRegAttrsS {
    const char     *name;
    Uns8            size;
    Uns8            cpNum;
    Int8            op1;
    Int8            op2;
    Int8            crn;
    Int8            crm;
    Bool            access[AA_LAST];
    armUnit         unit;
    armArchitecture variant;
    Uns32           hiVersion;
    regPriority     priority;
    Bool            wEndBlock;
    Bool            hide;
    armCpReadFn     readCB;
    armCpWriteFn    writeCB;
    Uns64           writeMask;
} cpRegAttrs;

#define _ -1

//
// Macro for defining coprocessor registers accessed using MCR/MRC
//
#define CP_ATTR1(_CPNUM, _ID, _OP1, _OP2, _CRN, _CRM, _PR,_PW,_UR,_UW, _UNIT, _VARIANT, _HIVERSION, _PRI, _WENDBLOCK, _HIDE, _READCB, _WRITECB) \
    [CP_ID(_ID)] = {#_ID, 1, _CPNUM, _OP1, _OP2, _CRN, _CRM, {_PR,_PW,_UR,_UW}, _UNIT, _VARIANT, _HIVERSION, _PRI, _WENDBLOCK, _HIDE, _READCB, _WRITECB, CP_WRITE_MASK_##_ID}

//
// Macro for defining coprocessor registers accessed using MCRR/MRRC
//
#define CP_ATTR2(_CPNUM, _ID, _OP1, _OP2, _CRN, _CRM, _PR,_PW,_UR,_UW, _UNIT, _VARIANT, _HIVERSION, _PRI, _WENDBLOCK, _HIDE, _READCB, _WRITECB) \
    [CP_ID(_ID)] = {#_ID, 2, _CPNUM, _OP1, _OP2, _CRN, _CRM, {_PR,_PW,_UR,_UW}, _UNIT, _VARIANT, _HIVERSION, _PRI, _WENDBLOCK, _HIDE, _READCB, _WRITECB, CP_WRITE_MASK_##_ID}

//
// Table of coprocessor register attributes accessed using MCR/MRC/MCRR/MRRC
//
const static cpRegAttrs cpRegInfo[CP_ID(Size)] = {

    // true registers, Jazelle/ThumbEE    op1 op2 crn crm  access   unit      variant hiVersion pri     endB hide readCB                   writeCB
    CP_ATTR1(14, TEECR,                    6,  0,  0,  0,  1,1,1,0, AU_TEE,   7,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(14, TEEHBR,                   6,  0,  1,  0,  1,1,1,1, AU_TEE,   7,      0,        RP_HI,  0,   0,   readCp14TEEHBR,          writeCp14TEEHBR          ),
    CP_ATTR1(14, JIDR,                     7,  0,  0,  0,  1,1,1,1, AU_JAZ,   0,      0,        RP_HI,  0,   0,   0,                       ignoreCpWrite            ),
    CP_ATTR1(14, JOSCR,                    7,  0,  1,  0,  1,1,0,0, AU_JAZ,   0,      0,        RP_HI,  0,   0,   0,                       ignoreCpWrite            ),
    CP_ATTR1(14, JMCR,                     7,  0,  2,  0,  1,1,0,0, AU_JAZ,   0,      0,        RP_HI,  0,   0,   0,                       ignoreCpWrite            ),

    // true registers, pre ARMv6          op1 op2 crn crm  access   unit      variant hiVersion pri     endB hide readCB                   writeCB
    CP_ATTR1(15, TTBR,                     0,  0,  2,  0,  1,1,0,0, AU_MMU,   0,      6,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, DCR,                      0,  0,  2,  0,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, ICR,                      0,  1,  2,  0,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, DBR,                      0,  0,  3,  0,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, FSR,                      0,  0,  5,  0,  1,1,0,0, AU_ALIGN, 0,      6,        RP_LO,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, FAR,                      0,  0,  6,  0,  1,1,0,0, AU_ALIGN, 0,      6,        RP_LO,  0,   0,   0,                       0                        ),

    // true registers                     op1 op2 crn crm  access   unit      variant hiVersion pri     endB hide readCB                   writeCB
    CP_ATTR1(15, MIDR,                     0,  _,  0,  0,  1,0,0,0, AU_ALL,   0,      0,        RP_LO,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, CTR,                      0,  1,  0,  0,  1,0,0,0, AU_ALL,   0,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, TCMTR,                    0,  2,  0,  0,  1,0,0,0, AU_ALL,   0,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, TLBTR,                    0,  3,  0,  0,  1,0,0,0, AU_MMU,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, MPUIR,                    0,  4,  0,  0,  1,0,0,0, AU_MPU,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, MPIDR,                    0,  5,  0,  0,  1,0,0,0, AU_ALL,   7,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, ID_PFR0,                  0,  0,  0,  1,  1,0,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, ID_PFR1,                  0,  1,  0,  1,  1,0,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, ID_DFR0,                  0,  2,  0,  1,  1,0,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, ID_AFR0,                  0,  3,  0,  1,  1,0,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, ID_MMFR0,                 0,  4,  0,  1,  1,0,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, ID_MMFR1,                 0,  5,  0,  1,  1,0,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, ID_MMFR2,                 0,  6,  0,  1,  1,0,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, ID_MMFR3,                 0,  7,  0,  1,  1,0,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, ID_ISAR0,                 0,  0,  0,  2,  1,0,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, ID_ISAR1,                 0,  1,  0,  2,  1,0,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, ID_ISAR2,                 0,  2,  0,  2,  1,0,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, ID_ISAR3,                 0,  3,  0,  2,  1,0,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, ID_ISAR4,                 0,  4,  0,  2,  1,0,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, ID_ISAR5,                 0,  5,  0,  2,  1,0,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, ID_RESERVED,              0,  _,  0, -8,  1,0,0,0, AU_ALL,   6,      0,        RP_LO,  0,   1,   0,                       0                        ),
    CP_ATTR1(15, CLIDR,                    1,  1,  0,  0,  1,0,0,0, AU_ALL,   7,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, AIDR,                     1,  7,  0,  0,  1,0,0,0, AU_ALL,   7,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, CSSELR,                   2,  0,  0,  0,  1,1,0,0, AU_ALL,   7,      0,        RP_HI,  0,   0,   0,                       writeCp15CSSELR          ),
    CP_ATTR1(15, SCTLR,                    0,  0,  1,  0,  1,1,0,0, AU_ALL,   0,      0,        RP_HI,  1,   0,   0,                       writeCp15SCTLR           ),
    CP_ATTR1(15, ACTLR,                    0,  1,  1,  0,  1,1,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, CPACR,                    0,  2,  1,  0,  1,1,0,0, AU_ALL,   6,      0,        RP_HI,  1,   0,   0,                       writeCp15CPACR           ),
    CP_ATTR1(15, TTBR0,                    0,  0,  2,  0,  1,1,0,0, AU_MMU,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, TTBR1,                    0,  1,  2,  0,  1,1,0,0, AU_MMU,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, TTBCR,                    0,  2,  2,  0,  1,1,0,0, AU_MMU,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, DACR,                     0,  0,  3,  0,  1,1,0,0, AU_MMU,   0,      0,        RP_HI,  0,   0,   0,                       writeCp15DACR            ),
    CP_ATTR1(15, DFSR,                     0,  0,  5,  0,  1,1,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, IFSR,                     0,  1,  5,  0,  1,1,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, ADFSR,                    0,  0,  5,  1,  1,1,0,0, AU_ALL,   7,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, AIFSR,                    0,  1,  5,  1,  1,1,0,0, AU_ALL,   7,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, DFAR,                     0,  0,  6,  0,  1,1,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, WFAR,                     0,  1,  6,  0,  1,1,0,0, AU_ALL,   6,      7,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, IFAR,                     0,  2,  6,  0,  1,1,0,0, AU_IFAR,  6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, RGNR,                     0,  0,  6,  2,  1,1,0,0, AU_MPU,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, PAR,                      0,  0,  7,  4,  1,1,0,0, AU_ALL,   7,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, DCLR,                     0,  0,  9,  0,  1,1,0,0, AU_ALL,   0,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, ICLR,                     0,  1,  9,  0,  1,1,0,0, AU_ALL,   0,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, DTCMRR,                   0,  0,  9,  1,  1,1,0,0, AU_DTCM,  0,      0,        RP_HI,  0,   0,   0,                       writeCp15DTCMRR          ),
    CP_ATTR1(15, ITCMRR,                   0,  1,  9,  1,  1,1,0,0, AU_ITCM,  0,      0,        RP_HI,  0,   0,   0,                       writeCp15ITCMRR          ),
    CP_ATTR1(15, PRRR,                     0,  0, 10,  2,  1,1,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       writeCp15PRRR            ),
    CP_ATTR1(15, NMRR,                     0,  1, 10,  2,  1,1,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, DMAPresent,               0,  0, 11,  0,  1,0,0,0, AU_DMA,   0,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, DMAQueued,                0,  1, 11,  0,  1,0,0,0, AU_DMA,   0,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, DMARunning,               0,  2, 11,  0,  1,0,0,0, AU_DMA,   0,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, DMAInterrupting,          0,  3, 11,  0,  1,0,0,0, AU_DMA,   0,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, DMAUserAccessibility,     0,  0, 11,  1,  1,1,0,0, AU_DMA,   0,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, DMAChannel,               0,  0, 11,  2,  1,1,1,1, AU_DMA,   0,      0,        RP_HI,  1,   0,   readCp15DMAChannel,      writeCp15DMAChannel      ),
    CP_ATTR1(15, FCSEIDR,                  0,  0, 13,  0,  1,1,0,0, AU_ALL,   0,      0,        RP_HI,  0,   0,   0,                       writeCp15FCSEIDR         ),
    CP_ATTR1(15, CONTEXTIDR,               0,  1, 13,  0,  1,1,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       writeCp15CONTEXTIDR      ),
    CP_ATTR1(15, TPIDRURW,                 0,  2, 13,  0,  1,1,1,1, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, TPIDRURO,                 0,  3, 13,  0,  1,1,1,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, TPIDRPRW,                 0,  4, 13,  0,  1,1,0,0, AU_ALL,   6,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, PCR,                      0,  0, 15,  0,  1,1,1,1, AU_PCR,   0,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, CBAR,                     4,  0, 15,  0,  1,1,1,0, AU_CBAR,  0,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, TLBHR,                    5,  0, 15,  5,  1,1,0,0, AU_TLBHR, 0,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, TLBRI,                    5,  2, 15,  4,  0,1,0,0, AU_TLBLD, 0,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, TLBWI,                    5,  4, 15,  4,  0,1,0,0, AU_TLBLD, 0,      0,        RP_HI,  0,   0,   0,                       0                        ),

    // TLB operations                     op1 op2 crn crm  access   unit      variant hiVersion pri     endB hide readCB                   writeCB
    CP_ATTR1(15, V2PCWPR,                  0,  0,  7,  8,  0,1,0,0, AU_MMU,   7,      0,        RP_HI,  1,   0,   0,                       writeCp15V2P             ),
    CP_ATTR1(15, V2PCWPW,                  0,  1,  7,  8,  0,1,0,0, AU_MMU,   7,      0,        RP_HI,  1,   0,   0,                       writeCp15V2P             ),
    CP_ATTR1(15, V2PCWUR,                  0,  2,  7,  8,  0,1,0,0, AU_MMU,   7,      0,        RP_HI,  1,   0,   0,                       writeCp15V2P             ),
    CP_ATTR1(15, V2PCWUW,                  0,  3,  7,  8,  0,1,0,0, AU_MMU,   7,      0,        RP_HI,  1,   0,   0,                       writeCp15V2P             ),
    CP_ATTR1(15, TLBIALLIS,                0,  0,  8,  3,  0,1,0,0, AU_MMUMP, 7,      0,        RP_HI,  0,   0,   0,                       writeCp15TLBIALLIS       ),
    CP_ATTR1(15, ITLBIALL,                 0,  0,  8,  5,  0,1,0,0, AU_MMU,   0,      0,        RP_HI,  0,   0,   0,                       writeCp15TLBIALL         ),
    CP_ATTR1(15, DTLBIALL,                 0,  0,  8,  6,  0,1,0,0, AU_MMU,   0,      0,        RP_HI,  0,   0,   0,                       writeCp15TLBIALL         ),
    CP_ATTR1(15, TLBIALL,                  0,  0,  8,  7,  0,1,0,0, AU_MMU,   0,      0,        RP_HI,  0,   0,   0,                       writeCp15TLBIALL         ),
    CP_ATTR1(15, TLBIMVAIS,                0,  1,  8,  3,  0,1,0,0, AU_MMUMP, 7,      0,        RP_HI,  0,   0,   0,                       writeCp15TLBIMVAIS       ),
    CP_ATTR1(15, ITLBIMVA,                 0,  1,  8,  5,  0,1,0,0, AU_MMU,   0,      0,        RP_HI,  0,   0,   0,                       writeCp15TLBIMVA         ),
    CP_ATTR1(15, DTLBIMVA,                 0,  1,  8,  6,  0,1,0,0, AU_MMU,   0,      0,        RP_HI,  0,   0,   0,                       writeCp15TLBIMVA         ),
    CP_ATTR1(15, TLBIMVA,                  0,  1,  8,  7,  0,1,0,0, AU_MMU,   0,      0,        RP_HI,  0,   0,   0,                       writeCp15TLBIMVA         ),
    CP_ATTR1(15, TLBIASIDIS,               0,  2,  8,  3,  0,1,0,0, AU_MMUMP, 7,      0,        RP_HI,  0,   0,   0,                       writeCp15TLBIASIDIS      ),
    CP_ATTR1(15, ITLBIASID,                0,  2,  8,  5,  0,1,0,0, AU_MMU,   6,      0,        RP_HI,  0,   0,   0,                       writeCp15TLBIASID        ),
    CP_ATTR1(15, DTLBIASID,                0,  2,  8,  6,  0,1,0,0, AU_MMU,   6,      0,        RP_HI,  0,   0,   0,                       writeCp15TLBIASID        ),
    CP_ATTR1(15, TLBIASID,                 0,  2,  8,  7,  0,1,0,0, AU_MMU,   6,      0,        RP_HI,  0,   0,   0,                       writeCp15TLBIASID        ),
    CP_ATTR1(15, TLBIMVAAIS,               0,  3,  8,  3,  0,1,0,0, AU_MMUMP, 7,      0,        RP_HI,  0,   0,   0,                       writeCp15TLBIMVAAIS      ),
    CP_ATTR1(15, TLBIMVAA,                 0,  3,  8,  7,  0,1,0,0, AU_MMUMP, 7,      0,        RP_HI,  0,   0,   0,                       writeCp15TLBIMVAA        ),
    CP_ATTR1(15, DTLBLR,                   0,  0, 10,  0,  1,1,0,0, AU_TLD,   6,      0,        RP_HI,  0,   0,   readCp15TLBLR,           writeCp15TLBLR           ),
    CP_ATTR1(15, ITLBLR,                   0,  1, 10,  0,  1,1,0,0, AU_TLDS,  6,      0,        RP_HI,  0,   0,   readCp15TLBLR,           writeCp15TLBLR           ),
    CP_ATTR1(15, TLBLDVA,                  5,  2, 15,  5,  1,1,0,0, AU_TLBLD, 0,      0,        RP_HI,  0,   0,   readCp15TLBLDVA,         writeCp15TLBLDVA         ),
    CP_ATTR1(15, TLBLDPA,                  5,  2, 15,  6,  1,1,0,0, AU_TLBLD, 0,      0,        RP_HI,  0,   0,   readCp15TLBLDPA,         writeCp15TLBLDPA         ),
    CP_ATTR1(15, TLBLDATTR,                5,  2, 15,  7,  1,1,0,0, AU_TLBLD, 0,      0,        RP_HI,  0,   0,   readCp15TLBLDATTR,       writeCp15TLBLDATTR       ),

    // MPU operations, pre ARMv6          op1 op2 crn crm  access   unit      variant hiVersion pri     endB hide readCB                   writeCB
    CP_ATTR1(15, DAPR,                     0,  0,  5,  0,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   readCp15APR,             writeCp15APR             ),
    CP_ATTR1(15, IAPR,                     0,  1,  5,  0,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   readCp15APR,             writeCp15APR             ),
    CP_ATTR1(15, DEAPR,                    0,  2,  5,  0,  1,1,0,0, AU_MPUX,  0,      6,        RP_HI,  0,   0,   readCp15EAPR,            writeCp15EAPR            ),
    CP_ATTR1(15, IEAPR,                    0,  3,  5,  0,  1,1,0,0, AU_MPUX,  0,      6,        RP_HI,  0,   0,   readCp15EAPR,            writeCp15EAPR            ),
    CP_ATTR1(15, DMRR0,                    0,  0,  6,  0,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   readCp15MRR,             writeCp15MRR             ),
    CP_ATTR1(15, DMRR1,                    0,  0,  6,  1,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   readCp15MRR,             writeCp15MRR             ),
    CP_ATTR1(15, DMRR2,                    0,  0,  6,  2,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   readCp15MRR,             writeCp15MRR             ),
    CP_ATTR1(15, DMRR3,                    0,  0,  6,  3,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   readCp15MRR,             writeCp15MRR             ),
    CP_ATTR1(15, DMRR4,                    0,  0,  6,  4,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   readCp15MRR,             writeCp15MRR             ),
    CP_ATTR1(15, DMRR5,                    0,  0,  6,  5,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   readCp15MRR,             writeCp15MRR             ),
    CP_ATTR1(15, DMRR6,                    0,  0,  6,  6,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   readCp15MRR,             writeCp15MRR             ),
    CP_ATTR1(15, DMRR7,                    0,  0,  6,  7,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   readCp15MRR,             writeCp15MRR             ),
    CP_ATTR1(15, IMRR0,                    0,  1,  6,  0,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   readCp15MRR,             writeCp15MRR             ),
    CP_ATTR1(15, IMRR1,                    0,  1,  6,  1,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   readCp15MRR,             writeCp15MRR             ),
    CP_ATTR1(15, IMRR2,                    0,  1,  6,  2,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   readCp15MRR,             writeCp15MRR             ),
    CP_ATTR1(15, IMRR3,                    0,  1,  6,  3,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   readCp15MRR,             writeCp15MRR             ),
    CP_ATTR1(15, IMRR4,                    0,  1,  6,  4,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   readCp15MRR,             writeCp15MRR             ),
    CP_ATTR1(15, IMRR5,                    0,  1,  6,  5,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   readCp15MRR,             writeCp15MRR             ),
    CP_ATTR1(15, IMRR6,                    0,  1,  6,  6,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   readCp15MRR,             writeCp15MRR             ),
    CP_ATTR1(15, IMRR7,                    0,  1,  6,  7,  1,1,0,0, AU_MPU,   0,      6,        RP_HI,  0,   0,   readCp15MRR,             writeCp15MRR             ),

    // MPU operations                     op1 op2 crn crm  access   unit      variant hiVersion pri     endB hide readCB                   writeCB
    CP_ATTR1(15, DRBAR,                    0,  0,  6,  1,  1,1,0,0, AU_MPU,   6,      0,        RP_HI,  0,   0,   readCp15RBAR,            writeCp15RBAR            ),
    CP_ATTR1(15, IRBAR,                    0,  1,  6,  1,  1,1,0,0, AU_MPUS,  6,      0,        RP_HI,  0,   0,   readCp15RBAR,            writeCp15RBAR            ),
    CP_ATTR1(15, DRSR,                     0,  2,  6,  1,  1,1,0,0, AU_MPU,   6,      0,        RP_HI,  0,   0,   readCp15RSR,             writeCp15RSR             ),
    CP_ATTR1(15, IRSR,                     0,  3,  6,  1,  1,1,0,0, AU_MPUS,  6,      0,        RP_HI,  0,   0,   readCp15RSR,             writeCp15RSR             ),
    CP_ATTR1(15, DRACR,                    0,  4,  6,  1,  1,1,0,0, AU_MPU,   6,      0,        RP_HI,  0,   0,   readCp15RACR,            writeCp15RACR            ),
    CP_ATTR1(15, IRACR,                    0,  5,  6,  1,  1,1,0,0, AU_MPUS,  6,      0,        RP_HI,  0,   0,   readCp15RACR,            writeCp15RACR            ),

    // common cache operations (MCR/MRC)  op1 op2 crn crm  access   unit      variant hiVersion pri     endB hide readCB                   writeCB
    CP_ATTR1(15, CP15WFI,                  0,  4,  7,  0,  0,1,0,0, AU_ALL,   0,      0,        RP_HI,  1,   0,   0,                       writeCp15WFI             ),
    CP_ATTR1(15, CP15ISB,                  0,  4,  7,  5,  0,1,0,1, AU_ALL,   0,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, CP15DSB,                  0,  4,  7, 10,  0,1,0,1, AU_ALL,   0,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, CP15DMB,                  0,  5,  7, 10,  0,1,0,1, AU_ALL,   0,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, PrefetchICacheLine,       0,  1,  7, 13,  0,1,0,0, AU_ALL,   0,      0,        RP_HI,  0,   0,   0,                       0                        ),

    // ARMv7 cache operations (MCR/MRC)   op1 op2 crn crm  access   unit      variant hiVersion pri     endB hide readCB                   writeCB
    CP_ATTR1(15, CCSIDR,                   1,  0,  0,  0,  1,0,0,0, AU_ALL,   7,      0,        RP_HI,  0,   0,   readCp15CCSIDR,          0                        ),
    CP_ATTR1(15, ICIALLUIS,                0,  0,  7,  1,  0,1,0,0, AU_MP,    7,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, BPIALLIS,                 0,  6,  7,  1,  0,1,0,0, AU_MP,    7,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, ICIALLU,                  0,  0,  7,  5,  0,1,0,0, AU_ALL,   7,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, ICIMVAU,                  0,  1,  7,  5,  0,1,0,0, AU_ALL,   7,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, BPIALL,                   0,  6,  7,  5,  0,1,0,0, AU_ALL,   7,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, BPIMVA,                   0,  7,  7,  5,  0,1,0,0, AU_ALL,   7,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, DCIMVAC,                  0,  1,  7,  6,  0,1,0,0, AU_ALL,   7,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, DCISW,                    0,  2,  7,  6,  0,1,0,0, AU_ALL,   7,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, DCCMVAC,                  0,  1,  7, 10,  0,1,0,0, AU_ALL,   7,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, DCCSW,                    0,  2,  7, 10,  0,1,0,0, AU_ALL,   7,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, DCCMVAU,                  0,  1,  7, 11,  0,1,0,0, AU_ALL,   7,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, DCCIMVAC,                 0,  1,  7, 14,  0,1,0,0, AU_ALL,   7,      0,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, DCCISW,                   0,  2,  7, 14,  0,1,0,0, AU_ALL,   7,      0,        RP_HI,  0,   0,   0,                       0                        ),

    // ARMv6 cache operations (MCR/MRC)   op1 op2 crn crm  access   unit      variant hiVersion pri     endB hide readCB                   writeCB
    CP_ATTR1(15, InvalidateICache,         0,  0,  7,  5,  0,1,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, InvalidateICacheLineMVA,  0,  1,  7,  5,  0,1,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, InvalidateICacheLineSW,   0,  2,  7,  5,  0,1,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, FlushBranchTargetCache,   0,  6,  7,  5,  0,1,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, FlushBranchTargetEntry,   0,  7,  7,  5,  0,1,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, InvalidateDCache,         0,  0,  7,  6,  0,1,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, InvalidateDCacheLineMVA,  0,  1,  7,  6,  0,1,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, InvalidateDCacheLineSW,   0,  2,  7,  6,  0,1,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, InvalidateUnified,        0,  _,  7,  7,  0,1,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, CleanDCache,              0,  0,  7, 10,  0,1,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       writeCp15CleanDCache     ),
    CP_ATTR1(15, CleanDCacheLineMVA,       0,  1,  7, 10,  0,1,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, CleanDCacheLineSW,        0,  2,  7, 10,  0,1,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, DirtyStatus,              0,  6,  7, 10,  1,0,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, CleanUnified,             0,  _,  7, 11,  0,1,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, BlockTransferStatus,      0,  4,  7, 12,  1,0,1,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, StopPrefetchRange,        0,  5,  7, 12,  0,1,0,1, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, CleanInvalDCache,         0,  0,  7, 14,  0,1,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       writeCp15CleanDCache     ),
    CP_ATTR1(15, CleanInvalDCacheLineMVA,  0,  1,  7, 14,  0,1,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, CleanInvalDCacheLineSW,   0,  2,  7, 14,  0,1,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       0                        ),
    CP_ATTR1(15, CleanInvalUnified,        0,  _,  7, 15,  0,1,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       0                        ),

    // ARMv5 cache operations (MCR/MRC)   op1 op2 crn crm  access   unit      variant hiVersion pri     endB hide readCB                   writeCB
    CP_ATTR1(15, TestCleanDCache,          0,  3,  7, 10,  1,0,0,0, AU_ALL,   0,      6,        RP_HI,  0,   0,   readCp15TestCleanDCache, 0                        ),
    CP_ATTR1(15, TestCleanInvalDCache,     0,  3,  7, 14,  1,0,0,0, AU_ALL,   0,      6,        RP_HI,  0,   0,   readCp15TestCleanDCache, 0                        ),

    // ARMv6 cache operations (MCRR/MRRC) op1 op2 crn crm  access   unit      variant hiVersion pri     endB hide readCB                   writeCB
    CP_ATTR2(15, InvalICacheRange,         0,  0,  0,  5,  0,1,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       writeCp15RangeOpCache    ),
    CP_ATTR2(15, InvalDCacheRange,         0,  0,  0,  6,  0,1,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       writeCp15RangeOpCache    ),
    CP_ATTR2(15, CleanDCacheRange,         0,  0,  0, 12,  0,1,0,1, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       writeCp15RangeOpCache    ),
    CP_ATTR2(15, PrefetchICacheRange,      1,  0,  0, 12,  0,1,0,1, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       writeCp15RangeOpCache    ),
    CP_ATTR2(15, PrefetchDCacheRange,      2,  0,  0, 12,  0,1,0,1, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       writeCp15RangeOpCache    ),
    CP_ATTR2(15, CleanInvalDCacheRange,    0,  0,  0, 14,  0,1,0,0, AU_ALL,   0,      7,        RP_HI,  0,   0,   0,                       writeCp15RangeOpCache    ),
    CP_ATTR2(15, Ignored,                  _,  _,  _,  _,  1,1,1,1, AU_ALL,   0,      7,        RP_LO,  0,   0,   0,                       0                        ),

    // DMA operations                     op1 op2 crn crm  access   unit      variant hiVersion pri     endB hide readCB                   writeCB
    CP_ATTR1(15, DMAStop,                  0,  0, 11,  3,  0,1,0,1, AU_DMA,   0,      0,        RP_HI,  1,   0,   0,                       writeCp15DMAStop         ),
    CP_ATTR1(15, DMAStart,                 0,  1, 11,  3,  0,1,0,1, AU_DMA,   0,      0,        RP_HI,  1,   0,   0,                       writeCp15DMAStart        ),
    CP_ATTR1(15, DMAClear,                 0,  2, 11,  3,  0,1,0,1, AU_DMA,   0,      0,        RP_HI,  1,   0,   0,                       writeCp15DMAClear        ),
    CP_ATTR1(15, DMAControl,               0,  0, 11,  4,  1,1,1,1, AU_DMA,   0,      0,        RP_HI,  1,   0,   readCp15DMAControl,      writeCp15DMAControl      ),
    CP_ATTR1(15, DMAInternalStart,         0,  0, 11,  5,  1,1,1,1, AU_DMA,   0,      0,        RP_HI,  1,   0,   readCp15DMAInternalStart,writeCp15DMAInternalStart),
    CP_ATTR1(15, DMAExternalStart,         0,  0, 11,  6,  1,1,1,1, AU_DMA,   0,      0,        RP_HI,  1,   0,   readCp15DMAExternalStart,writeCp15DMAExternalStart),
    CP_ATTR1(15, DMAInternalEnd,           0,  0, 11,  7,  1,1,1,1, AU_DMA,   0,      0,        RP_HI,  1,   0,   readCp15DMAInternalEnd,  writeCp15DMAInternalEnd  ),
    CP_ATTR1(15, DMAStatus,                0,  0, 11,  8,  1,0,1,0, AU_DMA,   0,      0,        RP_HI,  0,   0,   readCp15DMAStatus,       0                        ),
    CP_ATTR1(15, DMAContextID,             0,  0, 11, 15,  1,1,0,0, AU_DMA,   0,      0,        RP_HI,  0,   0,   readCp15DMAContextID,    writeCp15DMAContextID    ),

    // CortexA9MP CRn15 operations        op1 op2 crn crm  access   unit      variant hiVersion pri     endB hide readCB                   writeCB
    CP_ATTR1(15, NEONB,                    0,  0, 15,  1,  1,0,1,0, AU_NEONB, 0,      0,        RP_HI,  0,   0,   0,                       0                        ),
};

//
// Return armCpRegDesc structure seeded from coprocessor register id
//
static armCpRegDesc getRegDescForCpId(armCPRegId id) {

    const cpRegAttrs *attrs = &cpRegInfo[id];

    armCpRegDesc result = {
        desc : {
            cpNum : attrs->cpNum,
            crn   : attrs->crn,
            crm   : attrs->crm,
            op1   : attrs->op1,
            op2   : attrs->op2
        }
    };

    return result;
}

//
// Iterator filling 'details' with the next coprocessor register details -
// 'details.name' should be initialized to NULL prior to the first call
//
Bool armGetCpRegisterDetails(armCpRegDetailsP details) {

    armCPRegId id = details->name ? details->id+1 : 0;

    if(id>=CP_ID(FirstPseudoReg)) {

        return False;

    } else {

        const cpRegAttrs *attrs = &cpRegInfo[id];

        details->name   = attrs->name;
        details->id     = id;
        details->cpNum  = attrs->cpNum;
        details->op1    = attrs->op1;
        details->op2    = attrs->op2;
        details->crn    = attrs->crn;
        details->crm    = attrs->crm;
        details->privRW = accessActionRW[attrs->access[AA_PRIV_READ]][attrs->access[AA_PRIV_WRITE]];
        details->userRW = accessActionRW[attrs->access[AA_USER_READ]][attrs->access[AA_USER_WRITE]];

        return True;
    }
}

//
// Can the indicated coprocessor register be accessed using the passed action on
// this ARM processor variant?
//
static Bool canAccessCPReg(
    armP              arm,
    const cpRegAttrs *attrs,
    accessAction      action
) {
    armArchitecture configVariant   = arm->configInfo.arch;
    armArchitecture requiredVariant = attrs->variant;
    armArchitecture hiVersion       = attrs->hiVersion;
    Uns32           configVersion   = ARM_INSTRUCTION_VERSION(configVariant);

    if(!attrs->access[action]) {
        return False;
    } else if(ARM_INSTRUCTION_VERSION(requiredVariant) > configVersion) {
        return False;
    } else if(hiVersion && (hiVersion<=configVersion)) {
        return False;
    } else if(!ARM_SUPPORT(configVariant, requiredVariant & ~ARM_MASK_VERSION)) {
        return False;
    } else switch(attrs->unit) {
        case AU_MP:    return CP_MP_IMPLEMENTED(arm);
        case AU_MMU:   return MMU_PRESENT(arm);
        case AU_MMUMP: return MMU_PRESENT(arm) && CP_MP_IMPLEMENTED(arm);
        case AU_MPU:   return MPU_PRESENT(arm);
        case AU_MPUX:  return MPU_PRESENT(arm) && EXT_AP_PRESENT(arm);
        case AU_MPUS:  return MPUS_PRESENT(arm);
        case AU_TLD:   return MMU_PRESENT(arm) && TLDD_SIZE(arm);
        case AU_TLDS:  return MMUS_PRESENT(arm) && TLDI_SIZE(arm);
        case AU_ALIGN: return ALIGN_PRESENT(arm);
        case AU_DTCM:  return DTCM_PRESENT(arm);
        case AU_ITCM:  return ITCM_PRESENT(arm);
        case AU_IFAR:  return IFAR_PRESENT(arm);
        case AU_PCR:   return PCR_PRESENT(arm);
        case AU_NEONB: return NEONB_PRESENT(arm);
        case AU_CBAR:  return CBAR_PRESENT(arm);
        case AU_TLBLD: return TLBLD_PRESENT(arm);
        case AU_TLBHR: return TLBHR_PRESENT(arm);
        case AU_DMA:   return DMA_PRESENT(arm);
        case AU_TEE:   return !CP_USE_CPUID(arm) || ARM_ISAR(3, T2ExeEnvExtn_instrs);
        case AU_JAZ:   return ARM_SUPPORT(configVariant, ARM_J);
        default:       return True;
    }
}

//
// Match a single index agianst a candidate from the coprocessor descriptions
//
inline static Bool matchCPIndex(Int32 given, Int32 pattern) {
    if(pattern==_) {
        return True;
    } else if(pattern<0) {
        return given<-pattern;
    } else {
        return pattern==given;
    }
}

//
// Return a boolean indicating whether a coprocessor register addressed using
// the passed cpOp1, cpOp2, crn and crm specifiers matches the passed attributes
//
static Bool matchCPReg(
    armP              arm,
    Uns32             cpNum,
    Uns32             op1,
    Uns32             op2,
    Uns32             crn,
    Uns32             crm,
    const cpRegAttrs *attrs,
    regPriority       priority,
    accessAction      action,
    Uns32             size
) {
    if(
        (priority == attrs->priority)          &&
        (size     == attrs->size)              &&
        (cpNum    == attrs->cpNum)             &&
        matchCPIndex(op1, attrs->op1)          &&
        matchCPIndex(op2, attrs->op2)          &&
        matchCPIndex(crn, attrs->crn)          &&
        matchCPIndex(crm, attrs->crm)
    ) {
        return canAccessCPReg(arm, attrs, action);
    } else {
        return False;
    }
}

#undef _

//
// Return identifier for coprocessor register addressed using the passed cpNum,
// op1, op2, crn and crm specifiers, or CP_INVALID if no register is identified
//
static armCPRegId getCPId(
    armP         arm,
    Uns32        cpNum,
    Uns32        op1,
    Uns32        op2,
    Uns32        crn,
    Uns32        crm,
    accessAction action,
    Uns32        size
) {
    const cpRegAttrs *attrs = cpRegInfo;
    regPriority       pri;
    armCPRegId        id;

    // search for a matching entry in cp14RegInfo
    for(pri=RP_HI; pri<=RP_LO; pri++) {
        for(id=0; id<CP_ID(Size); id++) {
            if(matchCPReg(arm, cpNum, op1, op2, crn, crm, attrs+id, pri, action, size)) {
                return id;
            }
        }
    }

    return CP_INVALID;
}


//
// Some early ARM cores ignore invalid coprocessor 15 accesses; later cores
// generate Invalid Instruction exceptions.
//
static Bool ignoreInvalidCpAccess(armP arm, Uns32 cpNum, accessAction action) {
    if((cpNum==15) && !(action&AA_USER)) {
        return arm->configInfo.ignoreBadCp15;
    } else {
        return False;
    }
}

//
// Fill armCPReadInfo structure with information about how to perform a
// coprocessor read
//
static Bool getCPReadInfo(
    armP           arm,
    armCPReadInfoP info,
    Uns32          cpNum,
    Uns32          id,
    accessAction   action
) {
    info->rs = VMI_NOREG;

    if(id==CP_INVALID) {

        // invalid register access
        if(ignoreInvalidCpAccess(arm, cpNum, action)) {
            info->cb = ignoreCpRead;
        } else {
            return False;
        }

    } else {

        // write of plain register or by callback
        if(cpRegInfo[id].readCB) {
            info->cb = cpRegInfo[id].readCB;
        } else if(id<CP_ID(FirstPseudoReg)) {
            info->rs = ARM_CP_REG(id);
        }
    }

    return True;
}

//
// Fill armCPWriteInfo structure with information about how to perform a
// coprocessor write
//
static Bool getCPWriteInfo(
    armP            arm,
    armCPWriteInfoP info,
    Uns32           cpNum,
    Uns32           crm,
    Uns32           id,
    accessAction    action
) {
    info->rd = VMI_NOREG;

    if(id==CP_INVALID) {

        // invalid register access
        if(ignoreInvalidCpAccess(arm, cpNum, action)) {
            info->cb = ignoreCpWrite;
        } else {
            return False;
        }

    } else {

        // indicate whether the current block should be terminated when this
        // register is written
        info->endBlock = cpRegInfo[id].wEndBlock;

        // write of plain register or by callback
        if(cpRegInfo[id].writeCB) {
            info->cb = cpRegInfo[id].writeCB;
        } else if(id<CP_ID(FirstPseudoReg)) {
            info->rd = ARM_CP_REG(id);
        }

        // get mask of writable bits
        info->writeMask = cpRegInfo[id].writeMask;
    }

    return True;
}


////////////////////////////////////////////////////////////////////////////////
// CODE EMISSION FOR READ AND WRITE CALLBACKS
////////////////////////////////////////////////////////////////////////////////

//
// A coprocessor read that allegedly targets R15 actually sets flags!
//
inline static Bool cpReadSetsFlags(vmiReg rd) {
    return VMI_REG_EQUAL(rd, ARM_PC);
}

//
// Set processor flags from value in register rs
//
static void emitCPReadSetFlags(armMorphStateP state, vmiReg rs) {

    // emit call to armWriteCPSR to perform flag update
    armEmitArgProcessor(state);
    armEmitArgReg(state, ARM_GPR_BITS, rs);
    armEmitArgUns32(state, CPSR_NZCV);
    armEmitCall(state, (vmiCallFn)armWriteCPSR);

    // terminate the code block (derived flags are invalid)
    armEmitEndBlock();
}

//
// Emit read of plain register
//
static void emitCPReadReg(armMorphStateP state, vmiReg rd, vmiReg rs) {
    if(cpReadSetsFlags(rd)) {
        emitCPReadSetFlags(state, rs);
    } else {
        armEmitMoveRR(state, ARM_GPR_BITS, rd, rs);
    }
}

//
// Emit read callback (one register)
//
static void emitCPReadCB(
    armMorphStateP state,
    armCpReadFn    cb,
    vmiReg         rd,
    armCpRegDesc   regDesc
) {
    Bool   setFlags = cpReadSetsFlags(rd);
    vmiReg t        = setFlags ? ARM_TEMP(state->tempIdx) : rd;

    // set register (or temporary if destination in PC)
    armEmitArgProcessor(state);
    armEmitArgUns32(state, regDesc.value);
    armEmitCallResult(state, (vmiCallFn)cb, ARM_GPR_BITS, t);

    // set CPSR flags if destination is PC
    if(setFlags) {
        emitCPReadSetFlags(state, t);
    }
}

//
// Emit write callback
//
static void emitCPWriteCB(
    armMorphStateP state,
    armCpWriteFn   cb,
    vmiReg         rs1,
    vmiReg         rs2,
    armCpRegDesc   regDesc
) {
    armEmitArgProcessor(state);
    armEmitArgUns32(state, regDesc.value);
    armEmitArgReg(state, ARM_GPR_BITS, rs1);
    armEmitArgReg(state, ARM_GPR_BITS, rs2);
    armEmitCall(state, (vmiCallFn)cb);
}

//
// Emit coprocessor read (one register)
//
Bool armEmitCpRead(armMorphStateP state, vmiReg rd) {

    armP         arm    = state->arm;
    Uns32        cpNum  = state->info.cpNum;
    Uns32        op1    = state->info.cpOp1;
    Uns32        op2    = state->info.cpOp2;
    Uns32        crn    = state->info.r2;
    Uns32        crm    = state->info.r3;
    accessAction action = IN_USER_MODE(arm) ? AA_USER_READ : AA_PRIV_READ;
    Uns32        id     = getCPId(arm, cpNum, op1, op2, crn, crm, action, 1);

    // seed armCPReadInfo using crn, crm, op1 and op2
    armCPReadInfo info = {
        regDesc : {desc : {crn:crn, crm:crm, op1:op1, op2:op2}}
    };

    if(!getCPReadInfo(arm, &info, cpNum, id, action)) {

        // register not readable
        return False;

    } else if(info.cb) {

        // register read requires function call
        emitCPReadCB(state, info.cb, rd, info.regDesc);

    } else {

        // readable as plain register
        emitCPReadReg(state, rd, info.rs);
    }

    return True;
}

//
// Emit coprocessor write (one register)
//
Bool armEmitCpWrite(armMorphStateP state, vmiReg rs) {

    Uns32        bits   = ARM_GPR_BITS;
    armP         arm    = state->arm;
    Uns32        cpNum  = state->info.cpNum;
    Uns32        op1    = state->info.cpOp1;
    Uns32        op2    = state->info.cpOp2;
    Uns32        crn    = state->info.r2;
    Uns32        crm    = state->info.r3;
    accessAction action = IN_USER_MODE(arm) ? AA_USER_WRITE : AA_PRIV_WRITE;
    Uns32        id     = getCPId(arm, cpNum, op1, op2, crn, crm, action, 1);

    // seed armCPWriteInfo using crn, crm, op1 and op2
    armCPWriteInfo info = {
        regDesc : {desc : {crn:crn, crm:crm, op1:op1, op2:op2}}
    };

    if(!getCPWriteInfo(arm, &info, cpNum, crm, id, action)) {

        // register not writable
        return False;

    } else if(info.cb) {

        // register write requires function call
        emitCPWriteCB(state, info.cb, rs, VMI_NOREG, info.regDesc);

    } else if(!VMI_ISNOREG(info.rd)) {

        Uns32 writeMask = info.writeMask;

        // writable as plain register
        if(info.writeMask==-1) {

            // no masking required
            armEmitMoveRR(state, bits, info.rd, rs);

        } else {

            // masking required
            vmiReg t = ARM_TEMP(state->tempIdx);

            // mask out current value from coprocessor register
            armEmitBinopRC(state, bits, vmi_AND, info.rd, ~writeMask, 0);

            // select required bits from the new value
            armEmitBinopRRC(state, bits, vmi_AND, t, rs, writeMask, 0);

            // combine the two values in coprocessor register
            armEmitBinopRR(state, bits, vmi_OR, info.rd, t, 0);
        }
    }

    // terminate the current block if required
    if(info.endBlock) {
        armEmitEndBlock();
    }

    return True;
}

//
// Emit coprocessor read (two registers)
//
Bool armEmitCpReadPair(armMorphStateP state, vmiReg rd1, vmiReg rd2) {

    armP         arm    = state->arm;
    Uns32        cpNum  = state->info.cpNum;
    Uns32        op1    = state->info.cpOp1;
    Uns32        op2    = 0;
    Uns32        crn    = 0;
    Uns32        crm    = state->info.r3;
    accessAction action = IN_USER_MODE(arm) ? AA_USER_READ : AA_PRIV_READ;
    Uns32        id     = getCPId(arm, cpNum, op1, op2, crn, crm, action, 2);

    // seed armCPReadInfo using crn, crm, op1 and op2
    armCPReadInfo info = {
        regDesc : {desc : {crn:crn, crm:crm, op1:op1, op2:op2}}
    };

    if(!getCPReadInfo(arm, &info, cpNum, id, action)) {

        // register not readable
        return False;

    } else if(info.cb) {

        // register read requires function call
        VMI_ABORT("callback read of coprocessor register pair unimplemented");

    } else {

        // readable as plain register
        vmiReg rsLSW = info.rs;
        vmiReg rsMSW = VMI_REG_DELTA(rsLSW, sizeof(Uns32));

        emitCPReadReg(state, rd1, rsLSW);
        emitCPReadReg(state, rd2, rsMSW);
    }

    return True;
}

//
// Emit coprocessor write (two registers)
//
Bool armEmitCpWritePair(armMorphStateP state, vmiReg rs1, vmiReg rs2) {

    armP         arm    = state->arm;
    Uns32        cpNum  = state->info.cpNum;
    Uns32        op1    = state->info.cpOp1;
    Uns32        op2    = 0;
    Uns32        crn    = 0;
    Uns32        crm    = state->info.r3;
    accessAction action = IN_USER_MODE(arm) ? AA_USER_WRITE : AA_PRIV_WRITE;
    Uns32        id     = getCPId(arm, cpNum, op1, op2, crn, crm, action, 2);

    // seed armCPWriteInfo using crn, crm, op1 and op2
    armCPWriteInfo info = {
        regDesc : {desc : {crn:crn, crm:crm, op1:op1, op2:op2}}
    };

    if(!getCPWriteInfo(arm, &info, cpNum, crm, id, action)) {

        // register not writable
        return False;

    } else if(info.cb) {

        // register write requires function call
        emitCPWriteCB(state, info.cb, rs1, rs2, info.regDesc);

    } else if(!VMI_ISNOREG(info.rd)) {

        // writable as plain register
        VMI_ABORT("plain write of coprocessor register pair unimplemented");
    }

    // terminate the current block if required
    if(info.endBlock) {
        armEmitEndBlock();
    }

    return True;
}


////////////////////////////////////////////////////////////////////////////////
// RESET AND INITIALIZATION
////////////////////////////////////////////////////////////////////////////////

//
// Macro used to set initial value of read-only coprocessor register from
// configuration
//
#define INITIALIZE_CP_REG(_P, _N) \
    CP_REG_STRUCT(_P, _N) = CP_REG_STRUCT_DEFAULT(_P, _N)

//
// Macro used to set initial value of read-only coprocessor field from
// configuration
//
#define INITIALIZE_CP_FIELD(_P, _N, _F) \
    CP_FIELD(_P, _N, _F) = CP_FIELD_DEFAULT(_P, _N, _F)

//
// Macro use to force reset of a coprocessor register to a fixed initial
// value
//
#define RESET_CP_REG_VALUE(_P, _R, _V) \
    armWriteCpRegPriv(CP_ID(_R), _P, _V)

//
// Macro use to force reset of a coprocessor register to its initial configured
// state
//
#define RESET_CP_REG_CONFIG(_P, _R) \
    RESET_CP_REG_VALUE(_P, _R, *(Uns32*)&CP_REG_STRUCT_DEFAULT(arm, _R))

//
// Call on initialization
//
void armCpInitialize(armP arm, Uns32 index) {

    // set read-only coprocessor registers
    INITIALIZE_CP_REG(arm, MIDR);
    INITIALIZE_CP_REG(arm, CTR);
    INITIALIZE_CP_REG(arm, TCMTR);
    INITIALIZE_CP_REG(arm, TLBTR);
    INITIALIZE_CP_REG(arm, MPUIR);
    INITIALIZE_CP_REG(arm, ID_PFR0);
    INITIALIZE_CP_REG(arm, ID_PFR1);
    INITIALIZE_CP_REG(arm, ID_DFR0);
    INITIALIZE_CP_REG(arm, ID_AFR0);
    INITIALIZE_CP_REG(arm, ID_MMFR0);
    INITIALIZE_CP_REG(arm, ID_MMFR1);
    INITIALIZE_CP_REG(arm, ID_MMFR2);
    INITIALIZE_CP_REG(arm, ID_MMFR3);
    INITIALIZE_CP_REG(arm, ID_ISAR0);
    INITIALIZE_CP_REG(arm, ID_ISAR1);
    INITIALIZE_CP_REG(arm, ID_ISAR2);
    INITIALIZE_CP_REG(arm, ID_ISAR3);
    INITIALIZE_CP_REG(arm, ID_ISAR4);
    INITIALIZE_CP_REG(arm, ID_ISAR5);
    INITIALIZE_CP_REG(arm, CLIDR);
    INITIALIZE_CP_REG(arm, AIDR);
    INITIALIZE_CP_REG(arm, DMAPresent);

    // initialize MPIDR from numCPUs and index
    if(armGetCpRegSupported(CP_ID(MPIDR), arm)) {
        CP_FIELD(arm, MPIDR, AffinityLevel0) = index;
        CP_FIELD(arm, MPIDR, U)              = !arm->configInfo.numCPUs;
        CP_FIELD(arm, MPIDR, Implemented)    = 1;
    }

    // set read-only coprocessor fields
    INITIALIZE_CP_FIELD(arm, DTCMRR, Size);
    INITIALIZE_CP_FIELD(arm, ITCMRR, Size);

    // TODO: this field is not really read-only, but updates to it are not
    // supported and rejected by writeCp15SCTLR
    INITIALIZE_CP_FIELD(arm, SCTLR, B);

    // determine whether ARMv5-format FSR register should be used - if False,
    // then ARMv6/ARMv7 DFSR and IFSR registers will be used instead
    arm->useARMv5FSR = armGetCpRegSupported(CP_ID(FSR), arm);

    // determine whether ARMv5-format TTBR register should be used - if False,
    // then ARMv6/ARMv7 TTBR0, TTBR1 and TTBRControl registers will be used
    // instead
    arm->useARMv5TTBR = armGetCpRegSupported(CP_ID(TTBR), arm);

    // determine whether ARMv5-format Protection Area Control registers should
    // be used
    arm->useARMv5PAC = armGetCpRegSupported(CP_ID(DMRR0), arm);
}

//
// Call on reset
//
void armCpReset(armP arm) {

    // coprocessor 14 reset
    RESET_CP_REG_VALUE (arm, TEECR, 0);

    // coprocessor 15 reset
    RESET_CP_REG_VALUE (arm, SCTLR, 0);
    RESET_CP_REG_CONFIG(arm, ACTLR);
    RESET_CP_REG_VALUE (arm, TTBR0, 0);
    RESET_CP_REG_VALUE (arm, TTBR1, 0);
    RESET_CP_REG_VALUE (arm, TTBCR, 0);
    RESET_CP_REG_CONFIG(arm, DTCMRR);
    RESET_CP_REG_CONFIG(arm, ITCMRR);
    RESET_CP_REG_VALUE (arm, PRRR, CP_PRRRReset);
    RESET_CP_REG_VALUE (arm, NMRR,  CP_NMRRReset);
    RESET_CP_REG_VALUE (arm, FCSEIDR, 0);
    RESET_CP_REG_VALUE (arm, CONTEXTIDR, 0);
    RESET_CP_REG_VALUE (arm, TPIDRURW, 0);
    RESET_CP_REG_VALUE (arm, TPIDRURO, 0);
    RESET_CP_REG_VALUE (arm, TPIDRPRW, 0);
    RESET_CP_REG_VALUE (arm, DMAUserAccessibility, 0);
    RESET_CP_REG_VALUE (arm, DMAChannel, 0);
    RESET_CP_REG_VALUE (arm, DMAControl, 0);
    RESET_CP_REG_VALUE (arm, DMAStatus, 0);
    RESET_CP_REG_VALUE (arm, CPACR, 0);
    RESET_CP_REG_CONFIG(arm, PCR);
    RESET_CP_REG_CONFIG(arm, CBAR);
    RESET_CP_REG_CONFIG(arm, CPACR);
}


////////////////////////////////////////////////////////////////////////////////
// DEBUGGER ACCESS TO COPROCESSORS
////////////////////////////////////////////////////////////////////////////////

//
// Return a value read from a fixed offset in the processor structure
//
static ARM_CP_READFN(readCp) {

    armCPRegId id = regDesc.value;

    return arm->cp.regs[id];
}

//
// Write a value to a fixed offset in the processor structure
//
static ARM_CP_WRITEFN(writeCp) {

    armCPRegId id        = regDesc.value;
    Uns32      writeMask = cpRegInfo[id].writeMask;
    Uns32      oldValue  = arm->cp.regs[id];

    arm->cp.regs[id] = ((oldValue&~writeMask) | (newValue&writeMask));
}

//
// Is the indicated coprocessor register supported on this processor?
//
Bool armGetCpRegSupported(armCPRegId id, armP arm) {

    const cpRegAttrs *attrs = &cpRegInfo[id];

    return (
        !attrs->hide &&
        (
            canAccessCPReg(arm, attrs, AA_PRIV_READ) ||
            canAccessCPReg(arm, attrs, AA_PRIV_WRITE)
        )
    );
}

//
// Get coprocessor register read callback (privileged mode)
//
static armCpReadFn getCpPrivRegReadCallBack(
    armCPRegId    id,
    armP          arm,
    armCpRegDescP regDesc
) {
    accessAction      action = AA_PRIV_READ;
    const cpRegAttrs *attrs  = &cpRegInfo[id];
    armCPReadInfo     info   = {regDesc : getRegDescForCpId(id)};

    if(!canAccessCPReg(arm, attrs, action)) {
        return 0;
    } else if(!getCPReadInfo(arm, &info, 0, id, action)) {
        return 0;
    } else if(info.cb) {
        *regDesc = info.regDesc;
        return info.cb;
    } else if(!VMI_ISNOREG(info.rs)) {
        regDesc->value = id;
        return readCp;
    } else {
        return 0;
    }
}

//
// Get coprocessor register write callback (privileged mode)
//
static armCpWriteFn getCpPrivRegWriteCallBack(
    armCPRegId    id,
    armP          arm,
    armCpRegDescP regDesc
) {
    accessAction      action = AA_PRIV_WRITE;
    const cpRegAttrs *attrs  = &cpRegInfo[id];
    Uns32             crm    = (attrs->crm!=-1) ? attrs->crm : 0;
    armCPWriteInfo    info   = {regDesc : getRegDescForCpId(id)};

    if(!canAccessCPReg(arm, attrs, action)) {
        return 0;
    } else if(!getCPWriteInfo(arm, &info, 0, crm, id, action)) {
        return 0;
    } else if(info.cb) {
        *regDesc = info.regDesc;
        return info.cb;
    } else if(!VMI_ISNOREG(info.rd)) {
        regDesc->value = id;
        return writeCp;
    } else {
        return 0;
    }
}

//
// Perform a privileged-mode read of the coprocessor register
//
Bool armReadCpRegPriv(armCPRegId id, armP arm, Uns32 *result) {

    // get the read callback to apply
    armCpRegDesc regDesc;
    armCpReadFn  readCB = getCpPrivRegReadCallBack(id, arm, &regDesc);

    // either apply the callback or set the result to 0
    if(readCB) {
        *(Uns32*)result = readCB(arm, regDesc);
        return True;
    } else {
        *(Uns32*)result = 0;
        return False;
    }
}

//
// Perform a privileged-mode write of the coprocessor register
//
Bool armWriteCpRegPriv(armCPRegId id, armP arm, Uns32 value) {

    // get the write callback to apply
    armCpRegDesc regDesc;
    armCpWriteFn writeCB = getCpPrivRegWriteCallBack(id, arm, &regDesc);

    // apply the callback if it is found
    if(writeCB) {
        writeCB(arm, regDesc, value);
        return True;
    } else {
        return False;
    }
}


////////////////////////////////////////////////////////////////////////////////
// COPROCESSOR PROGRAMMER'S VIEW
////////////////////////////////////////////////////////////////////////////////

//
// Callback to obtain the value of a coprocessor register
//
static VMI_VIEW_VALUE_FN(cpRegViewCB) {

    // get the processor for this view object
    vmiViewObjectP baseObject      = vmiviewGetViewObjectParent(object);
    vmiViewObjectP processorObject = vmiviewGetViewObjectParent(baseObject);
    armP           arm             = vmirtGetViewObjectUserData(processorObject);
    Uns32          id              = (Uns32)clientData;

    armReadCpRegPriv(id, arm, (Uns32*)buffer);

    return VMI_VVT_UNS32;
}

//
// Add programmer's view of coprocessor register
//
void armAddCpRegisterView(
    armCPRegId     id,
    armP           arm,
    vmiViewObjectP baseObject,
    const char    *name
) {
    accessAction  action = AA_PRIV_READ;
    armCPReadInfo info   = {regDesc : getRegDescForCpId(id)};

    if(!getCPReadInfo(arm, &info, 0, id, action)) {

        // unknown register

    } else if(!info.cb && VMI_ISNOREG(info.rs)) {

        // register not readable on this variant

    } else {

        // extract fields from id (where known)
        const cpRegAttrs *attrs = &cpRegInfo[id];
        Uns32             cpNum = attrs->cpNum;
        Uns32             op1   = (attrs->op1!=-1) ? attrs->op1 : 0;
        Uns32             op2   = (attrs->op2!=-1) ? attrs->op2 : 0;
        Uns32             crm   = (attrs->crm!=-1) ? attrs->crm : 0;
        Uns32             crn   = (attrs->crn!=-1) ? attrs->crn : 0;

        // create register description string
        char description[32];
        snprintf(
            description, sizeof(description), "CP%u(%u,%u,%u,%u)",
            cpNum, op1, crn, crm, op2
        );

        // create new register object
        vmiViewObjectP regObject = vmirtAddViewObject(
            baseObject, name, description
        );

        // either reference the value directly or use a callback
        if(info.cb) {
            vmirtSetViewObjectValueCallback(regObject, cpRegViewCB, (void *)id);
        } else {
            Uns32 *refValue = &arm->cp.regs[id];
            vmirtSetViewObjectRefValue(regObject, VMI_VVT_UNS32, refValue);
        }
    }
}
