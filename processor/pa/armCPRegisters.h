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

#ifndef ARM_CP_REGISTERS_H
#define ARM_CP_REGISTERS_H

// model header files
#include "armRegisters.h"


// -----------------------------------------------------------------------------
// COMMON DEFINITIONS
// -----------------------------------------------------------------------------

//
// Implementor codes
//
typedef enum armImplementorE {
    AI_ARM   = 0x41,
    AI_DEC   = 0x44,
    AI_INTEL = 0x69,
} armImplementor;

// this code defines an invalid coprocessor register specification
#define CP_INVALID -1

// construct enumeration member name from register name
#define CP_ID(_R)                       CP_ID_##_R

// morph-time macro to access a coprocessor register
#define ARM_CP_REG(_ID)                 ARM_CPU_REG(cp.regs[_ID])

// access an entire coprocessor register as an Uns32
#define CP_REG_UNS32(_P, _R)            ((_P)->cp.regs[CP_ID(_R)])

// access an entire coprocessor register as a structure
#define CP_REG_STRUCT(_P, _R)           ((_P)->cp.fields._R)

// access a field in an coprocessor register
#define CP_FIELD(_P, _R, _F)            (CP_REG_STRUCT(_P, _R)._F)

// get mask to use when writing a coprocessor register
#define CP_MASK_UNS32(_P, _R)           ((_P)->configInfo.cpRegMasks._R.value32)

// get mask to use when writing a coprocessor register field
#define CP_MASK_FIELD(_P, _R, _F)       ((_P)->configInfo.cpRegMasks._R.fields._F)

// access default value for an entire coprocessor register as a structure
#define CP_REG_STRUCT_DEFAULT(_P, _R)   ((_P)->configInfo.cpRegDefaults._R)

// access default value for a field in a coprocessor register
#define CP_FIELD_DEFAULT(_P, _R, _F)    (CP_REG_STRUCT_DEFAULT(_P, _R)._F)

// are multiprocessing extensions implemented?
#define CP_MP_IMPLEMENTED(_P)           CP_FIELD(_P, MPIDR, Implemented)

// should CPUID registers be used to determine feature presence?
#define CP_USE_CPUID(_P) (CP_FIELD(_P, MIDR.postArm7, Architecture)==0xf)

// access to ID_ISAR field
#define ARM_ISAR(_N, _F) CP_FIELD(arm, ID_ISAR##_N, _F)

// access to ID_MMFR field
#define ARM_MMFR(_N, _F) CP_FIELD(arm, ID_MMFR##_N, _F)


//
// Identifiers for each implemented coprocessor register
//
typedef enum armCPRegIdE {

    // coprocessor 14 register entries represented in processor structure
    CP_ID(TEECR),
    CP_ID(TEEHBR),
    CP_ID(JIDR),
    CP_ID(JOSCR),
    CP_ID(JMCR),

    // coprocessor 15 register entries represented in processor structure
    CP_ID(MIDR),
    CP_ID(CTR),
    CP_ID(TCMTR),
    CP_ID(TLBTR),
    CP_ID(MPUIR),
    CP_ID(MPIDR),
    CP_ID(ID_PFR0),
    CP_ID(ID_PFR1),
    CP_ID(ID_DFR0),
    CP_ID(ID_AFR0),
    CP_ID(ID_MMFR0),
    CP_ID(ID_MMFR1),
    CP_ID(ID_MMFR2),
    CP_ID(ID_MMFR3),
    CP_ID(ID_ISAR0),
    CP_ID(ID_ISAR1),
    CP_ID(ID_ISAR2),
    CP_ID(ID_ISAR3),
    CP_ID(ID_ISAR4),
    CP_ID(ID_ISAR5),
    CP_ID(ID_RESERVED),
    CP_ID(CLIDR),
    CP_ID(AIDR),
    CP_ID(CSSELR),
    CP_ID(SCTLR),
    CP_ID(ACTLR),
    CP_ID(CPACR),
    CP_ID(TTBR),
    CP_ID(TTBR0),
    CP_ID(TTBR1),
    CP_ID(TTBCR),
    CP_ID(DCR),
    CP_ID(ICR),
    CP_ID(DACR),
    CP_ID(DBR),
    CP_ID(FSR),
    CP_ID(DFSR),
    CP_ID(IFSR),
    CP_ID(ADFSR),
    CP_ID(AIFSR),
    CP_ID(FAR),
    CP_ID(DFAR),
    CP_ID(WFAR),
    CP_ID(IFAR),
    CP_ID(RGNR),
    CP_ID(PAR),
    CP_ID(DCLR),
    CP_ID(ICLR),
    CP_ID(DTCMRR),
    CP_ID(ITCMRR),
    CP_ID(PRRR),
    CP_ID(NMRR),
    CP_ID(DMAPresent),
    CP_ID(DMAQueued),
    CP_ID(DMARunning),
    CP_ID(DMAInterrupting),
    CP_ID(DMAUserAccessibility),
    CP_ID(DMAChannel),
    CP_ID(FCSEIDR),
    CP_ID(CONTEXTIDR),
    CP_ID(TPIDRURW),
    CP_ID(TPIDRURO),
    CP_ID(TPIDRPRW),
    CP_ID(PCR),
    CP_ID(CBAR),
    CP_ID(TLBHR),
    CP_ID(TLBRI),
    CP_ID(TLBWI),

    // marker for pseudo-registers (not represented in processor structure)
    CP_ID(FirstPseudoReg),

    // TLB operations
    CP_ID(V2PCWPR) = CP_ID(FirstPseudoReg),
    CP_ID(V2PCWPW),
    CP_ID(V2PCWUR),
    CP_ID(V2PCWUW),
    CP_ID(TLBIALLIS),
    CP_ID(ITLBIALL),
    CP_ID(DTLBIALL),
    CP_ID(TLBIALL),
    CP_ID(TLBIMVAIS),
    CP_ID(ITLBIMVA),
    CP_ID(DTLBIMVA),
    CP_ID(TLBIMVA),
    CP_ID(TLBIASIDIS),
    CP_ID(ITLBIASID),
    CP_ID(DTLBIASID),
    CP_ID(TLBIASID),
    CP_ID(TLBIMVAAIS),
    CP_ID(TLBIMVAA),
    CP_ID(DTLBLR),
    CP_ID(ITLBLR),
    CP_ID(TLBLDVA),
    CP_ID(TLBLDPA),
    CP_ID(TLBLDATTR),

    // MPU operations, pre ARMv6
    CP_ID(DAPR),
    CP_ID(IAPR),
    CP_ID(DEAPR),
    CP_ID(IEAPR),
    CP_ID(DMRR0),
    CP_ID(DMRR1),
    CP_ID(DMRR2),
    CP_ID(DMRR3),
    CP_ID(DMRR4),
    CP_ID(DMRR5),
    CP_ID(DMRR6),
    CP_ID(DMRR7),
    CP_ID(IMRR0),
    CP_ID(IMRR1),
    CP_ID(IMRR2),
    CP_ID(IMRR3),
    CP_ID(IMRR4),
    CP_ID(IMRR5),
    CP_ID(IMRR6),
    CP_ID(IMRR7),

    // MPU operations
    CP_ID(DRBAR),
    CP_ID(IRBAR),
    CP_ID(DRSR),
    CP_ID(IRSR),
    CP_ID(DRACR),
    CP_ID(IRACR),

    // common cache operations (MCR/MRC)
    CP_ID(CP15WFI),
    CP_ID(CP15ISB),
    CP_ID(CP15DSB),
    CP_ID(CP15DMB),
    CP_ID(PrefetchICacheLine),

    // ARMv7 cache operations (MCR/MRC)
    CP_ID(CCSIDR),
    CP_ID(ICIALLUIS),
    CP_ID(BPIALLIS),
    CP_ID(ICIALLU),
    CP_ID(ICIMVAU),
    CP_ID(BPIALL),
    CP_ID(BPIMVA),
    CP_ID(DCIMVAC),
    CP_ID(DCISW),
    CP_ID(DCCMVAC),
    CP_ID(DCCSW),
    CP_ID(DCCMVAU),
    CP_ID(DCCIMVAC),
    CP_ID(DCCISW),

    // ARMv6 cache operations (MCR/MRC)
    CP_ID(InvalidateICache),
    CP_ID(InvalidateICacheLineMVA),
    CP_ID(InvalidateICacheLineSW),
    CP_ID(FlushBranchTargetCache),
    CP_ID(FlushBranchTargetEntry),
    CP_ID(InvalidateDCache),
    CP_ID(InvalidateDCacheLineMVA),
    CP_ID(InvalidateDCacheLineSW),
    CP_ID(InvalidateUnified),
    CP_ID(CleanDCache),
    CP_ID(CleanDCacheLineMVA),
    CP_ID(CleanDCacheLineSW),
    CP_ID(DirtyStatus),
    CP_ID(CleanUnified),
    CP_ID(BlockTransferStatus),
    CP_ID(StopPrefetchRange),
    CP_ID(CleanInvalDCache),
    CP_ID(CleanInvalDCacheLineMVA),
    CP_ID(CleanInvalDCacheLineSW),
    CP_ID(CleanInvalUnified),

    // ARMv5 cache operations (MCR/MRC)
    CP_ID(TestCleanDCache),
    CP_ID(TestCleanInvalDCache),

    // ARMv6 cache operations (MCRR/MRRC)
    CP_ID(InvalICacheRange),
    CP_ID(InvalDCacheRange),
    CP_ID(CleanDCacheRange),
    CP_ID(PrefetchICacheRange),
    CP_ID(PrefetchDCacheRange),
    CP_ID(CleanInvalDCacheRange),
    CP_ID(Ignored),

    // DMA operations
    CP_ID(DMAStop),
    CP_ID(DMAStart),
    CP_ID(DMAClear),
    CP_ID(DMAControl),
    CP_ID(DMAInternalStart),
    CP_ID(DMAExternalStart),
    CP_ID(DMAInternalEnd),
    CP_ID(DMAStatus),
    CP_ID(DMAContextID),

    // CortexA9MP CRn15 operations
    CP_ID(NEONB),

    // keep last (used to define size of the enumeration)
    CP_ID(Size),

} armCPRegId;

