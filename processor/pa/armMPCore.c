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
#include <stdlib.h>

// Imperas header files
#include "hostapi/impAlloc.h"

// VMI header files
#include "vmi/vmiMessage.h"
#include "vmi/vmiRt.h"

// model header files
#include "armConfig.h"
#include "armCPRegisters.h"
#include "armExceptions.h"
#include "armFunctions.h"
#include "armMessage.h"
#include "armMPCore.h"
#include "armMPCoreRegisters.h"
#include "armStructure.h"
#include "armTypeRefs.h"


//
// Prefix for messages from this module
//
#define CPU_PREFIX "ARM_MP"


////////////////////////////////////////////////////////////////////////////////
// STRUCTURES
////////////////////////////////////////////////////////////////////////////////

//
// This code is used to identify spurious interrupts
//
#define MP_SPURIOUS_INT 1023

//
// Define number of SGI interrupts and a mask for PPI and SGI interrupts
//
#define SGI_NUM         16
#define MP_MASK_SGI     ((1<<SGI_NUM)-1)
#define MP_MASK_PPI     0xf8000000
#define MP_MASK_PPI_SGI (MP_MASK_PPI|MP_MASK_SGI)

//
// Define masks for IRQ and FIQ interrupts in pending interrupt register
//
#define MP_MASK_GT      (1<<27)
#define MP_MASK_FIQ     (1<<28)
#define MP_MASK_PT      (1<<29)
#define MP_MASK_WT      (1<<30)
#define MP_MASK_IRQ     (1<<31)

//
// Define initial value for ICDICFR[0] and ICDICFR[1]
//
#define MP_INITIAL_ICDICFR0 0xaaaaaaaa
#define MP_INITIAL_ICDICFR1 0x7dc00000
#define MP_INITIAL_ICDICFRN 0x55555555

//
// This is the size of the MPCore memory-mapped control block
//
#define ARM_MP_BLOCK_SIZE 8192

//
// Accesses to this range cause memory abort exceptions
//
#define MP_ABORT_LOW  0x0700
#define MP_ABORT_HIGH 0x0fff

//
// This is the maximum number of interrupts which can be configured
//
#define MAX_INTERRUPTS 256

//
// Use these to map from interrupt number to word and mask, and from word and
// offset to interrupt number
//
#define INTERRUPTS_PER_WORD     32
#define INT_WORDS_MAX           (MAX_INTERRUPTS/INTERRUPTS_PER_WORD)
#define INT_WORDS_LOCAL         1
#define INT_WORDS_GLOBAL        (INT_WORDS_MAX-INT_WORDS_LOCAL)
#define INT_WORDS_LOCALx2       (2*INT_WORDS_LOCAL)
#define INT_WORDS_GLOBALx2      (2*INT_WORDS_GLOBAL)
#define INT_WORDS_LOCALx8       (8*INT_WORDS_LOCAL)
#define INT_WORDS_GLOBALx8      (8*INT_WORDS_GLOBAL)
#define INTERRUPT_WORD(_B)      ((_B)/INTERRUPTS_PER_WORD)
#define INTERRUPT_MASK(_B)      (1 << ((_B) & (INTERRUPTS_PER_WORD-1)))
#define INTERRUPT_INDEX(_W, _O) (((_W)*INTERRUPTS_PER_WORD) + _O)

//
// These are the two magic numbers required for watchdog reset
//
#define RESET_MAGIC1 0x12345678
#define RESET_MAGIC2 0x87654321

//
// This enumerates local timer types (32-bit, countdown)
//
typedef enum armLTimerTypeE {
    LT_PRIVATE,     // private timer
    LT_WATCHDOG,    // watchdog timer
    LT_LAST         // KEEP LAST: for sizing
} armLTimerType;

//
// local timer control register (private & watchdog timers)
//
typedef struct armLTControl {
    Uns32 Enable     :  1;
    Uns32 AutoReload :  1;
    Uns32 ITEnable   :  1;
    Uns32 WDMode     :  1;
    Uns32 _u1        :  4;
    Uns32 Prescaler  :  8;
    Uns32 _u2        : 16;
} armLTControl;

//
// global timer control register
//
typedef struct armGTControl {
    Uns32 TimerEnable   :  1;
    Uns32 CompEnable    :  1;
    Uns32 ITEnable      :  1;
    Uns32 AutoIncrement :  1;
    Uns32 _u1           :  4;
    Uns32 Prescaler     :  8;
    Uns32 _u2           : 16;
} armGTControl;

//
// local timer (private & watchdog timers)
//
typedef struct armLTimerS {
    // memory-mapped register values
    armLTControl   control;     // control register
    Uns32          load;        // load register
    Uns32          counter;     // counter register
    Bool           event;       // interrupt status event
    Bool           reset;       // is reset active?
    // simulation artifacts
    const char    *name;        // timer name
    vmiModelTimerP vmiTimer;    // equivalent VMI timer
    Uns32          partial;     // instruction count through current cycle
    Uns32          scale;       // scale factor
    Uns32          intMask;     // mask to use when timer generates interrupt
    Uns64          base;        // instruction count when timer explicitly set
    Uns64          period;      // timer period
    Bool           running;     // is timer active?
    Bool           inDisable;   // is disable sequence active?
} armLTimer, *armLTimerP;

//
// global timer (global section)
//
typedef struct armGTimerGS {
    // memory-mapped register values
    Uns8           Prescaler;   // global control.Prescaler value
    Bool           TimerEnable; // global control.TimerEnable value
    // simulation artifacts
    Uns32          scale;       // scale factor
} armGTimerG, *armGTimerGP;

//
// global timer (local section)
//
typedef struct armGTimerS {
    // memory-mapped register values
    armGTControl   control;     // control register
    Uns64          counter;     // counter register
    Uns64          comparator;  // comparator register
    Uns32          autoinc;     // auto-increment register
    Bool           event;       // interrupt status event
    // simulation artifacts
    vmiModelTimerP vmiTimer;    // equivalent VMI timer
    Uns32          partial;     // instruction count through current cycle
    Uns32          intMask;     // mask to use when timer generates interrupt
    Uns64          base;        // instruction count when timer explicitly set
    Bool           reschedule;  // is reschedule required on expiry?
} armGTimer, *armGTimerP;

//
// MPCore global register block
//
typedef struct armMPGlobalsS {
    // configuration fields
    armP       root;
    memDomainP physicalDomain;
    memDomainP mpDomain;
    memDomainP controlDomain;
    Uns32      lowAddr;
    // memory-mapped register values
    armMPGRegs regValues;
    // non-memory-mapped register values
    Uns32      ICDGRPR[INT_WORDS_GLOBAL];       // interrupt groups
    Uns32      ICDIER[INT_WORDS_GLOBAL];        // enabled interrupts
    Uns32      ICDIPR[INT_WORDS_GLOBAL];        // pending registers
    Uns32      ICDIPRI[INT_WORDS_GLOBALx8];     // interrupt priorities
    Uns32      ICDIPTR[INT_WORDS_GLOBALx8];     // processor targets
    Uns32      ICDICFR[INT_WORDS_GLOBALx2];     // interrupt configuration
    Uns32      pending[INT_WORDS_GLOBAL];       // pending interrupts
    Uns32      active[INT_WORDS_GLOBAL];        // active interrupts
    Uns32      externalSPI[INT_WORDS_GLOBAL];   // external SPI values
    // simulation artifacts
    Uns32      edgeMask[INT_WORDS_GLOBAL];      // edge-sensitive interrupt mask
    Uns8       priMask;
    // timers
    armGTimerG globalTimer;                     // global timer (global section)
} armMPGlobals;

//
// MPCore local register block
//
typedef struct armMPLocalsS {
    // configuration fields
    armP       arm;
    // memory-mapped register values
    armMPLRegs regValues;
    // non-memory-mapped register values
    Uns32      ICDGRPR[INT_WORDS_LOCAL];        // interrupt groups
    Uns32      ICDIER[INT_WORDS_LOCAL];         // enabled interrupts
    Uns32      ICDIPR[INT_WORDS_LOCAL];         // pending registers
    Uns32      ICDIPRI[INT_WORDS_LOCALx8];      // interrupt priorities
    Uns32      ICDIPTR[INT_WORDS_LOCALx8];      // processor targets
    Uns32      ICDICFR[INT_WORDS_LOCALx2];      // interrupt configuration
    Uns32      pending[INT_WORDS_LOCAL];        // pending interrupts
    Uns32      active[INT_WORDS_LOCAL];         // active interrupts
    Uns32      externalSPI[INT_WORDS_LOCAL];    // dummy SPI values
    // simulation artifacts
    Uns32      edgeMask[INT_WORDS_LOCAL];       // edge-sensitive interrupt mask
    Uns8       cpuSGI[SGI_NUM];                 // CPU triggering each SGI
    Uns8       useGICInt;                       // select GIC/legacy sources
    // timers
    armLTimer  localTimers[LT_LAST];            // local timers
    armGTimer  globalTimer;                     // global timer (local section)
} armMPLocals;


////////////////////////////////////////////////////////////////////////////////
// UTILITIES
////////////////////////////////////////////////////////////////////////////////

//
// Is the processor in big endian mode?
//
inline static Bool isBigEndian(armP arm) {
    return armGetEndian((vmiProcessorP)arm, False) == MEM_ENDIAN_BIG;
}

//
// Return the highest address that can be represented with the passed bits
//
inline static Uns64 getHighAddress(Uns32 bits) {
    return (1ULL<<bits)-1;
}

//
// Return any parent of the passed processor
//
inline static armP getParent(armP arm) {
    return (armP)vmirtGetSMPParent((vmiProcessorP)arm);
}

//
// Return any child of the passed processor
//
inline static armP getChild(armP arm) {
    return (armP)vmirtGetSMPChild((vmiProcessorP)arm);
}

//
// Return the first CPU in the MPCore cluster
//
inline static armP getFirstCPU(armP arm) {
    return getChild(arm->mpGlobals->root);
}

//
// Return any next sibling of the passed processor
//
inline static armP getNextSibling(armP arm) {
    return (armP)vmirtGetSMPNextSibling((vmiProcessorP)arm);
}

//
// Return index number of the passed CPU
//
inline static Uns32 getIndex(armP arm) {
    return CP_FIELD(arm, MPIDR, AffinityLevel0);
}

//
// Is this a multiprocessor implementation?
//
inline static Bool isMultiprocessor(armP arm) {
    return (arm->configInfo.numCPUs>1);
}

//
// Return the MPCore cluster root
//
static armP getClusterRoot(armP arm) {

    armP parent;

    while((parent=getParent(arm))) {
        arm = parent;
    }

    return arm;
}

//
// Return the indexed local timer
//
static armLTimerP getLT(armP arm, armLTimerType type) {
    VMI_ASSERT(type<LT_LAST, "illegal timer index");
    return &arm->mpLocals->localTimers[type];
}

//
// Return global part of the global timer
//
inline static armGTimerGP getGTG(armP arm) {
    return &arm->mpGlobals->globalTimer;
}

//
// Return local part of the global timer
//
inline static armGTimerP getGT(armP arm) {
    return &arm->mpLocals->globalTimer;
}

//
// Is the passed local timer raising a PPI?
//
inline static Bool raisePPILT(armLTimerP timer) {
    return timer->event && timer->control.ITEnable && !timer->control.WDMode;
}

//
// Is the passed global timer raising a PPI?
//
inline static Bool raisePPIGT(armGTimerP timer) {
    return timer->event && timer->control.ITEnable;
}

//
// Write a value to the passed net
//
inline static void writeNet(armP arm, Uns32 netHandle, Uns32 value) {
    if(netHandle) {
        vmirtWriteNetPort((vmiProcessorP)arm, netHandle, value);
    }
}

//
// Return the 64-bit instruction count for a processor
//
inline static Uns64 getThisICount(armP arm) {
    return vmirtGetICount((vmiProcessorP)arm);
}


////////////////////////////////////////////////////////////////////////////////
// GLOBAL REGISTERS
////////////////////////////////////////////////////////////////////////////////

DEFINE_CS(mpRegAttrs);

//
// MPCore register read callback type
//
#define ARM_MP_READFN(_NAME) Uns32 _NAME( \
    armP       arm,         \
    armMPRegId id,          \
    Uns32      byteOffset   \
)
typedef ARM_MP_READFN((*armMPReadFn));

//
// MPCore register write callback type
//
#define ARM_MP_WRITEFN(_NAME) void _NAME( \
    armP       arm,         \
    armMPRegId id,          \
    Uns32      newValue,    \
    Uns32      byteOffset   \
)
typedef ARM_MP_WRITEFN((*armMPWriteFn));

//
// This defines access constraints to each MPCore register
//
typedef enum mpRegAccessE {
    MPA_NONE,       // no access constraint
    MPA_SAC,        // access constrained by SCU SAC register
} mpRegAccess;

//
// This structure records information about each MPCore register
//
typedef struct mpRegAttrsS {
    const char   *name;
    armMPRegId   id;
    armMPReadFn  readCB;
    armMPWriteFn writeCB;
    Uns32        writeMask;
    Uns32        address;
    Uns32        numWords;
    mpRegAccess  access;
    Bool         isGlobal;
    Bool         byteOK;
} mpRegAttrs;

//
// Macro to initialize single global entry in mpRegInfo
//
#define MPG_ATTR(_ID, _BYTEOK, _ACCESS, _READCB, _WRITECB) \
    {                                               \
        name      : #_ID,                           \
        id        : MPG_ID(_ID),                    \
        readCB    : _READCB,                        \
        writeCB   : _WRITECB,                       \
        writeMask : MP_WRITE_MASK_##_ID,            \
        address   : MP_ADDRESS_##_ID,               \
        numWords  : 1,                              \
        access    : _ACCESS,                        \
        isGlobal  : 1,                              \
        byteOK    : _BYTEOK                         \
    }

