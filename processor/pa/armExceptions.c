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

#include <string.h>
#include <stdio.h>

// Imperas header files
#include "hostapi/impAlloc.h"

// VMI header files
#include "vmi/vmiMessage.h"
#include "vmi/vmiRt.h"
#include "vmi/vmiPorts.h"

// model header files
#include "armConfig.h"
#include "armCP.h"
#include "armCPRegisters.h"
#include "armDecode.h"
#include "armExceptions.h"
#include "armFunctions.h"
#include "armMessage.h"
#include "armMPCore.h"
#include "armStructure.h"
#include "armSIMDVFP.h"
#include "armUtils.h"
#include "armVariant.h"
#include "armVM.h"

//
// Prefix for messages from this module
//
#define CPU_PREFIX "ARM_EXCEPTION"

////////////////////////////////////////////////////////////////////////////////
// SMP CLUSTER UTILITIES
////////////////////////////////////////////////////////////////////////////////

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
// Return the cluster root of the passed processor
//
static armP getClusterRoot(armP arm) {

    armP parent;

    while((parent=getParent(arm))) {
        arm = parent;
    }

    return arm;
}


////////////////////////////////////////////////////////////////////////////////
// NET PORT LIST MANAGEMENT
////////////////////////////////////////////////////////////////////////////////

//
// Create new port specification
//
static void newNetPortSpec(
    armP           arm,
    const char    *netName,
    const char    *desc,
    vmiNetPortType type,
    vmiNetChangeFn cb,
    Uns32          userData,
    Uns32         *handle
) {
    armP root = getClusterRoot(arm);
    char tmp[1024];

    // if this is not the root processor, construct a unique name for it based
    // on the processor type and index
    if(root != arm) {
        Uns32 index = vmirtGetSMPIndex((vmiProcessorP)arm);
        sprintf(tmp, "%s_CPU%u", netName, index);
        netName = tmp;
    }

    // allocate information structure to enable vectoring interrupt to specific
    // processor instance
    armInterruptInfoP ii = STYPE_CALLOC(armInterruptInfo);
    ii->proc     = arm;
    ii->userData = userData;

    // allocate arm-specific port structure
    armNetPortP n = STYPE_CALLOC(armNetPort);
    n->port.name        = strdup(netName);
    n->port.description = desc;
    n->port.type        = type;
    n->port.netChangeCB = cb;
    n->port.handle      = handle;
    n->port.userData    = ii;

    // chain into list on processor
    if(!root->lastPort) {
        root->firstPort = n;
    } else {
        root->lastPort->next = n;
    }

    root->lastPort = n;
}

//
// Create new input port specification
//
void armAddNetInputPort(
    armP           arm,
    const char    *name,
    vmiNetChangeFn cb,
    Uns32          userData,
    const char    *desc
) {
    newNetPortSpec(arm, name, desc, vmi_NP_INPUT, cb, userData, 0);
}

//
// Create new output port specification
//
void armAddNetOutputPort(
    armP        arm,
    const char *name,
    Uns32      *handle,
    const char *desc
) {
    newNetPortSpec(arm, name, desc, vmi_NP_OUTPUT, 0, 0, handle);
}

//
// Free port specifications
//
void armFreePortSpecs(armP arm) {

    armNetPortP n;
    armNetPortP next;

    for(n=arm->firstPort; n; n=next) {
        next = n->next;
        STYPE_FREE(n->port.name);
        STYPE_FREE(n);
    }

    arm->firstPort = 0;
    arm->lastPort  = 0;
}

//
// Net port iteration
//
VMI_NET_PORT_SPECS_FN(armGetNetPortSpec) {

    armNetPortP port = (armNetPortP)prev;

    if(!port) {
        port = ((armP)processor)->firstPort;
    } else {
        port = port->next;
    }

    return &port->port;
}


////////////////////////////////////////////////////////////////////////////////
// EXCEPTION INFORMATION (FOR DEBUGGER INTEGRATION)
////////////////////////////////////////////////////////////////////////////////