// use this to declare a register structure below
#define CP_REG_STRUCT_DECL(_N) armCPReg_##_N


// -----------------------------------------------------------------------------
// COPROCESSOR 14 REGISTERS
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Coprocessor 14 register 6/0: TEECR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 XED :  1;
    Uns32 _u1 : 31;
} CP_REG_STRUCT_DECL(TEECR);

#define CP_WRITE_MASK_TEECR 0x00000001

// -----------------------------------------------------------------------------
// Coprocessor 14 register 6/1: TEEHBR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 SBZ         :  2;
    Uns32 HandlerBase : 30;
} CP_REG_STRUCT_DECL(TEEHBR);

#define CP_WRITE_MASK_TEEHBR 0xfffffffc

// -----------------------------------------------------------------------------
// Coprocessor 14 register 7/0: JIDR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32          _u1            : 12;
    Uns32          Subarchitecture:  8;
    armImplementor Implementor    :  8;
    Uns32          Architecture   :  4;
} CP_REG_STRUCT_DECL(JIDR);

#define CP_WRITE_MASK_JIDR 0x00000000

// -----------------------------------------------------------------------------
// Coprocessor 14 register 7/1: JOSCR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 CD  :  1;
    Uns32 CV  :  1;
    Uns32 _u1 : 30;
} CP_REG_STRUCT_DECL(JOSCR);

#define CP_WRITE_MASK_JOSCR 0x00000000

// -----------------------------------------------------------------------------
// Coprocessor 14 register 7/2: JMCR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 JE  :  1;
    Uns32 _u1 : 31;
} CP_REG_STRUCT_DECL(JMCR);

#define CP_WRITE_MASK_JMCR 0x00000000


// -----------------------------------------------------------------------------
// COPROCESSOR 15 REGISTERS
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=0 opc2=0: MIDR
// -----------------------------------------------------------------------------

typedef union {
    struct {
        Uns32          Revision    :  4;
        Uns32          PartNum     : 12;
        Uns32          Variant     :  7;
        Uns32          A           :  1;
        armImplementor Implementor :  8;
    } arm7;
    struct {
        Uns32          Revision    :  4;
        Uns32          PartNum     : 12;
        Uns32          Architecture:  4;
        Uns32          Variant     :  4;
        armImplementor Implementor :  8;
    } postArm7;
} CP_REG_STRUCT_DECL(MIDR);

#define CP_WRITE_MASK_MIDR 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=0 opc2=1: CTR
// -----------------------------------------------------------------------------

#define CT_PRE_V7  0
#define CT_POST_V7 4

typedef union {
    struct {
        Uns32 Isize_len   :  2;
        Uns32 Isize_M     :  1;
        Uns32 Isize_assoc :  3;
        Uns32 Isize_size  :  4;
        Uns32 Isize_u1    :  1;
        Uns32 Isize_P     :  1;
        Uns32 Dsize_len   :  2;
        Uns32 Dsize_M     :  1;
        Uns32 Dsize_assoc :  3;
        Uns32 Dsize_size  :  4;
        Uns32 Dsize_u1    :  1;
        Uns32 Dsize_P     :  1;
        Uns32 S           :  1;
        Uns32 ctype       :  4;
        Uns32 format      :  3;
    } preV7;
    struct {
        Uns32 IMinLine    :  4;
        Uns32 _u1         : 10;
        Uns32 L1Ipolicy   :  2;
        Uns32 DMinLine    :  4;
        Uns32 ERG         :  4;
        Uns32 CWG         :  4;
        Uns32 _u3         :  1;
        Uns32 format      :  3;
    } postV7;
} CP_REG_STRUCT_DECL(CTR);

#define CP_WRITE_MASK_CTR 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=0 opc2=2: TCMTR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 ITCM        :  3;
    Uns32 _u1         : 13;
    Uns32 DTCM        :  3;
    Uns32 _u2         : 13;
} CP_REG_STRUCT_DECL(TCMTR);

#define CP_WRITE_MASK_TCMTR 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=0 opc2=3: TLBTR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 nU     : 1;
    Uns32 _u1    : 7;
    Uns32 DLsize : 8;
    Uns32 ILsize : 8;
    Uns32 _u2    : 8;
} CP_REG_STRUCT_DECL(TLBTR);

#define CP_WRITE_MASK_TLBTR 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=0 opc2=4: MPUIR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 nU      : 1;
    Uns32 _u1     : 7;
    Uns32 DRegion : 8;
    Uns32 IRegion : 8;
    Uns32 _u2     : 8;
} CP_REG_STRUCT_DECL(MPUIR);

#define CP_WRITE_MASK_MPUIR 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=0 opc2=5: MPIDR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 AffinityLevel0 : 8;
    Uns32 AffinityLevel1 : 8;
    Uns32 AffinityLevel2 : 8;
    Uns32 MT             : 1;
    Uns32 _u1            : 5;
    Uns32 U              : 1;
    Uns32 Implemented    : 1;
} CP_REG_STRUCT_DECL(MPIDR);