//
// Macro to initialize array global entry in mpRegInfo
//
#define MPG_ATTRx1(_ID, _NUM, _BYTEOK, _ACCESS, _READCB, _WRITECB) \
    {                                               \
        name      : #_ID#_NUM,                      \
        id        : MPG_ID(_ID##_NUM),              \
        readCB    : _READCB,                        \
        writeCB   : _WRITECB,                       \
        writeMask : MP_WRITE_MASK_##_ID,            \
        address   : MP_ADDRESS_##_ID +(4*0x##_NUM), \
        numWords  : 1,                              \
        access    : _ACCESS,                        \
        isGlobal  : 1,                              \
        byteOK    : _BYTEOK                         \
    }

//
// Macro to initialize 4 global entries in mpRegInfo
//
#define MPG_ATTRx4(_ID, _BYTEOK, _ACCESS, _READCB, _WRITECB) \
    MPG_ATTRx1(_ID, 0, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 1, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 2, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 3, _BYTEOK, _ACCESS, _READCB, _WRITECB)

//
// Macro to initialize 7 global entries in mpRegInfo
//
#define MPG_ATTRx7(_ID, _BYTEOK, _ACCESS, _READCB, _WRITECB) \
    MPG_ATTRx1(_ID, 0, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 1, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 2, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 3, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 4, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 5, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 6, _BYTEOK, _ACCESS, _READCB, _WRITECB)

//
// Macro to initialize 8 global entries in mpRegInfo
//
#define MPG_ATTRx8(_ID, _BYTEOK, _ACCESS, _READCB, _WRITECB) \
    MPG_ATTRx1(_ID, 0, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 1, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 2, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 3, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 4, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 5, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 6, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 7, _BYTEOK, _ACCESS, _READCB, _WRITECB)

//
// Macro to initialize 16 global entries in mpRegInfo
//
#define MPG_ATTRx16(_ID, _BYTEOK, _ACCESS, _READCB, _WRITECB) \
    MPG_ATTRx1(_ID, 0, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 1, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 2, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 3, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 4, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 5, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 6, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 7, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 8, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 9, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, A, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, B, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, C, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, D, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, E, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, F, _BYTEOK, _ACCESS, _READCB, _WRITECB)

//
// Macro to initialize 64 global entries in mpRegInfo
//
#define MPG_ATTRx64(_ID, _BYTEOK, _ACCESS, _READCB, _WRITECB) \
    MPG_ATTRx1(_ID, 00, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 01, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 02, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 03, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 04, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 05, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 06, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 07, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 08, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 09, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 0A, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 0B, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 0C, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 0D, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 0E, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 0F, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 10, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 11, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 12, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 13, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 14, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 15, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 16, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 17, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 18, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 19, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 1A, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 1B, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 1C, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 1D, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 1E, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 1F, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 20, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 21, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 22, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 23, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 24, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 25, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 26, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 27, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 28, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 29, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 2A, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 2B, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 2C, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 2D, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 2E, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 2F, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 30, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 31, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 32, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 33, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 34, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 35, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 36, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 37, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 38, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 39, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 3A, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 3B, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 3C, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 3D, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 3E, _BYTEOK, _ACCESS, _READCB, _WRITECB), \
    MPG_ATTRx1(_ID, 3F, _BYTEOK, _ACCESS, _READCB, _WRITECB)

//
// Macro to initialize 1-word local entry in mpRegInfo
//
#define MPL_ATTR(_ID, _BYTEOK, _ACCESS, _READCB, _WRITECB) \
    {                                               \
        name      : #_ID,                           \
        id        : MPL_ID(_ID),                    \
        readCB    : _READCB,                        \
        writeCB   : _WRITECB,                       \
        writeMask : MP_WRITE_MASK_##_ID,            \
        address   : MP_ADDRESS_##_ID,               \
        numWords  : 1,                              \
        access    : _ACCESS,                        \
        isGlobal  : 0,                              \
        byteOK    : _BYTEOK                         \
    }

//
// Macro to initialize 2-word local entry in mpRegInfo
//
#define MPL_ATTRD(_ID, _BYTEOK, _ACCESS, _READCB, _WRITECB) \
    {                                               \
        name      : #_ID,                           \
        id        : MPL_ID(_ID),                    \
        readCB    : _READCB,                        \
        writeCB   : _WRITECB,                       \
        writeMask : MP_WRITE_MASK_##_ID,            \
        address   : MP_ADDRESS_##_ID,               \
        numWords  : 2,                              \
        access    : _ACCESS,                        \
        isGlobal  : 0,                              \
        byteOK    : _BYTEOK                         \
    }

//
// Callback for read of register with no effect
//
static ARM_MP_READFN(readIgnore) {
    return 0;
}

//
// Callback for write of register with no effect
//
static ARM_MP_WRITEFN(writeIgnore) {
    // no action
}


////////////////////////////////////////////////////////////////////////////////
// INTERRUPT CONTROLLER
////////////////////////////////////////////////////////////////////////////////

//
// Return word mask created by inserting the passed byteMask into each byte of
// a word for which there is an implemented interrupt
//
static Uns32 getWordMaskxN(
    Uns32 wordxN,
    Uns32 intMask,
    Uns8  value,
    Uns32 fieldWidth
) {
    Uns32 fieldNum     = 32/fieldWidth;
    Uns32 intValidMask = intMask >> (wordxN*fieldNum);
    Uns32 result       = 0;
    Uns32 i;

    for(i=0; i<fieldNum; i++) {
        result |= (intValidMask & (1<<i)) ? (value<<(i*fieldWidth)) : 0;
    }

    return result;
}

//
// Return word mask created by inserting the passed value into each twobit of
// a word for which there is an implemented interrupt
//
inline static Uns32 getWordMaskx2(Uns32 wordx2, Uns32 intMask, Uns8 value) {
    return getWordMaskxN(wordx2, intMask, value, 2);
}

//
// Return word mask created by inserting the passed value into each byte of
// a word for which there is an implemented interrupt
//
inline static Uns32 getWordMaskx8(Uns32 wordx8, Uns32 intMask, Uns8 value) {
    return getWordMaskxN(wordx8, intMask, value, 8);
}

//
// Return priority mask
//
static Uns32 getPriMask(armP arm) {

    Uns32 ICCPMRBits = arm->configInfo.ICCPMRBits;
    Uns32 ROBits     = MP_PRIORITY_BITS-ICCPMRBits;

    return MP_WRITE_MASK_ICCPMR & ~((1<<ROBits)-1);
}

//
// Return reset value for ICDIER register
//
static Uns32 getICDIERReset(armP arm, Uns32 word) {
    if(!word && !arm->configInfo.SGIDisable) {
        return MP_MASK_SGI;
    } else {
        return 0;
    }
}

//
// Return write mask for ICDGRPR register
//
static Uns32 getICDGRPRWriteMask(armP arm, Uns32 word) {
    if(word) {
        return -1;
    } else {
        return MP_MASK_PPI_SGI;
    }
}

//
// Return write mask for ICDIER register
//
static Uns32 getICDIERWriteMask(armP arm, Uns32 word) {
    if(word) {
        return -1;
    } else if(arm->configInfo.SGIDisable) {
        return MP_MASK_PPI_SGI;
    } else {
        return MP_MASK_PPI;
    }
}

//
// Return write mask for ICDIPR register
//
static Uns32 getICDIPRWriteMask(armP arm, Uns32 word) {
    if(word) {
        return -1;
    } else {
        return MP_MASK_PPI;
    }
}

//
// Is the passed interrupt word valid?
//
inline static Bool isValidInterruptWord(armP arm, Uns32 word) {
    return word <= MPG_FIELD(arm, ICDICTR, ITLines);
}

//
// Macro selects either local or global value location in an array of words
// with one bit per interrupt
//
#define RETURN_LOCAL_GLOBAL_WORD(_ARM, _FIELD, _WORD) {             \
                                                                    \
    /* sanity check word in range */                                \
    VMI_ASSERT(                                                     \
        _WORD<INT_WORDS_MAX,                                        \
        "word index %u exceeds maximum %u", _WORD, INT_WORDS_MAX    \
    );                                                              \
                                                                    \
    /* select either local or global value */                       \
    if(_WORD<INT_WORDS_LOCAL) {                                     \
        return &_ARM->mpLocals->_FIELD[_WORD];                      \
    } else {                                                        \
        return &_ARM->mpGlobals->_FIELD[_WORD-INT_WORDS_LOCAL];     \
    }                                                               \
}

//
// Macro selects either local or global value location in an array of words
// with two bits per interrupt
//
#define RETURN_LOCAL_GLOBAL_WORDx2(_ARM, _FIELD, _WORDx2) {         \
                                                                    \
    Uns32 _WORD = _WORDx2/2;                                        \
                                                                    \
    /* sanity check word in range */                                \
    VMI_ASSERT(                                                     \
        _WORD<INT_WORDS_MAX,                                        \
        "word index %u exceeds maximum %u", _WORD, INT_WORDS_MAX    \
    );                                                              \
                                                                    \
    /* select either local or global value */                       \
    if(_WORD<INT_WORDS_LOCAL) {                                     \
        return &_ARM->mpLocals->_FIELD[_WORDx2];                    \
    } else {                                                        \
        return &_ARM->mpGlobals->_FIELD[_WORDx2-INT_WORDS_LOCALx2]; \
    }                                                               \
}

//
// Macro selects either local or global value location in an array of words
// with eight bits per interrupt
//
#define RETURN_LOCAL_GLOBAL_WORDx8(_ARM, _FIELD, _WORDx8) {         \
                                                                    \
    Uns32 _WORD = _WORDx8/8;                                        \
                                                                    \
    /* sanity check word in range */                                \
    VMI_ASSERT(                                                     \
        _WORD<INT_WORDS_MAX,                                        \
        "word index %u exceeds maximum %u", _WORD, INT_WORDS_MAX    \
    );                                                              \
                                                                    \
    /* select either local or global value */                       \
    if(_WORD<INT_WORDS_LOCAL) {                                     \
        return &_ARM->mpLocals->_FIELD[_WORDx8];                    \
    } else {                                                        \
        return &_ARM->mpGlobals->_FIELD[_WORDx8-INT_WORDS_LOCALx8]; \
    }                                                               \
}

//
// Macro returns field of width _BITS extracted from the correct word obtained
// by _GET_WORD_FN
//
#define GET_INT_FIELD(_ARM, _INTNUM, _BITS, _GET_WORD_FN) { \
                                                            \
    Uns32 fieldsPerWord = 32/(_BITS);                       \
    Uns32 wordxN        = (_INTNUM)/fieldsPerWord;          \
    Uns32 offset        = (_INTNUM)%fieldsPerWord;          \
    Uns32 shift         = offset*(_BITS);                   \
    Uns32 mask          = (1<<(_BITS))-1;                   \
    Uns32 value         = *_GET_WORD_FN(_ARM, wordxN);      \
                                                            \
    return (value>>shift) & mask;                           \
}

//
// Return a pointer to the indexed ICDGRPR word (may be local or global)
//
static Uns32 *getICDGRPR(armP arm, Uns32 word) {
    RETURN_LOCAL_GLOBAL_WORD(arm, ICDGRPR, word);
}

//
// Return a pointer to the indexed ICDIER word (may be local or global)
//
static Uns32 *getICDIER(armP arm, Uns32 word) {
    RETURN_LOCAL_GLOBAL_WORD(arm, ICDIER, word);
}

//
// Return a pointer to the indexed ICDIPR word (may be local or global)
//
static Uns32 *getICDIPR(armP arm, Uns32 word) {
    RETURN_LOCAL_GLOBAL_WORD(arm, ICDIPR, word);
}

//
// Return a pointer to the indexed pending word (may be local or global)
//
static Uns32 *getPending(armP arm, Uns32 word) {
    RETURN_LOCAL_GLOBAL_WORD(arm, pending, word);
}

//
// Return a pointer to the indexed active word (may be local or global)
//
static Uns32 *getActive(armP arm, Uns32 word) {
    RETURN_LOCAL_GLOBAL_WORD(arm, active, word);
}

//
// Return a pointer to the indexed edgeMask word (may be local or global)
//
static Uns32 *getEdgeMask(armP arm, Uns32 word) {
    RETURN_LOCAL_GLOBAL_WORD(arm, edgeMask, word);
}

//
// Return a pointer to the indexed external SPI word (may be local or global,
// but local value is a placeholder and always zero)
//
static Uns32 *getExternalSPI(armP arm, Uns32 word) {
    RETURN_LOCAL_GLOBAL_WORD(arm, externalSPI, word);
}

//
// Return a pointer to the indexed ICDIPRIR word (may be local or global)
//
static Uns32 *getICDIPRI(armP arm, Uns32 wordx8) {
    RETURN_LOCAL_GLOBAL_WORDx8(arm, ICDIPRI, wordx8);
}

//
// Return a pointer to the indexed ICDIPTR word (may be local or global)
//
static Uns32 *getICDIPTR(armP arm, Uns32 wordx8) {
    RETURN_LOCAL_GLOBAL_WORDx8(arm, ICDIPTR, wordx8);
}

//
// Return a pointer to the indexed ICDICFR word (may be local or global)
//
static Uns32 *getICDICFR(armP arm, Uns32 wordx2) {
    RETURN_LOCAL_GLOBAL_WORDx2(arm, ICDICFR, wordx2);
}

//
// Get the group for the indexed interrupt on the passed processor
//
static Uns32 getIntGroup(armP arm, Uns32 intNum) {
    GET_INT_FIELD(arm, intNum, 1, getICDGRPR);
}

//
// Get the target list for the indexed interrupt on the passed processor - note
// that in a uniprocessor implementation, the local CPU is always targeted
//
static Uns32 getIntTargetList(armP arm, Uns32 intNum) {
    if(isMultiprocessor(arm)) {
        GET_INT_FIELD(arm, intNum, 8, getICDIPTR);
    } else {
        return 1;
    }
}

//
// Get the priority for the indexed interrupt on the passed processor
//
static Uns32 getIntPriority(armP arm, Uns32 intNum) {
    GET_INT_FIELD(arm, intNum, 8, getICDIPRI);
}

//
// Return the configuration
//
static Uns32 getIntConfig(armP arm, Uns32 intNum) {
    GET_INT_FIELD(arm, intNum, 2, getICDICFR);
}

//
// Is interrupt intNum edge-sensitive?
//
inline static Bool isEdgeSensitive(armP arm, Uns32 intNum) {
    return (getIntConfig(arm, intNum)&2) && True;
}

//
// Return the group priority for the passed priority
//
inline static Uns32 groupPriority(armP arm, Uns32 priority) {
    return priority & (0xfe << MPL_REG_UNS32(arm, ICCBPR));
}

//
// Get FIQ/IRQ mask for the indexed interrupt on the passed processor
//
static Uns32 getIntMask(armP arm, Uns32 intNum) {
    if(getIntGroup(arm, intNum)) {
        return IRQ_MASK;
    } else if(MPL_FIELD(arm, ICCICR, FIQEn)) {
        return FIQ_MASK;
    } else {
        return IRQ_MASK;
    }
}

//
// Is forwarding of the interrupt enabled?
//
static Bool isIntForwardingEnabled(armP arm, Uns32 intNum) {
    if(getIntGroup(arm, intNum)) {
        return MPG_FIELD(arm, ICDDCR, EnableGrp1);
    } else {
        return MPG_FIELD(arm, ICDDCR, EnableGrp0);
    }
}

//
// Is handling of the interrupt enabled?
//
static Bool isIntHandlingEnabled(armP arm, Uns32 intNum) {
    if(getIntGroup(arm, intNum)) {
        return MPL_FIELD(arm, ICCICR, EnableGrp1);
    } else {
        return MPL_FIELD(arm, ICCICR, EnableGrp0);
    }
}

