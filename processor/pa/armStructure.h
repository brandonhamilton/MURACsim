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

#ifndef ARM_STRUCTURE_H
#define ARM_STRUCTURE_H

// VMI header files
#include "vmi/vmiTypes.h"

// model header files
#include "armConfig.h"
#include "armCPRegisters.h"
#include "armExceptionTypes.h"
#include "armMode.h"
#include "armParameters.h"
#include "armSIMDVFP.h"
#include "armSIMDVFPRegisters.h"
#include "armTypeRefs.h"
#include "armVariant.h"

// processor debug flags
#define ARM_DISASSEMBLE_MASK    0x00000001
#define ARM_THUMB_MASK          0x00000002
#define ARM_DEBUG_MMU_MASK      0x00000004
#define ARM_JAZ_EE_MASK         0x00000008
#define ARM_DUMP_SDFP_REG_MASK  0x00000010
#define ARM_DEBUG_MP_MASK       0x00000020

#define ARM_DISASSEMBLE(_P)     ((_P)->flags & ARM_DISASSEMBLE_MASK)
#define ARM_THUMB(_P)           ((_P)->flags & ARM_THUMB_MASK)
#define ARM_DEBUG_MMU(_P)       ((_P)->flags & ARM_DEBUG_MMU_MASK)
#define ARM_JAZ_EE(_P)          ((_P)->flags & ARM_JAZ_EE_MASK)
#define ARM_DUMP_SDFP_REG(_P)   ((_P)->flags & ARM_DUMP_SDFP_REG_MASK)
#define ARM_DEBUG_MP(_P)        ((_P)->flags & ARM_DEBUG_MP_MASK)

#define ARM_GPR_BITS            32
#define ARM_GPR_BYTES           (ARM_GPR_BITS/8)
#define ARM_GPR_NUM             16
#define ARM_TEMP_NUM            12
#define ARM_SIMD_REG_NUM        32
#define ARM_SIMD_REG_BITS       64
#define ARM_SIMD_REG_BYTES      8
#define ARM_VFP16_REG_NUM       16

#define ARM_NO_TAG              -1

// simulator compatibility modes
typedef enum armCompatModeE {
    COMPAT_ISA,             // conform to the documented ISA
    COMPAT_GDB,             // conform to gdb simulator
    COMPAT_CODE_SOURCERY    // conform to ARM CodeSourcery toolchain output (ignore SVC)
} armCompatMode;

// arithmetic flags
typedef struct armArithFlagsS {
    Uns8 ZF;        // zero flag
    Uns8 NF;        // sign flag
    Uns8 CF;        // carry flag
    Uns8 VF;        // overflow flag
} armArithFlags;

// other flags
typedef struct armOtherFlagsS {
    Uns8 HI;        // hi flag (created on demand)
    Uns8 LT;        // lt flag (created on demand)
    Uns8 LE;        // le flag (created on demand)
    Uns8 QF;        // saturation flag
} armOtherFlags;

// CPSR structure
typedef struct armCPSRS {
    union {
        struct {
            Uns32 mode : 5;     // mode bits
            Uns32 T    : 1;     // thumb mode bit (mastered in blockMask)
            Uns32 F    : 1;     // FIQ disable bit
            Uns32 I    : 1;     // IRQ disable bit
            Uns32 A    : 1;     // asynchronous abort disable bit
            Uns32 E    : 1;     // endianness execution state bit
            Uns32 IT72 : 6;     // if-then state, bits 7:2
            Uns32 GE   : 4;     // SIMD greater-or-equal flags
            Uns32 _u2  : 4;     // unused bits
            Uns32 J    : 1;     // jazelle mode bit (mastered in blockMask)
            Uns32 IT10 : 2;     // if-then state, bits 1:0
            Uns32 Q    : 1;     // DSP overflow/saturate flag
            Uns32 V    : 1;     // overflow flag
            Uns32 C    : 1;     // carry flag
            Uns32 Z    : 1;     // zero flag
            Uns32 N    : 1;     // sign flag
        } fields;
        Uns32 reg;
    };
} armCPSR;