#define CP_WRITE_MASK_MPIDR 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=1 opc2=0: ID_PFR0
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 State0 :  4;
    Uns32 State1 :  4;
    Uns32 State2 :  4;
    Uns32 State3 :  4;
    Uns32 _u1    : 16;
} CP_REG_STRUCT_DECL(ID_PFR0);

#define CP_WRITE_MASK_ID_PFR0 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=1 opc2=1: ID_PFR1
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 ProgrammersModel      :  4;
    Uns32 SecurityExtension     :  4;
    Uns32 MicroProgrammersModel :  4;
    Uns32 _u1                   : 20;
} CP_REG_STRUCT_DECL(ID_PFR1);

#define CP_WRITE_MASK_ID_PFR1 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=1 opc2=2: ID_DFR0
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 CoreDebug     : 4;
    Uns32 SecureDebug   : 4;
    Uns32 EmbeddedDebug : 4;
    Uns32 TraceDebugCP  : 4;
    Uns32 TraceDebugMM  : 4;
    Uns32 MicroDebug    : 4;
    Uns32 _u1           : 8;
} CP_REG_STRUCT_DECL(ID_DFR0);

#define CP_WRITE_MASK_ID_DFR0 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=1 opc2=3: ID_AFR0
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 _u1 : 32;
} CP_REG_STRUCT_DECL(ID_AFR0);

#define CP_WRITE_MASK_ID_AFR0 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=1 opc2=4: ID_MMFR0
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 VMSA               : 4;
    Uns32 PMSA               : 4;
    Uns32 OuterShareability  : 4;
    Uns32 ShareabilityLevels : 4;
    Uns32 TCMSupport         : 4;
    Uns32 AuxillaryRegisters : 4;
    Uns32 FCSESupport        : 4;
    Uns32 InnerShareability  : 4;
} CP_REG_STRUCT_DECL(ID_MMFR0);

#define CP_WRITE_MASK_ID_MMFR0 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=1 opc2=5: ID_MMFR1
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 L1VAHarvard : 4;
    Uns32 L1VAUnified : 4;
    Uns32 L1SWHarvard : 4;
    Uns32 L1SWUnified : 4;
    Uns32 L1Harvard   : 4;
    Uns32 L1Unified   : 4;
    Uns32 L1TestClean : 4;
    Uns32 BTB         : 4;
} CP_REG_STRUCT_DECL(ID_MMFR1);

#define CP_WRITE_MASK_ID_MMFR1 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=1 opc2=6: ID_MMFR2
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 L1FgndPrefetchHarvard : 4;
    Uns32 L1BgndPrefetchHarvard : 4;
    Uns32 L1MaintRangeHarvard   : 4;
    Uns32 TLBMaintHarvard       : 4;
    Uns32 TLBMaintUnified       : 4;
    Uns32 MemoryBarrierCP15     : 4;
    Uns32 WaitForInterruptStall : 4;
    Uns32 HWAccessFlag          : 4;
} CP_REG_STRUCT_DECL(ID_MMFR2);

#define CP_WRITE_MASK_ID_MMFR2 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=1 opc2=7: ID_MMFR3
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 HierMaintSW  : 4;
    Uns32 HierMaintMVA : 4;
    Uns32 BPMaint      : 4;
    Uns32 MaintBCast   : 4;
    Uns32 _u1          : 4;
    Uns32 CoherentWalk : 4;
    Uns32 _u2          : 4;
    Uns32 Supersection : 4;
} CP_REG_STRUCT_DECL(ID_MMFR3);

#define CP_WRITE_MASK_ID_MMFR3 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=2 opc2=0: ID_ISAR0
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Swap_instrs      : 4;
    Uns32 BitCount_instrs  : 4;
    Uns32 BitField_instrs  : 4;
    Uns32 CmpBranch_instrs : 4;
    Uns32 Coproc_instrs    : 4;
    Uns32 Debug_instrs     : 4;
    Uns32 Divide_instrs    : 4;
    Uns32 _u1              : 4;
} CP_REG_STRUCT_DECL(ID_ISAR0);

#define CP_WRITE_MASK_ID_ISAR0 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=2 opc2=1: ID_ISAR1
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Endian_instrs    : 4;
    Uns32 Except_instrs    : 4;
    Uns32 Except_AR_instrs : 4;
    Uns32 Extend_instrs    : 4;
    Uns32 IfThen_instrs    : 4;
    Uns32 Immediate_instrs : 4;
    Uns32 Interwork_instrs : 4;
    Uns32 Jazelle_instrs   : 4;
    Uns32 Murac_instrs     : 4;
} CP_REG_STRUCT_DECL(ID_ISAR1);

#define CP_WRITE_MASK_ID_ISAR1 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=2 opc2=2: ID_ISAR2
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 LoadStore_instrs      : 4;
    Uns32 MemHint_instrs        : 4;
    Uns32 MultiAccessInt_instrs : 4;
    Uns32 Mult_instrs           : 4;
    Uns32 MultS_instrs          : 4;
    Uns32 MultU_instrs          : 4;
    Uns32 PSR_AR_instrs         : 4;
    Uns32 Reversal_instrs       : 4;
} CP_REG_STRUCT_DECL(ID_ISAR2);

#define CP_WRITE_MASK_ID_ISAR2 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=2 opc2=3: ID_ISAR3
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Saturate_instrs     : 4;
    Uns32 SIMD_instrs         : 4;
    Uns32 SVC_instrs          : 4;
    Uns32 SynchPrim_instrs    : 4;
    Uns32 TabBranch_instrs    : 4;
    Uns32 ThumbCopy_instrs    : 4;
    Uns32 TrueNOP_instrs      : 4;
    Uns32 T2ExeEnvExtn_instrs : 4;
} CP_REG_STRUCT_DECL(ID_ISAR3);

#define CP_WRITE_MASK_ID_ISAR3 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=2 opc2=4: ID_ISAR4
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Unpriv_instrs         : 4;
    Uns32 WithShifts_instrs     : 4;
    Uns32 Writeback_instrs      : 4;
    Uns32 SMI_instrs            : 4;
    Uns32 Barrier_instrs        : 4;
    Uns32 SynchPrim_instrs_frac : 4;
    Uns32 PSR_M_instrs          : 4;
    Uns32 SWP_frac              : 4;
} CP_REG_STRUCT_DECL(ID_ISAR4);

#define CP_WRITE_MASK_ID_ISAR4 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=2 opc2=5: ID_ISAR5
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 _u1 : 32;
} CP_REG_STRUCT_DECL(ID_ISAR5);

#define CP_WRITE_MASK_ID_ISAR5 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=0 CRm=2 opc2=5: ID_RESERVED
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 _u1 : 32;
} CP_REG_STRUCT_DECL(ID_RESERVED);

#define CP_WRITE_MASK_ID_RESERVED 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=1 CRm=0 opc2=0: CCSIDR
// -----------------------------------------------------------------------------

#define ARM_NUM_CCSIDR 7

typedef struct {
    Uns32 LineSize      :  3;
    Uns32 Associativity : 10;
    Uns32 NumSets       : 15;
    Uns32 WA            :  1;
    Uns32 RA            :  1;
    Uns32 WB            :  1;
    Uns32 WT            :  1;
} CP_REG_STRUCT_DECL(CCSIDR);