//
// Refresh the plain mask of edge-triggered interrupt states when interrupt
// configuration changes
//
static void refreshEdgeMask(armP arm, Uns32 word) {

    Uns32 intBase  = word*32;
    Uns32 edgeMask = 0;
    Int32 i;

    for(i=31; i>=0; i--) {
        edgeMask = (edgeMask<<1) + isEdgeSensitive(arm, intBase+i);
    }

    *getEdgeMask(arm, word) = edgeMask;
}

//
// Return a mask of external level-sensitive SPI signals that are active
//
static Uns32 getPendingSPIWord(armP arm, Uns32 word) {

    Uns32 edgeMask    = *getEdgeMask(arm, word);
    Uns32 externalSPI = *getExternalSPI(arm, word);

    return externalSPI & ~edgeMask;
}

//
// Return the value of a pending interrupt word with the passed word offset
//
static Uns32 getICDIPRWord(armP arm, Uns32 word) {

    Uns32 ICDIPR      = *getICDIPR(arm, word);
    Uns32 externalSPI = getPendingSPIWord(arm, word);
    Uns32 result      = (ICDIPR|externalSPI);

    // if this is a multiprocessor implementation, construct mask of interrupts
    // for which there is at least one target
    if(isMultiprocessor(arm)) {

        Uns32 mask = 0;
        Int32 i;

        for(i=7; i>=0; i--) {

            union {Uns32 u32; Uns8 u8[4];} u = {*getICDIPTR(arm, (word*8)+i)};

            mask <<= 4;

            if(u.u8[0]) mask |= 1;
            if(u.u8[1]) mask |= 2;
            if(u.u8[2]) mask |= 4;
            if(u.u8[3]) mask |= 8;
        }

        // mask raw result to select only interrupts with a processor target
        result &= mask;
    }

    return result;
}

//
// Update interrupt lines on the local CPU when some interrupt controller or
// distributor state has changed
//
static void updateExceptionsLocal(armP arm) {

    Uns32 cpuMask  = 1<<getIndex(arm);
    Uns32 intWords = MPG_FIELD(arm, ICDICTR, ITLines)+1;
    Uns32 runPri   = MP_IDLE_PRIORITY;
    Uns32 pendPri  = -1;
    Uns32 pendNum  = MP_SPURIOUS_INT;
    Uns32 word;

    // process each interrupt word in turn
    for(word=0; word<intWords; word++) {

        // get active and pending interrupt words
        Uns32 activeMask  = *getActive(arm, word);
        Uns32 pendingMask = *getPending(arm, word);
        Uns32 tryMask;
        Uns32 offset;

        // process each interrupt in turn
        for(
            offset=0, tryMask=1;
            activeMask || pendingMask;
            offset++, tryMask<<=1
        ) {
            if(tryMask & activeMask) {

                // interrupt is ACTIVE
                Uns32 intNum = INTERRUPT_INDEX(word, offset);
                Uns32 intPri = getIntPriority(arm, intNum);

                // select the highest-priority running interrupt
                if(runPri>intPri) {
                    runPri = intPri;
                }

            } else if(tryMask & pendingMask) {

                // interrupt is PENDING (and not ACTIVE)
                Uns32 intNum     = INTERRUPT_INDEX(word, offset);
                Uns32 targetMask = getIntTargetList(arm, intNum);

                // is this CPU targeted by the interrupt?
                if(targetMask & cpuMask) {

                    Uns32 intPri = getIntPriority(arm, intNum);

                    // select the highest-priority pending interrupt
                    if((pendPri>intPri) && isIntForwardingEnabled(arm, intNum)) {
                        pendPri = intPri;
                        pendNum = intNum;
                    }
                }
            }

            // remove selected bit from the active and pending masks
            activeMask  &= ~tryMask;
            pendingMask &= ~tryMask;
        }
    }

    // derive CPU number for pending SGI if required
    Uns32 pendCPUID = pendNum<SGI_NUM ? arm->mpLocals->cpuSGI[pendNum] : 0;

    // update running-priority register
    MPL_FIELD(arm, ICCRPR, Priority) = runPri;

    // update highest-priority-pending-interrupt register
    MPL_FIELD(arm, ICCHPIR, InterruptID) = pendNum;
    MPL_FIELD(arm, ICCHPIR, CPUID)       = pendCPUID;

    // behavior depends on whether the running priority indicates an exception
    // is currently active
    Uns32 priMaskPri = MPL_FIELD(arm, ICCPMR, Priority);
    Uns32 GICInt;

    if(priMaskPri <= pendPri) {
        // exception masked by priority mask
        GICInt = 0;
    } else if(!isIntHandlingEnabled(arm, pendNum)) {
        // exception handling disabled
        GICInt = 0;
    } else if(runPri==MP_IDLE_PRIORITY) {
        // no exception is active: always signal the interrupt
        GICInt = getIntMask(arm, pendNum);
    } else if(groupPriority(arm, runPri) <= groupPriority(arm, pendPri)) {
        // exception masked by active exception of equal or greater group priority
        GICInt = 0;
    } else {
        // active exception preemption
        GICInt = getIntMask(arm, pendNum);
    }

    // update interrupt acknowledge register
    if(GICInt) {
        MPL_FIELD(arm, ICCIAR,  InterruptID) = GICInt ? pendNum   : MP_SPURIOUS_INT;
        MPL_FIELD(arm, ICCIAR,  CPUID)       = GICInt ? pendCPUID : 0;
    } else {
        MPL_FIELD(arm, ICCIAR,  InterruptID) = MP_SPURIOUS_INT;
        MPL_FIELD(arm, ICCIAR,  CPUID)       = 0;
    }

    // modify pending interrupts if required
    if(arm->GICInt != GICInt) {
        arm->GICInt = GICInt;
        armRefreshInterrupt(arm);
    }
}

//
// Update interrupt lines on the all CPUs when some interrupt controller or
// distributor state has changed
//
static void updateExceptionsGlobal(armP arm) {
    for(arm=getFirstCPU(arm); arm; arm=getNextSibling(arm)) {
        updateExceptionsLocal(arm);
    }
}

//
// Update interrupt lines on the all CPUs depending on whether the passed word
// index addresses a local or global interrupt
//
static void updateExceptionsLocalOrGlobal(armP arm, Uns32 word) {
    if(word<INT_WORDS_LOCAL) {
        updateExceptionsLocal(arm);
    } else {
        updateExceptionsGlobal(arm);
    }
}

//
// Derive a new pending interrupt value
//
static Uns32 derivePending(armP arm, Uns32 word) {

    Uns32 ICDIER = *getICDIER(arm, word);
    Uns32 ICDIPR =  getICDIPRWord(arm, word);

    return ICDIPR & ICDIER;
}

//
// Update interrupt lines on CPUs when the passed word index into ICDIER or
// ICDIPR has changed
//
static void updateExceptionsIfChanged(armP arm, Uns32 word) {

    Uns32 *pending    = getPending(arm, word);
    Uns32  newPending = derivePending(arm, word);

    if(*pending!=newPending) {
        *pending = newPending;
        updateExceptionsLocalOrGlobal(arm, word);
    }
}

//
// Raise SGI interrupt
//
static void raiseSGI(armP arm, Uns32 sgiID, Uns32 srcID) {

    Uns32  word     = 0;
    Uns32  mask     = (1<<sgiID);
    Uns32 *valuePtr = getICDIPR(arm, word);

    if(!(*valuePtr & mask)) {

        // set pending bit for this SGI
        *valuePtr |= mask;

        // indicate source of SGI
        arm->mpLocals->cpuSGI[sgiID] = srcID;

        // update interrupt lines on individual CPUs
        updateExceptionsIfChanged(arm, word);
    }
}

//
// Raise PPI interrupt
//
static void raisePPI(armP arm, Uns32 mask) {

    Uns32  word     = 0;
    Uns32 *valuePtr = getICDIPR(arm, word);
    Uns32  oldValue = *valuePtr;
    Uns32  newValue = oldValue | mask;

    if(oldValue != newValue) {

        *valuePtr = newValue;

        // update interrupt lines on individual CPUs
        updateExceptionsIfChanged(arm, word);
    }
}

//
// Refresh GIC state when IRQ/FIQ change, if these are routed to the GIC
//
static void refreshFIQIRQ(armP arm) {

    Uns32  word        = 0;
    Uns8   externalInt = arm->externalInt;
    Uns8   useGICInt   = arm->mpLocals->useGICInt;
    Bool   newIRQ      = (externalInt & IRQ_MASK & useGICInt) && True;
    Bool   newFIQ      = (externalInt & FIQ_MASK & useGICInt) && True;
    Uns32 *valuePtr    = getICDIPR(arm, word);
    Uns32  oldValue    = *valuePtr;
    Uns32  newValue    = oldValue;

    // update IRQ state
    if(newIRQ) {
        newValue |= MP_MASK_IRQ;
    } else {
        newValue &= ~MP_MASK_IRQ;
    }

    // update FIQ state
    if(newFIQ) {
        newValue |= MP_MASK_FIQ;
    } else {
        newValue &= ~MP_MASK_FIQ;
    }

    // update interrupt state if required
    if(oldValue!=newValue) {

        // set required value
        *valuePtr = newValue;

        // derive new effective pending state
        *getPending(arm, word) = derivePending(arm, word);
    }
}

//
// Write ICDDCR (may modify pending interrupts)
//
static ARM_MP_WRITEFN(writeICDDCR) {

    Uns32 oldValue = MPG_REG_UNS32(arm, ICDDCR);

    // update raw value
    MPG_REG_UNS32(arm, ICDDCR) = newValue;

    // update interrupt state if required
    if(oldValue!=newValue) {
        updateExceptionsGlobal(arm);
    }
}

//
// Write ICCICR (may modify pending interrupts)
//
static ARM_MP_WRITEFN(writeICCICR) {

    Uns32 oldValue = MPL_REG_UNS32(arm, ICCICR);

    // update raw value
    MPL_REG_UNS32(arm, ICCICR) = newValue;

    // update interrupt state if required
    if(oldValue!=newValue) {

        // get fields used to control interrupt source
        Uns32 FIQEn      = MPL_FIELD(arm, ICCICR, FIQEn);
        Uns32 EnableGrp0 = MPL_FIELD(arm, ICCICR, EnableGrp0);
        Uns32 EnableGrp1 = MPL_FIELD(arm, ICCICR, EnableGrp1);

        // calculate index for table lookup (0..7)
        Uns32 index = (FIQEn*4) + (EnableGrp0*2) + (EnableGrp1);

        // static table mapping interrupt sources from indices
        const static Uns8 src[8] = {
            0        | 0,
            IRQ_MASK | 0,
            IRQ_MASK | 0,
            IRQ_MASK | 0,
            0        | 0,
            IRQ_MASK | 0,
            0        | FIQ_MASK,
            IRQ_MASK | FIQ_MASK
        };

        // select mask for GIC-sourced interrupt requests
        Uns8 newGICInt = src[index];

        // detect changes in interrupt sources
        if(arm->mpLocals->useGICInt != newGICInt) {

            // indicate interrupt source change
            if(ARM_DEBUG_MP(arm)) {
                vmiMessage("I", CPU_PREFIX"_ISC",
                    NO_SRCREF_FMT "Interrupt sources changed, IRQ=%s, FIQ=%s",
                    NO_SRCREF_ARGS(arm),
                    (newGICInt & IRQ_MASK) ? "GIC" : "legacy",
                    (newGICInt & FIQ_MASK) ? "GIC" : "legacy"
                );
            }

            // update interrupt source mask
            arm->mpLocals->useGICInt = newGICInt;

            // refresh GIC state to take account of IRQ/FIQ change
            refreshFIQIRQ(arm);
        }

        // regenerate exception state
        updateExceptionsLocal(arm);
    }
}

//
// Write ICCPMR (requires configuration-dependent masking)
//
static ARM_MP_WRITEFN(writeICCPMR) {

    // mask off priMask
    newValue &= arm->mpGlobals->priMask;

    // take actions required if ICCPMR changes
    if(MPL_REG_UNS32(arm, ICCPMR) != newValue) {

        // update ICCPMR
        MPL_REG_UNS32(arm, ICCPMR) = newValue;

        // a change may cause a pending exception to be activated
        updateExceptionsLocal(arm);
    }
}

//
// Write ICCBPR (requires configuration-dependent bounding)
//
static ARM_MP_WRITEFN(writeICCBPR) {

    Uns32 minValue = arm->configInfo.minICCBPR;

    // restrict ICCBPR to the minimum possible value
    if(newValue<minValue) {
        newValue = minValue;
    }

    // take actions required if ICCBPR changes
    if(MPL_REG_UNS32(arm, ICCBPR) != newValue) {

        // update ICCBPR
        MPL_REG_UNS32(arm, ICCBPR) = newValue;

        // a change may cause preemption if there is an active exception
        if(MPL_FIELD(arm, ICCRPR, Priority)) {
            updateExceptionsLocal(arm);
        }
    }
}

//
// Read ICCIAR
//
static ARM_MP_READFN(readICCIAR) {

    Uns32 intNum = MPL_FIELD(arm, ICCIAR, InterruptID);
    Uns32 result = MPL_REG_UNS32(arm, ICCIAR);

    // do actions required for interrupt acknowledge
    if(intNum != MP_SPURIOUS_INT) {

        Uns32 word = INTERRUPT_WORD(intNum);
        Uns32 mask = INTERRUPT_MASK(intNum);

        // indicate exception is now active
        *getActive(arm, word) |= mask;

        // clear pending state in register
        *getICDIPR(arm, word) &= ~mask;

        // derive new effective pending state
        *getPending(arm, word) = derivePending(arm, word);

        // refresh exception state
        updateExceptionsLocalOrGlobal(arm, word);
    }

    return result;
}

//
// Write ICCEOIR
//
static ARM_MP_WRITEFN(writeICCEOIR) {

    // update raw value
    MPL_REG_UNS32(arm, ICCEOIR) = newValue;

    Uns32 intNum = MPL_FIELD(arm, ICCEOIR, InterruptID);

    // do actions required for end-of-interrupt
    if(intNum != MP_SPURIOUS_INT) {

        Uns32  word   = INTERRUPT_WORD(intNum);
        Uns32  mask   = INTERRUPT_MASK(intNum);
        Uns32 *active = getActive(arm, word);

        // is the specified interrupt active?
        if(*active & mask) {

            // clear down active bit
            *active &= ~mask;

            // refresh exception state
            updateExceptionsLocalOrGlobal(arm, word);
        }
    }
}