// Banked registers
typedef struct armBankRegsS {

    // FIQ mode banked registers
    Uns32   R8_fiq;
    Uns32   R9_fiq;
    Uns32   R10_fiq;
    Uns32   R11_fiq;
    Uns32   R12_fiq;
    Uns32   R13_fiq;
    Uns32   R14_fiq;
    armCPSR SPSR_fiq;

    // IRQ mode banked registers
    Uns32   R13_irq;
    Uns32   R14_irq;
    armCPSR SPSR_irq;

    // Supervisor mode banked registers
    Uns32   R13_svc;
    Uns32   R14_svc;
    armCPSR SPSR_svc;

    // Abort mode banked registers
    Uns32   R13_abt;
    Uns32   R14_abt;
    armCPSR SPSR_abt;

    // Undefined mode banked registers
    Uns32   R13_und;
    Uns32   R14_und;
    armCPSR SPSR_und;

} armBankRegs;

// Mask for flags and mode bits in CPSR
#define CPSR_FLAGS  0xff000000
#define CPSR_NZCV   0xf0000000
#define CPSR_MODE   0x0000003f
#define CPSR_THUMB  0x00000020
#define CPSR_FIQE   0x00000040
#define CPSR_IRQE   0x00000080
#define CPSR_ABORTE 0x00000100
#define CPSR_ENDIAN 0x00000200
#define CPSR_IE     (CPSR_FIQE|CPSR_IRQE|CPSR_ABORTE)
#define CPSR_GE3    0x00080000
#define CPSR_GE2    0x00040000
#define CPSR_GE1    0x00020000
#define CPSR_GE0    0x00010000
#define CPSR_GE32   (CPSR_GE3|CPSR_GE2)
#define CPSR_GE10   (CPSR_GE1|CPSR_GE0)
#define CPSR_GE30   (CPSR_GE32|CPSR_GE10)
#define CPSR_J      0x01000000
#define CPSR_IT10   0x06000000
#define CPSR_IT72   0x0000fc00
#define CPSR_IT     (CPSR_IT72|CPSR_IT10)
#define CPSR_EXEC   (CPSR_IT|CPSR_J|CPSR_THUMB)

typedef struct armDomainSetS {
    memDomainP external;        // external memory domain
    memDomainP postTCM;         // external memory domain, post-TCM
    memDomainP postFCSE;        // external memory domain, post-FCSE
    memDomainP vmPriv;          // virtual code domain, privileged mode
    memDomainP vmUser;          // virtual code domain, user mode
    memDomainP tcm;             // TCM domain
} armDomainSet, *armDomainSetP;

typedef enum armPIDSetE {
    APS_PHYS,                   // physical domains (privileged or user mode)
    APS_VM_P,                   // MMU/MPU-managed privileged mode domains
    APS_VM_U,                   // MMU/MPU-managed user mode domains
    APS_LAST                    // KEEP LAST: for sizing
} armPIDSet;

// decoder callback function to decode instruction at the passed address
#define ARM_DECODER_FN(_NAME) void _NAME( \
    armP                arm,     \
    Uns32               thisPC,  \
    armInstructionInfoP info     \
)
typedef ARM_DECODER_FN((*armDecoderFn));

// Callback function to return size of instruction at the passed for the specified mode
#define ARM_ISIZE_FN(_NAME) Uns32 _NAME( \
    armP  arm,     \
    Uns32 thisPC,  \
    Bool  isThumb  \
)
typedef ARM_ISIZE_FN((*armIsizeFn));

// opaque type for MPU protection region
typedef struct protRegionS *protRegionP;

// opaque type for TLB structure
typedef struct armTLBS *armTLBP;

// opaque type for DMA unit structure
typedef struct armDMAUnitS *armDMAUnitP;

// SIMD/VFP register bank
typedef union armVFPRS {
	Uns8  b[ARM_SIMD_REG_NUM*8];  // when viewed as bytes
	Uns16 h[ARM_SIMD_REG_NUM*4];  // When viewed as 16 bit halfwords
    Uns32 w[ARM_SIMD_REG_NUM*2];  // when viewed as 32-bit words
    Uns64 d[ARM_SIMD_REG_NUM];    // when viewed as 64-bit double words
} armVFPR;