#define CP_WRITE_MASK_CCSIDR 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=1 CRm=0 opc2=1: CLIDR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Ctype1 : 3;
    Uns32 Ctype2 : 3;
    Uns32 Ctype3 : 3;
    Uns32 Ctype4 : 3;
    Uns32 Ctype5 : 3;
    Uns32 Ctype6 : 3;
    Uns32 Ctype7 : 3;
    Uns32 LoUIS  : 3;
    Uns32 LoC    : 3;
    Uns32 LoUU   : 3;
    Uns32 _u1    : 2;
} CP_REG_STRUCT_DECL(CLIDR);

#define CP_WRITE_MASK_CLIDR 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=1 CRm=0 opc2=7: AIDR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 value;
} CP_REG_STRUCT_DECL(AIDR);

#define CP_WRITE_MASK_AIDR 0x00000000

// -----------------------------------------------------------------------------
// CRn=0 opc1=2 CRm=0 opc2=0: CSSELR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 InD   :  1;
    Uns32 Level :  3;
    Uns32 _u1   : 28;
} CP_REG_STRUCT_DECL(CSSELR);

#define CP_WRITE_MASK_CSSELR 0x0000000f

// -----------------------------------------------------------------------------
// CRn=1 opc1=0 CRm=0 opc2=0: SCTLR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 M   : 1;      //  0
    Uns32 A   : 1;      //  1
    Uns32 C   : 1;      //  2
    Uns32 W   : 1;      //  3
    Uns32 P   : 1;      //  4
    Uns32 D   : 1;      //  5
    Uns32 L   : 1;      //  6
    Uns32 B   : 1;      //  7
    Uns32 S   : 1;      //  8
    Uns32 R   : 1;      //  9
    Uns32 SW  : 1;      // 10
    Uns32 Z   : 1;      // 11
    Uns32 I   : 1;      // 12
    Uns32 V   : 1;      // 13
    Uns32 RR  : 1;      // 14
    Uns32 L4  : 1;      // 15
    Uns32 DT  : 1;      // 16
    Uns32 HA  : 1;      // 17
    Uns32 IT  : 1;      // 18
    Uns32 DZ  : 1;      // 19
    Uns32 _u1 : 1;      // 20
    Uns32 FI  : 1;      // 21
    Uns32 U   : 1;      // 22
    Uns32 XP  : 1;      // 23
    Uns32 VE  : 1;      // 24
    Uns32 EE  : 1;      // 25
    Uns32 L2  : 1;      // 26
    Uns32 NMI : 1;      // 27
    Uns32 TRE : 1;      // 28
    Uns32 AFE : 1;      // 29
    Uns32 TE  : 1;      // 30
    Uns32 IE  : 1;      // 31
} CP_REG_STRUCT_DECL(SCTLR);

// true mask for SCTLR register comes from configuration information
#define CP_WRITE_MASK_SCTLR 0x00000000

// -----------------------------------------------------------------------------
// CRn=1 opc1=0 CRm=0 opc2=1: ACTLR
// -----------------------------------------------------------------------------

typedef union {
    struct {
        Uns32 RS  :  1;
        Uns32 DB  :  1;
        Uns32 SB  :  1;
        Uns32 TR  :  1;
        Uns32 RA  :  1;
        Uns32 RV  :  1;
        Uns32 CZ  :  1;
        Uns32 _u1 : 25;
    } preV7;
    struct {
        Uns32 FW        :  1;
        Uns32 L2PF      :  1;
        Uns32 L1PF      :  1;
        Uns32 WFLZ      :  1;
        Uns32 _u1       :  2;
        Uns32 SMP       :  1;
        Uns32 EXCL      :  1;
        Uns32 Alloc1Way :  1;
        Uns32 Parity    :  1;
        Uns32 _u2       : 22;
    } postV7;
} CP_REG_STRUCT_DECL(ACTLR);

#define CP_WRITE_MASK_ACTLR 0xffffffff

// -----------------------------------------------------------------------------
// CRn=1 opc1=0 CRm=0 opc2=2: CPACR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 cp0    : 2;
    Uns32 cp1    : 2;
    Uns32 cp2    : 2;
    Uns32 cp3    : 2;
    Uns32 cp4    : 2;
    Uns32 cp5    : 2;
    Uns32 cp6    : 2;
    Uns32 cp7    : 2;
    Uns32 cp8    : 2;
    Uns32 cp9    : 2;
    Uns32 cp10   : 2;
    Uns32 cp11   : 2;
    Uns32 cp12   : 2;
    Uns32 cp13   : 2;
    Uns32 _u1    : 2;
    Uns32 D32DIS : 1;
    Uns32 ASEDIS : 1;
} CP_REG_STRUCT_DECL(CPACR);

// true mask for CPACR register comes from configuration information
#define CP_WRITE_MASK_CPACR 0x00000000

// -----------------------------------------------------------------------------
// CRn=2 opc1=0 CRm=0 opc2=0: TTBR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 _u1  : 14;
    Uns32 Base : 18;
} CP_REG_STRUCT_DECL(TTBR);

#define CP_WRITE_MASK_TTBR 0xffffc000

// -----------------------------------------------------------------------------
// CRn=2 opc1=0 CRm=0 opc2=0: TTBR0
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 IRGN1   :  1;
    Uns32 S       :  1;
    Uns32 _u1     :  1;
    Uns32 RGN     :  2;
    Uns32 NOS     :  1;     // ARMv7 only
    Uns32 IRGN0   :  1;     // multiprocessing extensions only
    Uns32 BaseLow :  7;
    Uns32 Base    : 18;
} CP_REG_STRUCT_DECL(TTBR0);

#define CP_WRITE_MASK_TTBR0 0xfffffffb

// -----------------------------------------------------------------------------
// CRn=2 opc1=0 CRm=0 opc2=1: TTBR1
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 IRGN1   :  1;
    Uns32 S       :  1;
    Uns32 _u1     :  1;
    Uns32 RGN     :  2;
    Uns32 NOS     :  1;     // ARMv7 only
    Uns32 IRGN0   :  1;     // multiprocessing extensions only
    Uns32 _u2     :  7;
    Uns32 Base    : 18;
} CP_REG_STRUCT_DECL(TTBR1);

#define CP_WRITE_MASK_TTBR1 0xffffc07b

// -----------------------------------------------------------------------------
// CRn=2 opc1=0 CRm=0 opc2=2: TTBCR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 N       :  3;
    Uns32 _u2     : 29;
} CP_REG_STRUCT_DECL(TTBCR);

#define CP_WRITE_MASK_TTBCR 0x00000007

// -----------------------------------------------------------------------------
// CRn=2 opc1=0 CRm=0 opc2=[0/1]: DCR and ICR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 c0  :  1;
    Uns32 c1  :  1;
    Uns32 c2  :  1;
    Uns32 c3  :  1;
    Uns32 c4  :  1;
    Uns32 c5  :  1;
    Uns32 c6  :  1;
    Uns32 c7  :  1;
    Uns32 _u1 : 24;
} CP_REG_STRUCT_DECL(DCR),
  CP_REG_STRUCT_DECL(ICR);

#define CP_WRITE_MASK_DCR 0x000000ff
#define CP_WRITE_MASK_ICR 0x000000ff

// -----------------------------------------------------------------------------
// CRn=3 opc1=0 CRm=0 opc2=0: DACR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 D0  : 2;
    Uns32 D1  : 2;
    Uns32 D2  : 2;
    Uns32 D3  : 2;
    Uns32 D4  : 2;
    Uns32 D5  : 2;
    Uns32 D6  : 2;
    Uns32 D7  : 2;
    Uns32 D8  : 2;
    Uns32 D9  : 2;
    Uns32 D10 : 2;
    Uns32 D11 : 2;
    Uns32 D12 : 2;
    Uns32 D13 : 2;
    Uns32 D14 : 2;
    Uns32 D15 : 2;
} CP_REG_STRUCT_DECL(DACR);