//
// Write ICCABPR (requires configuration-dependent bounding)
//
static ARM_MP_WRITEFN(writeICCABPR) {

    Uns32 minValue = arm->configInfo.minICCBPR+1;

    // restrict ICCABPR to the minimum possible value
    if(newValue<minValue) {
        newValue = minValue;
    }

    // take actions required if ICCABPR changes
    if(MPL_REG_UNS32(arm, ICCABPR) != newValue) {

        // update ICCABPR
        MPL_REG_UNS32(arm, ICCABPR) = newValue;

        // a change may cause preemption if there is an active exception
        if(MPL_FIELD(arm, ICCRPR, Priority)) {
            updateExceptionsLocal(arm);
        }
    }
}

//
// Read ICDGRPR
//
static ARM_MP_READFN(readICDGRPR) {
    return *getICDGRPR(arm, id-MPG_ID(ICDGRPR0));
}

//
// Write ICDGRPR
//
static ARM_MP_WRITEFN(writeICDGRPR) {

    Uns32 word = id-MPG_ID(ICDGRPR0);

    if(isValidInterruptWord(arm, word)) {

        Uns32 *valuePtr  = getICDGRPR(arm, word);
        Uns32  writeMask = getICDGRPRWriteMask(arm, word);

        // select only writable bits in ICDGRPR word
        newValue = (*valuePtr & ~writeMask) | (newValue & writeMask);

        // update value if required
        if(*valuePtr != newValue) {
            *valuePtr = newValue;
            updateExceptionsLocalOrGlobal(arm, word);
        }
    }
}

//
// Read ICDISER
//
static ARM_MP_READFN(readICDISER) {
    return *getICDIER(arm, id-MPG_ID(ICDISER0));
}

//
// Write ICDISER
//
static ARM_MP_WRITEFN(writeICDISER) {

    Uns32 word = id-MPG_ID(ICDISER0);

    if(isValidInterruptWord(arm, word)) {

        Uns32 *valuePtr  = getICDIER(arm, word);
        Uns32  writeMask = getICDIERWriteMask(arm, word);

        // set required bits
        *valuePtr |= (newValue & writeMask);

        // update interrupt lines on individual CPUs
        updateExceptionsIfChanged(arm, word);
    }
}

//
// Read ICDICER
//
static ARM_MP_READFN(readICDICER) {
    return *getICDIER(arm, id-MPG_ID(ICDICER0));
}

//
// Write ICDICER
//
static ARM_MP_WRITEFN(writeICDICER) {

    Uns32 word = id-MPG_ID(ICDICER0);

    if(isValidInterruptWord(arm, word)) {

        Uns32 *valuePtr  = getICDIER(arm, word);
        Uns32  writeMask = getICDIERWriteMask(arm, word);

        // clear required bits
        *valuePtr &= ~(newValue & writeMask);

        // update interrupt lines on individual CPUs
        updateExceptionsIfChanged(arm, word);
    }
}

//
// Read ICDISPR
//
static ARM_MP_READFN(readICDISPR) {
    return getICDIPRWord(arm, id-MPG_ID(ICDISPR0));
}

//
// Write ICDISPR
//
static ARM_MP_WRITEFN(writeICDISPR) {

    Uns32 word = id-MPG_ID(ICDISPR0);

    if(isValidInterruptWord(arm, word)) {

        Uns32 *valuePtr  = getICDIPR(arm, word);
        Uns32  writeMask = getICDIPRWriteMask(arm, word);

        // set required bits
        *valuePtr |= (newValue & writeMask);

        // update interrupt lines on individual CPUs
        updateExceptionsIfChanged(arm, word);
    }
}

//
// Read ICDICPR
//
static ARM_MP_READFN(readICDICPR) {
    return getICDIPRWord(arm, id-MPG_ID(ICDICPR0));
}

//
// Write ICDISPR
//
static ARM_MP_WRITEFN(writeICDICPR) {

    Uns32 word = id-MPG_ID(ICDICPR0);

    if(isValidInterruptWord(arm, word)) {

        Uns32 *valuePtr  = getICDIPR(arm, word);
        Uns32  writeMask = getICDIPRWriteMask(arm, word);

        // clear required bits
        *valuePtr &= ~(newValue & writeMask);

        // update interrupt lines on individual CPUs
        updateExceptionsIfChanged(arm, word);
    }
}

//
// Read ICDICPR
//
static ARM_MP_READFN(readICDABR) {
    return *getActive(arm, id-MPG_ID(ICDABR0));
}

//
// Read ICDIPRI
//
static ARM_MP_READFN(readICDIPRI) {
    return *getICDIPRI(arm, id-MPG_ID(ICDIPR00));
}

//
// Write ICDIPRI
//
static ARM_MP_WRITEFN(writeICDIPRI) {

    Uns32 wordx8 = id-MPG_ID(ICDIPR00);
    Uns32 word   = wordx8/8;

    if(isValidInterruptWord(arm, word)) {

        Uns32 *valuePtr = getICDIPRI(arm, wordx8);
        Uns32  intMask  = word ? -1 : MP_MASK_PPI_SGI;
        Uns8   priMask  = arm->mpGlobals->priMask;

        // select only writable bits in ICDIPR word
        newValue &= getWordMaskx8(wordx8, intMask, priMask);

        // update value if required
        if(*valuePtr != newValue) {
            *valuePtr = newValue;
            updateExceptionsLocalOrGlobal(arm, word);
        }
    }
}

//
// Read ICDIPTR
//
static ARM_MP_READFN(readICDIPTR) {
    return *getICDIPTR(arm, id-MPG_ID(ICDIPTR00));
}

//
// Write ICDIPTR
//
static ARM_MP_WRITEFN(writeICDIPTR) {

    Uns32 wordx8 = id-MPG_ID(ICDIPTR00);
    Uns32 word   = wordx8/8;

    if(isMultiprocessor(arm) && isValidInterruptWord(arm, word)) {

        // local registers are not writable
        if(word>=INT_WORDS_LOCAL) {

            Uns32  numCPUs  = arm->configInfo.numCPUs;
            Uns32 *valuePtr = getICDIPTR(arm, wordx8);

            // select only writable bits in ICDIPTR word
            Uns32 mask = (1<<numCPUs)-1;
            mask = (mask<<0) | (mask<<8) | (mask<<16) | (mask<<24);
            newValue &= mask;

            // update value if required
            if(*valuePtr != newValue) {
                *valuePtr = newValue;
                updateExceptionsLocalOrGlobal(arm, word);
            }
        }
    }
}

//
// Read ICDICFR
//
static ARM_MP_READFN(readICDICFR) {
    return *getICDICFR(arm, id-MPG_ID(ICDICFR0));
}

//
// Write ICDICFR
//
static ARM_MP_WRITEFN(writeICDICFR) {

    Uns32 wordx2 = id-MPG_ID(ICDICFR0);
    Uns32 word   = wordx2/2;

    if(isValidInterruptWord(arm, word)) {

        Uns32 *valuePtr  = getICDICFR(arm, wordx2);
        Uns32  intMask   = word ? -1 : 0;
        Uns32  writeMask = getWordMaskx2(wordx2, intMask, 2);
        Uns32  oldValue  = *valuePtr;

        // update writable bits
        newValue = (oldValue & ~writeMask) | (newValue & writeMask);

        // update value if required
        if(oldValue != newValue) {

            *valuePtr = newValue;

            // refresh the plain mask of edge-triggered interrupts
            refreshEdgeMask(arm, word);

            // update interrupt lines on individual CPUs
            updateExceptionsIfChanged(arm, word);
        }
    }
}

//
// Read SPI_STATUS
//
static ARM_MP_READFN(readSPI_STATUS) {
    return *getExternalSPI(arm, id-MPG_ID(SPI_STATUS0)+1);
}

//
// Read PPI_STATUS
//
static ARM_MP_READFN(readPPI_STATUS) {

    // construct return value
    MPG_FIELD(arm, PPI_STATUS, PPI0) = raisePPIGT(getGT(arm));
    MPG_FIELD(arm, PPI_STATUS, FIQ ) = (arm->externalInt & FIQ_MASK) && True;
    MPG_FIELD(arm, PPI_STATUS, PPI2) = raisePPILT(getLT(arm, LT_PRIVATE));
    MPG_FIELD(arm, PPI_STATUS, PPI3) = raisePPILT(getLT(arm, LT_WATCHDOG));
    MPG_FIELD(arm, PPI_STATUS, IRQ ) = (arm->externalInt & IRQ_MASK) && True;

    // return composed value
    return MPG_REG_UNS32(arm, PPI_STATUS);
}

//
// Write ICDSGIR
//
static ARM_MP_WRITEFN(writeICDSGIR) {

    armP root = arm->mpGlobals->root;
    armP dst;

    MPG_REG_UNS32(arm, ICDSGIR) = newValue;

    Uns32 sgiID = MPG_FIELD(arm, ICDSGIR, SGIINTID);
    Uns32 srcID = getIndex(arm);

    switch(MPG_FIELD(arm, ICDSGIR, TargetListFilter)) {

        case 0: {

            // raise the interrupt on all CPUs in the CPUTargetList field
            Uns32 targetList = MPG_FIELD(arm, ICDSGIR, CPUTargetList);

            for(dst=getChild(root); dst; dst=getNextSibling(dst)) {
                if(targetList & (1<<getIndex(dst))) {
                    raiseSGI(dst, sgiID, srcID);
                }
            }

            break;
        }

        case 1: {

            // raise the interrupt on all CPUs except this one
            for(dst=getChild(root); dst; dst=getNextSibling(dst)) {
                if(dst!=arm) {
                    raiseSGI(dst, sgiID, srcID);
                }
            }

            break;
        }

        case 2:

            // raise the interrupt on this CPU only
            raiseSGI(arm, sgiID, srcID);

            break;

        default:

            // ignore other TargetListFilter values
            if(ARM_DEBUG_MP(arm)) {
                vmiMessage("W", CPU_PREFIX"_ITLF",
                    NO_SRCREF_FMT "Illegal ICDSGIR target list filter %u",
                    NO_SRCREF_ARGS(arm),
                    MPG_FIELD(arm, ICDSGIR, TargetListFilter)
                );
            }

            break;
    }
}

//
// Refresh internal IRQ/FIQ state
//
void armMPRefreshBypass(armP arm) {

    Uns8 useGICInt = arm->mpLocals->useGICInt;

    if(useGICInt) {

        // refresh GIC state to take account of IRQ/FIQ change
        refreshFIQIRQ(arm);

        // regenerate GIC exception state
        updateExceptionsLocal(arm);
    }

    // construct internal interrupt request mask values from external sources
    arm->internalInt = (
        (arm->externalInt & ~useGICInt) |
        (arm->GICInt      &  useGICInt)
    );
}


////////////////////////////////////////////////////////////////////////////////
// LOCAL TIMERS
////////////////////////////////////////////////////////////////////////////////

//
// Create a local timer object
//
static void newLT(
    armP        arm,
    armLTimerP  timer,
    const char *name,
    Uns32       intMask,
    vmiICountFn icountCB
) {
    timer->name     = name;
    timer->intMask  = intMask;
    timer->vmiTimer = vmirtCreateModelTimer((vmiProcessorP)arm, icountCB, timer);
}

//
// Free a local timer object
//
static void freeLT(armLTimerP timer) {
    vmirtDeleteModelTimer(timer->vmiTimer);
}

//
// Derive correct local counter value
//
static void deriveCounterLT(armP arm, armLTimerP timer) {

    if(timer->running) {

        Uns32 scale      = timer->scale;
        Uns64 thisICount = getThisICount(arm);

        // action is only required if time has moved on since last recalculation
        if(thisICount!=timer->base) {

            // adjust base past assignment instruction
            if(timer->partial==-1) {
                timer->base++;
            }

            // calculate instruction and cycle deltas
            Uns64 iDelta     = thisICount - timer->base;
            Uns64 cDelta     = iDelta / scale;
            Uns64 oldCounter = timer->counter;
            Uns64 newCounter = oldCounter - cDelta;

            // handle wraparound
            if(newCounter>oldCounter) {
                Uns64 load64P1   = timer->load+1ULL;
                Uns64 counterMod = (-newCounter) % load64P1;
                newCounter = counterMod ? load64P1-counterMod : 0;
            }

            // rebase to preceding complete cycle
            timer->base   += cDelta * scale;
            timer->counter = newCounter;
            timer->partial = thisICount - timer->base;
        }
    }
}

//
// Return the number of instructions until the local timer wraps
//
static Uns64 instructionsToWrapLT(armP arm, armLTimerP timer) {

    // derive current cycle and partial count
    deriveCounterLT(arm, timer);

    // calculate instructions remaining, excluding current instruction
    Uns32 remainder = timer->scale - timer->partial - 1;
    Uns64 counter64 = timer->counter;

    // return total instruction count
    return (counter64*timer->scale) + remainder;
}

//
// Schedule a local timer interrupt
//
static void scheduleIntLT(armP arm, armLTimerP timer, const char *reason) {

    // sanity check timer is running
    VMI_ASSERT(timer->running, "%s timer not running", timer->name);

    Int64 delta = instructionsToWrapLT(arm, timer);

    // delta for timeout should be on transition *to* zero, so reduce it by
    // one decrement of the timer
    delta -= timer->scale;

    // handle the case where the counter is currently zero - in this case,
    // the interrupt should not occur for another full cycle
    if(delta<=0) {
        delta += timer->period;
    }

    // set VMI timer
    vmirtSetModelTimer(timer->vmiTimer, delta);

    // emit debug output if required
    if(ARM_DEBUG_MP(arm)) {

        Uns64 thisICount = getThisICount(arm);

        vmiMessage("I", CPU_PREFIX"_STI",
            NO_SRCREF_FMT
            "%s - icount="FMT_64u" schedule %s timer interrupt at "FMT_64u,
            NO_SRCREF_ARGS(arm),
            reason,
            thisICount,
            timer->name,
            thisICount + delta
        );
    }
}

//
// Clear a local timer interrupt
//
static void clearIntLT(armP arm, armLTimerP timer, const char *reason) {

    // sanity check timer is not running
    VMI_ASSERT(!timer->running, "%s timer running", timer->name);

    // clear VMI timer
    vmirtClearModelTimer(timer->vmiTimer);

    // emit debug output if required
    if(ARM_DEBUG_MP(arm)) {
        vmiMessage("I", CPU_PREFIX"_CTI",
            NO_SRCREF_FMT
            "%s - icount="FMT_64u" clear %s timer interrupt",
            NO_SRCREF_ARGS(arm),
            reason,
            getThisICount(arm),
            timer->name
        );
    }
}

//
// Handle generation of pending interrupts if required
//
inline static void refreshEventLT(armP arm, armLTimerP timer) {
    if(raisePPILT(timer)) {
        raisePPI(arm, timer->intMask);
    }
}