// floating point control word type
typedef union armFPCWU {
    Uns32            u32;   // when viewed as composed value
    vmiFPControlWord cw;    // when viewed as fields
} armFPCW;

// member of net port list
typedef struct armNetPortS {
    vmiNetPort port;
    struct     armNetPortS *next;
} armNetPort;

// this holds processor and vector information for an interrupt
typedef struct arm32InterruptInfoS {
    armP  proc;
    Uns32 userData;
} armInterruptInfo, *armInterruptInfoP , **armInterruptInfoPP;

// processor structure
typedef struct armS {

    // TRUE PROCESSOR REGISTERS
    armArithFlags  aflags;              // arithmetic flags
    armOtherFlags  oflags;              // other flags
    Uns32          regs[ARM_GPR_NUM];   // current mode GPR registers
    armCPSR        CPSR;                // current program status register
    armCPSR        SPSR;                // saved program status register

    // SIMULATOR SUPPORT
    Uns32          exclusiveTag;        // tag for active exclusive access
    Uns32          exclusiveTagMask;    // mask to select exclusive tag bits
    armBlockMask   blockMask;           // current blockMask
    Uns32          temps[ARM_TEMP_NUM]; // temporary registers
    Uns8           divideTarget;        // target of divide instruction
    Uns8           fpModeRT;            // current floating point mode (run time)
    Uns8           itStateRT;           // if-then state (run time)
    Uns8           itStateMT;           // if-then state (morph time)
    Uns8           disable;             // reason for disable
    Uns8           event;               // event register
    armP           parent;              // parent (in multiprocessor cluster)
    Uns32          teeNZMask      :16;  // mask of registers already zero-checked
    armMode        mode           : 8;  // current processor mode
    armException   exception      : 8;  // current processor exception
    armFPMode      fpModeMT       : 2;  // current floating point mode (morph time)
    Bool           validHI        : 1;  // is hi flag valid?
    Bool           validLT        : 1;  // is lt flag valid?
    Bool           validLE        : 1;  // is le flag valid?
    Bool           checkEndian    : 1;  // check endian using blockMask?
    Bool           checkInterwork : 1;  // check interwork using blockMask?
    Bool           checkThumbEE   : 1;  // check Thumb/ThumbEE using blockMask?
    Bool           checkL4        : 1;  // check ARMv5T interwork using blockMask?
    Bool           checkUnaligned : 1;  // check alignment mode using blockMask?
    Bool           checkEnableSWP : 1;  // check SWP/SWPB enable using blockMask?
    Bool           denormalInput  : 1;  // input denormal sticky flag

    // COPROCESSOR AND BANKED REGISTERS (INFREQUENTLY USED AT RUN TIME)
    Uns8           internalInt;         // internal (effective) pending mask
    Uns8           externalInt;         // external (legacy) pending mask
    Uns8           GICInt;              // GIC pending mask
    armBankRegs    bank;                // banked registers
    armCPRegs      cp;                  // coprocessor registers
    armCPWarningP  cpWarn;              // bad accesses already warned

    // MPCORE REGISTERS
    armMPGlobalsP  mpGlobals;           // MPCore global registers
    armMPLocalsP   mpLocals;            // MPCore local registers

    // SIMD/VFP REGISTERS
    armFPCW        currentCW;           // current control word
    armFPCW        VFPCW;               // VFP control word
    armArithFlags  sdfpAFlags;          // FPU comparison flags
    Uns8           sdfpFlags;           // FPU operation flags
    Uns8           sdfpSticky;          // FPU sticky flags
    Uns8		   simdQC;				// SIMD FPSCR.QC (cumulative saturation) flag
    armVFPR        vregs;               // SIMD/VFP data registers
    armSDFPRegs    sdfp;                // SIMD/VFP control registers

    // VARIANT CONFIGURATION
    Uns32          flags;               // configuration flags
    armConfig      configInfo;          // configuration register defaults
    Uns32          rMaskAPSR;           // APSR read mask
    Uns32          wMaskAPSR;           // APSR write mask
    Uns32          rMaskCPSR;           // CPSR read mask
    Uns32          wMaskCPSR;           // CPSR write mask
    Uns32          wMaskSPSR;           // SPSR write mask
    Bool           simEx          :1;   // simulate exceptions?
    Bool           verbose        :1;   // verbose messages enabled?
    armCompatMode  compatMode     :2;   // compatibility mode
    Bool           showHiddenRegs :1;   // show hidden registers in reg dump
    Bool           UAL            :1;   // disassemble using UAL syntax
    Bool           useARMv5FSR    :1;   // use ARMv5-format FSR?
    Bool           useARMv5TTBR   :1;   // use ARMv5-format TTBR?
    Bool           useARMv5PAC    :1;   // use ARMv5-format PAC registers?

    // WARNING CONTROL
    Bool           warnCacheFuncR :1;   // warned about Cache Function read?
    Bool           warnCacheFuncW :1;   // warned about Cache Function write?
    Bool           warnVFP        :1;   // warned about VFP not on variant?
    Bool           warnAdvSIMD    :1;   // warned about AdvSIMD not on variant?

    // MEMORY SUBSYSTEM SUPPORT
    Bool           restoreDomain  :1;   // whether to resore domain (LDRT, STRT)
    Bool           isVAtoPA       :1;   // whether VA->PA translation active
    union {
        armTLBP     itlb;               // instruction/unified TLB
        protRegionP impu;               // instruction/unified MPU
    };
    union {
        armTLBP     dtlb;               // data TLB (if not unified)
        protRegionP dmpu;               // data MPU (if not unified)
    };
    armDomainSet   ids;                 // instruction domain set
    armDomainSet   dds;                 // data domain set
    Uns8           pids[APS_LAST];      // assumed PID for each domain set
    Uns32          dacs[APS_LAST];      // assumed DAC for each domain set
    armDMAUnitP    dmaUnits;            // DMA units
    armDMAUnitP    dmaActive;           // active DMA unit

    // NET HANDLES (used for external interrupt controllers)
    Uns32          nDMAIRQ;             // DMA interrupt request
    Uns32          wdResetReq;          // MPCore watchdog interrupt request

    // PROGRAMMER'S VIEW HANDLES
    vmiViewEventP  mmuEnableEvent;      // view event for processor exceptions

    // INTERCEPT LIBRARY SUPPORT
    armDecoderFn   decoderCB;			// generic instruction decoder
    armIsizeFn     isizeCB;				// instruction size callback

    // PAREMETER VALUES
    armParamValuesP parameterValues;

    // MURAC IRQ
    Uns32          brarch;

    // PORT LIST
    armNetPortP    firstPort;           // first port in port list
    armNetPortP    lastPort;            // last port in port list

} arm;