#define CP_WRITE_MASK_DACR 0xffffffff

// -----------------------------------------------------------------------------
// CRn=3 opc1=0 CRm=0 opc2=0: DBR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 b0  :  1;
    Uns32 b1  :  1;
    Uns32 b2  :  1;
    Uns32 b3  :  1;
    Uns32 b4  :  1;
    Uns32 b5  :  1;
    Uns32 b6  :  1;
    Uns32 b7  :  1;
    Uns32 _u1 : 24;
} CP_REG_STRUCT_DECL(DBR);

#define CP_WRITE_MASK_DBR 0x000000ff

// -----------------------------------------------------------------------------
// CRn=5 opc1=0 CRm=0 opc2=0: FSR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Status :  4;
    Uns32 Domain :  4;
    Uns32 Zero   :  1;
    Uns32 _u1    : 23;
} CP_REG_STRUCT_DECL(FSR);

#define CP_WRITE_MASK_FSR 0x000000ff

// -----------------------------------------------------------------------------
// CRn=5 opc1=0 CRm=0 opc2=0: DFSR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Status30 :  4;
    Uns32 Domain   :  4;
    Uns32 Zero     :  2;
    Uns32 Status4  :  1;
    Uns32 RW       :  1;
    Uns32 Ext      :  1;
    Uns32 _u1      : 19;
} CP_REG_STRUCT_DECL(DFSR);

#define CP_WRITE_MASK_DFSR 0x00001cff

// -----------------------------------------------------------------------------
// CRn=5 opc1=0 CRm=0 opc2=1: IFSR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Status30 :  4;
    Uns32 _u1      :  6;
    Uns32 Status4  :  1;
    Uns32 _u2      :  1;
    Uns32 Ext      :  1;
    Uns32 _u3      : 19;
} CP_REG_STRUCT_DECL(IFSR);

#define CP_WRITE_MASK_IFSR 0x0000100f

// -----------------------------------------------------------------------------
// CRn=5 opc1=0 CRm=1 opc2=[0/1]: ADFSR, AIFSR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 value;
} CP_REG_STRUCT_DECL(ADFSR), CP_REG_STRUCT_DECL(AIFSR);

#define CP_WRITE_MASK_ADFSR 0xffffffff
#define CP_WRITE_MASK_AIFSR 0xffffffff

// -----------------------------------------------------------------------------
// CRn=5 opc1=0 CRm=0 opc2=[0/1]: DAPR, IAPR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 ap0 :  2;
    Uns32 ap1 :  2;
    Uns32 ap2 :  2;
    Uns32 ap3 :  2;
    Uns32 ap4 :  2;
    Uns32 ap5 :  2;
    Uns32 ap6 :  2;
    Uns32 ap7 :  2;
    Uns32 _u1 : 16;
} armMPUAP,
  CP_REG_STRUCT_DECL(DAPR),
  CP_REG_STRUCT_DECL(IAPR);

#define CP_WRITE_MASK_DAPR 0x0000ffff
#define CP_WRITE_MASK_IAPR 0x0000ffff

// -----------------------------------------------------------------------------
// CRn=5 opc1=0 CRm=0 opc2=[2/3]: DEAPR, IEAPR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 ap0 : 4;
    Uns32 ap1 : 4;
    Uns32 ap2 : 4;
    Uns32 ap3 : 4;
    Uns32 ap4 : 4;
    Uns32 ap5 : 4;
    Uns32 ap6 : 4;
    Uns32 ap7 : 4;
} armMPUAPExt,
  CP_REG_STRUCT_DECL(DEAPR),
  CP_REG_STRUCT_DECL(IEAPR);

#define CP_WRITE_MASK_DEAPR 0xffffffff
#define CP_WRITE_MASK_IEAPR 0xffffffff

// -----------------------------------------------------------------------------
// CRn=6 opc1=0 CRm=0 opc2=[0/1/2]: FAR, DFAR, WFAR, IFAR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Address;
} CP_REG_STRUCT_DECL(FAR),
  CP_REG_STRUCT_DECL(DFAR),
  CP_REG_STRUCT_DECL(WFAR),
  CP_REG_STRUCT_DECL(IFAR);

#define CP_WRITE_MASK_FAR  0xffffffff
#define CP_WRITE_MASK_DFAR 0xffffffff
#define CP_WRITE_MASK_WFAR 0xffffffff
#define CP_WRITE_MASK_IFAR 0xffffffff

// -----------------------------------------------------------------------------
// CRn=6 opc1=0 CRm=1 opc2=[0/1]: DRBAR, IRBAR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 _u1  :  5;
    Uns32 Base : 27;
} CP_REG_STRUCT_DECL(DRBAR),
  CP_REG_STRUCT_DECL(IRBAR);

#define CP_WRITE_MASK_DRBAR 0xffffffe0
#define CP_WRITE_MASK_IRBAR 0xffffffe0

// -----------------------------------------------------------------------------
// CRn=6 opc1=0 CRm=1 opc2=[2/3]: DRSR, IRSR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 E    :  1;
    Uns32 Size :  5;
    Uns32 _u1  : 26;
} CP_REG_STRUCT_DECL(DRSR),
  CP_REG_STRUCT_DECL(IRSR);

#define CP_WRITE_MASK_DRSR 0x0000003f
#define CP_WRITE_MASK_IRSR 0x0000003f

// -----------------------------------------------------------------------------
// CRn=6 opc1=0 CRm=1 opc2=[4/5]: DRACR, IRACR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 B    :  1;
    Uns32 C    :  1;
    Uns32 S    :  1;
    Uns32 TEX  :  3;
    Uns32 _u1  :  2;
    Uns32 AP   :  3;
    Uns32 _u2  :  1;
    Uns32 XN   :  1;
    Uns32 _u3  : 19;
} CP_REG_STRUCT_DECL(DRACR),
  CP_REG_STRUCT_DECL(IRACR);

#define CP_WRITE_MASK_DRACR 0x0000173f
#define CP_WRITE_MASK_IRACR 0x0000173f

// -----------------------------------------------------------------------------
// CRn=6 opc1=0 CRm=2 opc2=0: RGNR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Region : 32;
} CP_REG_STRUCT_DECL(RGNR);

#define CP_WRITE_MASK_RGNR 0xffffffff

// -----------------------------------------------------------------------------
// CRn=6 opc1=0 CRm=2 opc2=0: PAR
// -----------------------------------------------------------------------------

typedef union {
    struct {
        Uns32 F     :  1;
        Uns32 SS    :  1;
        Uns32 Outer :  2;
        Uns32 Inner :  3;
        Uns32 SH    :  1;
        Uns32 _u1   :  1;
        Uns32 NS    :  1;
        Uns32 NOS   :  1;
        Uns32 _u2   :  1;
        Uns32 PA    : 20;
    } ok;
    struct {
        Uns32 F     :  1;
        Uns32 FS    :  6;
        Uns32 Zero  : 25;
    } fail;
} CP_REG_STRUCT_DECL(PAR);

#define CP_WRITE_MASK_PAR 0xffffffff