//
// Is the local timer running?
//
static Bool isRunningLT(armLTimerP timer) {
    if(!timer->control.Enable) {
        return False;
    } else if(timer->counter) {
        return True;
    } else if(timer->control.WDMode) {
        return False;
    } else if(!timer->control.AutoReload) {
        return False;
    } else if(timer->load) {
        return True;
    } else {
        return False;
    }
}

//
// This is called when a local timer has expired
//
static VMI_ICOUNT_FN(expiredLT) {

    armP       arm      = (armP)processor;
    armLTimerP armTimer = userData;

    // emit debug output if required
    if(ARM_DEBUG_MP(arm)) {
        vmiMessage("I", CPU_PREFIX"_TE",
            NO_SRCREF_FMT "Expiry of %s timer at "FMT_64u,
            NO_SRCREF_ARGS(arm),
            armTimer->name,
            getThisICount(arm)
        );
    }

    // handle either stopping the timer or autoloading it
    if(
        armTimer->control.WDMode      ||
        !armTimer->control.AutoReload ||
        !armTimer->load
    ) {
        armTimer->running = False;
        armTimer->counter = 0;
    } else {
        scheduleIntLT(arm, armTimer, "Timer expiry");
    }

    if(armTimer->control.WDMode) {

        // watchdog expiry
        armTimer->reset = 1;

        // write wdResetReq
        writeNet(arm, arm->wdResetReq, 1);
        writeNet(arm, arm->wdResetReq, 0);

    } else if(!armTimer->event) {

        // timer expiry
        armTimer->event = 1;

        refreshEventLT(arm, armTimer);
    }
}

//
// Recalculate local timer restart period
//
static void refreshPeriodLT(armP arm, armLTimerP timer) {

    Uns64 load64     = timer->load;
    Uns32 PERIPH_CLK = arm->configInfo.mpRegDefaults.PERIPH_CLK;
    Uns32 Prescaler  = timer->control.Prescaler;

    // refresh the time scale
    timer->scale = (Prescaler+1) * PERIPH_CLK;

    // calculate timer period, assuming one instruction per cycle
    timer->period = (load64+1) * timer->scale;
}

//
// Recalculate local timer counter base
//
static void refreshBaseLT(armP arm, armLTimerP timer) {
    timer->base    = getThisICount(arm);
    timer->partial = -1;
}

//
// Take actions required when local counter is changed
//
static void setCounterLT(armP arm, armLTimerP timer, Uns32 newValue) {

    // was timer running before local counter value set?
    Bool oldRunning = timer->running;

    // record raw value of counter
    timer->counter = newValue;
    timer->running = isRunningLT(timer);

    // is timer running after local counter value set?
    Bool newRunning = timer->running;

    // if the timer is running, refresh the base to allow for the new start
    // count and reschedule the timer interrupt
    if(newRunning) {
        refreshBaseLT(arm, timer);
        scheduleIntLT(arm, timer, "Counter modified");
    } else if(oldRunning) {
        clearIntLT(arm, timer, "Counter modified");
    }
}

//
// Read PTLoad or WTLoad
//
static ARM_MP_READFN(readLoadLT) {

    armLTimerP timer = getLT(arm, id-MPL_ID(PTLoad));

    return timer->load;
}

//
// Write PTLoad or WTLoad
//
static ARM_MP_WRITEFN(writeLoadLT) {

    armLTimerP timer = getLT(arm, id-MPL_ID(PTLoad));

    // update raw value
    timer->load = newValue;

    // refresh the timer period
    refreshPeriodLT(arm, timer);

    // refresh counter (may require timeout recalculation)
    setCounterLT(arm, timer, newValue);
}

//
// Read PTCounter or WTCounter
//
static ARM_MP_READFN(readCounterLT) {

    armLTimerP timer = getLT(arm, id-MPL_ID(PTCounter));

    deriveCounterLT(arm, timer);

    return timer->counter;
}

//
// Write PTCounter or WTCounter
//
static ARM_MP_WRITEFN(writeCounterLT) {

    armLTimerP timer = getLT(arm, id-MPL_ID(PTCounter));

    if(!timer->control.WDMode) {
        setCounterLT(arm, timer, newValue);
    }
}

//
// Read PTControl or WTControl
//
static ARM_MP_READFN(readControlLT) {

    armLTimerP timer = getLT(arm, id-MPL_ID(PTControl));

    union {armLTControl control; Uns32 u32;} u = {timer->control};

    return u.u32;
}

//
// Write PTControl or WTControl
//
static ARM_MP_WRITEFN(writeControlLT) {

    armLTimerP timer = getLT(arm, id-MPL_ID(PTControl));

    union {Uns32 u32; armLTControl control;} u = {newValue};

    // update PTControl, recording any change in Prescaler
    Uns8 oldPrescaler = timer->control.Prescaler;
    Bool oldWDMode    = timer->control.WDMode;
    timer->control    = u.control;
    Uns8 newPrescaler = timer->control.Prescaler;

    // software may not clear WDMode directly, only set it
    timer->control.WDMode |= oldWDMode;

    // derive current counter value (required for isRunningLT)
    deriveCounterLT(arm, timer);

    // update timer running status (after counter value derivation)
    Bool oldRunning = timer->running;
    timer->running = isRunningLT(timer);
    Bool newRunning = timer->running;

    // refresh the timer period
    refreshPeriodLT(arm, timer);

    // derive counter base if required
    if(
        (newRunning && !oldRunning) ||
        (newRunning && (oldPrescaler!=newPrescaler))
    ) {
        refreshBaseLT(arm, timer);
    }

    // handle state change on disable/enable
    if(newRunning) {
        scheduleIntLT(arm, timer, "Control modified");
    } else if(oldRunning) {
        clearIntLT(arm, timer, "Control modified");
    }

    // an interrupt may be required if ITEnable has been set
    refreshEventLT(arm, timer);
}

//
// Read PTInterruptStatus or WTInterruptStatus
//
static ARM_MP_READFN(readStatusLT) {

    armLTimerP timer = getLT(arm, id-MPL_ID(PTInterruptStatus));

    return timer->event;
}

//
// Write PTInterruptStatus or WTInterruptStatus
//
static ARM_MP_WRITEFN(writeStatusLT) {

    armLTimerP timer = getLT(arm, id-MPL_ID(PTInterruptStatus));

    // event is cleared if written with a non-zero value
    if(newValue&1) {
        timer->event = 0;
    }
}

//
// Read WTReset
//
static ARM_MP_READFN(readResetLT) {

    armLTimerP timer = getLT(arm, id-MPL_ID(PTResetStatus));

    return timer->reset;
}

//
// Write WTReset
//
static ARM_MP_WRITEFN(writeResetLT) {

    armLTimerP timer = getLT(arm, id-MPL_ID(PTResetStatus));

    // event is cleared if written with a non-zero value
    if(newValue&1) {
        timer->reset = 0;
    }
}

//
// Write WTDisable
// TODO: validate no other writes between RESET_MAGIC1 and RESET_MAGIC2 writes
//
static ARM_MP_WRITEFN(writeDisableLT) {

    armLTimerP timer = getLT(arm, id-MPL_ID(PTDisable));

    if(newValue==RESET_MAGIC1) {

        // enter intermediate state
        timer->inDisable = True;

    } else if(timer->inDisable && (newValue==RESET_MAGIC2)) {

        // enter initial state
        timer->inDisable = False;

        // clear WDMode
        timer->control.WDMode = 0;

        // an interrupt may be required if ITEnable is set
        refreshEventLT(arm, timer);

    } else {

        // enter initial state
        timer->inDisable = False;
    }
}


////////////////////////////////////////////////////////////////////////////////
// GLOBAL TIMER
////////////////////////////////////////////////////////////////////////////////

//
// Create global timer object
//
static void newGT(
    armP        arm,
    armGTimerP  timer,
    Uns32       intMask,
    vmiICountFn icountCB
) {
    timer->intMask  = intMask;
    timer->vmiTimer = vmirtCreateModelTimer((vmiProcessorP)arm, icountCB, timer);
}

//
// Free global timer object
//
static void freeGT(armGTimerP timer) {
    vmirtDeleteModelTimer(timer->vmiTimer);
}

//
// Derive correct global counter value
//
static void deriveCounterGT(armP arm) {

    armGTimerGP timerG = getGTG(arm);

    if(timerG->TimerEnable) {

        armGTimerP timer      = getGT(arm);
        Uns32      scale      = timerG->scale;
        Uns64      thisICount = getThisICount(arm);

        // action is only required if time has moved on since last recalculation
        if(thisICount!=timer->base) {

            // adjust base past assignment instruction
            if(timer->partial==-1) {
                timer->base++;
            }

            // calculate instruction and cycle deltas
            Uns64 iDelta = thisICount - timer->base;
            Uns64 cDelta = iDelta / scale;

            // rebase to preceding complete cycle
            timer->base    += cDelta * scale;
            timer->counter += cDelta;
            timer->partial  = thisICount - timer->base;
        }
    }
}

//
// Derive correct global counter value for all CPUs on an MPCore
//
inline static void deriveCounterGTHier(armP arm) {
    for(arm=getFirstCPU(arm); arm; arm=getNextSibling(arm)) {
        deriveCounterGT(arm);
    }
}

//
// Return the number of counts remaining until the comparator matches, comprised
// of a complete number of decrements (returned) and a number of instructions
// remaining until the next decrement (by-reference remainder argument)
//
static Uns64 countsToExpiryGT(armP arm, Uns32 *remainder) {

    armGTimerP  timer  = getGT(arm);
    armGTimerGP timerG = getGTG(arm);

    // derive current cycle and partial count
    deriveCounterGT(arm);

    // return cycles and instructions remaining, excluding current instruction
    *remainder = timerG->scale - timer->partial - 1;
    return timer->comparator - timer->counter - 1;
}

//
// Schedule a global timer interrupt
//
static void scheduleIntGT(armP arm, const char *reason) {

    armGTimerP  timer  = getGT(arm);
    armGTimerGP timerG = getGTG(arm);

    // sanity check timer is running
    VMI_ASSERT(timerG->TimerEnable, "global timer not running");

    // get cycles and instructions until the comparator expires
    Uns32 iRemaining;
    Uns64 cRemaining = countsToExpiryGT(arm, &iRemaining);

    // attempt to convert to an instruction delay - this may overflow
    Uns32 scale       = timerG->scale;
    Uns64 iCountCycle = cRemaining*scale;
    Uns64 delta       = iCountCycle + iRemaining;

    if(((iCountCycle/scale)!=cRemaining) || (delta<iCountCycle)) {

        // enable rescheduling after a long period on overflow
        timer->reschedule = True;
        delta             = cRemaining;

    } else if(!delta) {

        // handle the case where the delta is currently zero - in this case,
        // the timer should not expire for another full cycle
        timer->reschedule = True;
        delta             = -1;

    } else {

        timer->reschedule = False;
    }

    // set VMI timer
    vmirtSetModelTimer(timer->vmiTimer, delta);

    // emit debug output if required
    if(ARM_DEBUG_MP(arm)) {

        Uns64 thisICount = getThisICount(arm);

        vmiMessage("I", CPU_PREFIX"_STI",
            NO_SRCREF_FMT
            "%s - icount="FMT_64u" schedule global timer interrupt at "FMT_64u,
            NO_SRCREF_ARGS(arm),
            reason,
            thisICount,
            thisICount + delta
        );
    }
}

//
// Schedule a global timer interrupt for all CPUs on an MPCore
//
inline static void scheduleIntGTHier(armP arm, const char *reason) {
    for(arm=getFirstCPU(arm); arm; arm=getNextSibling(arm)) {
        scheduleIntGT(arm, reason);
    }
}

//
// Clear a global timer interrupt
//
static void clearIntGT(armP arm, const char *reason) {

    armGTimerP  timer  = getGT(arm);
    armGTimerGP timerG = getGTG(arm);

    // sanity check timer is not running
    VMI_ASSERT(!timerG->TimerEnable, "global timer running");

    // clear VMI timer
    vmirtClearModelTimer(timer->vmiTimer);

    // emit debug output if required
    if(ARM_DEBUG_MP(arm)) {
        vmiMessage("I", CPU_PREFIX"_CTI",
            NO_SRCREF_FMT
            "%s - icount="FMT_64u" clear global timer interrupt",
            NO_SRCREF_ARGS(arm),
            reason,
            getThisICount(arm)
        );
    }
}

//
// Clear a global timer interrupt for all CPUs on an MPCore
//
inline static void clearIntGTHier(armP arm, const char *reason) {
    for(arm=getFirstCPU(arm); arm; arm=getNextSibling(arm)) {
        clearIntGT(arm, reason);
    }
}

//
// Handle generation of pending interrupts if required
//
inline static void refreshEventGT(armP arm) {

    armGTimerP timer = getGT(arm);

    if(raisePPIGT(timer)) {
        raisePPI(arm, timer->intMask);
    }
}

//
// This is called when the global timer has expired
//
static VMI_ICOUNT_FN(expiredGT) {

    armP        arm      = (armP)processor;
    armGTimerP armTimer = userData;

    if(armTimer->reschedule || !armTimer->control.CompEnable) {

        // schedule the next iteration
        scheduleIntGT(arm, "Timer iteration");

    } else {

        // emit debug output if required
        if(ARM_DEBUG_MP(arm)) {
            vmiMessage("I", CPU_PREFIX"_TE",
                NO_SRCREF_FMT "Expiry of global timer at "FMT_64u,
                NO_SRCREF_ARGS(arm),
                getThisICount(arm)
            );
        }

        // handle auto-increment of comparator if required
        if(armTimer->control.AutoIncrement) {
            armTimer->comparator += armTimer->autoinc;
        }

        // schedule the next timer event
        scheduleIntGT(arm, "Timer expiry");

        // handle expiry event
        if(!armTimer->event) {
            armTimer->event = 1;
            refreshEventGT(arm);
        }
    }
}

//
// Recalculate global timer scale
//
static void refreshScaleGT(armP arm) {

    armGTimerGP timerG     = getGTG(arm);
    Uns32       PERIPH_CLK = arm->configInfo.mpRegDefaults.PERIPH_CLK;

    // refresh the time scale
    timerG->scale = (timerG->Prescaler+1) * PERIPH_CLK;
}

//
// Recalculate global timer counter base
//
static void refreshBaseGT(armP arm) {

    armGTimerP timer = getGT(arm);

    timer->base    = getThisICount(arm);
    timer->partial = -1;
}

//
// Recalculate global timer counter base for all CPUs on an MPCore
//
inline static void refreshBaseGTHier(armP arm) {
    for(arm=getFirstCPU(arm); arm; arm=getNextSibling(arm)) {
        refreshBaseGT(arm);
    }
}