//
// Exception vectors
//
#define ARM_VECTOR_HIGH           0xffff0000
#define ARM_VECTOR_Reset          0x00000000
#define ARM_VECTOR_Undefined      0x00000004
#define ARM_VECTOR_SupervisorCall 0x00000008
#define ARM_VECTOR_PrefetchAbort  0x0000000C
#define ARM_VECTOR_DataAbort      0x00000010
#define ARM_VECTOR_IRQ            0x00000018
#define ARM_VECTOR_FIQ            0x0000001C

//
// helper macro for filling exception table
//
#define ARM_EXCEPTION_INFO(_D) [AE_##_D] = {name:#_D, code:ARM_VECTOR_##_D}

//
// Table of exception mode descriptors
//
static const vmiExceptionInfo exceptions[AE_LAST] = {
    ARM_EXCEPTION_INFO(Reset),
    ARM_EXCEPTION_INFO(Undefined),
    ARM_EXCEPTION_INFO(SupervisorCall),
    ARM_EXCEPTION_INFO(PrefetchAbort),
    ARM_EXCEPTION_INFO(DataAbort),
    ARM_EXCEPTION_INFO(IRQ),
    ARM_EXCEPTION_INFO(FIQ),
};

//
// Return the current processor exception
//
VMI_GET_EXCEPTION_FN(armGetException) {
    armP arm = (armP)processor;
    return &exceptions[arm->exception];
}