// -----------------------------------------------------------------------------
// CRn=6 opc1=0 CRm=[0..7] opc2=[0/1]: DMRR0..7, IMRR0..7
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 E    :  1;
    Uns32 Size :  5;
    Uns32 _u1  :  6;
    Uns32 Base : 20;
} CP_REG_STRUCT_DECL(DMRR0),
  CP_REG_STRUCT_DECL(DMRR1),
  CP_REG_STRUCT_DECL(DMRR2),
  CP_REG_STRUCT_DECL(DMRR3),
  CP_REG_STRUCT_DECL(DMRR4),
  CP_REG_STRUCT_DECL(DMRR5),
  CP_REG_STRUCT_DECL(DMRR6),
  CP_REG_STRUCT_DECL(DMRR7),
  CP_REG_STRUCT_DECL(IMRR0),
  CP_REG_STRUCT_DECL(IMRR1),
  CP_REG_STRUCT_DECL(IMRR2),
  CP_REG_STRUCT_DECL(IMRR3),
  CP_REG_STRUCT_DECL(IMRR4),
  CP_REG_STRUCT_DECL(IMRR5),
  CP_REG_STRUCT_DECL(IMRR6),
  CP_REG_STRUCT_DECL(IMRR7);

#define CP_WRITE_MASK_DMRR0 0xfffff03f
#define CP_WRITE_MASK_DMRR1 0xfffff03f
#define CP_WRITE_MASK_DMRR2 0xfffff03f
#define CP_WRITE_MASK_DMRR3 0xfffff03f
#define CP_WRITE_MASK_DMRR4 0xfffff03f
#define CP_WRITE_MASK_DMRR5 0xfffff03f
#define CP_WRITE_MASK_DMRR6 0xfffff03f
#define CP_WRITE_MASK_DMRR7 0xfffff03f
#define CP_WRITE_MASK_IMRR0 0xfffff03f
#define CP_WRITE_MASK_IMRR1 0xfffff03f
#define CP_WRITE_MASK_IMRR2 0xfffff03f
#define CP_WRITE_MASK_IMRR3 0xfffff03f
#define CP_WRITE_MASK_IMRR4 0xfffff03f
#define CP_WRITE_MASK_IMRR5 0xfffff03f
#define CP_WRITE_MASK_IMRR6 0xfffff03f
#define CP_WRITE_MASK_IMRR7 0xfffff03f

// -----------------------------------------------------------------------------
// CRn=8 opc1=0 CRm=[5/6/7] opc2=[0/1/2]: TLB Operations
// -----------------------------------------------------------------------------

#define CP_TLBInvalidateTLB_MVA  0xfffff000
#define CP_TLBInvalidateTLB_ASID 0x000000ff

// -----------------------------------------------------------------------------
// CRn=9 opc1=0 CRm=0 opc2=[0/1]: DCLR, ICLR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 value;
} CP_REG_STRUCT_DECL(DCLR), CP_REG_STRUCT_DECL(ICLR);

#define CP_WRITE_MASK_DCLR 0xffffffff
#define CP_WRITE_MASK_ICLR 0xffffffff

// -----------------------------------------------------------------------------
// CRn=9 opc1=0 CRm=1 opc2=[0/1]: DTCMRR, ITCMRR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 E    :  1;
    Uns32 _u1  :  1;
    Uns32 Size :  5;
    Uns32 _u2  :  5;
    Uns32 Base : 20;
} armTCMRegion,
  CP_REG_STRUCT_DECL(DTCMRR), CP_REG_STRUCT_DECL(ITCMRR);

#define CP_WRITE_MASK_DTCMRR 0xfffff001
#define CP_WRITE_MASK_ITCMRR 0xfffff001

// -----------------------------------------------------------------------------
// CRn=10 opc1=0 CRm=0 opc2=[0/1]: DTLBLR, ITLBLR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 P     :  1;
    Uns32 Value : 31;
} CP_REG_STRUCT_DECL(DTLBLR),
  CP_REG_STRUCT_DECL(ITLBLR);

#define CP_WRITE_MASK_DTLBLR 0xffffffff
#define CP_WRITE_MASK_ITLBLR 0xffffffff

// -----------------------------------------------------------------------------
// CRn=10 opc1=0 CRm=2 opc2=0: PRRR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 TR0 : 2;
    Uns32 TR1 : 2;
    Uns32 TR2 : 2;
    Uns32 TR3 : 2;
    Uns32 TR4 : 2;
    Uns32 TR5 : 2;
    Uns32 TR6 : 2;
    Uns32 TR7 : 2;
    Uns32 DS0 : 1;
    Uns32 DS1 : 1;
    Uns32 NS0 : 1;
    Uns32 NS1 : 1;
    Uns32 _u1 : 4;
    Uns32 NOS : 8;
} CP_REG_STRUCT_DECL(PRRR);

#define CP_WRITE_MASK_PRRR 0xff0fcfff
#define CP_PRRRReset       0x00098aa4

// -----------------------------------------------------------------------------
// CRn=10 opc1=0 CRm=2 opc2=1: NMRR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 IR0 : 2;
    Uns32 IR1 : 2;
    Uns32 IR2 : 2;
    Uns32 IR3 : 2;
    Uns32 IR4 : 2;
    Uns32 IR5 : 2;
    Uns32 IR6 : 2;
    Uns32 IR7 : 2;
    Uns32 OR0 : 2;
    Uns32 OR1 : 2;
    Uns32 OR2 : 2;
    Uns32 OR3 : 2;
    Uns32 OR4 : 2;
    Uns32 OR5 : 2;
    Uns32 OR6 : 2;
    Uns32 OR7 : 2;
} CP_REG_STRUCT_DECL(NMRR);

#define CP_WRITE_MASK_NMRR 0xcfffcfff
#define CP_NMRRReset       0x44e048e0

// -----------------------------------------------------------------------------
// CRn=11 opc1=0 CRm=0 opc2=[0..3]: DMAPresent, DMAQueued, DMARunning, DMAInterrupting
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 CH0 :  1;
    Uns32 CH1 :  1;
    Uns32 _u1 : 30;
} CP_REG_STRUCT_DECL(DMAPresent),
  CP_REG_STRUCT_DECL(DMAQueued),
  CP_REG_STRUCT_DECL(DMARunning),
  CP_REG_STRUCT_DECL(DMAInterrupting);

#define CP_WRITE_MASK_DMAPresent      0x00000000
#define CP_WRITE_MASK_DMAQueued       0x00000000
#define CP_WRITE_MASK_DMARunning      0x00000000
#define CP_WRITE_MASK_DMAInterrupting 0x00000000

// -----------------------------------------------------------------------------
// CRn=11 opc1=0 CRm=1 opc2=0: DMAUserAccessibility
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 U0  :  1;
    Uns32 U1  :  1;
    Uns32 _u1 : 30;
} CP_REG_STRUCT_DECL(DMAUserAccessibility);

#define CP_WRITE_MASK_DMAUserAccessibility 0x00000003

// -----------------------------------------------------------------------------
// CRn=11 opc1=0 CRm=2 opc2=0: DMAChannel
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Number :  1;
    Uns32 _u1    : 31;
} CP_REG_STRUCT_DECL(DMAChannel);

#define CP_WRITE_MASK_DMAChannel 0x00000001

// -----------------------------------------------------------------------------
// CRn=11 opc1=0 CRm=3 opc2=[0/1/2]: DMAStop, DMAStart, DMAClear
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 _u1 : 32;
} CP_REG_STRUCT_DECL(DMAStop),
  CP_REG_STRUCT_DECL(DMAStart),
  CP_REG_STRUCT_DECL(DMAClear);

#define CP_WRITE_MASK_DMAStop  0xffffffff
#define CP_WRITE_MASK_DMAStart 0xffffffff
#define CP_WRITE_MASK_DMAClear 0xffffffff