//
// Set global counter value
//
static void setCounterGT(armP arm, Uns64 newValue) {

    armGTimerP  timer  = getGT(arm);
    armGTimerGP timerG = getGTG(arm);

    // record raw value of counter
    timer->counter = newValue;

    // if the timer is running, refresh the base to allow for the new
    // count and reschedule the timer interrupt
    if(timerG->TimerEnable) {
        refreshBaseGT(arm);
        scheduleIntGT(arm, "Counter modified");
    }
}

//
// Set global counter value for all CPUs on an MPCore
//
inline static void setCounterGTHier(armP arm, Uns64 newValue) {
    for(arm=getFirstCPU(arm); arm; arm=getNextSibling(arm)) {
        setCounterGT(arm, newValue);
    }
}

//
// Take actions required when global comparator is changed
//
static void setComparatorGT(armP arm, Uns64 newValue) {

    armGTimerP  timer  = getGT(arm);
    armGTimerGP timerG = getGTG(arm);

    if(timer->comparator != newValue) {

        // record raw value of counter
        timer->comparator = newValue;

        // if the timer is running, refresh the base to allow for the new
        // comparator and reschedule the timer interrupt
        if(timerG->TimerEnable) {
            scheduleIntGT(arm, "Comparator modified");
        }
    }
}

//
// Read GTCounter
//
static ARM_MP_READFN(readCounterGT) {

    armGTimerP timer = getGT(arm);

    deriveCounterGT(arm);

    union {Uns64 u64; Uns32 u32[2];} u = {timer->counter};

    return u.u32[byteOffset/4];
}

//
// Write GTCounter
//
static ARM_MP_WRITEFN(writeCounterGT) {

    armGTimerP timer = getGT(arm);

    deriveCounterGT(arm);

    union {Uns64 u64; Uns32 u32[2];} u = {timer->counter};

    u.u32[byteOffset/4] = newValue;

    setCounterGTHier(arm, u.u64);
}

//
// Read GTControl
//
static ARM_MP_READFN(readControlGT) {

    armGTimerP  timer  = getGT(arm);
    armGTimerGP timerG = getGTG(arm);

    // refresh propeties mastered globally
    timer->control.TimerEnable = timerG->TimerEnable;
    timer->control.Prescaler   = timerG->Prescaler;

    union {armGTControl control; Uns32 u32;} u = {timer->control};

    return u.u32;
}

//
// Write GTControl
//
static ARM_MP_WRITEFN(writeControlGT) {

    armGTimerP  timer  = getGT(arm);
    armGTimerGP timerG = getGTG(arm);

    union {Uns32 u32; armGTControl control;} u = {newValue};

    // update GTControl, recording any change in TimerEnable or Prescaler
    Bool oldEnable    = timerG->TimerEnable;
    Uns8 oldPrescaler = timerG->Prescaler;
    timer->control    = u.control;
    Bool newEnable    = timer->control.TimerEnable;
    Uns8 newPrescaler = timer->control.Prescaler;

    // derive current counter value if required
    if(
        (!newEnable && oldEnable) ||
        (newEnable && (oldPrescaler!=newPrescaler))
    ) {
        deriveCounterGTHier(arm);
    }

    // refresh propeties mastered globally
    timerG->TimerEnable = newEnable;
    timerG->Prescaler   = newPrescaler;

    // refresh the scale
    refreshScaleGT(arm);

    // derive counter base if required
    if(
        (newEnable && !oldEnable) ||
        (newEnable && (oldPrescaler!=newPrescaler))
    ) {
        refreshBaseGTHier(arm);
    }

    // handle state change on disable/enable
    if(newEnable) {
        scheduleIntGTHier(arm, "Control modified");
    } else if(oldEnable) {
        clearIntGTHier(arm, "Control modified");
    }

    // an interrupt may be required if ITEnable has been set
    refreshEventGT(arm);
}

//
// Read GTInterruptStatus
//
static ARM_MP_READFN(readStatusGT) {

    armGTimerP timer = getGT(arm);

    return timer->event;
}

//
// Write GTInterruptStatus
//
static ARM_MP_WRITEFN(writeStatusGT) {

    armGTimerP timer = getGT(arm);

    // event is cleared if written with a non-zero value
    if(newValue&1) {
        timer->event = 0;
    }
}

//
// Read GTComparator
//
static ARM_MP_READFN(readCompareGT) {

    armGTimerP timer = getGT(arm);

    union {Uns64 u64; Uns32 u32[2];} u = {timer->comparator};

    return u.u32[byteOffset/4];
}

//
// Write GTComparator
//
static ARM_MP_WRITEFN(writeCompareGT) {

    armGTimerP timer = getGT(arm);

    union {Uns64 u64; Uns32 u32[2];} u = {timer->comparator};

    u.u32[byteOffset/4] = newValue;

    setComparatorGT(arm, u.u64);
}

//
// Read GTAutoIncrement
//
static ARM_MP_READFN(readAutoIncGT) {

    armGTimerP timer = getGT(arm);

    union {Uns64 u64; Uns32 u32[2];} u = {timer->autoinc};

    return u.u32[byteOffset/4];
}

//
// Write GTAutoIncrement
//
static ARM_MP_WRITEFN(writeAutoIncGT) {

    armGTimerP timer = getGT(arm);

    union {Uns64 u64; Uns32 u32[2];} u = {timer->autoinc};

    u.u32[byteOffset/4] = newValue;

    timer->autoinc = u.u64;
}


////////////////////////////////////////////////////////////////////////////////
// MP REGISTER DEFINITIONS
////////////////////////////////////////////////////////////////////////////////

//
// Table of MPCore global register attributes
//
const static mpRegAttrs mpRegInfo[] = {

    ////////////////////////////////////////////////////////////////////////////
    // true registers (represented in MPCore structure)
    ////////////////////////////////////////////////////////////////////////////

    //           id                   byteOK, access,   readCB          writeCB
    MPG_ATTR(    SCUControl,          1,      MPA_SAC,  0,              0            ),
    MPG_ATTR(    SCUConfiguration,    1,      MPA_NONE, 0,              0            ),
    MPG_ATTR(    SCUCPUPowerStatus,   1,      MPA_SAC,  0,              0            ),
    MPG_ATTR(    SCUFilteringStart,   1,      MPA_SAC,  0,              0            ),
    MPG_ATTR(    SCUFilteringEnd,     1,      MPA_SAC,  0,              0            ),
    MPG_ATTR(    SCUSAC,              1,      MPA_SAC,  0,              0            ),
    MPG_ATTR(    SCUNSAC,             1,      MPA_SAC,  0,              0            ),

    //           id                   byteOK, access,   readCB          writeCB
    MPG_ATTR(    ICDDCR,              1,      MPA_NONE, 0,              writeICDDCR  ),
    MPG_ATTR(    ICDICTR,             1,      MPA_NONE, 0,              0            ),
    MPG_ATTR(    ICDIIDR,             1,      MPA_NONE, 0,              0            ),
    MPG_ATTR(    PPI_STATUS,          1,      MPA_NONE, readPPI_STATUS, 0            ),
    MPG_ATTR(    ICDSGIR,             1,      MPA_NONE, readIgnore,     writeICDSGIR ),
    MPG_ATTRx8(  PERIPH_ID,           1,      MPA_NONE, 0,              0            ),
    MPG_ATTRx4(  COMPONENT_ID,        1,      MPA_NONE, 0,              0            ),

    //           id                   byteOK, access,   readCB          writeCB
    MPL_ATTR(    ICCICR,              1,      MPA_NONE, 0,              writeICCICR  ),
    MPL_ATTR(    ICCPMR,              1,      MPA_NONE, 0,              writeICCPMR  ),
    MPL_ATTR(    ICCBPR,              1,      MPA_NONE, 0,              writeICCBPR  ),
    MPL_ATTR(    ICCIAR,              1,      MPA_NONE, readICCIAR,     0            ),
    MPL_ATTR(    ICCEOIR,             1,      MPA_NONE, readIgnore,     writeICCEOIR ),
    MPL_ATTR(    ICCRPR,              1,      MPA_NONE, 0,              0            ),
    MPL_ATTR(    ICCHPIR,             1,      MPA_NONE, 0,              0            ),
    MPL_ATTR(    ICCABPR,             1,      MPA_NONE, 0,              writeICCABPR ),
    MPL_ATTR(    ICCIDR,              1,      MPA_NONE, 0,              0            ),

    ////////////////////////////////////////////////////////////////////////////
    // pseudo-registers (not represented in MPCore structure)
    ////////////////////////////////////////////////////////////////////////////

    //           id                   byteOK, access,   readCB          writeCB
    MPG_ATTR(    SCUInvalidateSecure, 1,      MPA_SAC,  readIgnore,     writeIgnore  ),

    //           id                   byteOK, access,   readCB          writeCB
    MPG_ATTRx8(  ICDGRPR,             1,      MPA_NONE, readICDGRPR,    writeICDGRPR ),
    MPG_ATTRx8(  ICDISER,             1,      MPA_NONE, readICDISER,    writeICDISER ),
    MPG_ATTRx8(  ICDICER,             1,      MPA_NONE, readICDICER,    writeICDICER ),
    MPG_ATTRx8(  ICDISPR,             1,      MPA_NONE, readICDISPR,    writeICDISPR ),
    MPG_ATTRx8(  ICDICPR,             1,      MPA_NONE, readICDICPR,    writeICDICPR ),
    MPG_ATTRx8(  ICDABR,              1,      MPA_NONE, readICDABR,     0            ),
    MPG_ATTRx64( ICDIPR,              1,      MPA_NONE, readICDIPRI,    writeICDIPRI ),
    MPG_ATTRx64( ICDIPTR,             1,      MPA_NONE, readICDIPTR,    writeICDIPTR ),
    MPG_ATTRx16( ICDICFR,             1,      MPA_NONE, readICDICFR,    writeICDICFR ),
    MPG_ATTRx7(  SPI_STATUS,          1,      MPA_NONE, readSPI_STATUS, 0            ),

    //           id                   byteOK, access,   readCB          writeCB
    MPL_ATTR(    PTLoad,              1,      MPA_NONE, readLoadLT,     writeLoadLT   ),
    MPL_ATTR(    WTLoad,              1,      MPA_NONE, readLoadLT,     writeLoadLT   ),
    MPL_ATTR(    PTCounter,           1,      MPA_NONE, readCounterLT,  writeCounterLT),
    MPL_ATTR(    WTCounter,           1,      MPA_NONE, readCounterLT,  writeCounterLT),
    MPL_ATTR(    PTControl,           1,      MPA_NONE, readControlLT,  writeControlLT),
    MPL_ATTR(    WTControl,           1,      MPA_NONE, readControlLT,  writeControlLT),
    MPL_ATTR(    PTInterruptStatus,   1,      MPA_NONE, readStatusLT,   writeStatusLT ),
    MPL_ATTR(    WTInterruptStatus,   1,      MPA_NONE, readStatusLT,   writeStatusLT ),
    MPL_ATTR(    WTResetStatus,       1,      MPA_NONE, readResetLT,    writeResetLT  ),
    MPL_ATTR(    WTDisable,           1,      MPA_NONE, readIgnore,     writeDisableLT),

    //           id                   byteOK, access,   readCB          writeCB
    MPL_ATTRD(   GTCounter,           1,      MPA_NONE, readCounterGT,  writeCounterGT),
    MPL_ATTR(    GTControl,           1,      MPA_NONE, readControlGT,  writeControlGT),
    MPL_ATTR(    GTInterruptStatus,   1,      MPA_NONE, readStatusGT,   writeStatusGT ),
    MPL_ATTRD(   GTComparator,        1,      MPA_NONE, readCompareGT,  writeCompareGT),
    MPL_ATTR(    GTAutoIncrement,     1,      MPA_NONE, readAutoIncGT,  writeAutoIncGT),
};

//
// Return number of elements in mpRegInfo
//
#define NUM_MEMBERS(_A) (sizeof(_A)/sizeof((_A)[0]))
#define MP_REG_NUM      NUM_MEMBERS(mpRegInfo)

//
// Is access to the global register allowed by SAC/NSAC?
//
static Bool validateAccess(mpRegAttrsCP attrs, armP arm) {

    switch(attrs->access) {

        case MPA_NONE:
            return True;

        case MPA_SAC:
            return (MPG_REG_UNS32(arm, SCUSAC) & (1<<getIndex(arm))) != 0;

        default:
            VMI_ABORT("unimplemented case");
            return False;
    }
}

//
// Return a pointer to the raw register value for the indicated register
//
static Uns32 *getMPRegValue(mpRegAttrsCP attrs, armP arm) {
    if(attrs->isGlobal) {
        return &arm->mpGlobals->regValues.regs[attrs->id];
    } else {
        return &arm->mpLocals->regValues.regs[attrs->id];
    }
}

//
// Return the offset of the address within the MPCore control block
//
inline static Uns32 getMPOffset(armP arm, Uns32 VA) {
    return VA - arm->mpGlobals->lowAddr;
}

//
// Read MPCore memory-mapped global register
//
static VMI_MEM_READ_FN(readMP) {

    if(processor) {

        armP         arm       = (armP)processor;
        mpRegAttrsCP attrs     = userData;
        Uns32        offset    = address - attrs->address;
        Uns32        result    = 0;
        Bool         bigEndian = isBigEndian(arm);

        if(!((bytes==4) || (attrs->byteOK && (bytes==1)))) {

            // invalid access size
            if(ARM_DEBUG_MP(arm)) {
                vmiMessage("W", CPU_PREFIX"_RMPIS",
                    NO_SRCREF_FMT "Read access to %s with invalid size %u bytes",
                    NO_SRCREF_ARGS(arm),
                    attrs->name,
                    bytes
                );
            }

            armExternalMemoryAbort(arm, VA, MEM_PRIV_R);

        } else if(!validateAccess(attrs, arm)) {

            // no access to this register
            if(ARM_DEBUG_MP(arm)) {
                vmiMessage("W", CPU_PREFIX"_RANP",
                    NO_SRCREF_FMT "Read access to %s not permitted",
                    NO_SRCREF_ARGS(arm),
                    attrs->name
                );
            }

        } else {

            if(attrs->readCB) {

                // read using callback
                Uns32 offset4 = offset&~3;
                Uns32 shift   = (offset&3) * 8;

                result = attrs->readCB(arm, attrs->id, offset4) >> shift;

            } else {

                // read from MPCore structure
                Uns32 *regValue = getMPRegValue(attrs, arm);
                Uns32  offset4  = offset/4;
                Uns32  shift    = (offset&3) * 8;

                result = regValue[offset4] >> shift;
            }

            // emit debug if required
            if(ARM_DEBUG_MP(arm)) {
                vmiMessage("I", CPU_PREFIX"_RMPR",
                    NO_SRCREF_FMT "Read %u bytes at offset 0x%x (%s) -> 0x%08x\n",
                    NO_SRCREF_ARGS(arm),
                    bytes,
                    getMPOffset(arm, VA),
                    attrs->name,
                    result
                );
            }
        }

        // get the value read (may be partial register)
        if(bytes==1) {
            *(Uns8*)value = result;
        } else if(bytes==4) {
            *(Uns32*)value = SWAP_4_BYTE_COND(result, bigEndian);
        }
    }
}