//
// Exception mode iterator
//
VMI_EXCEPTION_INFO_FN(armExceptionInfo) {

    vmiExceptionInfoCP end = exceptions+AE_LAST;
    vmiExceptionInfoCP this;

    // on the first call, start with the first member of the table
    if(!prev) {
        prev = exceptions-1;
    }

    // search for the next member
    for(this=prev+1; this!=end; this++) {
        return this;
    }

    // no more exceptions
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
// UTILITIES
////////////////////////////////////////////////////////////////////////////////

//
// Return processor endianness
//
inline static memEndian getEndian(armP arm) {
    return armGetEndian((vmiProcessorP)arm, False);
}

//
// Is the processor halted in a WFI instruction?
//
inline static Bool inWFI(armP arm) {
    return arm->disable & AD_WFI;
}

//
// Is the processor halted in a WFE instruction?
//
inline static Bool inWFE(armP arm) {
    return arm->disable & AD_WFE;
}

//
// Return current program counter
//
inline static Uns32 getPC(armP arm) {
    return vmirtGetPC((vmiProcessorP)arm);
}

//
// Jump to vector
//
static void jumpToExceptionVector(armP arm, armException exception) {

    Uns32 vectorBase = CP_FIELD(arm, SCTLR, V) ? ARM_VECTOR_HIGH : 0x0;
    Uns32 vector     = exceptions[exception].code;

    arm->exception = exception;

    vmirtSetPCException((vmiProcessorP)arm, vector|vectorBase);
}

//
// Is FIQ interrupt pending and enabled?
//
inline static Bool fiqPendingAndEnabled(armP arm) {
    return (arm->internalInt & FIQ_MASK) && !arm->CPSR.fields.F;
}

//
// Is IRQ interrupt pending and enabled?
//
inline static Bool irqPendingAndEnabled(armP arm) {
    return (arm->internalInt & IRQ_MASK) && !arm->CPSR.fields.I;
}

//
// Is there a pending interrupt on this processor? Note that if the processor
// is halted by WFI, interrupts are not masked by CPSR fields.
//
Bool armInterruptPending(armP arm) {
    if(inWFI(arm)) {
        return (arm->internalInt != 0);
    } else {
        return fiqPendingAndEnabled(arm) || irqPendingAndEnabled(arm);
    }
}


////////////////////////////////////////////////////////////////////////////////
// EXCEPTION ACTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Do common actions to update the CPSR on an exception
//
#define WRITE_CPSR_EXCEPT(_P, _CPSR, _MODE, _F, _I, _A)                 \
                                                                        \
    /* switch mode, update Thumb/Jazelle state and interrupt masks */   \
    _CPSR.fields.mode = _MODE;                                          \
    _CPSR.fields.J    = 0;                                              \
    _CPSR.fields.T    = CP_FIELD(_P, SCTLR, TE);                        \
    _CPSR.fields.F    = _F;                                             \
    _CPSR.fields.I    = _I;                                             \
    _CPSR.fields.A    = _A;                                             \
                                                                        \
    /* clear ITSTATE */                                                 \
    _CPSR.fields.IT72 = 0;                                              \
    _CPSR.fields.IT10 = 0;                                              \
                                                                        \
    /* switch endianness in the exception handler if required */        \
    if(CP_FEATURE_PRESENT(_P, SCTLR, EE)) {                             \
        _CPSR.fields.E = CP_FIELD(_P, SCTLR, EE);                       \
    }                                                                   \
                                                                        \
    /* update CPSR */                                                   \
    armWriteCPSR(_P, _CPSR.reg, _P->wMaskCPSR)

//
// Do common exception actions
//
static void doException(
    armP         arm,
    armCPSRMode  mode,
    armException exception,
    Uns32        lr,
    Bool         setA
) {
    armCPSR oldCPSR = {{.reg = armReadCPSR(arm)}};
    armCPSR newCPSR = oldCPSR;
    Bool    F       = newCPSR.fields.F;
    Bool    I       = 1;
    Bool    A       = setA || newCPSR.fields.A;

    // update CPSR
    WRITE_CPSR_EXCEPT(arm, newCPSR, mode, F, I, A);

    // save return address and CPSR
    arm->regs[ARM_REG_LR] = lr;
    arm->SPSR             = oldCPSR;

    // jump to exception vector
    jumpToExceptionVector(arm, exception);
}

//
// Perform actions required for data abort
//
inline static void doDAbort(armP arm) {

    // restore normal data domain for this mode (in case this abort is a result
    // of LDRT or STRT, for example)
    if(arm->restoreDomain) {
        armVMRestoreNormalDataDomain(arm);
    }

    doException(arm, ARM_CPSR_ABORT, AE_DataAbort, getPC(arm)+8, True);
}

//
// Perform actions required for prefetch abort
//
inline static void doPAbort(armP arm) {
    doException(arm, ARM_CPSR_ABORT, AE_PrefetchAbort, getPC(arm)+4, True);
}

//
// Do breakpoint exception (prefetch abort)
//
void armBKPT(armP arm, Uns32 thisPC) {
    doException(arm, ARM_CPSR_ABORT, AE_PrefetchAbort, thisPC+4, True);
}

//
// Do software exception
//
void armSWI(armP arm, Uns32 thisPC) {
    Uns32 bytes = IN_THUMB_MODE(arm) ? 2 : 4;
    doException(arm, ARM_CPSR_SUPERVISOR, AE_SupervisorCall, thisPC+bytes, False);
}

//
// Do undefined instruction exception
//
void armUndefined(armP arm, Uns32 thisPC, Uns32 setDEX) {

    Uns32 bytes = IN_THUMB_MODE(arm) ? 2 : 4;

    if (FPEXC_DEX_PRESENT(arm)) {
        SDFP_FIELD(arm, FPEXC, DEX) = (setDEX != 0);
    }

    doException(arm, ARM_CPSR_UNDEFINED, AE_Undefined, thisPC+bytes, False);
}

//
// Do data/prefetch abort exception
//
void armMemoryAbort(
    armP    arm,
    Uns32   faultStatusD,
    Uns32   faultAddress,
    memPriv priv
) {
    // extract fault status and domain from faultStatusD
    armFaultStatus faultStatus = faultStatusD &  ARM_FAULT_STATUS_MASK;
    Uns8           domain      = faultStatusD >> ARM_DOMAIN_SHIFT;

    if(arm->useARMv5FSR) {

        // update ARMv5 FSR and FAR registers (NOTE: it is implementation-
        // dependent whether this should happen for a prefetch abort)
        CP_FIELD(arm, FSR, Status)  = faultStatus;
        CP_FIELD(arm, FAR, Address) = faultAddress;

        // update domain unless invalid
        if(domain != INVALID_DOMAIN) {
            CP_FIELD(arm, FSR, Domain) = domain;
        }

    } else if(priv==MEM_PRIV_X) {

        // update ARMv6 IFSR and IFAR registers
        CP_FIELD(arm, IFSR, Status30) = faultStatus;
        CP_FIELD(arm, IFSR, Status4)  = (faultStatus>>4);
        CP_FIELD(arm, IFSR, Ext)      = 0;
        CP_FIELD(arm, IFAR, Address)  = faultAddress;

    } else {

        // update ARMv6 DFSR and DFAR registers
        CP_FIELD(arm, DFSR, Status30) = faultStatus;
        CP_FIELD(arm, DFSR, Status4)  = (faultStatus>>4);
        CP_FIELD(arm, DFSR, RW)       = (priv==MEM_PRIV_W);
        CP_FIELD(arm, DFSR, Ext)      = 0;
        CP_FIELD(arm, DFAR, Address)  = faultAddress;

        // update domain unless invalid
        if(domain != INVALID_DOMAIN) {
            CP_FIELD(arm, DFSR, Domain) = domain;
        }
    }

    // do memory abort actions
    if(priv==MEM_PRIV_X) {
        doPAbort(arm);
    } else {
        doDAbort(arm);
    }
}

//
// Do external memory abort exception
//
void armExternalMemoryAbort(
    armP    arm,
    Uns32   faultAddress,
    memPriv priv
) {
    armMemoryAbort(arm, AFS_SynchronousExternal, faultAddress, priv);

    // indicate that this is an external memory abort
    if(priv==MEM_PRIV_X) {
        CP_FIELD(arm, IFSR, Ext) = 1;
    } else {
        CP_FIELD(arm, DFSR, Ext) = 1;
    }
}

//
// Do reset exception
//
void armReset(armP arm, Bool intMask) {

    armCPSR CPSR = {{.reg = armReadCPSR(arm)}};

    // update CPSR (possibly switching endianness)
    WRITE_CPSR_EXCEPT(arm, CPSR, ARM_CPSR_SUPERVISOR, intMask, intMask, intMask);

    // reset coprocessors
    armCpReset(arm);

    // reset SIMD/VFP
    // TODO: should be NEONRESET?
    armFPReset(arm);

    // reset VM structures
    armVMReset(arm);

    // initially the processor is not in exclusive access mode
    arm->exclusiveTag = ARM_NO_TAG;

    // jump to reset vector
    jumpToExceptionVector(arm, AE_Reset);
}


////////////////////////////////////////////////////////////////////////////////
// PRIVILEGE, ALIGNMENT AND FETCH EXCEPTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Generate assertion message of read/write error
//
static void privilegeAssertion(
    armP    arm,
    Uns32   faultAddr,
    Uns8    bytes,
    memPriv priv
) {
    Uns32       thisPC = getPC(arm);
    const char *type;

    // get description of type of access
    switch(priv) {
        case MEM_PRIV_R: type = "read";   break;
        case MEM_PRIV_W: type = "write";  break;
        case MEM_PRIV_X: type = "fetch";  break;
        default:         type = "access"; break;
    }

    // generate assertion
    vmiMessage("a", CPU_PREFIX"_PAX",
        SRCREF_FMT "no access for %u-byte %s at address 0x%08x",
        SRCREF_ARGS(arm, thisPC),
        bytes, type, faultAddr
    );
}

//
// Read privilege exception handler
//
VMI_RD_PRIV_EXCEPT_FN(armRdPrivExceptionCB) {

    armP        arm       = (armP)processor;
    Uns32       faultAddr = (Uns32)address;
    memPriv     priv      = MEM_PRIV_R;
    armVMAction action    = armVMMiss(arm, priv, faultAddr, bytes, True);

    if(action == MA_OK) {

        // if memory is successfully mapped, redo the read
        memDomainP domain = vmirtGetProcessorDataDomain(processor);
        vmirtReadNByteDomain(domain, faultAddr, value, bytes, 0, True);

    } else if(action == MA_BAD) {

        // memory mapping failed and not a simulated exception
        privilegeAssertion(arm, faultAddr, bytes, priv);
    }
}

//
// Write privilege exception handler
//
VMI_WR_PRIV_EXCEPT_FN(armWrPrivExceptionCB) {

    armP        arm       = (armP)processor;
    Uns32       faultAddr = (Uns32)address;
    memPriv     priv      = MEM_PRIV_W;
    armVMAction action    = armVMMiss(arm, priv, faultAddr, bytes, True);

    if(action == MA_OK) {

        // if memory is successfully mapped, redo the write
        memDomainP domain = vmirtGetProcessorDataDomain(processor);
        vmirtWriteNByteDomain(domain, faultAddr, value, bytes, 0, True);

    } else if(action == MA_BAD) {

        // memory mapping failed and not a simulated exception
        privilegeAssertion(arm, faultAddr, bytes, priv);
    }
}

//
// Return FaultStatus for misaligned read or write
//
static Uns32 getAlignFaultStatus(armP arm, Uns8 bytes) {

    armFaultStatus faultStatus;

    if(!arm->useARMv5FSR) {
        faultStatus = AFS_Alignment;
    } else if(bytes==2) {
        faultStatus = AFS_Alignment1;
    } else {
        faultStatus = AFS_Alignment3;
    }

    return FAULT_STATUS(faultStatus, INVALID_DOMAIN);
}

//
// Return action to be performed for an unaligned access in an instruction
//
inline static armUnalignedAction getUnalignedAction(armP arm) {
    armInstructionInfo info;
    arm->itStateMT = arm->itStateRT;
    armDecode(arm, getPC(arm), &info);
    return info.ua;
}

//
// Read alignment exception handler - may take an exception, snap read address
// and rotate the value, or snap read address without rotating the read value
//
VMI_RD_ALIGN_EXCEPT_FN(armRdAlignExceptionCB) {

    armP               arm = (armP)processor;
    armUnalignedAction ua;

    if(ALIGN_ENABLED(arm) || ((ua=getUnalignedAction(arm))==ARM_UA_DABORT)) {

        // read aborts
        Uns32 faultStatus = getAlignFaultStatus(arm, bytes);
        armMemoryAbort(arm, faultStatus, address, MEM_PRIV_R);
        return 0;

    } else if((ua==ARM_UA_ROTATE) && arm->configInfo.rotateUnaligned) {

        // read snaps address and loads rotated value
        Uns32 rotate = address&(bytes-1);

        if(getEndian(arm)==MEM_ENDIAN_LITTLE) {
            rotate = bytes - rotate;
        }

        return MEM_SNAP(bytes, rotate*8);
    }

    // read snaps address
    return MEM_SNAP(bytes, 0);
}

//
// Write alignment exception handler - may take an exception or snap the write
// address
//
VMI_WR_ALIGN_EXCEPT_FN(armWrAlignExceptionCB) {

    armP arm = (armP)processor;

    if(ALIGN_ENABLED(arm) || (getUnalignedAction(arm)==ARM_UA_DABORT)) {
        Uns32 faultStatus = getAlignFaultStatus(arm, bytes);
        armMemoryAbort(arm, faultStatus, address, MEM_PRIV_W);
        return 0;
    }

    // write snaps address
    return MEM_SNAP(bytes, 0);
}

//
// Validate instruction fetch from the passed address
//
static Bool validateFetchAddress(
    armP  arm,
    Uns32 thisPC,
    Uns32 snap,
    Bool  complete
) {
    vmiProcessorP processor = (vmiProcessorP)arm;
    armVMAction   action;

    if(vmirtIsExecutable(processor, thisPC)) {

        // no exception pending
        return True;

    } else if((action=armVMMiss(arm, MEM_PRIV_X, thisPC, 2, complete)) == MA_EXCEPTION) {

        // permission exception of some kind, handled by armVMMiss, so no
        // further action required here.
        return False;

    } else if(action == MA_BAD) {

        // memory mapping failed and not a simulated exception
        privilegeAssertion(arm, thisPC, snap+1, MEM_PRIV_X);
        return True;

    } else {

        // no exception pending
        return True;
    }
}

//
// This is called by the simulator when fetching from an instruction address.
// It gives the model an opportunity to take an exception instead.
//
VMI_IFETCH_FN(armIFetchExceptionCB) {

    armP  arm     = (armP)processor;
    Uns32 thisPC  = (Uns32)address;
    Uns32 snap    = IN_THUMB_MODE(arm) ? 1 : 3;
    Bool  fetchOK;

    if(fiqPendingAndEnabled(arm)) {
        // FIQ exception pending
        fetchOK = False;
        // update registers to complete exception if required
        if(complete) {
            doException(arm, ARM_CPSR_FIQ, AE_FIQ, thisPC+4, True);
            arm->CPSR.fields.F = 1;
            arm->CPSR.fields.I = 1;
        }
    } else if(irqPendingAndEnabled(arm)) {

        // IRQ exception pending
        fetchOK = False;

        // update registers to complete exception if required
        if(complete) {
            doException(arm, ARM_CPSR_IRQ, AE_IRQ, thisPC+4, True);
            arm->CPSR.fields.I = 1;
        }

    } else if(!validateFetchAddress(arm, thisPC, snap, complete)) {

        // fetch exception (handled in validateFetchAddress)
        fetchOK = False;

    } else if((thisPC+2) & (MIN_PAGE_SIZE-1)) {

        // simPC isn't two bytes before page end - success
        fetchOK = True;

    } else if(armGetInstructionSize(arm, thisPC) <= 2) {

        // instruction at simPC is a one-byte (Jazelle) or two-byte instruction
        fetchOK = True;

    } else if(!validateFetchAddress(arm, thisPC+2, snap, complete)) {

        // fetch exception (handled in validateFetchAddress)
        fetchOK = False;

    } else {

        // no exception
        fetchOK = True;
    }

    // return appropriate result
    if(fetchOK) {
        return VMI_FETCH_NONE;
    } else if(complete) {
        return VMI_FETCH_EXCEPTION_COMPLETE;
    } else {
        return VMI_FETCH_EXCEPTION_PENDING;
    }
}

//
// This is called by the simulator on a simulated arithmetic exception
//
VMI_ARITH_EXCEPT_FN(armArithExceptionCB) {

    armP arm = (armP)processor;

    if(exceptionContext==VMI_EXCEPT_CXT_CALL) {

        // not expecting any arithmetic exceptions in calls from morphed code
        return VMI_NUMERIC_UNHANDLED;

    } else switch(exceptionType) {

        case VMI_INTEGER_DIVIDE_BY_ZERO:
            // handle divide-by-zero
            if(CP_FIELD(arm, SCTLR, DZ)) {
                armUndefined(arm, getPC(arm), False);
            } else {
                arm->regs[arm->divideTarget] = 0;
            }
            return VMI_NUMERIC_ABORT;

        case VMI_INTEGER_OVERFLOW:
            // handle overflow (MIN_INT / -1)
            arm->regs[arm->divideTarget] = 0x80000000;
            return VMI_NUMERIC_ABORT;

        default:
            // not expecting any other arithmetic exception types
            return VMI_NUMERIC_UNHANDLED;
    }
}

//
// This is called to perform an exception-like branch to a ThumbEE check handler
//
static void doCheckHandler(armP arm, Uns32 handler) {

    arm->itStateRT        = 0;
    arm->regs[ARM_REG_LR] = (getPC(arm)+ARM_PC_DELTA(arm)) | IN_THUMB_MODE(arm);

    vmirtSetPC((vmiProcessorP)arm, CP_REG_UNS32(arm, TEEHBR)+handler);
}

//
// This is called to perform an exception-like branch to the null handler
//
void armCheckHandlerNull(armP arm) {
    doCheckHandler(arm, -4);
}

//
// This is called to perform an exception-like branch to the array handler
//
void armCheckHandlerArray(armP arm) {
    doCheckHandler(arm, -8);
}


////////////////////////////////////////////////////////////////////////////////
// EXTERNAL INTERRUPTS AND EVENTS
////////////////////////////////////////////////////////////////////////////////

//
// Halt the passed processor
//
static void haltProcessor(armP arm, armDisableReason reason) {

    if(!arm->disable) {
        vmirtHalt((vmiProcessorP)arm);
    }

    arm->disable |= reason;
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

//
// Callback to send event to one processor in a cluster
//
static VMI_SMP_ITER_FN(doSEVCB) {

    armP arm = (armP)processor;

    if(!isLeaf(arm)) {

        // no action unless this is a leaf

    } else if(inWFE(arm)) {

        // restart any processor waiting for the event (NOTE: it is
        // implementation defined whether the event register is clear after this
        // point: this model *does* leave it clear)
        restartProcessor(arm, AD_WFE);

    } else {

        // set event register
        arm->event = 1;
    }
}

//
// Send event to all processors in this cluster
//
void armDoSEV(armP arm) {
    vmirtIterAllDescendants((vmiProcessorP)getClusterRoot(arm), doSEVCB, 0);
}

//
// Take processor interrupt (may require processor restart)
//
static void doInterrupt(armP arm) {

    // wake up the processor if required
    restartProcessor(arm, AD_WFE|AD_WFI);

    // take the interrupt on the next instruction
    if(!arm->disable) {
        vmirtDoSynchronousInterrupt((vmiProcessorP)arm);
    }
}

//
// Called by the simulator when an external reset is raised
//
static VMI_NET_CHANGE_FN(externalReset) {

    armInterruptInfoP ii  = userData;
    armP              arm = ii->proc;

    // raise or lower interrupt line
    if(newValue) {

        // halt the processor while reset goes high
        haltProcessor(arm, AD_RESET);

    } else {

        // restart the processor when reset goes low
        restartProcessor(arm, AD_RESET);

        // reset the processor
        armReset(arm, True);

        // take interrupt
        doInterrupt(arm);
    }
}

//
// Perform actions on a possible change to IRQ/FIQ state
//
void armRefreshInterrupt(armP arm) {

    // if this processor is a member of an MPCore cluster, refresh internalInt
    // based on the current CPU interface register settings
    if(arm->mpGlobals) {
    	armMPRefreshBypass(arm);
    } else {
        arm->internalInt = arm->externalInt;
    }

    // take interrupt if it is pending and enabled
    if(armInterruptPending(arm)) {
        doInterrupt(arm);
    }
}

//
// Common routine for external IRQ/FIQ change
//
inline static void externalInterrupt(armP arm, Uns32 newValue, Uns32 mask) {

    // raise or lower interrupt line
    if(!newValue) {
        arm->externalInt &= ~mask;
    } else {
        arm->externalInt |= mask;
    }

    // refresh internal interrupt state
    armRefreshInterrupt(arm);
}

//
// Called by the simulator when an external FIQ is raised
//
static VMI_NET_CHANGE_FN(externalFIQ) {

    armInterruptInfoP ii  = userData;
    armP              arm = ii->proc;

    externalInterrupt(arm, newValue, FIQ_MASK);
}

//
// Called by the simulator when an external IRQ is raised
//
static VMI_NET_CHANGE_FN(externalIRQ) {

    armInterruptInfoP ii  = userData;
    armP              arm = ii->proc;

    externalInterrupt(arm, newValue, IRQ_MASK);
}

//
// Called by the simulator when an external memory prefetch abort is raised
//
static VMI_NET_CHANGE_FN(externalPAbort) {

    armInterruptInfoP ii  = userData;
    armP              arm = ii->proc;

    if(newValue) {
        doPAbort(arm);
    }
}

//
// Called by the simulator when an external memory data abort is raised
//
static VMI_NET_CHANGE_FN(externalDAbort) {

    armInterruptInfoP ii  = userData;
    armP              arm = ii->proc;

    if(newValue) {
        doDAbort(arm);
    }
}

////////////////////////////////////////////////////////////////////////////////
// NET PORTS
////////////////////////////////////////////////////////////////////////////////

//
// Connect up CPU nets
//
void armConnectCPUNets(armP cpu) {
    armAddNetInputPort(cpu, "fiq",    externalFIQ,    0, "Fast external interrupt"      );
    armAddNetInputPort(cpu, "irq",    externalIRQ,    0, "External interrupt"           );
    armAddNetInputPort(cpu, "reset",  externalReset,  0, "Processor reset (active high)");
    armAddNetInputPort(cpu, "pabort", externalPAbort, 0, "Prefetch abort"               );
    armAddNetInputPort(cpu, "dabort", externalDAbort, 0, "Data abort"                   );
    armAddNetOutputPort(cpu, "brarch", &cpu->brarch, "MURAC Branch Architecture" );
}