// -----------------------------------------------------------------------------
// CRn=11 opc1=0 CRm=4 opc2=0: DMAControl
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 TS  :  2;
    Uns32 _u1 :  6;
    Uns32 ST  : 12;
    Uns32 _u2 :  6;
    Uns32 UM  :  1;
    Uns32 FT  :  1;
    Uns32 IE  :  1;
    Uns32 IC  :  1;
    Uns32 DT  :  1;
    Uns32 TR  :  1;
} CP_REG_STRUCT_DECL(DMAControl);

#define CP_WRITE_MASK_DMAControl 0xfc0fff03

// -----------------------------------------------------------------------------
// CRn=11 opc1=0 CRm=[5/6/7] opc2=0: DMAInternalStart, DMAExternalStart, DMAInternalEnd
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 _u1 : 32;
} CP_REG_STRUCT_DECL(DMAInternalStart),
  CP_REG_STRUCT_DECL(DMAExternalStart),
  CP_REG_STRUCT_DECL(DMAInternalEnd);

#define CP_WRITE_MASK_DMAInternalStart 0xffffffff
#define CP_WRITE_MASK_DMAExternalStart 0xffffffff
#define CP_WRITE_MASK_DMAInternalEnd   0xffffffff

// -----------------------------------------------------------------------------
// CRn=11 opc1=0 CRm=8 opc2=0: DMAStatus
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Status :  2;
    Uns32 IS     :  5;
    Uns32 ES     :  5;
    Uns32 BP     :  1;
    Uns32 _u1    : 19;
} CP_REG_STRUCT_DECL(DMAStatus);

#define CP_WRITE_MASK_DMAStatus 0x00000000

// -----------------------------------------------------------------------------
// CRn=13 opc1=0 CRm=0 opc2=0: FCSEIDR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 _u1 : 25;
    Uns32 PID :  7;
} CP_REG_STRUCT_DECL(FCSEIDR);

#define CP_WRITE_MASK_FCSEIDR 0xfe000000

// -----------------------------------------------------------------------------
// CRn=11 opc1=0 CRm=5 opc2=0: DMAContextID
// CRn=13 opc1=0 CRm=0 opc2=1: CONTEXTIDR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 ASID   :  8;
    Uns32 PROCID : 24;
} CP_REG_STRUCT_DECL(DMAContextID),
  CP_REG_STRUCT_DECL(CONTEXTIDR);

#define CP_WRITE_MASK_DMAContextID 0xffffffff
#define CP_WRITE_MASK_CONTEXTIDR   0xffffffff

// -----------------------------------------------------------------------------
// CRn=13 opc1=0 CRm=0 opc2=[2/3/4]: TPIDRURW, TPIDRURO, TPIDRPRW
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 value;
} CP_REG_STRUCT_DECL(TPIDRURW),
  CP_REG_STRUCT_DECL(TPIDRURO),
  CP_REG_STRUCT_DECL(TPIDRPRW);

#define CP_WRITE_MASK_TPIDRURW 0xffffffff
#define CP_WRITE_MASK_TPIDRURO 0xffffffff
#define CP_WRITE_MASK_TPIDRPRW 0xffffffff

// -----------------------------------------------------------------------------
// CRn=15 opc1=0 CRm=0 opc2=0: PCR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 DCG     :  1;
    Uns32 _u1     :  7;
    Uns32 Latency :  3;
    Uns32 _u2     : 21;
} CP_REG_STRUCT_DECL(PCR);

#define CP_WRITE_MASK_PCR 0xffffffff

// -----------------------------------------------------------------------------
// CRn=15 opc1=4 CRm=0 opc2=0: CBAR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 value;
} CP_REG_STRUCT_DECL(CBAR);

#define CP_WRITE_MASK_CBAR 0xffffffff

// -----------------------------------------------------------------------------
// CRn=15 opc1=0 CRm=1 opc2=0: NEONB
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Busy :  1;
    Uns32 _u1  : 31;
} CP_REG_STRUCT_DECL(NEONB);

#define CP_WRITE_MASK_NEONB 0x00000000

// -----------------------------------------------------------------------------
// CRn=15 opc1=5 CRm=0 opc2=0: TLBHR (TLB HITMAP - not supported except as r/w register)
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 value;
} CP_REG_STRUCT_DECL(TLBHR);

#define CP_WRITE_MASK_TLBHR 0xffffffff

// -----------------------------------------------------------------------------
// CRn=15 opc1=5 CRm=4 opc2=2/4: TLBRI, TLBWI
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 value;
} CP_REG_STRUCT_DECL(TLBRI),
  CP_REG_STRUCT_DECL(TLBWI);

#define CP_WRITE_MASK_TLBRI 0xffffffff
#define CP_WRITE_MASK_TLBWI 0xffffffff

// -----------------------------------------------------------------------------
// CRn=15 opc1=5 CRm=5 opc2=2: TLBLDVA
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 ASID :  8;
    Uns32 _u1  :  1;
    Uns32 G    :  1;
    Uns32 NS   :  1;
    Uns32 _u2  :  1;
    Uns32 VPN  : 20;
} CP_REG_STRUCT_DECL(TLBLDVA);

#define CP_WRITE_MASK_TLBLDVA 0xffffffff

// -----------------------------------------------------------------------------
// CRn=15 opc1=5 CRm=6 opc2=2: TLBLDPA
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 V   :  1;
    Uns32 AP  :  3;
    Uns32 _u1 :  2;
    Uns32 SZ  :  2;
    Uns32 _u2 :  4;
    Uns32 PPN : 20;
} CP_REG_STRUCT_DECL(TLBLDPA);

#define CP_WRITE_MASK_TLBLDPA 0xffffffff

// -----------------------------------------------------------------------------
// CRn=15 opc1=5 CRm=7 opc2=2: TLBLDATTR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 S      :  1;
    Uns32 TEXCB  :  5;
    Uns32 XN     :  1;
    Uns32 Domain :  4;
    Uns32 NS     :  1;
    Uns32 _u2    : 20;
} CP_REG_STRUCT_DECL(TLBLDATTR);

#define CP_WRITE_MASK_TLBLDATTR 0xffffffff

// -----------------------------------------------------------------------------
// CACHE OPERATION PSEUDO-REGISTER WRITE MASKS
// -----------------------------------------------------------------------------

#define CP_WRITE_MASK_CP15WFI                 0x00000000
#define CP_WRITE_MASK_CP15ISB                 0x00000000
#define CP_WRITE_MASK_CP15DSB                 0x00000000
#define CP_WRITE_MASK_CP15DMB                 0x00000000
#define CP_WRITE_MASK_PrefetchICacheLine      0x00000000

#define CP_WRITE_MASK_ICIALLUIS               0x00000000
#define CP_WRITE_MASK_BPIALLIS                0x00000000
#define CP_WRITE_MASK_ICIALLU                 0x00000000
#define CP_WRITE_MASK_ICIMVAU                 0x00000000
#define CP_WRITE_MASK_BPIALL                  0x00000000
#define CP_WRITE_MASK_BPIMVA                  0x00000000
#define CP_WRITE_MASK_DCIMVAC                 0x00000000
#define CP_WRITE_MASK_DCISW                   0x00000000
#define CP_WRITE_MASK_DCCMVAC                 0x00000000
#define CP_WRITE_MASK_DCCSW                   0x00000000
#define CP_WRITE_MASK_DCCMVAU                 0x00000000
#define CP_WRITE_MASK_DCCIMVAC                0x00000000
#define CP_WRITE_MASK_DCCISW                  0x00000000