//
// Write MPCore memory-mapped global register
//
static VMI_MEM_WRITE_FN(writeMP) {

    if(processor) {

        armP         arm       = (armP)processor;
        mpRegAttrsCP attrs     = userData;
        Uns32        offset    = address - attrs->address;
        Uns32        writeMask = attrs->writeMask;
        Bool         bigEndian = isBigEndian(arm);
        Uns32        newValue;

        // get the value to write (may be partial register)
        if(bytes==1) {
            newValue = *(Uns8*)value;
        } else if(bytes==4) {
            newValue = SWAP_4_BYTE_COND(*(Uns32*)value, bigEndian);
        } else {
            newValue = 0;
        }

        if(!((bytes==4) || (attrs->byteOK && (bytes==1)))) {

            // invalid access size
            if(ARM_DEBUG_MP(arm)) {
                vmiMessage("W", CPU_PREFIX"_WMPIS",
                    NO_SRCREF_FMT "Write access to %s with invalid size %u bytes",
                    NO_SRCREF_ARGS(arm),
                    attrs->name,
                    bytes
                );
            }

            armExternalMemoryAbort(arm, VA, MEM_PRIV_W);

        } else if(!(writeMask && validateAccess(attrs, arm))) {

            // no access to this register
            if(ARM_DEBUG_MP(arm)) {
                vmiMessage("W", CPU_PREFIX"_WANP",
                    NO_SRCREF_FMT "Write access to %s not permitted",
                    NO_SRCREF_ARGS(arm),
                    attrs->name
                );
            }

        } else {

            // emit debug if required
            if(ARM_DEBUG_MP(arm)) {
                vmiMessage("I", CPU_PREFIX"_WMPR",
                    NO_SRCREF_FMT "Write %u bytes at offset 0x%x (%s) <- 0x%08x\n",
                    NO_SRCREF_ARGS(arm),
                    bytes,
                    getMPOffset(arm, VA),
                    attrs->name,
                    newValue
                );
            }

           if(attrs->writeCB) {

                // detect partial register write
                if((offset&3) || (bytes!=4)) {

                    // access within register - do read-modify-write of entire
                    // 4-byte register
                    Uns32 offset4  = offset&~3;
                    Uns32 oldValue;
                    Uns32 i;

                    // get 4-byte value, either directly or by callback
                    if(attrs->readCB) {
                        oldValue = attrs->readCB(arm, attrs->id, offset4);
                    } else {
                        oldValue = *getMPRegValue(attrs, arm);
                    }

                    // get 4-byte value with its mask
                    union {Uns32 u32; Uns8 u8[4];} uValue = {oldValue};
                    union {Uns32 u32; Uns8 u8[4];} uMask  = {writeMask};

                    // update writable bits in the subrange
                    for(i=offset; i<offset+bytes; i++) {

                        // update one byte, preserving read-only bits
                        uValue.u8[i] = (
                            (uValue.u8[i] & ~uMask.u8[i]) |
                            (newValue     &  uMask.u8[i])
                        );

                        // shift newValue for next iteration
                        newValue >>= 8;
                    }

                    // put back the modified value
                    attrs->writeCB(arm, attrs->id, uValue.u32, offset4);

                } else {

                    // write full register
                    attrs->writeCB(arm, attrs->id, newValue&writeMask, offset);
                }

            } else {

                Uns32 *regValue = getMPRegValue(attrs, arm);

                // detect partial register write
                if((offset&3) || (bytes!=4)) {

                    // access within register
                    Uns8 *regValue8 = (Uns8 *)regValue;
                    Uns32 i;

                    // align writeMask
                    writeMask >>= (offset*8);

                    // update writable bits in the subrange
                    for(i=offset; i<offset+bytes; i++) {

                        // update one byte, preserving read-only bits
                        regValue8[i] = (
                            (regValue8[i] & ~writeMask) |
                            (newValue     &  writeMask)
                        );

                        // shift newValue for next iteration
                        newValue  >>= 8;
                        writeMask >>= 8;
                    }

                } else {

                    // write full register
                    *regValue = (
                        (*regValue & ~writeMask) |
                        (newValue  &  writeMask)
                    );
                }
            }
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
// MP UNDEFINED ADDRESS ACCESS
////////////////////////////////////////////////////////////////////////////////

//
// Read undefined MPCore memory-mapped register
//
static VMI_MEM_READ_FN(readUndefinedIgnore) {

    if(processor) {

        armP arm = (armP)processor;

        // no access to this register
        vmiMessage("W", CPU_PREFIX"_RUMPRZ",
            NO_SRCREF_FMT "Read unimplemented MPCore register at offset 0x%x: return 0",
            NO_SRCREF_ARGS(arm),
            getMPOffset(arm, VA)
        );

        // register is RAZ
        memset(value, 0, bytes);
    }
}

//
// Write undefined MPCore memory-mapped register
//
static VMI_MEM_WRITE_FN(writeUndefinedIgnore) {

    if(processor) {

        armP arm = (armP)processor;

        // no access to this register
        vmiMessage("W", CPU_PREFIX"_WUMPRI",
            NO_SRCREF_FMT "Write unimplemented MPCore register at offset 0x%x: ignored",
            NO_SRCREF_ARGS(arm),
            getMPOffset(arm, VA)
        );
    }
}

//
// Read undefined MPCore memory-mapped register
//
static VMI_MEM_READ_FN(readUndefinedAbort) {

    if(processor) {

        armP arm = (armP)processor;

        // no access to this register
        vmiMessage("W", CPU_PREFIX"_RUMPRA",
            NO_SRCREF_FMT "Read unimplemented MPCore register at offset 0x%x: abort",
            NO_SRCREF_ARGS(arm),
            getMPOffset(arm, VA)
        );

        // take external memory abort
        armExternalMemoryAbort(arm, VA, MEM_PRIV_R);
    }
}

//
// Write undefined MPCore memory-mapped register
//
static VMI_MEM_WRITE_FN(writeUndefinedAbort) {

    if(processor) {

        armP arm = (armP)processor;

        // no access to this register
        vmiMessage("W", CPU_PREFIX"_WUMPRA",
            NO_SRCREF_FMT "Write unimplemented MPCore register at offset 0x%x: abort",
            NO_SRCREF_ARGS(arm),
            getMPOffset(arm, VA)
        );

        // take external memory abort
        armExternalMemoryAbort(arm, VA, MEM_PRIV_W);
    }
}


////////////////////////////////////////////////////////////////////////////////
// MP REGISTER DEFINITION
////////////////////////////////////////////////////////////////////////////////

//
// Type used to record MPCore register bounds
//
typedef struct MPRegAddrS {
    Uns32 lowAddr;      // low address
    Uns32 highAddr;     // high address
} MPRegAddr, *MPRegAddrP;

//
// Callback used to sort MPCore register entries in ascending address
//
static Int32 compareRegAddr(const void *va, const void *vb) {

    const MPRegAddr *a = va;
    const MPRegAddr *b = vb;

    return a->lowAddr<b->lowAddr ? -1 : 1;
}

//
// Install callbacks in a private memory region area that has no mapped
// registers; depending on the register range, this either results in no action
// or an abort
//
static void installUndefinedCBsInt(memDomainP domain, Uns32 low, Uns32 high) {

    Bool          doAbort = ((low>=MP_ABORT_LOW) && (high<=MP_ABORT_HIGH));
    vmiMemReadFn  readCB  = doAbort ? readUndefinedAbort  : readUndefinedIgnore;
    vmiMemWriteFn writeCB = doAbort ? writeUndefinedAbort : writeUndefinedIgnore;

    vmirtMapCallbacks(domain, low, high, readCB, writeCB, 0);
}

//
// Install callbacks in a private memory region area that has no mapped
// registers; depending on the register range, this either results in no action
// or an abort
//
static void installUndefinedCBs(memDomainP domain, Uns32 low, Uns32 high) {
    if((low<MP_ABORT_LOW) && (MP_ABORT_LOW<=high)) {
        installUndefinedCBs(domain, low, MP_ABORT_LOW-1);
        installUndefinedCBs(domain, MP_ABORT_LOW, high);
    } else if((low<=MP_ABORT_HIGH) && (MP_ABORT_HIGH<high)) {
        installUndefinedCBs(domain, low, MP_ABORT_HIGH);
        installUndefinedCBs(domain, MP_ABORT_HIGH+1, high);
    } else {
        installUndefinedCBsInt(domain, low, high);
    }
}

//
// Allocate domain for a set of memory-mapped control registers
//
static void allocControlDomain(armMPGlobalsP mpGlobals) {

    MPRegAddr mpRegAddrs[MP_REG_NUM];
    Uns32     prevLow = 0;
    Uns32     i;

    // create new control domain
    memDomainP domain = vmirtNewDomain("MP control", 32);

    // install MPCore registers
    for(i=0; i<MP_REG_NUM; i++) {

        mpRegAttrsCP attrs   = &mpRegInfo[i];
        MPRegAddrP   regAddr = &mpRegAddrs[i];

        // record register bounds
        regAddr->lowAddr  = attrs->address;
        regAddr->highAddr = attrs->address + attrs->numWords*4 - 1;

        // install callbacks to implement the register
        vmirtMapCallbacks(
            domain, regAddr->lowAddr, regAddr->highAddr,
            readMP, writeMP, (void *)attrs
        );
    }

    // sort register descriptions in ascending address order
    qsort(mpRegAddrs, MP_REG_NUM, sizeof(mpRegAddrs[0]), compareRegAddr);

    // fill unmapped sections in MPCore region with default callback
    for(i=0; i<MP_REG_NUM; i++) {

        MPRegAddrP regAddr = &mpRegAddrs[i];

        if(regAddr->lowAddr > prevLow) {
            installUndefinedCBs(domain, prevLow, regAddr->lowAddr-1);
        }

        prevLow = regAddr->highAddr+1;
    }

    // map final MPCore subregion using default callbacks
    if(prevLow<ARM_MP_BLOCK_SIZE) {
        installUndefinedCBs(domain, prevLow, ARM_MP_BLOCK_SIZE);
    }

    // save control domain
    mpGlobals->controlDomain = domain;
}


////////////////////////////////////////////////////////////////////////////////
// MP REGISTER INSTALLATION AND UNINSTALLATION
////////////////////////////////////////////////////////////////////////////////

//
// Uninstall control register set
//
static void uninstallPrivateMemory(armP arm) {

    armMPGlobalsP mpGlobals      = arm->mpGlobals;
    memDomainP    physicalDomain = mpGlobals->physicalDomain;
    memDomainP    mpDomain       = mpGlobals->mpDomain;
    Uns32         lowAddr        = mpGlobals->lowAddr;
    Uns32         highAddr       = lowAddr+ARM_MP_BLOCK_SIZE-1;

    // emit debug output if required
    if(ARM_DEBUG_MP(arm)) {
        vmiMessage("I", CPU_PREFIX"_UMPC",
            NO_SRCREF_FMT "Uninstall MPCore registers at 0x%08x:0x%08x",
            NO_SRCREF_ARGS(arm),
            lowAddr,
            highAddr
        );
    }

    // replace alias of control registers in MP domain with alias to physical
    // domain
    vmirtAliasMemory(
        physicalDomain,     // physicalDomain
        mpDomain,           // virtualDomain
        lowAddr,            // physicalLowAddr
        highAddr,           // physicalHighAddr
        lowAddr,            // virtualLowAddr
        0                   // mruSet
    );
}

//
// Install the MPCore private memory block in the appropriate location
//
static void installPrivateMemory(armP arm) {

    armMPGlobalsP mpGlobals     = arm->mpGlobals;
    memDomainP    controlDomain = mpGlobals->controlDomain;
    memDomainP    mpDomain      = mpGlobals->mpDomain;
    Uns32         lowAddr       = CP_REG_UNS32(arm, CBAR);
    Uns32         highAddr      = lowAddr+ARM_MP_BLOCK_SIZE-1;

    // emit debug output if required
    if(ARM_DEBUG_MP(arm)) {
        vmiMessage("I", CPU_PREFIX"_IMPC",
            NO_SRCREF_FMT "Install MPCore registers at 0x%08x:0x%08x",
            NO_SRCREF_ARGS(arm),
            lowAddr,
            highAddr
        );
    }

    // create alias of control registers registers in MP domain
    vmirtAliasMemory(
        controlDomain,      // physicalDomain
        mpDomain,           // virtualDomain
        0x00000000,         // physicalLowAddr
        highAddr-lowAddr,   // physicalHighAddr
        lowAddr,            // virtualLowAddr
        0                   // mruSet
    );

    // record the current control block base address
    mpGlobals->lowAddr = lowAddr;
}


////////////////////////////////////////////////////////////////////////////////
// SHARED PERIPHERAL INTERRUPTS
////////////////////////////////////////////////////////////////////////////////

//
// Called by the simulator when an external memory data abort is raised
//
static VMI_NET_CHANGE_FN(externalSPI) {

    armInterruptInfoP ii          = userData;
    armP              arm         = ii->proc;
    Uns32             intNum      = ii->userData;
    Uns32             word        = INTERRUPT_WORD(intNum);
    Uns32             mask        = INTERRUPT_MASK(intNum);
    Uns32            *externalSPI = getExternalSPI(arm, word);
    Bool              oldValue    = (*externalSPI & mask) && True;

    if(newValue != oldValue) {

        // update the correct bit in the externalSPI bitmask
        if(newValue) {
            *externalSPI |= mask;
        } else {
            *externalSPI &= ~mask;
        }

        // update ICDIPR on the rising edge of an edge-sensitive input
        if(newValue && isEdgeSensitive(arm, intNum)) {
            *getICDIPR(arm, word) |= mask;
        }

        // update interrupt lines on individual CPUs
        updateExceptionsIfChanged(arm, word);
    }
}


////////////////////////////////////////////////////////////////////////////////
// RESET
////////////////////////////////////////////////////////////////////////////////

//
// Reset SCU (see nSCURESET)
//
static void resetSCU(armP root) {

    Uns32 numCPUs  = root->configInfo.numCPUs;
    Uns32 powerOff = 3;

    // reset SCUControl
    MPG_REG_UNS32(root, SCUControl) = 0;

    // reset SCUCPUPowerStatus
    MPG_REG_UNS32(root, SCUCPUPowerStatus) = 0;
    if(numCPUs<4) {MPG_FIELD(root, SCUCPUPowerStatus, CPU3status) = powerOff;}
    if(numCPUs<3) {MPG_FIELD(root, SCUCPUPowerStatus, CPU2status) = powerOff;}
    if(numCPUs<2) {MPG_FIELD(root, SCUCPUPowerStatus, CPU1status) = powerOff;}

    // reset SCUFilteringStart and SCUFilteringEnd
    MPG_REG_UNS32(root, SCUFilteringStart) = 0;
    MPG_REG_UNS32(root, SCUFilteringEnd)   = 0;

    // reset SAC and NSAC
    MPG_REG_UNS32(root, SCUSAC)  = MP_WRITE_MASK_SCUSAC;
    MPG_REG_UNS32(root, SCUNSAC) = 0;
}

//
// Reset interrupt distributor registers with one bit per interrupt for word
// index word
//
static void resetDestributorWord(armP arm, Uns32 word) {

    // reset ICDGRPR (Interrupt Group registers)
    *getICDGRPR(arm, word) = 0;

    // reset ICDIER (Interrupt Enable registers)
    *getICDIER(arm, word) = getICDIERReset(arm, word);

    // reset ICDIPR (Interrupt Pending registers)
    writeICDICPR(arm, MPG_ID(ICDICPR0)+word, -1, 0);

    // reset Active Bit registers
    *getActive(arm, word) = 0;
}

//
// Reset global interrupt controller state (see nPERIPHRESET)
//
static void resetInterruptControllerGlobal(armP root) {

    Uns32 intWordsMax = MPG_FIELD(root, ICDICTR, ITLines)+1;
    Uns32 i;

    // reset ICDDCR (Distrubutor Control register)
    writeICDDCR(root, MPG_ID(ICDDCR), 0, 0);

    // set initial state of global bit-per-interrupt registers
    for(i=INT_WORDS_LOCAL; i<intWordsMax; i++) {
        resetDestributorWord(root, i);
    }

    // set initial state of global byte-per-interrupt registers
    for(i=INT_WORDS_LOCALx8; i<intWordsMax*8; i++) {

        // reset ICDIPRI (Interrupt Priority registers)
        writeICDIPRI(root, MPG_ID(ICDIPR00)+i, 0, 0);

        // reset ICDIPTR (Processor Target registers)
        writeICDIPTR(root, MPG_ID(ICDIPTR00)+i, 0, 0);
    }

    // set initial state of global ICDICFR registers
    for(i=INT_WORDS_LOCALx2; i<intWordsMax*2; i++) {
        *getICDICFR(root, i) = MP_INITIAL_ICDICFRN;
    }

    // make initial edge-triggered mask consistent
    for(i=INT_WORDS_LOCAL; i<intWordsMax; i++) {
        refreshEdgeMask(root, i);
    }

    // update exception state
    updateExceptionsGlobal(root);
}

//
// Reset global interrupt controller / timer state (see nPERIPHRESET)
//
static void resetPeriphGlobal(armP root) {
    resetInterruptControllerGlobal(root);
}

//
// Reset local interrupt controller state (see nPERIPHRESET)
//
static void resetInterruptControllerLocal(armP arm) {

    Uns32 i;

    // reset interrupt controller registers with callbacks
    writeICCICR (arm, MPL_ID(ICCICR),  0, 0);
    writeICCPMR (arm, MPL_ID(ICCPMR),  0, 0);
    writeICCBPR (arm, MPL_ID(ICCBPR),  0, 0);
    writeICCABPR(arm, MPL_ID(ICCABPR), 0, 0);

    // set initial state of local bit-per-interrupt registers
    for(i=0; i<INT_WORDS_LOCAL; i++) {
        resetDestributorWord(arm, i);
    }

    // set initial state of local byte-per-interrupt registers
    for(i=0; i<INT_WORDS_LOCALx8; i++) {
        // reset ICDIPRI (Interrupt Priority registers)
        writeICDIPRI(arm, MPG_ID(ICDIPR00)+i, 0, 0);
    }

    // make initial edge-triggered mask consistent
    for(i=0; i<INT_WORDS_LOCAL; i++) {
        refreshEdgeMask(arm, i);
    }

    // update exception state
    updateExceptionsLocal(arm);
}

//
// Reset local timer state (see nPERIPHRESET)
//
static void resetTimerLocal(armP arm) {

    // reset private timer
    writeLoadLT   (arm, MPL_ID(PTLoad),            0, 0);
    writeControlLT(arm, MPL_ID(PTControl),         0, 0);
    writeStatusLT (arm, MPL_ID(PTInterruptStatus), 1, 0);

    // reset watchdog timer (but *not* WTResetStatus)
    writeLoadLT   (arm, MPL_ID(WTLoad),            0, 0);
    writeControlLT(arm, MPL_ID(WTControl),         0, 0);
    writeStatusLT (arm, MPL_ID(WTInterruptStatus), 1, 0);

    // reset WTControl.WDMode
    writeDisableLT(arm, MPL_ID(WTDisable), RESET_MAGIC1, 0);
    writeDisableLT(arm, MPL_ID(WTDisable), RESET_MAGIC2, 0);

    // reset global timer
    writeCounterGT(arm, MPL_ID(GTCounter),         0, 0);
    writeCounterGT(arm, MPL_ID(GTCounter),         0, 4);
    writeControlGT(arm, MPL_ID(GTControl),         0, 0);
    writeStatusGT (arm, MPL_ID(GTInterruptStatus), 1, 0);
    writeCompareGT(arm, MPL_ID(GTComparator),      0, 0);
    writeCompareGT(arm, MPL_ID(GTComparator),      0, 4);
    writeAutoIncGT(arm, MPL_ID(GTAutoIncrement),   0, 0);
}

//
// Reset local interrupt controller / timer state (see nPERIPHRESET)
//
static void resetPeriphLocal(armP arm) {
    resetInterruptControllerLocal(arm);
    resetTimerLocal(arm);
}

//
// Watchdog reset (see nWDRESET)
//
static void resetWatchdog(armP arm) {
    writeResetLT(arm, MPL_ID(WTResetStatus), 1, 0);
}


////////////////////////////////////////////////////////////////////////////////
// RESET CALLBACKS
////////////////////////////////////////////////////////////////////////////////

//
// Watchdog reset signal
//
static VMI_NET_CHANGE_FN(externalResetWD) {

    armInterruptInfoP ii  = userData;
    armP              arm = ii->proc;

    if(newValue) {
        resetWatchdog(arm);
    }
}

//
// SCU reset signal
//
static VMI_NET_CHANGE_FN(externalResetSCU) {

    armInterruptInfoP ii  = userData;
    armP              arm = ii->proc;

    if(newValue) {
        resetSCU(arm);
    }
}

//
// Interrupt controller / timer reset signal
//
static VMI_NET_CHANGE_FN(externalResetPeriph) {

    armInterruptInfoP ii   = userData;
    armP              root = ii->proc;

    if(newValue) {

        armP arm;

        // reset structures local to each CPU
        for(arm=getFirstCPU(root); arm; arm=getNextSibling(arm)) {
            resetPeriphLocal(arm);
        }

        // reset global structures
        resetPeriphGlobal(root);
    }
}


////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Macro used to set initial value of MPCore global register from configuration
//
#define INITIALIZE_MPG_REG(_P, _N) \
    MPG_REG_STRUCT(_P, _N) = MP_REG_STRUCT_DEFAULT(_P, _N)

//
// Macro used to set initial value of MPCore local register from configuration
//
#define INITIALIZE_MPL_REG(_P, _N) \
    MPL_REG_STRUCT(_P, _N) = MP_REG_STRUCT_DEFAULT(_P, _N)

//
// Macro used to set initial value of MPCore register member from configuration
//
#define INITIALIZE_MPG_REG_I(_P, _N, _I) \
    MPG_REG_STRUCT(_P, _N)[_I] = MP_REG_STRUCT_DEFAULT(_P, _N)[_I]

//
// Allocate global MP data structures for the passed processor
//
void armMPAllocGlobal(armP root) {

    Uns32 numCPUs    = root->configInfo.numCPUs;
    Uns32 tagRAMMask = (1 << (2*numCPUs)) - 1;
    Uns32 i;

    // allocate mpGlobals
    armMPGlobalsP mpGlobals = STYPE_CALLOC(armMPGlobals);

    // associate mpGlobals with cluster root processor
    mpGlobals->root = root;
    root->mpGlobals = mpGlobals;

    // save priority mask
    mpGlobals->priMask = getPriMask(root);

    // set constant register values from configuration
    INITIALIZE_MPG_REG(root, SCUConfiguration);
    INITIALIZE_MPG_REG(root, ICDICTR);
    INITIALIZE_MPG_REG(root, ICDIIDR);

    // set constant PERIPH_ID values from configuration
    for(i=0; i<NUM_MEMBERS(MPG_REG_STRUCT(root, PERIPH_ID)); i++) {
        INITIALIZE_MPG_REG_I(root, PERIPH_ID, i);
    }

    // set constant COMPONENT_ID values from configuration
    for(i=0; i<NUM_MEMBERS(MPG_REG_STRUCT(root, COMPONENT_ID)); i++) {
        INITIALIZE_MPG_REG_I(root, COMPONENT_ID, i);
    }

    // update configurable field settings
    MPG_FIELD(root, SCUConfiguration, Number_of_CPUs) = numCPUs-1;
    MPG_FIELD(root, SCUConfiguration, Tag_RAM_Sizes) &= tagRAMMask;
    MPG_FIELD(root, ICDICTR,          CPUNumber)      = numCPUs-1;

    // create cluster-level SPI input nets
    for(i=0; i<MPG_FIELD(root, ICDICTR, ITLines)*32; i++) {
        Uns32 SPINum = i+32;
        char name[32];
        sprintf(name, "SPI%u", SPINum);
        armAddNetInputPort(root, name, externalSPI, SPINum, "Shared peripheral interrupt");
    }

    // connect reset nets
    armAddNetInputPort(root, "scuReset",    externalResetSCU,    0, "SCU reset (active high)");
    armAddNetInputPort(root, "periphReset", externalResetPeriph, 0, "Peripheral reset (active high)");

    // reset cluster-level structures
    resetSCU(root);
    resetPeriphGlobal(root);
}

//
// Allocate local MP data structures for the passed processor
//
void armMPAllocLocal(armP arm) {

    armP          root      = getClusterRoot(arm);
    armMPGlobalsP mpGlobals = root->mpGlobals;

    if(mpGlobals) {

        Uns32         cpuNum   = getIndex(arm);
        Uns32         cpuMask  = (1<<cpuNum);
        armMPLocalsP  mpLocals = STYPE_CALLOC(armMPLocals);
        Uns32         i;

        // duplicate mpGlobals on leaf-level processor object
        arm->mpGlobals = mpGlobals;

        // associate mpLocals with processor
        mpLocals->arm = arm;
        arm->mpLocals = mpLocals;

        // initialize ID register
        INITIALIZE_MPL_REG(arm, ICCIDR);

        // initialize local interrupt enable state
        for(i=0; i<INT_WORDS_LOCAL; i++) {
            *getICDIER(arm, i) = ~getICDIERWriteMask(arm, i);
        }

        // if a multiprocessor implementation, initialize local processor
        // targets registers
        if(isMultiprocessor(arm)) {
            for(i=0; i<INT_WORDS_LOCALx8; i++) {
                *getICDIPTR(arm, i) = getWordMaskx8(i, MP_MASK_PPI_SGI, cpuMask);
            }
        }

        // initialize ICDICFR
        *getICDICFR(arm, 0) = MP_INITIAL_ICDICFR0;
        *getICDICFR(arm, 1) = MP_INITIAL_ICDICFR1;

        // initialize local timers
        newLT(arm, getLT(arm, LT_PRIVATE),  "private",  MP_MASK_PT, expiredLT);
        newLT(arm, getLT(arm, LT_WATCHDOG), "watchdog", MP_MASK_WT, expiredLT);

        // initialize global timer
        newGT(arm, getGT(arm), MP_MASK_GT, expiredGT);

        // connect nets
        armAddNetOutputPort(arm, "wdResetReq", &arm->wdResetReq, "Watchdog interrupt request");
        armAddNetInputPort (arm, "wdReset", externalResetWD, 0, "Watchdog reset (active high)");

        // reset CPU-level structures
        resetPeriphLocal(arm);
        resetWatchdog(arm);
    }
}

//
// Free MP global data structures for the passed root processor
//
void armMPFreeGlobal(armP root) {

    if(root->mpGlobals) {

        STYPE_FREE(root->mpGlobals);

        root->mpGlobals = 0;
    }
}

//
// Free MP local data structures for the passed root processor
//
void armMPFreeLocal(armP arm) {

    armMPLocalsP mpLocals = arm->mpLocals;

    if(mpLocals) {

        // free local timers
        freeLT(getLT(arm, LT_PRIVATE));
        freeLT(getLT(arm, LT_WATCHDOG));

        // free global timer
        freeGT(getGT(arm));

        STYPE_FREE(mpLocals);

        arm->mpLocals = 0;
    }
}

//
// Install private memory region in physical domain if required
//
memDomainP armMPVMInit(armP arm, memDomainP physicalDomain) {

    armMPGlobalsP mpGlobals = arm->mpGlobals;

    if(!mpGlobals) {

        // MPCore not in use
        return physicalDomain;

    } else if(mpGlobals->mpDomain) {

        // MP domain already created
        return mpGlobals->mpDomain;

    } else {

        // create MP domain
        Uns32      physicalBits = vmirtGetDomainAddressBits(physicalDomain);
        Uns64      highAddr     = getHighAddress(physicalBits);
        memDomainP mpDomain     = vmirtNewDomain("MP physical", physicalBits);

        // save domains in mpGlobals structure
        mpGlobals->physicalDomain = physicalDomain;
        mpGlobals->mpDomain       = mpDomain;

        // create control register domain
        allocControlDomain(mpGlobals);

        // create initial mapping of entire physical address space in mpDomain
        vmirtAliasMemory(
            physicalDomain,     // physicalDomain
            mpDomain,           // virtualDomain
            0x00000000,         // physicalLowAddr
            highAddr,           // physicalHighAddr
            0x00000000,         // virtualLowAddr
            0                   // mruSet
        );

        // install private memory block in the effective physical domain at the
        // default location
        installPrivateMemory(arm);

        // return MP domain
        return mpGlobals->mpDomain;
    }
}

//
// Relocate private memory region on CBAR update (not used currently - changes
// to CBAR apparently have no effect on the region location)
//
void armMPRelocate(armP arm) {

    if(arm->mpGlobals) {
        uninstallPrivateMemory(arm);
        installPrivateMemory(arm);
    }
}