// mode predicates
#define GET_MODE(_A)                ((_A)->CPSR.fields.mode)
#define IN_USER_MODE(_A)            (GET_MODE(_A)==ARM_CPSR_USER)
#define IN_SYSTEM_MODE(_A)          (GET_MODE(_A)==ARM_CPSR_SYSTEM)
#define IN_BASE_MODE(_A)            (IN_USER_MODE(_A) || IN_SYSTEM_MODE(_A))
#define IN_THUMB_MODE(_A)           (((_A)->blockMask & ARM_BM_THUMB)  && 1)
#define IN_JAZ_EE_MODE(_A)          (((_A)->blockMask & ARM_BM_JAZ_EE) && 1)
#define IN_JAZELLE_MODE(_A)         (IN_JAZ_EE_MODE(_A) && !IN_THUMB_MODE(_A))
#define IN_THUMB_EE_MODE(_A)        (IN_JAZ_EE_MODE(_A) &&  IN_THUMB_MODE(_A))
#define IN_PRIV_MMU_MPU_MODE(_A)    (!IN_USER_MODE(_A) && MMU_MPU_ENABLED(_A))
#define IN_SIMD_MODE(_A)            ((_A)->fpModeRT==ARM_FPM_SIMD)
#define IN_VFP_MODE(_A)             ((_A)->fpModeRT==ARM_FPM_VFP)
#define ARM_PC_DELTA(_A)            (IN_THUMB_MODE(_A) ? 4 : 8)

#endif