#define CP_WRITE_MASK_InvalidateICache        0x00000000
#define CP_WRITE_MASK_InvalidateICacheLineMVA 0x00000000
#define CP_WRITE_MASK_InvalidateICacheLineSW  0x00000000
#define CP_WRITE_MASK_FlushBranchTargetCache  0x00000000
#define CP_WRITE_MASK_FlushBranchTargetEntry  0x00000000
#define CP_WRITE_MASK_InvalidateDCache        0x00000000
#define CP_WRITE_MASK_InvalidateDCacheLineMVA 0x00000000
#define CP_WRITE_MASK_InvalidateDCacheLineSW  0x00000000
#define CP_WRITE_MASK_InvalidateUnified       0x00000000
#define CP_WRITE_MASK_CleanDCache             0x00000000
#define CP_WRITE_MASK_CleanDCacheLineMVA      0x00000000
#define CP_WRITE_MASK_CleanDCacheLineSW       0x00000000
#define CP_WRITE_MASK_DirtyStatus             0x00000000
#define CP_WRITE_MASK_CleanUnified            0x00000000
#define CP_WRITE_MASK_BlockTransferStatus     0x00000000
#define CP_WRITE_MASK_StopPrefetchRange       0x00000000
#define CP_WRITE_MASK_CleanInvalDCache        0x00000000
#define CP_WRITE_MASK_CleanInvalDCacheLineMVA 0x00000000
#define CP_WRITE_MASK_CleanInvalDCacheLineSW  0x00000000
#define CP_WRITE_MASK_CleanInvalUnified       0x00000000
#define CP_WRITE_MASK_InvalICacheRange        0x00000000
#define CP_WRITE_MASK_InvalDCacheRange        0x00000000
#define CP_WRITE_MASK_CleanDCacheRange        0x00000000
#define CP_WRITE_MASK_PrefetchICacheRange     0x00000000
#define CP_WRITE_MASK_PrefetchDCacheRange     0x00000000
#define CP_WRITE_MASK_CleanInvalDCacheRange   0x00000000

#define CP_WRITE_MASK_TestCleanDCache         0x00000000
#define CP_WRITE_MASK_TestCleanInvalDCache    0x00000000

// -----------------------------------------------------------------------------
// TLB OPERATION PSEUDO-REGISTER WRITE MASKS
// -----------------------------------------------------------------------------

#define CP_WRITE_MASK_V2PCWPR    0x00000000
#define CP_WRITE_MASK_V2PCWPW    0x00000000
#define CP_WRITE_MASK_V2PCWUR    0x00000000
#define CP_WRITE_MASK_V2PCWUW    0x00000000
#define CP_WRITE_MASK_TLBIALLIS  0x00000000
#define CP_WRITE_MASK_TLBIMVAIS  0x00000000
#define CP_WRITE_MASK_TLBIASIDIS 0x00000000
#define CP_WRITE_MASK_TLBIMVAAIS 0x00000000
#define CP_WRITE_MASK_ITLBIALL   0x00000000
#define CP_WRITE_MASK_DTLBIALL   0x00000000
#define CP_WRITE_MASK_TLBIALL    0x00000000
#define CP_WRITE_MASK_ITLBIMVA   0x00000000
#define CP_WRITE_MASK_DTLBIMVA   0x00000000
#define CP_WRITE_MASK_TLBIMVA    0x00000000
#define CP_WRITE_MASK_ITLBIASID  0x00000000
#define CP_WRITE_MASK_DTLBIASID  0x00000000
#define CP_WRITE_MASK_TLBIASID   0x00000000
#define CP_WRITE_MASK_TLBIMVAA   0x00000000

// -----------------------------------------------------------------------------
// IGNORED OPERATION WRITE MASK
// -----------------------------------------------------------------------------

#define CP_WRITE_MASK_Ignored 0x00000000


// -----------------------------------------------------------------------------
// CONTAINER
// -----------------------------------------------------------------------------

// use this to define a field-based register entry in armCPRegsU below
#define CP_REG_DECL(_N) CP_REG_STRUCT_DECL(_N) _N

//
// This type defines the entire implemented coprocessor register set
//
typedef union armCPRegsU {

    Uns32 regs[CP_ID(Size)];            // use this for by-register access

    struct {                            // use this for by-field access

        // coprocessor 14 register entries
        CP_REG_DECL(TEECR);
        CP_REG_DECL(TEEHBR);
        CP_REG_DECL(JIDR);
        CP_REG_DECL(JOSCR);
        CP_REG_DECL(JMCR);

        // coprocessor 15 register entries
        CP_REG_DECL(MIDR);
        CP_REG_DECL(CTR);
        CP_REG_DECL(TCMTR);
        CP_REG_DECL(TLBTR);
        CP_REG_DECL(MPUIR);
        CP_REG_DECL(MPIDR);
        CP_REG_DECL(ID_PFR0);
        CP_REG_DECL(ID_PFR1);
        CP_REG_DECL(ID_DFR0);
        CP_REG_DECL(ID_AFR0);
        CP_REG_DECL(ID_MMFR0);
        CP_REG_DECL(ID_MMFR1);
        CP_REG_DECL(ID_MMFR2);
        CP_REG_DECL(ID_MMFR3);
        CP_REG_DECL(ID_ISAR0);
        CP_REG_DECL(ID_ISAR1);
        CP_REG_DECL(ID_ISAR2);
        CP_REG_DECL(ID_ISAR3);
        CP_REG_DECL(ID_ISAR4);
        CP_REG_DECL(ID_ISAR5);
        CP_REG_DECL(ID_RESERVED);
        CP_REG_DECL(CLIDR);
        CP_REG_DECL(AIDR);
        CP_REG_DECL(CSSELR);
        CP_REG_DECL(SCTLR);
        CP_REG_DECL(ACTLR);
        CP_REG_DECL(CPACR);
        CP_REG_DECL(TTBR);
        CP_REG_DECL(TTBR0);
        CP_REG_DECL(TTBR1);
        CP_REG_DECL(TTBCR);
        CP_REG_DECL(DCR);
        CP_REG_DECL(ICR);
        CP_REG_DECL(DACR);
        CP_REG_DECL(DBR);
        CP_REG_DECL(FSR);
        CP_REG_DECL(DFSR);
        CP_REG_DECL(IFSR);
        CP_REG_DECL(ADFSR);
        CP_REG_DECL(AIFSR);
        CP_REG_DECL(FAR);
        CP_REG_DECL(DFAR);
        CP_REG_DECL(WFAR);
        CP_REG_DECL(IFAR);
        CP_REG_DECL(RGNR);
        CP_REG_DECL(PAR);
        CP_REG_DECL(DCLR);
        CP_REG_DECL(ICLR);
        CP_REG_DECL(DTCMRR);
        CP_REG_DECL(ITCMRR);
        CP_REG_DECL(PRRR);
        CP_REG_DECL(NMRR);
        CP_REG_DECL(DMAPresent);
        CP_REG_DECL(DMAQueued);
        CP_REG_DECL(DMARunning);
        CP_REG_DECL(DMAInterrupting);
        CP_REG_DECL(DMAUserAccessibility);
        CP_REG_DECL(DMAChannel);
        CP_REG_DECL(FCSEIDR);
        CP_REG_DECL(CONTEXTIDR);
        CP_REG_DECL(TPIDRURW);
        CP_REG_DECL(TPIDRURO);
        CP_REG_DECL(TPIDRPRW);
        CP_REG_DECL(PCR);
        CP_REG_DECL(CBAR);
        CP_REG_DECL(TLBHR);
        CP_REG_DECL(TLBRI);
        CP_REG_DECL(TLBWI);

    } fields;

} armCPRegs, *armCPRegsP;

#endif
