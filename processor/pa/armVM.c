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

#include <stdio.h>      // for sprintf

// Imperas header files
#include "hostapi/impAlloc.h"

// VMI header files
#include "vmi/vmiAttrs.h"
#include "vmi/vmiMessage.h"
#include "vmi/vmiRt.h"
#include "vmi/vmiTypes.h"

// model header files
#include "armExceptions.h"
#include "armFunctions.h"
#include "armStructure.h"
#include "armMessage.h"
#include "armMPCore.h"
#include "armVM.h"
#include "armUtils.h"

//
// Prefix for messages from this module
//
#define CPU_PREFIX "ARM_VM"

//
// This is the highest address
//
#define ARM_MAX_ADDR 0xffffffff


////////////////////////////////////////////////////////////////////////////////
// TLB TYPES
////////////////////////////////////////////////////////////////////////////////

//
// This describes the TLB entry type (required to determine fault status)
//
typedef enum tlbEntryTypeE {
    TET_SECTION,        // section entry
    TET_PAGE,           // page entry
} tlbEntryType;

//
// Structure representing a TLB entry
//
typedef struct tlbEntryS {

    // entry virtual & physical addresses
    Uns32        lowVA;         // entry low virtual address
    Uns32        highVA;        // entry high virtual address
    Uns64        lowPA;         // entry low physical address (max 40 bits)

    // entry attributes
    Uns8         ASID    :  8;  // ASID (for non-global entry)
    Uns8         domain  :  4;  // entry domain
    Uns8         APsp0   :  3;  // AP, first subpage
    Uns8         APsp1   :  3;  // AP, second subpage
    Uns8         APsp2   :  3;  // AP, third subpage
    Uns8         APsp3   :  3;  // AP, fourth subpage
    Uns8         TEXCB   :  5;  // TEX, C and B bits
    Bool         S       :  1;  // S bit
    tlbEntryType type    :  1;  // entry type (section or page)
    Bool         XN      :  1;  // execute-never bit
    Bool         G       :  1;  // global bit
    Bool         SCTLR_S :  1;  // SCTLR.S bit at entry creation
    Bool         SCTLR_R :  1;  // SCTLR.R bit at entry creation
    Bool         SS      :  1;  // is the entry a supersection?
    Bool         NS      :  1;  // non-secure bit
    Bool         LDV     :  1;  // valid bit (lockdown entries only)
    Bool         isMapped:  1;  // is the TLB entry mapped?

    // range LUT entry (for fast lookup by address)
    union {
        struct tlbEntryS *nextFree; // when in free list
        vmiRangeEntryP    lutEntry; // equivalent range entry
    };

} tlbEntry, *tlbEntryP;

//
// Structure representing a TLB
//
typedef struct armTLBS {

    // range LUT entry (for fast lookup by address)
    vmiRangeTableP lut;

    // list of free TLB entries available for reuse
    tlbEntryP      free;

    // TLBLockdown support
    Uns32          ldSize;
    Uns32          ldEntryBits;
    Uns32          ldMask;
    tlbEntryP      ldEntries;

    // representation of TLBLockdown register
    union {
        CP_REG_DECL(DTLBLR);
        Uns32 TLBLRu32;
    };

} armTLB;


////////////////////////////////////////////////////////////////////////////////
// DMA TYPES
////////////////////////////////////////////////////////////////////////////////

//
// Enum representing DMA channel state
//
typedef enum armDMAStatusE {
    ADS_Idle     = 0,
    ADS_Queued   = 1,
    ADS_Running  = 2,
    ADS_Complete = 3
} armDMAStatus;

//
// Type representing a DMA unit
//
typedef struct armDMAUnitS {

    // address registers
    Uns32 internalStart;
    Uns32 externalStart;
    Uns32 internalEnd;

    // representation of DMAControl register
    union {
        CP_REG_DECL(DMAControl);
        Uns32 DMAControlU32;
    };

    // representation of DMAStatus register
    union {
        CP_REG_DECL(DMAStatus);
        Uns32 DMAStatusU32;
    };

    // representation of DMAContextID register
    union {
        CP_REG_DECL(DMAContextID);
        Uns32 DMAContextIDU32;
    };

    // control flags
    Bool isExternal;        // whether the current access is external
    Bool abort;             // whether the current access aborted

} armDMAUnit;


////////////////////////////////////////////////////////////////////////////////
// COMMON DESCRIPTOR TYPES
////////////////////////////////////////////////////////////////////////////////

//
// Fault descriptor
//
typedef struct level1DescFaultS {
    Uns32 type :  2;
    Uns32 _u1  : 30;
} level1DescFault, level2DescFault;

//
// Level 1 coarse page table descriptor
//
typedef struct level1DescCoarseS {
    Uns32 type   :  2;
    Uns32 _u1    :  1;
    Uns32 NS     :  1;
    Uns32 _u2    :  1;
    Uns32 Domain :  4;
    Uns32 P      :  1;
    Uns32 Base   : 22;
} level1DescCoarse;

//
// Level 1 section descriptor
//
typedef struct level1DescSectionS {
    Uns32 type   :  2;
    Uns32 B      :  1;
    Uns32 C      :  1;
    Uns32 XN     :  1;
    Uns32 Domain :  4;
    Uns32 P      :  1;
    Uns32 AP     :  2;
    Uns32 TEX    :  3;
    Uns32 AP2    :  1;
    Uns32 S      :  1;
    Uns32 nG     :  1;
    Uns32 isSS   :  1;
    Uns32 NS     :  1;
    Uns32 Base   : 12;
} level1DescSection;

//
// Level 1 supersection descriptor
//
typedef struct level1DescSuperSectionS {
    Uns32 type      : 2;
    Uns32 B         : 1;
    Uns32 C         : 1;
    Uns32 XN        : 1;
    Uns32 XBA_39_36 : 4;
    Uns32 P         : 1;
    Uns32 AP        : 2;
    Uns32 TEX       : 3;
    Uns32 AP2       : 1;
    Uns32 S         : 1;
    Uns32 nG        : 1;
    Uns32 isSS      : 1;
    Uns32 NS        : 1;
    Uns32 XBA_35_32 : 4;
    Uns32 Base      : 8;
} level1DescSuperSection;

//
// Level 1 fine page table descriptor (ARMv4/ARMv5 only)
//
typedef struct level1DescFineS {
    Uns32 type   :  2;
    Uns32 _u1    :  3;
    Uns32 Domain :  4;
    Uns32 P      :  1;
    Uns32 _u2    :  2;
    Uns32 Base   : 20;
} level1DescFine;

//
// Level 1 descriptor
//
typedef union level1DescU {
    Uns32                  raw;
    level1DescFault        fault;
    level1DescCoarse       coarse;
    level1DescSection      section;
    level1DescSuperSection superSection;
    level1DescFine         fine;
} level1Desc;

//
// Level 2 large page descriptor (ARMv4/ARMv5 only)
//
typedef struct level2DescLargeV5S {
    Uns32 type  :  2;
    Uns32 B     :  1;
    Uns32 C     :  1;
    Uns32 APsp0 :  2;
    Uns32 APsp1 :  2;
    Uns32 APsp2 :  2;
    Uns32 APsp3 :  2;
    Uns32 TEX   :  3;
    Uns32 _u1   :  1;
    Uns32 Base  : 16;
} level2DescLargeV5;

//
// Level 2 large page descriptor (ARMv6/ARMv7 only)
//
typedef struct level2DescLargeV6S {
    Uns32 type :  2;
    Uns32 B    :  1;
    Uns32 C    :  1;
    Uns32 AP   :  2;
    Uns32 _u1  :  3;
    Uns32 AP2  :  1;
    Uns32 S    :  1;
    Uns32 nG   :  1;
    Uns32 TEX  :  3;
    Uns32 XN   :  1;
    Uns32 Base : 16;
} level2DescLargeV6;

//
// Level 2 small page descriptor (ARMv4/ARMv5 only)
//
typedef struct level2DescSmallV5S {
    Uns32 type  :  2;
    Uns32 B     :  1;
    Uns32 C     :  1;
    Uns32 APsp0 :  2;
    Uns32 APsp1 :  2;
    Uns32 APsp2 :  2;
    Uns32 APsp3 :  2;
    Uns32 Base  : 20;
} level2DescSmallV5;

//
// Level 2 small page descriptor (ARMv6/ARMv7 only)
// NOTE: also the format for what was called "Extended Page" in ARMv5TE
//
typedef struct level2DescSmallV6S {
    Uns32 XN    :  1;
    Uns32 type1 :  1;
    Uns32 B     :  1;
    Uns32 C     :  1;
    Uns32 AP    :  2;
    Uns32 TEX   :  3;
    Uns32 AP2   :  1;
    Uns32 S     :  1;
    Uns32 nG    :  1;
    Uns32 Base  : 20;
} level2DescSmallV6;

//
// Level 2 tiny page descriptor (ARMv4/ARMv5 only)
//
typedef struct level2DescTinyV5S {
    Uns32 type :  2;
    Uns32 B    :  1;
    Uns32 C    :  1;
    Uns32 AP   :  2;
    Uns32 _u1  :  4;
    Uns32 Base : 22;
} level2DescTinyV5;

//
// Level 2 descriptor
//
typedef union level2DescU {
    Uns32             raw;
    level2DescFault   fault;
    level2DescLargeV5 largeV5;
    level2DescLargeV6 largeV6;
    level2DescSmallV5 smallV5;
    level2DescSmallV6 smallV6;
    level2DescTinyV5  tinyV5;
} level2Desc;


////////////////////////////////////////////////////////////////////////////////
// FIELD EXTRACTION UTILITY STRUCTURES
////////////////////////////////////////////////////////////////////////////////

//
// Structure used to interpret a virtual address in conjunction with FCSE
//
typedef union fcseVAU {
    Uns32 raw;
    struct {
        Uns32 low : 25;
        Uns32 PID :  7;
    } fields;
} fcseVA;

//
// Union used to interpret a virtual address in conjunction with a coarse page
// descriptor
//
typedef union coarseVAU {
    Uns32 raw;
    struct {
        Uns32 _u1         : 12;
        Uns32 secondIndex :  8;
        Uns32 firstIndex  : 12;
    } fields;
} coarseVA;

//
// Union used to interpret a virtual address in conjunction with a fine page
// descriptor
//
typedef union fineVAU {
    Uns32 raw;
    struct {
        Uns32 _u1         : 10;
        Uns32 secondIndex : 10;
        Uns32 firstIndex  : 12;
    } fields;
} fineVA;

//
// Union used to interpret a physical address in conjunction with a supersection
// descriptor
//
typedef union superSectionPAU {
    Uns64 raw;
    struct {
        Uns32 low        : 24;
        Uns32 base_31_24 :  8;
        Uns32 base_35_32 :  4;
        Uns32 base_39_36 : 28;
    } fields;
} superSectionPA;

//
// Union used to interpret a physical address in conjunction with a section
// descriptor
//
typedef union sectionPAU {
    Uns32 raw;
    struct {
        Uns32 low  : 20;
        Uns32 high : 12;
    } fields;
} sectionPA;

//
// Union used to interpret a physical address in conjunction with a coarse page
// descriptor
//
typedef union coarsePAU {
    Uns32 raw;
    struct {
        Uns32 zero :  2;
        Uns32 low  :  8;
        Uns32 high : 22;
    } fields;
} coarsePA;

//
// Union used to interpret a physical address in conjunction with a fine page
// descriptor
//
typedef union finePAU {
    Uns32 raw;
    struct {
        Uns32 zero :  2;
        Uns32 low  : 10;
        Uns32 high : 20;
    } fields;
} finePA;

//
// Union used to interpret a physical address in conjunction with a large page
// descriptor
//
typedef union largePAU {
    Uns32 raw;
    struct {
        Uns32 low  : 16;
        Uns32 high : 16;
    } fields;
} largePA;

//
// Union used to interpret a physical address in conjunction with a small page
// descriptor
//
typedef union smallPAU {
    Uns32 raw;
    struct {
        Uns32 low  : 12;
        Uns32 high : 20;
    } fields;
} smallPA;

//
// Union used to interpret a physical address in conjunction with a tiny page
// descriptor
//
typedef union tinyPAU {
    Uns32 raw;
    struct {
        Uns32 low  : 10;
        Uns32 high : 22;
    } fields;
} tinyPA;


////////////////////////////////////////////////////////////////////////////////
// UTILITIES
////////////////////////////////////////////////////////////////////////////////

//
// Type enumerating per-domain access rights
//
typedef enum domainAccessE {
    DA_NONE     = 0,            // no access allowed
    DA_CLIENT   = 1,            // client access (use access permissions)
    DA_RESERVED = 2,            // reserved
    DA_MANAGER  = 3             // manager access (unrestricted)
} domainAccess;

//
// Bitmask specifying DAC value with illegal value 2 in each field
//
#define DAC_TWOS 0xaaaaaaaa

//
// Get the current processor ASID
//
static Uns8 getASID(armP arm) {
    if(arm->dmaActive) {
        return arm->dmaActive->DMAContextID.ASID;
    } else {
        return CP_FIELD(arm, CONTEXTIDR, ASID);
    }
}

//
// Does the processor support supersections?
//
inline static Bool supportSuperSection(armP arm) {
    return ARM_SUPPORT(arm->configInfo.arch, ARM_SS);
}

//
// Does the processor support extended small pages?
//
inline static Bool supportExtendedSmall(armP arm) {
    return ARM_SUPPORT(arm->configInfo.arch, ARM_SS);
}

//
// Does the processor support fine page tables?
//
inline static Bool supportFine(armP arm) {
    return !ARM_SUPPORT(arm->configInfo.arch, ARM_SS);
}

//
// Is extended-page mode enabled?
//
inline static Bool getXP(armP arm) {
    return CP_FIELD(arm, SCTLR, XP);
}

//
// Is access flag mode enabled?
//
inline static Bool getAFE(armP arm) {
    return CP_FIELD(arm, SCTLR, AFE);
}

//
// Return the AP[2] bit if enabled
//
inline static Uns8 getAP2(armP arm, Bool AP2) {
    return getXP(arm) ? AP2 : 0;
}

//
// Return AP[2:0] constructed from AP[1:0] and AP[2]
//
inline static Uns8 getFullAP(Uns8 AP10, Uns8 AP2) {
    return (AP2<<2) | AP10;
}

//
// Return TEXCB constructed from TEX[2:0], C and B bits
//
inline static Uns8 getTEXCB(Uns8 TEX, Uns8 C, Uns8 B) {
    return (TEX<<2) | (C<<1) | B;
}

//
// Is TEX remap enabled?
//
inline static Bool getTRE(armP arm) {
    return CP_FIELD(arm, SCTLR, TRE);
}

//
// Return the execute-never bit if enabled
//
inline static Bool getXN(armP arm, Bool XN) {
    return getXP(arm) ? XN : False;
}

//
// Return the global bit if enabled
//
inline static Bool getG(armP arm, Bool G) {
    return getXP(arm) ? G : True;
}

//
// Return current program counter
//
inline static Uns32 getPC(armP arm) {
    return vmirtGetPC((vmiProcessorP)arm);
}

//
// Return instruction memory domain set for the passed processor
//
inline static armDomainSetP getDomainSetI(armP arm) {
    return &arm->ids;
}

//
// Return data memory domain set for the passed processor
//
inline static armDomainSetP getDomainSetD(armP arm) {
    return &arm->dds;
}

//
// Get the current code memDomain
//
inline static memDomainP getVirtualCodeDomain(armP arm) {
    return vmirtGetProcessorCodeDomain((vmiProcessorP)arm);
}

//
// Get the current data memDomain
//
inline static memDomainP getVirtualDataDomain(armP arm) {
    return vmirtGetProcessorDataDomain((vmiProcessorP)arm);
}

//
// Is code demain required for the passed privilege?
//
inline static Bool isFetch(memPriv priv) {
    return priv==MEM_PRIV_X;
}

//
// Return the memory domain set to use for the passed memory access type
//
static armDomainSetP getDomainSetPriv(armP arm, memPriv priv) {
    return isFetch(priv) ? getDomainSetI(arm) : getDomainSetD(arm);
}

//
// Return the virtual memory domain to use for the passed memory access type
//
static memDomainP getVirtualDomainPriv(armP arm, memPriv priv) {
    return isFetch(priv) ? getVirtualCodeDomain(arm) : getVirtualDataDomain(arm);
}

//
// Can the virtual address be accessed with the reguired privilege?
//
static Bool accessOK(armP arm, memPriv requiredPriv, Uns32 address) {
    memDomainP domainV = getVirtualDomainPriv(arm, requiredPriv);
    return vmirtGetDomainPrivileges(domainV, address) & requiredPriv;
}

//
// Set the current data memDomain
//
inline static void setVirtualDataDomain(armP arm, memDomainP domain) {
    vmirtSetProcessorDataDomain((vmiProcessorP)arm, domain);
}

//
// Return the virtual data domain for the passed mode
//
static memDomainP getVirtualDataDomainMode(armP arm) {
    armDomainSetP set = getDomainSetD(arm);
    return IN_USER_MODE(arm) ? set->vmUser : set->vmPriv;
}

//
// Push to the user memDomain, marking the processor to indicate restoration
// is required
//
inline static void pushVirtualDataDomain(armP arm) {
    setVirtualDataDomain(arm, getDomainSetD(arm)->vmUser);
    arm->restoreDomain = True;
}

//
// Restore correct virtual code domain for the processor, if required
//
static void restoreVirtualDataDomain(armP arm) {
    setVirtualDataDomain(arm, getVirtualDataDomainMode(arm));
    arm->restoreDomain = False;
}

//
// Return translation table endianness - for variants implementing SCTLR.EE,
// this specifies the endianness; otherwise, it is the processor endianness
//
static memEndian getTransTableEndian(armP arm) {
    if(CP_FEATURE_PRESENT(arm, SCTLR, EE)) {
        return CP_FIELD(arm, SCTLR, EE) ? MEM_ENDIAN_BIG : MEM_ENDIAN_LITTLE;
    } else {
        return armGetEndian((vmiProcessorP)arm, False);
    }
}

//
// Return 4-byte translation table entry read from the passed memDomain
//
inline static Uns32 transTableRead(armP arm, memDomainP domain, Uns32 address) {
    return vmirtRead4ByteDomain(domain, address, getTransTableEndian(arm), True);
}

//
// Remove all access permissions in the current code and data domains in the
// address range low:high
//
static void removePermissionsCurrent(armP arm, Uns32 low, Uns32 high) {
    vmirtProtectMemory(
        getVirtualCodeDomain(arm), low, high, MEM_PRIV_NONE, MEM_PRIV_SET
    );
    vmirtProtectMemory(
        getVirtualDataDomain(arm), low, high, MEM_PRIV_NONE, MEM_PRIV_SET
    );
}

//
// Determine the memDomain set currently in use
//
static armPIDSet getPIDSet(armP arm) {

    if(!MMU_MPU_ENABLED(arm)) {
        return APS_PHYS;
    } else if(IN_USER_MODE(arm)) {
        return APS_VM_U;
    } else {
        return APS_VM_P;
    }
}

//
// Get per-domain access rights
//
inline static domainAccess getDomainAccess(armP arm, Uns8 domain) {
    return (CP_REG_UNS32(arm, DACR) >> (domain*2)) & 3;
}

//
// Return a mask of bits that must be cleared in a DAC to get the effective
// value
//
inline static Uns32 getDACClearBits(Uns32 DAC) {
    return (DAC & DAC_TWOS) & ((~(DAC & ~DAC_TWOS))<<1);
}

//
// When Domain Access Control changes from oldDAC to newDAC, is it possible that
// privileges have been downgraded?
//
static Bool downgradeDACPriv(armP arm, Uns32 oldDAC, Uns32 newDAC) {

    oldDAC &= ~getDACClearBits(oldDAC);
    newDAC &= ~getDACClearBits(newDAC);

    return (MMU_MPU_ENABLED(arm) && (oldDAC & ~newDAC));
}

//
// Report if the DAC value contains undefined values
//
static void validateDAC(armP arm, Uns32 DAC) {
    if(getDACClearBits(DAC)) {
        vmiMessage("a", CPU_PREFIX"_IDV",
            SRCREF_FMT "Domain Access Control value 0x%08x contains "
            "reserved value 2 in one or more positions (treated as 0)",
            SRCREF_ARGS(arm, getPC(arm)),
            DAC
        );
    }
}

//
// Given a raw privilege (AP format), return the effective privilege once
// SCTLR.S, SCTLR.R and mode have been taken into account for user mode accesses
// to a client domain
//
static memPriv getPrivTLBClientU(tlbEntryP entry, Uns8 AP) {

    static const memPriv privMap[] = {
        [0] = MEM_PRIV_NONE,
        [1] = MEM_PRIV_NONE,
        [2] = MEM_PRIV_RX,
        [3] = MEM_PRIV_RWX,
        [4] = MEM_PRIV_NONE,
        [5] = MEM_PRIV_NONE,
        [6] = MEM_PRIV_RX,
        [7] = MEM_PRIV_RX
    };

    if((AP==0) && entry->SCTLR_R) {
        return MEM_PRIV_RX;
    } else {
        return privMap[AP];
    }
}

//
// Given a raw privilege (AP format), return the effective privilege once
// SCTLR.S, SCTLR.R and mode have been taken into account for privileged mode
// accesses to a client domain
//
static memPriv getPrivTLBClientP(tlbEntryP entry, Uns8 AP) {

    static const memPriv privMap[] = {
        [0] = MEM_PRIV_NONE,
        [1] = MEM_PRIV_RWX,
        [2] = MEM_PRIV_RWX,
        [3] = MEM_PRIV_RWX,
        [4] = MEM_PRIV_NONE,
        [5] = MEM_PRIV_RX,
        [6] = MEM_PRIV_RX,
        [7] = MEM_PRIV_RX
    };

    if((AP==0) && (entry->SCTLR_S || entry->SCTLR_R)) {
        return MEM_PRIV_RX;
    } else {
        return privMap[AP];
    }
}

//
// Is the processor making a user-mode access?
//
inline static Bool isUserAccess(armP arm) {
    return getVirtualDataDomain(arm)==getDomainSetD(arm)->vmUser;
}

//
// Given a raw privilege (AP format), return the effective privilege once
// SCTLR.S, SCTLR.R and mode have been taken into account (client domain TLB
// accesses). A special case is that accesses in privileged mode where the
// current data memDomain is actually the user mode memDomain should be treated
// as user mode accesses (LDRT, STRT).
//
static memPriv getPrivTLBClient(armP arm, tlbEntryP entry, Uns8 AP) {

    memPriv result;

    // get privileges based on access mode
    if(isUserAccess(arm)) {
        result = getPrivTLBClientU(entry, AP);
    } else {
        result = getPrivTLBClientP(entry, AP);
    }

    // apply XN bit if required
    if(entry->XN) {result &= ~MEM_PRIV_X;}

    return result;
}

//
// Given a raw privilege (AP format), return the effective privilege based on
// mode only for user mode MPU accesses
//
static memPriv getPrivMPUU(armP arm, Uns8 AP, Bool XN) {

    static const memPriv privMap[] = {
        [0] = MEM_PRIV_NONE,
        [1] = MEM_PRIV_NONE,
        [2] = MEM_PRIV_RX,
        [3] = MEM_PRIV_RWX,
        [4] = MEM_PRIV_NONE,
        [5] = MEM_PRIV_NONE,
        [6] = MEM_PRIV_RX,
        [7] = MEM_PRIV_NONE
    };

    memPriv result = (AP>7) ? MEM_PRIV_NONE : privMap[AP];

    // apply XN bit if required
    if(XN) {result &= ~MEM_PRIV_X;}

    return result;
}

//
// Given a raw privilege (AP format), return the effective privilege based on
// mode only for privileged mode MPU accesses
//
static memPriv getPrivMPUP(armP arm, Uns8 AP, Bool XN) {

    static const memPriv privMap[] = {
        [0] = MEM_PRIV_NONE,
        [1] = MEM_PRIV_RWX,
        [2] = MEM_PRIV_RWX,
        [3] = MEM_PRIV_RWX,
        [4] = MEM_PRIV_NONE,
        [5] = MEM_PRIV_RX,
        [6] = MEM_PRIV_RX,
        [7] = MEM_PRIV_NONE
    };

    memPriv result = (AP>7) ? MEM_PRIV_NONE : privMap[AP];

    // apply XN bit if required
    if(XN) {result &= ~MEM_PRIV_X;}

    return result;
}

//
// Given a raw privilege (AP format), return the effective privilege considering
// mode only (MPU accesses). A special case is that accesses in kernel mode
// where the current data memDomain is actually the user mode memDomain should
// be treated as user mode accesses (LDRT, STRT).
//
static memPriv getPrivMPU(armP arm, Uns8 AP, Bool XN) {

    // get privileges based on access mode
    if(isUserAccess(arm)) {
        return getPrivMPUU(arm, AP, XN);
    } else {
        return getPrivMPUP(arm, AP, XN);
    }
}

//
// Return an address mask for the passed number of address bits (the largest
// valid address)
//
inline static Uns64 getAddressMask(Uns32 bits) {
    return (bits==64) ? -1ULL : ((1ULL<<bits)-1);
}

//
// Create an alias of the passed memDomain
//
static memDomainP makeDomainAliasBits(
    memDomainP  master,
    const char *name,
    Uns32       bits
) {
    memDomainP slave = vmirtNewDomain(name, bits);
    vmirtAliasMemory(master, slave, 0, getAddressMask(bits), 0, 0);
    return slave;
}

//
// Create an alias of the passed memDomain
//
static memDomainP makeDomainAlias(memDomainP master, const char *name) {
    return makeDomainAliasBits(master, name, vmirtGetDomainAddressBits(master));
}

//
// Create a mapping from virtual address 'lowVA' in the 'postFCSE' memDomain to
// physical address 'lowPA' in the 'postTCM' memDomain. The mapped range up to
// high physical address 'highPA' is valid.
//
static void mapFCSE(
    armDomainSetP domainSet,
    Uns64         lowPA,
    Uns64         highPA,
    Uns32         lowVA
) {
    vmirtAliasMemory(
        domainSet->postTCM,
        domainSet->postFCSE,
        lowPA,
        highPA,
        lowVA,
        0
    );
}

//
// Map memory virtual addresses starting at 'lowVA' to the physical address
// range 'lowPA':'highPA' with privilege 'priv'.
//
static void mapDomainPairRange(
    armP    arm,
    memPriv requiredPriv,
    memPriv priv,
    Uns64   lowPA,
    Uns64   highPA,
    Uns32   lowVA,
    Bool    G,
    Uns8    ASID
) {
    armDomainSetP domainSet = getDomainSetPriv(arm, requiredPriv);
    memDomainP    domainV   = getVirtualDomainPriv(arm, requiredPriv);
    memDomainP    domainP   = domainSet->postFCSE;

    // create the required mapping (with ASID, if required)
    vmirtAliasMemoryVM(domainP, domainV, lowPA, highPA, lowVA, 0, priv, G, ASID);
}

//
// Return the currently-selected DMA unit
//
inline static armDMAUnitP getCurrentDMAUnit(armP arm) {
    return arm->dmaUnits + CP_REG_UNS32(arm, DMAChannel);
}

//
// Write a value to the nDMAIRQ net, if connected
//
inline static void writeDMAIRQ(armP arm, Uns32 value) {
    vmirtWriteNetPort((vmiProcessorP)arm, arm->nDMAIRQ, 1);
}

//
// Is user-mode DMA enabled for the current DMA channel?
//
static Bool allowCurrentChannelUserModeDMA(armP arm) {

    Uns32 mask = 1<<CP_REG_UNS32(arm, DMAChannel);

    return CP_REG_UNS32(arm, DMAUserAccessibility) & mask;
}

//
// Do required actions when an invalid DMA memory access occurs
//
static void handleInvalidAccessDMA(
    armP        arm,
    Uns32       faultStatusD,
    armDMAUnitP unit
) {
    // indicate that the DMA transaction was aborted
    unit->abort = True;

    // set external or internal field
    if(unit->isExternal) {
        unit->DMAStatus.ES = faultStatusD;
    } else {
        unit->DMAStatus.IS = faultStatusD;
    }

    // signal interrupt request (if either DMAControl.IE or U bit for the
    // current channel is set in DMAUserAccessibility register)
    if(unit->DMAControl.IE || allowCurrentChannelUserModeDMA(arm)) {
        writeDMAIRQ(arm, 1);
    }
}

//
// Do required actions when an invalid memory access occurs, either a processor
// access or a DMA unit access
//
static void handleInvalidAccess(
    armP    arm,
    Uns32   faultStatusD,
    Uns32   MVA,
    memPriv requiredPriv,
    Bool    complete
) {
    armDMAUnitP dmaActive;

    if(!complete) {

        // no action

    } else if(arm->isVAtoPA) {

        // VA->PA address translation is active
        CP_FIELD(arm, PAR, fail.F)    = 1;
        CP_FIELD(arm, PAR, fail.FS)   = faultStatusD;
        CP_FIELD(arm, PAR, fail.Zero) = 0;

    } else if(!(dmaActive=arm->dmaActive)) {

        // processor-generated exception
        armMemoryAbort(arm, faultStatusD, MVA, requiredPriv);

    } else {

        // DMA-unit-generated exception
        handleInvalidAccessDMA(arm, faultStatusD|AFS_DMA, dmaActive);
    }
}


////////////////////////////////////////////////////////////////////////////////
//FCSE ADDRESS MAPPING
////////////////////////////////////////////////////////////////////////////////

//
// Is FCSE enabled?
//
inline static Bool enabledFCSE(armP arm) {
    return CP_FIELD(arm, FCSEIDR, PID);
}

//
// Return the limiting upper address of the FCSE-mapped page
//
inline static Uns32 getFCSELimit(void) {
    fcseVA va = {fields:{low:-1}};
    return va.raw;
}

//
// Apply FCSE mapping to the passed address
//
static Uns32 translateFCSE(armP arm, Uns32 address) {

    fcseVA va = {address};

    if(!va.fields.PID) {
        va.fields.PID = CP_FIELD(arm, FCSEIDR, PID);
    }

    return va.raw;
}

//
// If the range low:high has an FCSE-mapped alias, update the bounds with the
// equivalent range in the FCSE area and return True; otherwise, return False.
//
static Bool mapToFCSERegion(armP arm, Uns32 *lowP, Uns32 *highP) {

    if(enabledFCSE(arm)) {

        Uns32 low      = *lowP;
        Uns32 high     = *highP;
        Uns32 delta    = translateFCSE(arm, 0);
        Uns32 lowFCSE  = delta;
        Uns32 highFCSE = lowFCSE + getFCSELimit();

        if(highFCSE<low) {

            // no overlap

        } else if(lowFCSE>high) {

            // no overlap

        } else {

            // clip FCSE range to input range
            if(lowFCSE<low) {
                lowFCSE = low;
            }
            if(highFCSE>high) {
                highFCSE = high;
            }

            // adjust back to FCSE range
            *lowP  = lowFCSE - delta;
            *highP = highFCSE - delta;

            // overlap found
            return True;
        }
    }

    // no overlap
    return False;
}


////////////////////////////////////////////////////////////////////////////////
// MMU ADDRESS MAPPING
////////////////////////////////////////////////////////////////////////////////

//
// Is the TLB entry broken down into subpages with different permissions?
//
static Bool hasSubPages(tlbEntryP entry) {
    return (
        (entry->APsp0 != entry->APsp1) ||
        (entry->APsp0 != entry->APsp2) ||
        (entry->APsp0 != entry->APsp3)
    );
}

//
// Return address of first level entry
//
static Uns32 getLevel1Address(armP arm, Uns32 MVA) {

    Uns32 N = CP_FIELD(arm, TTBCR, N);
    Uns32 base;
    Uns32 baseLSB;
    Uns32 maskBase;
    Uns32 maskIndex;

    // select appropriate base register value and mask shift to select bits from
    // base register
    if(arm->useARMv5TTBR) {
        base    = CP_REG_UNS32(arm, TTBR);
        baseLSB = 14;
    } else if(N && (MVA>>(32-N))) {
        base    = CP_REG_UNS32(arm, TTBR1);
        baseLSB = 14;
    } else {
        base    = CP_REG_UNS32(arm, TTBR0);
        baseLSB = 14-N;
    }

    // construct masks for base and index selection
    maskBase  = (-1) << baseLSB;
    maskIndex = ~maskBase & ~0x3;

    // derive base address
    return (base & maskBase) | ((MVA>>18) & maskIndex);
}

//
// Fill ARMv5-format TLB entry (with subpages)
//
#define FILL_TLB_ENTRY_V5_SP(_A, _ENTRY, _TYPE, _DESC) { \
    _ENTRY->type    = _TYPE;                                \
    _ENTRY->ASID    = getASID(_A);                          \
    _ENTRY->APsp0   = _DESC.APsp0;                          \
    _ENTRY->APsp1   = _DESC.APsp1;                          \
    _ENTRY->APsp2   = _DESC.APsp2;                          \
    _ENTRY->APsp3   = _DESC.APsp3;                          \
    _ENTRY->TEXCB   = getTEXCB(0, _DESC.C, _DESC.B);        \
    _ENTRY->S       = 0;                                    \
    _ENTRY->XN      = False;                                \
    _ENTRY->G       = True;                                 \
    _ENTRY->SCTLR_S = CP_FIELD(_A, SCTLR, S);               \
    _ENTRY->SCTLR_R = CP_FIELD(_A, SCTLR, R);               \
}

//
// Fill ARMv5-format TLB entry (without subpages)
//
#define FILL_TLB_ENTRY_V5_NSP(_A, _ENTRY, _TYPE, _DESC) { \
    _ENTRY->type    = _TYPE;                                \
    _ENTRY->ASID    = getASID(_A);                          \
    _ENTRY->APsp0   = _DESC.AP;                             \
    _ENTRY->APsp1   = _DESC.AP;                             \
    _ENTRY->APsp2   = _DESC.AP;                             \
    _ENTRY->APsp3   = _DESC.AP;                             \
    _ENTRY->TEXCB   = getTEXCB(0, _DESC.C, _DESC.B);        \
    _ENTRY->S       = 0;                                    \
    _ENTRY->XN      = False;                                \
    _ENTRY->G       = True;                                 \
    _ENTRY->SCTLR_S = CP_FIELD(_A, SCTLR, S);               \
    _ENTRY->SCTLR_R = CP_FIELD(_A, SCTLR, R);               \
}

//
// Fill ARMv6-format TLB entry
//
#define FILL_TLB_ENTRY_V6(_A, _ENTRY, _TYPE, _DESC) { \
                                                            \
    Uns8 AP2 = getAP2(_A, _DESC.AP2);                       \
    Uns8 AP  = getFullAP(_DESC.AP, AP2);                    \
                                                            \
    _ENTRY->type    = _TYPE;                                \
    _ENTRY->ASID    = getASID(_A);                          \
    _ENTRY->APsp0   = AP;                                   \
    _ENTRY->APsp1   = AP;                                   \
    _ENTRY->APsp2   = AP;                                   \
    _ENTRY->APsp3   = AP;                                   \
    _ENTRY->TEXCB   = getTEXCB(_DESC.TEX, _DESC.C, _DESC.B);\
    _ENTRY->S       = _DESC.S;                              \
    _ENTRY->XN      = getXN(_A, _DESC.XN);                  \
    _ENTRY->G       = getG(_A, !_DESC.nG);                  \
    _ENTRY->SCTLR_S = CP_FIELD(_A, SCTLR, S);               \
    _ENTRY->SCTLR_R = CP_FIELD(_A, SCTLR, R);               \
}

//
// Fill virtual and physical addresses in the TLB entry, common code
//
#define FILL_TLB_ENTRY_COMMON(_ENTRY, _PA) { \
                                                            \
    /* get low physical address */                          \
    _ENTRY->lowPA  = _PA.raw;                               \
                                                            \
    /* get virtual address range */                         \
    _PA.fields.low = -1;                                    \
    Uns32 delta = _PA.raw-_ENTRY->lowPA;                    \
    _ENTRY->lowVA  = _ENTRY->lowVA & ~delta;                \
    _ENTRY->highVA = _ENTRY->lowVA + delta;                 \
}

//
// Fill virtual and physical addresses in the non-supersection TLB entry
//
#define FILL_TLB_ENTRY_ADDRESSES(_ENTRY, _TYPE, _DESC) { \
    _TYPE pa = {0};                                         \
    pa.fields.high = _DESC.Base;                            \
    _ENTRY->SS = 0;                                         \
    FILL_TLB_ENTRY_COMMON(_ENTRY, pa);                      \
}

//
// Fill virtual and physical addresses in the supersection TLB entry
//
#define FILL_TLB_ENTRY_ADDRESSES_SS(_ENTRY, _TYPE, _DESC) { \
    _TYPE pa = {0};                                         \
    pa.fields.base_39_36 = _DESC.XBA_39_36;                 \
    pa.fields.base_35_32 = _DESC.XBA_35_32;                 \
    pa.fields.base_31_24 = _DESC.Base;                      \
    _ENTRY->SS = 1;                                         \
    FILL_TLB_ENTRY_COMMON(_ENTRY, pa);                      \
}

//
// Interpret an ARMv4/ARMv5 large page entry in the MMU table and fill byref
// argument 'entry' with the details of that entry.
//
static armFaultStatus getMMUEntryLargeV5(
    armP              arm,
    level2DescLargeV5 large,
    tlbEntryP         entry

) {
    // fill basic TLB entry fields
    FILL_TLB_ENTRY_V5_SP(arm, entry, TET_PAGE, large);

    // fill TLB entry addresses
    FILL_TLB_ENTRY_ADDRESSES(entry, largePA, large);

    // page entries always match
    return AFS_OK;
}

//
// Interpret an ARMv6/ARMv7 large page entry in the MMU table and fill byref
// argument 'entry' with the details of that entry.
//
static armFaultStatus getMMUEntryLargeV6(
    armP              arm,
    level2DescLargeV6 large,
    tlbEntryP         entry
) {
    // fill basic TLB entry fields
    FILL_TLB_ENTRY_V6(arm, entry, TET_PAGE, large);

    // fill TLB entry addresses
    FILL_TLB_ENTRY_ADDRESSES(entry, largePA, large);

    // page entries always match
    return AFS_OK;
}

//
// Interpret an ARMv4/ARMv5 small page entry in the MMU table and fill byref
// argument 'entry' with the details of that entry.
//
static armFaultStatus getMMUEntrySmallV5(
    armP              arm,
    level2DescSmallV5 small,
    tlbEntryP         entry
) {
    // fill basic TLB entry fields
    FILL_TLB_ENTRY_V5_SP(arm, entry, TET_PAGE, small);

    // fill TLB entry addresses
    FILL_TLB_ENTRY_ADDRESSES(entry, smallPA, small);

    // page entries always match
    return AFS_OK;
}

//
// Interpret an ARMv6/ARMv7 small page entry in the MMU table and fill byref
// argument 'entry' with the details of that entry.
//
static armFaultStatus getMMUEntrySmallV6(
    armP              arm,
    level2DescSmallV6 small,
    tlbEntryP         entry
) {
    // fill basic TLB entry fields
    FILL_TLB_ENTRY_V6(arm, entry, TET_PAGE, small);

    // fill TLB entry addresses
    FILL_TLB_ENTRY_ADDRESSES(entry, smallPA, small);

    // page entries always match
    return AFS_OK;
}

//
// Interpret a tiny page entry in the MMU table and fill byref argument 'entry'
// with the details of that entry.
//
static armFaultStatus getMMUEntryTinyV5(
    armP             arm,
    level2DescTinyV5 tiny,
    tlbEntryP        entry
) {
    // fill basic TLB entry fields
    FILL_TLB_ENTRY_V5_NSP(arm, entry, TET_PAGE, tiny);

    // fill TLB entry addresses
    FILL_TLB_ENTRY_ADDRESSES(entry, tinyPA, tiny);

    // page entries always match
    return AFS_OK;
}

//
// Interpret a section entry in the MMU table and fill byref argument 'entry'
// with the details of that entry.
//
static armFaultStatus getMMUEntrySection(
    armP              arm,
    level1DescSection section,
    tlbEntryP         entry
) {
    // fill entry domain from section
    entry->domain = section.Domain;

    // fill non-secure bit
    entry->NS = section.NS;

    // fill basic TLB entry fields
    FILL_TLB_ENTRY_V6(arm, entry, TET_SECTION, section);

    // fill TLB entry addresses
    FILL_TLB_ENTRY_ADDRESSES(entry, sectionPA, section);

    // section entries always match
    return AFS_OK;
}

//
// Interpret a supersection entry in the MMU table and fill byref argument
// 'entry' with the details of that entry.
//
static armFaultStatus getMMUEntrySuperSection(
    armP                   arm,
    level1DescSuperSection superSection,
    tlbEntryP              entry
) {
    // supersection domain is always 0
    entry->domain = 0;

    // fill non-secure bit
    entry->NS = superSection.NS;

    // fill basic TLB entry fields
    FILL_TLB_ENTRY_V6(arm, entry, TET_SECTION, superSection);

    // fill TLB entry addresses
    FILL_TLB_ENTRY_ADDRESSES_SS(entry, superSectionPA, superSection);

    // supersection entries always match
    return AFS_OK;
}

//
// Interpret a section or supersection entry in the MMU table and fill byref
// argument 'entry' with the details of that entry.
//
static armFaultStatus getMMUEntryAnySection(
    armP       arm,
    level1Desc l1Desc,
    tlbEntryP  entry
) {
    Bool XP  = getXP(arm);
    Bool AFE = getAFE(arm);

    if(XP && AFE && !(l1Desc.section.AP & 1)) {
        return FAULT_STATUS(AFS_AccessFlagSection, INVALID_DOMAIN);
    } else if(l1Desc.section.isSS && supportSuperSection(arm)) {
        return getMMUEntrySuperSection(arm, l1Desc.superSection, entry);
    } else {
        return getMMUEntrySection(arm, l1Desc.section, entry);
    }
}

//
// Interpret a coarse page table entry in the MMU table and fill byref argument
// 'entry' with the details of that entry.
//
static Uns32 getMMUEntryCoarse(
    armP             arm,
    level1DescCoarse coarse,
    tlbEntryP        entry
) {
    coarseVA va = {entry->lowVA};
    coarsePA pa;

    // fill second level entry address
    pa.fields.zero = 0;
    pa.fields.low  = va.fields.secondIndex;
    pa.fields.high = coarse.Base;

    // get second level entry
    memDomainP memDomain = getDomainSetD(arm)->external;
    level2Desc l2Desc    = {transTableRead(arm, memDomain, pa.raw)};

    // fill entry domain and non-secure bit
    entry->domain = coarse.Domain;
    entry->NS     = coarse.NS;

    // handle the entry
    switch(l2Desc.fault.type) {

        case 1:
            if(getXP(arm)) {
                return getMMUEntryLargeV6(arm, l2Desc.largeV6, entry);
            } else {
                return getMMUEntryLargeV5(arm, l2Desc.largeV5, entry);
            }

        case 2:
            if(getXP(arm)) {
                return getMMUEntrySmallV6(arm, l2Desc.smallV6, entry);
            } else {
                return getMMUEntrySmallV5(arm, l2Desc.smallV5, entry);
            }

        case 3:
            if(getXP(arm) || supportExtendedSmall(arm)) {
                return getMMUEntrySmallV6(arm, l2Desc.smallV6, entry);
            }
            // fallthru

        default:
            return FAULT_STATUS(AFS_TranslationPage, coarse.Domain);
    }
}

//
// Interpret a fine page table entry in the MMU table and fill byref argument
// 'entry' with the details of that entry.
//
static Uns32 getMMUEntryFineV5(
    armP           arm,
    level1DescFine fine,
    tlbEntryP      entry
) {
    fineVA va = {entry->lowVA};
    finePA pa;

    // fill second level entry address
    pa.fields.zero = 0;
    pa.fields.low  = va.fields.secondIndex;
    pa.fields.high = fine.Base;

    // get second level entry
    memDomainP memDomain = getDomainSetD(arm)->external;
    level2Desc l2Desc    = {transTableRead(arm, memDomain, pa.raw)};

    // fill entry domain
    entry->domain = fine.Domain;

    // handle the entry
    switch(l2Desc.fault.type) {

        case 1:
            return getMMUEntryLargeV5(arm, l2Desc.largeV5, entry);

        case 2:
            return getMMUEntrySmallV5(arm, l2Desc.smallV5, entry);

        case 3:
            return getMMUEntryTinyV5(arm, l2Desc.tinyV5, entry);

        default:
            return FAULT_STATUS(AFS_TranslationPage, fine.Domain);
    }
}

//
// Look up and TLB entry for the passed address and fill byref argument 'entry'
// with the details.
//
static Uns32 tlbLookup(armP arm, tlbEntryP entry) {

    memDomainP memDomain     = getDomainSetD(arm)->external;
    Uns32      level1Address = getLevel1Address(arm, entry->lowVA);
    level1Desc l1Desc        = {transTableRead(arm, memDomain, level1Address)};

    switch(l1Desc.fault.type) {

        case 1:
            return getMMUEntryCoarse(arm, l1Desc.coarse, entry);

        case 2:
            return getMMUEntryAnySection(arm, l1Desc, entry);

        case 3:
            if(supportFine(arm)) {
                return getMMUEntryFineV5(arm, l1Desc.fine, entry);
            }
            // fallthru

        default:
            return FAULT_STATUS(AFS_TranslationSection, INVALID_DOMAIN);
    }
}


////////////////////////////////////////////////////////////////////////////////
// TLB ENTRIES
////////////////////////////////////////////////////////////////////////////////

//
// This macro implements an iterator to traverse all TLB entries in a range
//
#define ITER_TLB_ENTRY_RANGE(_TLB, _LOWVA, _HIGHVA, _ENTRY, _B) {   \
                                                                    \
    tlbEntryP _ENTRY;                                               \
    Uns32     _lowVA  = _LOWVA;                                     \
    Uns32     _highVA = _HIGHVA;                                    \
    Uns32     _I;                                                   \
                                                                    \
    /* handle lockdown entries */                                   \
    for(_I=0; _I<_TLB->ldSize; _I++) {                              \
        _ENTRY = _TLB->ldEntries+_I;                                \
        if(                                                         \
            _ENTRY->LDV              &&                             \
            (_highVA>=_ENTRY->lowVA) &&                             \
            (_lowVA<=_ENTRY->highVA)                                \
        ) {                                                         \
            _B;                                                     \
        }                                                           \
    }                                                               \
                                                                    \
    /* handle normal entries */                                     \
    while((_ENTRY=findTLBEntryRange(_TLB, _lowVA, _highVA))) {      \
        _lowVA = (_ENTRY)->highVA+1;                                \
        _B;                                                         \
        if(!_lowVA || (_lowVA>_highVA)) break;                      \
    }                                                               \
}

//
// Dump contents of the TLB entry (forward reference)
//
static void dumpTLBEntry(tlbEntryP entry);

//
// Get the processor data TLB
//
inline static armTLBP getDTLB(armP arm) {
    return arm->dtlb;
}

//
// Get the processor instruction TLB
//
inline static armTLBP getITLB(armP arm) {
    return TLB_UNIFIED(arm) ? arm->dtlb : arm->itlb;
}

//
// Return the TLB to use for the passed memory access type
//
inline static armTLBP getTLBPriv(armP arm, memPriv requiredPriv) {
    return (requiredPriv==MEM_PRIV_X) ? getITLB(arm) : getDTLB(arm);
}

//
// Is the passed TLB the instruction TLB?
//
inline static Bool isITLB(armP arm, armTLBP tlb) {
    return tlb==getITLB(arm);
}

//
// Is the passed TLB the data TLB?
//
inline static Bool isDTLB(armP arm, armTLBP tlb) {
    return tlb==getDTLB(arm);
}

//
// Get descriptive name of the passed TLB
//
static const char *getTLBName(armP arm, armTLBP tlb) {
    if(TLB_UNIFIED(arm)) {
        return "UNIFIED";
    } else if(isITLB(arm, tlb)) {
        return "INSTRUCTION";
    } else {
        return "DATA";
    }
}

//
// Return the indexed lockdown entry
//
inline static tlbEntryP getLockDownEntry(armTLBP tlb, Uns32 index) {
    return &tlb->ldEntries[index % tlb->ldSize];
}

//
// Return the index of the passed lockdown entry
//
inline static Uns32 getLockDownEntryIndex(armTLBP tlb, tlbEntryP entry) {
    return entry - tlb->ldEntries;
}

//
// Return size of memory mapped by the TLB entry
//
inline static Uns32 getTLBEntrySize(tlbEntryP entry) {
    return entry->highVA - entry->lowVA + 1;
}

//
// Table mapping from TLBLDPA.SZ to size
//
#define ARM_MAX_SZ 4
static Uns32 szMap[ARM_MAX_SZ] = {
    [1] = 0x1000,
    [2] = 0x10000,
    [3] = 0x100000,
    [0] = 0x1000000
};

//
// Convert from TLBLDPA.SZ code to TLB region size
//
static Uns32 szToSize(Uns32 sz) {
    VMI_ASSERT(sz<ARM_MAX_SZ, "unexpected size %u", sz);
    return szMap[sz];
}

//
// Convert from TLB region size to TLBLDPA.SZ code
//
static Uns32 sizeToSZ(Uns32 size) {

    Uns32 i;

    // search for member index matching this size
    for(i=0; i<ARM_MAX_SZ; i++) {
        if(szMap[i]==size) {
            return i;
        }
    }

    // here if there was no match (possible for uninitialized lockdown entries)
    return 0;
}

//
// Extract the Victim field from a TLBLockdown register
//
static Uns32 getTLBVictim(armTLBP tlb) {

    Uns32 bits      = tlb->ldEntryBits;
    Uns32 shift     = 32-(bits*2);
    Uns32 fieldMask = (1<<bits)-1;

    return (tlb->TLBLRu32 >> shift) & fieldMask;
}

//
// Set the Victim field in a TLBLockdown register
//
static void setTLBVictim(armTLBP tlb, Uns32 victim) {

    Uns32 bits      = tlb->ldEntryBits;
    Uns32 shift     = 32-(bits*2);
    Uns32 fieldMask = (1<<bits)-1;
    Uns32 regMask   = ~(fieldMask << shift);

    tlb->TLBLRu32 = ((tlb->TLBLRu32 & regMask) | (victim<<shift));
}

//
// Extract the Base field from a TLBLockdown register
//
static Uns32 getTLBBase(armTLBP tlb) {

    Uns32 bits      = tlb->ldEntryBits;
    Uns32 shift     = 32-(bits*1);
    Uns32 fieldMask = (1<<bits)-1;

    return (tlb->TLBLRu32 >> shift) & fieldMask;
}

//
// Return the next lockdown entry in the TLB (and increment the Victim)
//
static tlbEntryP getNextLockdownEntry(armP arm, armTLBP tlb) {

    Uns32     victim = getTLBVictim(tlb);
    tlbEntryP result = tlb->ldEntries + victim;

    // emit debug if required
    if(ARM_DEBUG_MMU(arm)) {
        vmiPrintf("SELECT %s TLB VICTIM %u\n", getTLBName(arm, tlb), victim);
    }

    // increment victim with wraparound
    if(++victim >= tlb->ldSize) {
        victim = getTLBBase(tlb);
    }

    // save new victim index
    setTLBVictim(tlb, victim);

    return result;
}

//
// Remove memory mappings for the passed domainSet and address range
//
static void deleteTLBEntryMappingsDomainSet(
    armDomainSetP domainSet,
    Uns32         lowVA,
    Uns32         highVA,
    Bool          G,
    Uns8          ASID
) {
    vmirtUnaliasMemoryVM(domainSet->vmPriv, lowVA, highVA, G, ASID);
    vmirtUnaliasMemoryVM(domainSet->vmUser, lowVA, highVA, G, ASID);
}

//
// Remove memory mappings for a TLB entry with the passed address range
//
static void deleteTLBEntryMappings(
    armP    arm,
    armTLBP tlb,
    Uns32   lowVA,
    Uns32   highVA,
    Bool    G,
    Uns8    ASID
) {
    if(TLB_UNIFIED(arm) || isITLB(arm, tlb)) {
        deleteTLBEntryMappingsDomainSet(
            getDomainSetI(arm), lowVA, highVA, G, ASID
        );
    }
    if(TLB_UNIFIED(arm) || isDTLB(arm, tlb)) {
        deleteTLBEntryMappingsDomainSet(
            getDomainSetD(arm), lowVA, highVA, G, ASID
        );
    }
}

//
// Delete a TLB entry
//
static void deleteTLBEntry(armP arm, armTLBP tlb, tlbEntryP entry) {

    // only handle allocated entries
    if(entry->LDV || entry->lutEntry) {

        // remove entry mappings if required
        if(entry->isMapped) {

            // get entry bounds
            Uns32 lowVA  = entry->lowVA;
            Uns32 highVA = entry->highVA;
            Bool  G      = entry->G;
            Uns8  ASID   = entry->ASID;

            // emit debug if required
            if(entry->LDV && ARM_DEBUG_MMU(arm)) {
                vmiPrintf(
                    "UNMAP %s TLB LOCKDOWN ENTRY %u:\n",
                    getTLBName(arm, tlb),
                    getLockDownEntryIndex(tlb, entry)
                );
                dumpTLBEntry(entry);
            }

            // remove mappings in both privileged and user memDomains
            deleteTLBEntryMappings(arm, tlb, lowVA, highVA, G, ASID);

            // also remove mappings in FCSE alias region if required
            if(mapToFCSERegion(arm, &lowVA, &highVA)) {
                deleteTLBEntryMappings(arm, tlb, lowVA, highVA, G, ASID);
            }

            // indicate entry is no longer mapped
            entry->isMapped = False;
        }

        // extra actions are required for non-lockdown entries
        if(entry->lutEntry) {

            // emit debug if required
            if(ARM_DEBUG_MMU(arm)) {
                vmiPrintf("DELETE %s TLB ENTRY:\n", getTLBName(arm, tlb));
                dumpTLBEntry(entry);
            }

            // remove the TLB entry from the range LUT
            vmirtRemoveRangeEntry(&tlb->lut, entry->lutEntry);
            entry->lutEntry = 0;

            // add the TLB entry to the free list
            entry->nextFree = tlb->free;
            tlb->free       = entry;
        }
    }
}

//
// Allocate a new TLB entry, filling it from the base object
//
static tlbEntryP allocateTLBEntry(armP arm, armTLBP tlb, tlbEntryP base) {

    Bool      isLockdown = tlb->DTLBLR.P;
    tlbEntryP entry;

    // get new entry structure
    if(isLockdown) {
        entry = getNextLockdownEntry(arm, tlb);
        deleteTLBEntry(arm, tlb, entry);
    } else if((entry=tlb->free)) {
        tlb->free = entry->nextFree;
    } else {
        entry = STYPE_ALLOC(tlbEntry);
    }

    // fill entry from base object
    *entry = *base;

    // set LDV if this is a lockdown entry
    entry->LDV = isLockdown;

    // return the new entry
    return entry;
}

//
// Return the first TLB entry overlapping the passed range, ignoring ASID
//
static tlbEntryP findTLBEntryRange(armTLBP tlb, Uns32 lowVA, Uns32 highVA) {

    vmiRangeEntryP lutEntry = vmirtGetFirstRangeEntry(&tlb->lut, lowVA, highVA);

    if(lutEntry) {

        union {Uns32 u32; tlbEntryP entry;} u = {
            vmirtGetRangeEntryUserData(lutEntry)
        };

        return u.entry;

    } else {

        return 0;
    }
}

//
// Return any TLB entry for the passed address and ASID
//
static tlbEntryP findTLBEntryWithASID(armTLBP tlb, Uns32 va, Uns32 ASID) {

    // return any entry with matching range and ASID
    ITER_TLB_ENTRY_RANGE(
        tlb, va, va, entry,
        if(entry->G || (entry->ASID==ASID)) {
            return entry;
        }
    );

    // here if there is no match
    return 0;
}

//
// Enumeration describing which TLB entries match a given ASID
//
typedef enum matchModeE {
    MM_ANY,         // match any TLB entry, irrespective of ASID
    MM_ANY_UNLOCKED,// match any unlocked TLB entry, irrespective of ASID
    MM_ASID,        // match any matching non-global entry
    MM_ASID_GLOBAL  // match any matching non-global entry or global TLB entry
} matchMode;

//
// Delete TLB entry if required by the passed matchMode
//
static void deleteTLBEntryMode(
    armP      arm,
    armTLBP   tlb,
    tlbEntryP entry,
    matchMode mode,
    Uns8      ASID
) {
    switch(mode) {

        case MM_ASID:
            if(!entry->G && (entry->ASID==ASID)) {
                deleteTLBEntry(arm, tlb, entry);
                entry->LDV = False;
            }
            break;

        case MM_ASID_GLOBAL:
            if(entry->G || (entry->ASID==ASID)) {
                deleteTLBEntry(arm, tlb, entry);
                entry->LDV = False;
            }
            break;

        case MM_ANY_UNLOCKED:
            if(!entry->LDV) {
                deleteTLBEntry(arm, tlb, entry);
            }
            break;

        default:
            // NOTE: lockdown entries deleted via this route remain valid
            deleteTLBEntry(arm, tlb, entry);
            break;
    }
}

//
// Delete TLB entries that overlap the passed range in the TLB.
// If mode is MM_ANY, then any matching entry is deleted, irrespective of ASID.
// If mode is MM_ANY_UNLOCKED, then any unlocked matching entry is deleted,
// irrespective of ASID.
// If mode is MM_ASID, then any matching non-global entry is deleted
// If mode is MM_ASID_GLOBAL, then any matching non-global entry or global entry
// is deleted
//
static void invalidateTLBEntriesRange(
    armP      arm,
    armTLBP   tlb,
    Uns32     lowVA,
    Uns32     highVA,
    matchMode mode,
    Uns8      ASID
) {
    ITER_TLB_ENTRY_RANGE(
        tlb, lowVA, highVA, entry,
        deleteTLBEntryMode(arm, tlb, entry, mode, ASID)
    );
}

//
// Invalidate TLB entries that conflict with the passed entry
//
inline static void invalidateOverlappingTLBEntries(
    armP      arm,
    armTLBP   tlb,
    tlbEntryP entry
) {
    invalidateTLBEntriesRange(
        arm, tlb, entry->lowVA, entry->highVA, MM_ANY, entry->ASID
    );
}

//
// Delete all TLB entries that overlap the passed range in memory domain sets
// determined by the passed privileges
//
static void invalidateRange(
    armP      arm,
    memPriv   priv,
    Uns32     lowVA,
    Uns32     highVA,
    matchMode mode,
    Uns8      ASID
) {
    Bool    invalidateI = priv & MEM_PRIV_X;
    Bool    invalidateD = priv & MEM_PRIV_RW;
    armTLBP itlb        = getITLB(arm);
    armTLBP dtlb        = getDTLB(arm);

    if(invalidateI) {
        invalidateTLBEntriesRange(arm, itlb, lowVA, highVA, mode, ASID);
    }

    if(invalidateD && !(invalidateI && (itlb==dtlb))) {
        invalidateTLBEntriesRange(arm, dtlb, lowVA, highVA, mode, ASID);
    }
}

//
// Type used to define mapping made by a TLB entry
//
typedef struct tlbMapInfoS {
    memPriv        priv;        // protection applied to mapped memory
    Uns64          lowPA;       // low physical address
    Uns64          highPA;      // high physical address
    Uns32          lowVA;       // low virtual address
    armFaultStatus faultStatus; // fault status if mapping fails
    Bool           G;           // is this a global entry?
    Uns8           ASID;        // ASID (if non-global)
} tlbMapInfo, *tlbMapInfoP;

//
// Exception codes for invalid domain permissions
//
const static armFaultStatus domainFaults[] = {
    [TET_SECTION] = AFS_DomainSection,
    [TET_PAGE]    = AFS_DomainPage
};

//
// Exception codes for invalid client permissions
//
const static armFaultStatus clientFaults[] = {
    [TET_SECTION] = AFS_PermissionSection,
    [TET_PAGE]    = AFS_PermissionPage
};

//
// Fill the byref tlbMapInfo structure with information about the mapping to be
// when the passed address is mapped using the TLB entry
//
static void getTLBEntryMapping(
    armP        arm,
    tlbMapInfoP mi,
    tlbEntryP   entry,
    Uns32       address,
    Uns32       MVA
) {
    // get lowVA and lowPA assuming that the entire entry will be mapped
    Uns32        size  = getTLBEntrySize(entry);
    Uns32        lowVA = address & ~(size-1);
    Uns64        lowPA = entry->lowPA;
    domainAccess da    = getDomainAccess(arm, entry->domain);

    // set basic ASID properties
    mi->G    = entry->G;
    mi->ASID = entry->ASID;

    if(da==DA_MANAGER) {

        // unrestricted access allowed
        mi->priv        = MEM_PRIV_RWX;
        mi->faultStatus = AFS_OK;

    } else if((da==DA_NONE) || (da==DA_RESERVED)) {

        // no access allowed
        mi->priv        = MEM_PRIV_NONE;
        mi->faultStatus = domainFaults[entry->type];

    } else if(!hasSubPages(entry)) {

        // get entry privileges and exception code for no access
        mi->priv        = getPrivTLBClient(arm, entry, entry->APsp0);
        mi->faultStatus = clientFaults[entry->type];

    } else {

        // TLB entry broken into four subentries with different permissions
        Uns8 AP;

        // get required subpage and size
        size = size/4;
        Uns32 index = (MVA-lowVA)/size;

        // determine permissions for subentry
        if(index==0) {
            AP = entry->APsp0;
        } else if(index==1) {
            AP = entry->APsp1;
        } else if(index==2) {
            AP = entry->APsp2;
        } else {
            AP = entry->APsp3;
        }

        // get subentry privileges and exception code for no access
        mi->priv        = getPrivTLBClient(arm, entry, AP);
        mi->faultStatus = clientFaults[entry->type];

        // adjust bounds for subentry
        lowVA += index*size;
        lowPA += index*size;
    }

    // fill result structure
    mi->lowPA  = lowPA;
    mi->highPA = lowPA + size - 1;
    mi->lowVA  = lowVA;
}

//
// Insert the TLB entry into the processor range table
//
static void insertTLBEntry(armP arm, armTLBP tlb, tlbEntryP entry) {

    // delete any existing entries that overlap this one
    invalidateOverlappingTLBEntries(arm, tlb, entry);

    // extra actions are required for non-lockdown entries
    if(!entry->LDV) {

        // emit debug if required
        if(ARM_DEBUG_MMU(arm)) {
            vmiPrintf("CREATE %s TLB ENTRY:\n", getTLBName(arm, tlb));
            dumpTLBEntry(entry);
        }

        // insert entry into TLB range table for fast lookup by address
        entry->lutEntry = vmirtInsertRangeEntry(
            &tlb->lut, entry->lowVA, entry->highVA, (Uns32)entry
        );
    }
}

//
// Find or create a TLB entry for the passed address in the passed memory
// domain set
//
static tlbEntryP getTLBEntry(
    armP    arm,
    armTLBP tlb,
    Uns32   MVA,
    memPriv requiredPriv,
    Bool    complete
) {
    tlbEntryP entry = findTLBEntryWithASID(tlb, MVA, getASID(arm));
    tlbEntry  new   = {lowVA:MVA};
    Uns32     faultStatusD;

    if(entry) {

        // entry already exists in TLB for this MVA (and ASID)

    } else if((faultStatusD=tlbLookup(arm, &new))) {

        // no matching entry
        handleInvalidAccess(arm, faultStatusD, MVA, requiredPriv, complete);

    } else {

        // determine the physical domain address mask
        armDomainSetP domainSet = getDomainSetPriv(arm, requiredPriv);
        memDomainP    domainP   = domainSet->postFCSE;
        Uns32         bits      = vmirtGetDomainAddressBits(domainP);
        Uns64         mask      = getAddressMask(bits);

        // mask PA to actual physical address bus width
        new.lowPA &= mask;

        // create a new TLB entry
        entry = allocateTLBEntry(arm, tlb, &new);

        // insert it into the processor TLB table
        insertTLBEntry(arm, tlb, entry);
    }

    return entry;
}

//
// Try mapping memory at the passed address for the specified access type and
// return a status code indicating whether the mapping succeeded
//
static armVMAction tlbMiss(
    armP    arm,
    memPriv requiredPriv,
    Uns32   address,
    Uns32  *lowVAP,
    Uns32  *highVAP,
    Bool    complete
) {
    Uns32     MVA = translateFCSE(arm, address);
    armTLBP   tlb = getTLBPriv(arm, requiredPriv);
    tlbEntryP entry;

    if(!MMU_MPU_ENABLED(arm)) {

        // invalid access if MMU is disabled
        return MA_BAD;

    } else if(!(entry=getTLBEntry(arm, tlb, MVA, requiredPriv, complete))) {

        // no matching entry
        return MA_EXCEPTION;

    } else {

        // get mapping to apply for the TLB entry
        tlbMapInfo mi;
        getTLBEntryMapping(arm, &mi, entry, address, MVA);

        // create entry mapping if required using non-FCSE-translated address
        mapDomainPairRange(
            arm,
            requiredPriv,
            mi.priv,
            mi.lowPA,
            mi.highPA,
            mi.lowVA,
            mi.G,
            mi.ASID
        );

        // indicate that this entry has some valid mappings
        entry->isMapped = True;

        // emit debug if required
        if(entry->LDV && ARM_DEBUG_MMU(arm)) {
            vmiPrintf(
                "MAP %s TLB LOCKDOWN ENTRY %u:\n",
                getTLBName(arm, tlb),
                getLockDownEntryIndex(tlb, entry)
            );
            dumpTLBEntry(entry);
        }

        if((mi.priv & requiredPriv) == requiredPriv) {

            // indicate the address range that has been mapped
            *lowVAP  = mi.lowVA;
            *highVAP = mi.lowVA + (mi.highPA-mi.lowPA);

            // TLB entry permissions are ok, but access may still not be
            // possible if no physical memory exists at the physical address
            return accessOK(arm, requiredPriv, address) ? MA_OK : MA_BAD;

        } else if(complete) {

            // invalid access permissions, do memory abort
            Uns32 faultStatusD = FAULT_STATUS(mi.faultStatus, entry->domain);

            handleInvalidAccess(arm, faultStatusD, MVA, requiredPriv, complete);

            return MA_EXCEPTION;

        } else {

            // invalid access permissions, don't do memory abort
            return MA_EXCEPTION;
        }
    }
}

//
// Type for inner-shareable attributes
//
typedef enum innerAttributesE {
    IATT_Non_Cacheable                   = 0,
    IATT_Strongly_Ordered                = 1,
    IATT_Device                          = 3,
    IATT_Write_Back_Write_Allocate       = 5,
    IATT_Write_Through_No_Write_Allocate = 6,
    IATT_Write_Back_No_Write_Allocate    = 7
} innerAttributes;

//
// Type for outer-shareable attributes
//
typedef enum outerAttributesE {
    OATT_Non_Cacheable                   = 0,
    OATT_Write_Back_Write_Allocate       = 1,
    OATT_Write_Through_No_Write_Allocate = 2,
    OATT_Write_Back_No_Write_Allocate    = 3
} outerAttributes;

//
// Memory type from TEX mapping
//
typedef enum primaryTEXMappingE {
    PTM_Strongly_Ordered = 0,
    PTM_Device           = 1,
    PTM_Normal_Memory    = 2,
    PTM_Reserved         = 3
} primaryTEXMapping;

//
// This type is used to describe region attributes
//
typedef struct regionAttributesS {
    innerAttributes inner;
    outerAttributes outer;
    Bool            shareable;
    Bool            NOS;
} regionAttributes, *regionAttributesP;

//
// Return an enumerated innerAttributes value
//
inline static innerAttributes getInnerAttributes(Uns32 AA) {

    const static innerAttributes mapAA[] = {
        IATT_Non_Cacheable,
        IATT_Write_Back_Write_Allocate,
        IATT_Write_Through_No_Write_Allocate,
        IATT_Write_Back_No_Write_Allocate
    };

    return mapAA[AA];
}

//
// Return an enumerated outerAttributes value
//
inline static outerAttributes getOuterAttributes(Uns32 BB) {
    return (outerAttributes)BB;
}

//
// Fill 'result' with region attributes when TEX remap is disabled
//
static void getRegionAttributesNoRemap(
    regionAttributesP result,
    Uns8              TEXCB,
    Bool              S
) {
    if(TEXCB==0) {              /* TEX=000 C=0 B=0 */

        result->inner     = IATT_Strongly_Ordered;
        result->outer     = OATT_Non_Cacheable;
        result->shareable = 1;

    } else if(TEXCB==1) {       /* TEX=000 C=0 B=1 */

        result->inner     = IATT_Device;
        result->outer     = OATT_Non_Cacheable;
        result->shareable = 1;

    } else if(TEXCB==2) {       /* TEX=000 C=1 B=0 */

        result->inner     = IATT_Write_Through_No_Write_Allocate;
        result->outer     = OATT_Write_Through_No_Write_Allocate;
        result->shareable = S;

    } else if(TEXCB==3) {       /* TEX=000 C=1 B=1 */

        result->inner     = IATT_Write_Back_No_Write_Allocate;
        result->outer     = OATT_Write_Back_No_Write_Allocate;
        result->shareable = S;

    } else if(TEXCB==7) {       /* TEX=001 C=1 B=1 */

        result->inner     = IATT_Write_Back_Write_Allocate;
        result->outer     = OATT_Write_Back_Write_Allocate;
        result->shareable = S;

    } else if(TEXCB==8) {       /* TEX=010 C=0 B=0 */

        result->inner     = IATT_Device;
        result->outer     = OATT_Non_Cacheable;
        result->shareable = 0;

    } else if(TEXCB & 0x10) {   /* TEX=1.. C=. B=. */

        result->inner     = getInnerAttributes((TEXCB>>0) & 0x3);
        result->outer     = getOuterAttributes((TEXCB>>2) & 0x3);
        result->shareable = S;

    } else {                    /* implementation defined or undefined */

        result->inner     = 0;
        result->outer     = 0;
        result->shareable = 0;
    }
}

//
// Fill 'result' with region attributes when TEX remap is enabled
//
static void getRegionAttributesRemap(
    armP              arm,
    regionAttributesP result,
    Uns8              TEXCB,
    Bool              S
) {
    // only TEX[0], C and B bits are used for this method
    TEXCB &= 0x7;

    // get PRRR & NMRR registers
    Uns32 PRRR = CP_REG_UNS32(arm, PRRR);
    Uns32 NMRR = CP_REG_UNS32(arm, NMRR);

    // extract memory type
    Uns32             shiftTR = TEXCB*2;
    primaryTEXMapping TR      = (PRRR >> shiftTR) & 0x3;

    // calculate shifts for field extraction
    Uns32 shiftIC  = shiftTR;
    Uns32 shiftNOS = TEXCB + 24;
    Uns32 shiftOC  = shiftIC + 16;

    // derive attributes based on region type
    switch(TR) {

        case PTM_Strongly_Ordered:
            result->shareable = 1;
            result->inner     = IATT_Strongly_Ordered;
            break;

        case PTM_Device:
            if(S) {
                result->shareable = CP_FIELD(arm, PRRR, DS1);
            } else {
                result->shareable = CP_FIELD(arm, PRRR, DS0);
            }
            result->inner = IATT_Device;
            break;

        default:
            if(S) {
                result->shareable = CP_FIELD(arm, PRRR, NS1);
            } else {
                result->shareable = CP_FIELD(arm, PRRR, NS0);
            }
            result->NOS   = result->shareable ? (PRRR>>shiftNOS) & 0x1 : 0;
            result->inner = getInnerAttributes((NMRR>>shiftIC) & 0x3);
            result->outer = getOuterAttributes((NMRR>>shiftOC) & 0x3);
            break;
    }
}

//
// Fill 'result' with region attributes for the TEX, C, B and S bits when TEX
// remap is disabled
//
static void getRegionAttributes(
    armP              arm,
    regionAttributesP result,
    Uns8              TEXCB,
    Bool              S
) {
    if(getTRE(arm)) {
        getRegionAttributesRemap(arm, result, TEXCB, S);
    } else {
        getRegionAttributesNoRemap(result, TEXCB, S);
    }
}

//
// Fill fields in PAR based on attributes and address
//
static void fillPAR(
    armP              arm,
    regionAttributesP attributes,
    Uns64             PA,
    Bool              SS
) {
    // fill fields independent of TEX, C, B and S
    CP_FIELD(arm, PAR, ok.F)   = 0;
    CP_FIELD(arm, PAR, ok._u1) = 0;
    CP_FIELD(arm, PAR, ok.SS)  = SS;
    CP_FIELD(arm, PAR, ok._u2) = 0;

    // extracted physical address depends on whether this is a SuperSection
    if(SS) {
        Uns32 PA_31_24 = (Uns32)(PA>>24) & 0xff;
        Uns32 PA_23_16 = (Uns32)(PA>>32) & 0xff;
        CP_FIELD(arm, PAR, ok.PA) = (PA_31_24<<12) | (PA_23_16<<4);
    } else {
        CP_FIELD(arm, PAR, ok.PA) = PA >> 12;
    }

    // non-secure bit is unimplemented unless Security Extensions are implemented
    CP_FIELD(arm, PAR, ok.NS) = 0;

    // fill remaining fields
    CP_FIELD(arm, PAR, ok.Outer) = attributes->outer;
    CP_FIELD(arm, PAR, ok.Inner) = attributes->inner;
    CP_FIELD(arm, PAR, ok.SH)    = attributes->shareable;
    CP_FIELD(arm, PAR, ok.NOS)   = attributes->NOS;
}

//
// Perform VA to PA mapping
//
static void translateVAtoPA(
    armP    arm,
    Uns32   VA,
    Bool    isUser,
    memPriv requiredPriv
) {
    Uns32 MVA = translateFCSE(arm, VA);

    // region attributes, initialized to default value
    regionAttributes attributes = {
        inner     : IATT_Strongly_Ordered,
        outer     : OATT_Non_Cacheable,
        shareable : True,
        NOS       : False
    };

    if(!MMU_MPU_ENABLED(arm)) {

        // return MMU-disabled privileges
        fillPAR(arm, &attributes, MVA, False);

    } else {

        armTLBP   tlb = getTLBPriv(arm, requiredPriv);
        tlbEntryP entry;

        // indicate that VA->PA translation is active
        arm->isVAtoPA = True;

        // switch to user-mode context if required
        if(isUser) {
            pushVirtualDataDomain(arm);
        }

        if(!(entry=getTLBEntry(arm, tlb, MVA, requiredPriv, True))) {

            // no matching entry (handled by handleInvalidAccess)

        } else {

            // get mapping to apply for the TLB entry
            tlbMapInfo mi;
            getTLBEntryMapping(arm, &mi, entry, VA, MVA);

            if(mi.priv & requiredPriv) {

                // fill region attributes
                getRegionAttributes(arm, &attributes, entry->TEXCB, entry->S);

                // calculate physical address within page
                Uns64 PA = mi.lowPA + (MVA - mi.lowVA);

                // compose PAR value
                fillPAR(arm, &attributes, PA, entry->SS);

            } else {

                // invalid access permissions, do memory abort
                Uns32 faultStatusD = FAULT_STATUS(mi.faultStatus, entry->domain);

                handleInvalidAccess(arm, faultStatusD, MVA, requiredPriv, True);
            }
        }

        // restore privileged-mode context if required
        if(isUser) {
            restoreVirtualDataDomain(arm);
        }

        // indicate that VA->PA translation is inactive
        arm->isVAtoPA = False;
    }
}

//
// Allocate TLB structure
//
static armTLBP newTLB(Uns32 ldSize) {

    armTLBP tlb = STYPE_CALLOC(armTLB);

    // record the number of lockdown entries
    tlb->ldSize = ldSize;

    // calculate size of bitfields required to hold TLBLockDown Victim
    while((1<<tlb->ldEntryBits)<ldSize) {
        tlb->ldEntryBits++;
    }

    // create writable mask for TLBLockDown fields (Base, Victim and P fields)
    Uns32 ldEntryBits = tlb->ldEntryBits*2;
    Uns32 maskShift   = 32-ldEntryBits;
    tlb->ldMask = (((1<<ldEntryBits)-1) << maskShift) | 1;

    // allocate TLB lockdown entries if required
    if(ldSize) {
        tlb->ldEntries = STYPE_CALLOC_N(tlbEntry, ldSize);
    }

    // allocate range table for fast TLB entry search
    vmirtNewRangeTable(&tlb->lut);

    return tlb;
}

//
// Free TLB structure
//
static void freeTLB(armP arm, armTLBP tlb) {

    if(tlb) {

        tlbEntryP entry;

        // delete all entries in the TLB (puts them in the free list)
        invalidateTLBEntriesRange(arm, tlb, 0, ARM_MAX_ADDR, MM_ANY, 0);

        // release entries in the free list
        while((entry=tlb->free)) {
            tlb->free = entry->nextFree;
            STYPE_FREE(entry);
        }

        // free TLB lockdown entries if required
        if(tlb->ldEntries) {
            STYPE_FREE(tlb->ldEntries);
        }

        // free the range table
        vmirtFreeRangeTable(&tlb->lut);

        // free the TLB structure
        STYPE_FREE(tlb);
    }
}

//
// Dump physical mappings of a TLB entry
//
static void dumpPA(
    tlbEntryP entry,
    Uns32     size,
    Uns32     index,
    Uns8      AP
) {
    Uns32   entryLowVA = entry->lowVA + size*index;
    Uns64   entryLowPA = entry->lowPA + size*index;
    char    asidString[32];

    // construct ASID string for non-global entries
    if(entry->G) {
        asidString[0] = 0;
    } else {
        sprintf(asidString, " ASID=%u", entry->ASID);
    }

    vmiPrintf(
        "    VA 0x%08x:0x%08x PA 0x"FMT_6408x":0x"FMT_6408x" AP %u%s%s\n",
        entryLowVA, entryLowVA+size-1,
        entryLowPA, entryLowPA+size-1,
        AP,
        entry->XN ? "-x" : "",
        asidString
    );
}

//
// Dump contents of the TLB entry
//
static void dumpTLBEntry(tlbEntryP entry) {

    // get entry bounds
    Uns32   entryLowVA  = entry->lowVA;
    Uns32   entryHighVA = entry->highVA;
    Uns32   size        = entryHighVA-entryLowVA+1;

    // show entry type and virtual address range
    vmiPrintf(
        "VA 0x%08x:0x%08x (%s)\n",
        entryLowVA, entryHighVA,
        entry->type==TET_SECTION ? "SECTION" : "PAGE"
    );

    // show physical mappings
    if(hasSubPages(entry)) {
        size /= 4;
        dumpPA(entry, size, 0, entry->APsp0);
        dumpPA(entry, size, 1, entry->APsp1);
        dumpPA(entry, size, 2, entry->APsp2);
        dumpPA(entry, size, 3, entry->APsp3);
    } else {
        dumpPA(entry, size, 0, entry->APsp0);
    }
}

//
// Dump contents of the TLB
//
static void dumpTLB(armP arm, armTLBP tlb) {

    vmiPrintf("%s TLB CONTENTS:\n", getTLBName(arm, tlb));

    ITER_TLB_ENTRY_RANGE(
        tlb, 0, ARM_MAX_ADDR, entry,
        dumpTLBEntry(entry)
    );
}

//
// Allocate TLB structures for the passed processor
//
static void newTLBs(armP arm) {

    // create data/unified TLB
    arm->dtlb = newTLB(TLDD_SIZE(arm));

    // create instruction TLB if required
    if(!TLB_UNIFIED(arm)) {
        arm->itlb = newTLB(TLDI_SIZE(arm));
    }
}

//
// Free TLB structures for the passed processor
//
static void freeTLBs(armP arm) {

    // show TLB content if debug enabled
    if(ARM_DEBUG_MMU(arm)) {
        armVMDumpTLB(arm);
    }

    // don't report TLB entries being deleted at end of simulation
    arm->flags &= ~ARM_DEBUG_MMU_MASK;

    // free instruction/unified TLB
    freeTLB(arm, getDTLB(arm));

    // free data TLB if required
    if(!TLB_UNIFIED(arm)) {
        freeTLB(arm, getITLB(arm));
    }
}


////////////////////////////////////////////////////////////////////////////////
// MPU FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

typedef enum protRegionCodeE {
    PEC_ENABLED,        // enabled area description
    PEC_DISABLED,       // disabled area description
    PEC_INVALID_SIZE,   // invalid size (less than minumum)
    PEC_INVALID_BASE,   // invalid base (not a multiple of size)
} protRegionCode;

// this type represents a single protection area entry
typedef struct protRegionS {
    Uns32 Base;         // entry base address
    Uns32 Size : 5;     // area size
    Uns32 TEX  : 3;     // TEX field (ARMv6 and later)
    Uns32 AP   : 3;     // area permissions
    Bool  E    : 1;     // enabled?
    Bool  XN   : 1;     // XN bit (ARMv6 and later)
    Bool  S    : 1;     // S bit (ARMv6 and later)
    Bool  C    : 1;     // C bit (ARMv6 and later)
    Bool  B    : 1;     // B bit (ARMv6 and later)
} protRegion;

//
// Return description string identifying MPU
//
static const char *getMPUName(armP arm, Bool isData) {
    return MPU_UNIFIED(arm) ? "" : isData ? " (D)" : " (I)";
}

//
// Return the count of the number of protection regions
//
static Uns32 mpuRegionNum(armP arm, Bool isData) {
    if(isData || MPU_UNIFIED(arm)) {
        return CP_FIELD(arm, MPUIR, DRegion);
    } else {
        return CP_FIELD(arm, MPUIR, IRegion);
    }
}

//
// Validate that the passed index is a valid MPU region index
//
static Bool validateRegionNum(armP arm, Bool isData, Uns32 index) {

    Uns32 regionNum = mpuRegionNum(arm, isData);

    if(index >= regionNum) {

        vmiMessage("a", CPU_PREFIX"_ROOR",
            SRCREF_FMT "protection area index %u exceeds maximum (%u)",
            SRCREF_ARGS(arm, getPC(arm)),
            index, regionNum-1
        );

        return False;

    } else {

        return True;
    }
}

//
// Return a pointer to the Protection Area Control registers
//
inline static protRegionP getPACRegs(armP arm, Bool isData) {
    return (isData && arm->dmpu) ? arm->dmpu : arm->impu;
}

//
// If the passed protection entry describes a valid region, set the bounds of
// that region and return PEC_ENABLED; otherwise return an enum member giving
// the reason why it is not valid
//
static protRegionCode getProtectionEntryRange(
    armP        arm,
    protRegionP pa,
    Uns32      *lowP,
    Uns32      *highP
) {
    if(!pa->E) {

        // disabled region
        return PEC_DISABLED;

    } else {

        // minumum region size allowed depends on MPU revision
        Uns32 minSize  = arm->useARMv5PAC ? 11 : 4;
        Uns32 minDelta = (2 << minSize) - 1;

        // get the region size (minus 1)
        Uns32 size  = pa->Size;
        Uns32 delta = (2 << size) - 1;

        // get the region base address
        Uns32 low = pa->Base;

        // return implied bounds
        *lowP  = low;
        *highP = low + delta;

        if(delta<minDelta) {
            // invalid size
            return PEC_INVALID_SIZE;
        } else if(low & delta) {
            // base not a multiple of size
            return PEC_INVALID_BASE;
        } else {
            // bounds valid
            return PEC_ENABLED;
        }
    }
}

//
// Remove all access privileges from the instruction and/or data domain pair,
// depending on whether this is an instruction or data access and whether the
// MPU is unified
//
static void removePrivMPU(
    armP       arm,
    memDomainP iDomain,
    memDomainP dDomain,
    Uns32      lowAddr,
    Uns32      highAddr,
    Bool       isData
) {
    Bool    unified = MPU_UNIFIED(arm);
    Bool    removeI = unified || !isData;
    Bool    removeD = unified ||  isData;
    memPriv priv    = MEM_PRIV_NONE;

    if(removeI) {
        vmirtProtectMemory(iDomain, lowAddr, highAddr, priv, MEM_PRIV_SET);
    }
    if(removeD) {
        vmirtProtectMemory(dDomain, lowAddr, highAddr, priv, MEM_PRIV_SET);
    }
}

//
// If the indexed protection entry is valid, unmap it, internal routine
//
static void unmapProtectionEntryInt(
    armP  arm,
    Uns32 low,
    Uns32 high,
    Bool  isData
) {
    // if debug mode is enabled, report the range being unmapped
    if(ARM_DEBUG_MMU(arm)) {
        vmiPrintf(
            "MPU%s UNMAP VA 0x%08x:0x%08x\n",
            getMPUName(arm, isData), low, high
        );
    }

    // remove privileged domain access permissions
    removePrivMPU(arm, arm->ids.vmPriv, arm->dds.vmPriv, low, high, isData);

    // remove user domain access permissions
    removePrivMPU(arm, arm->ids.vmUser, arm->dds.vmUser, low, high, isData);
}

//
// If the indexed protection entry is valid, unmap it
//
static void unmapProtectionEntry(armP arm, Uns32 low, Uns32 high, Bool isData) {

    // remove main mappings
    unmapProtectionEntryInt(arm, low, high, isData);

    // also remove mappings in FCSE alias region if required
    if(mapToFCSERegion(arm, &low, &high)) {
        unmapProtectionEntryInt(arm, low, high, isData);
    }
}

//
// Update MPU status when an MPU region entry has been updated
//
static void resetProtectionEntryRange(
    armP  arm,
    Uns32 index,
    Bool  isData,
    Bool  verbose
) {
    protRegionP pacRegs = getPACRegs(arm, isData);
    protRegionP pacReg  = pacRegs+index;
    Uns32       low;
    Uns32       high;

    // validate new protection area permissions
    switch(getProtectionEntryRange(arm, pacReg, &low, &high)) {

        case PEC_ENABLED:
            // remove any existing mapping for the new protection entry range
            unmapProtectionEntry(arm, low, high, isData);
            break;

        case PEC_INVALID_SIZE:
            // size error assertion
            if(verbose) {
                vmiMessage("a", CPU_PREFIX"_PEIS",
                    SRCREF_FMT "protection area %u has invalid size (%u bytes)",
                    SRCREF_ARGS(arm, getPC(arm)),
                    index, high - low + 1
                );
            }
            break;

        case PEC_INVALID_BASE:
            // base error assertion
            if(verbose) {
                vmiMessage("a", CPU_PREFIX"_PEIB",
                    SRCREF_FMT "protection area %u base 0x%08x is not a "
                    "multiple of size",
                    SRCREF_ARGS(arm, getPC(arm)),
                    index, low
                );
            }
            break;

        default:
            // disabled entry
            break;
    }
}

//
// Update the indexed region Base, Size or E fields
//
static void updateRegionBaseSizeE(
    armP  arm,
    Uns32 index,
    Bool  isData,
    Uns32 Base,
    Uns32 Size,
    Uns32 E
) {
    protRegionP pacRegs = getPACRegs(arm, isData);
    protRegionP pacReg  = pacRegs+index;

    // action is required only if the register changes value
    if((pacReg->Base!=Base) || (pacReg->Size!=Size) || (pacReg->E!=E)) {

        // remove any current mapping for the protection entry
        resetProtectionEntryRange(arm, index, isData, False);

        // update the entry
        pacReg->Base = Base;
        pacReg->Size = Size;
        pacReg->E    = E;

        // remove any mapping for the new region location
        resetProtectionEntryRange(arm, index, isData, True);
    }
}

//
// Read ARMv5 MPU Extended Access Permissions register
//
static Uns32 readAccessExtARMv5(armP arm, Bool isData) {

    protRegionP pacRegs   = getPACRegs(arm, isData);
    Uns32       regionNum = mpuRegionNum(arm, isData);
    Uns32       result    = 0;
    Int32       i;

    for(i=regionNum-1; i>=0; i--) {
        result = (result<<4) | pacRegs[i].AP;
    }

    return result;
}

//
// Modify AP or XN fields for the indexed MPU region
//
static void updateRegionAPXN(
    armP  arm,
    Uns32 index,
    Bool  isData,
    Uns8  newAP,
    Bool  newXN
) {
    protRegionP pacRegs = getPACRegs(arm, isData);
    protRegionP pacReg  = pacRegs+index;
    Uns8        oldAP   = pacReg->AP;
    Uns8        oldXN   = pacReg->XN;

    // update AP and XN fields
    pacReg->AP = newAP;
    pacReg->XN = newXN;

    // have region permissions on an enabled region changed?
    if(pacReg->E && ((oldAP!=newAP) || (oldXN!=newXN))) {

        // get privileges implied by old and new permissions in each mode
        memPriv oldPrivU = getPrivMPUU(arm, oldAP, oldXN);
        memPriv oldPrivP = getPrivMPUP(arm, oldAP, oldXN);
        memPriv newPrivU = getPrivMPUU(arm, newAP, newXN);
        memPriv newPrivP = getPrivMPUP(arm, newAP, newXN);

        // if privileges have been reduced, disable all access to the area of
        // memory assocaited with the region
        if((oldPrivU & ~newPrivU) || (oldPrivP & ~newPrivP)) {
            resetProtectionEntryRange(arm, index, isData, False);
        }
    }
}

//
// Write ARMv5 MPU Extended Access Permissions register
//
static void writeAccessExtARMv5(armP arm, Bool isData, Uns32 newValue) {

    protRegionP pacRegs   = getPACRegs(arm, isData);
    Uns32       regionNum = mpuRegionNum(arm, isData);
    Uns32       i;

    for(i=0; i<regionNum; i++) {

        // update entry permissions (preserve XN)
        updateRegionAPXN(arm, i, isData, newValue&0xf, pacRegs[i].XN);

        // handle next value
        newValue >>= 4;
    }
}

//
// Read ARMv5 MPU Access Permissions register
//
static Uns32 readAccessARMv5(armP arm, Bool isData) {

    union {Uns32 u32; armMPUAPExt ap;} u1 = {readAccessExtARMv5(arm, isData)};
    union {Uns32 u32; armMPUAP    ap;} u2 = {0};

    // convert to legacy-format permissions
    u2.ap.ap0 = u1.ap.ap0;
    u2.ap.ap1 = u1.ap.ap1;
    u2.ap.ap2 = u1.ap.ap2;
    u2.ap.ap3 = u1.ap.ap3;
    u2.ap.ap4 = u1.ap.ap4;
    u2.ap.ap5 = u1.ap.ap5;
    u2.ap.ap6 = u1.ap.ap6;
    u2.ap.ap7 = u1.ap.ap7;

    // return calculated value
    return u2.u32;
}

//
// Write ARMv5 MPU Access Permissions register
//
static void writeAccessARMv5(armP arm, Bool isData, Uns32 newValue) {

    union {Uns32 u32; armMPUAP    ap;} u1 = {newValue};
    union {Uns32 u32; armMPUAPExt ap;} u2;

    // convert to extended-format permissions
    u2.ap.ap0 = u1.ap.ap0;
    u2.ap.ap1 = u1.ap.ap1;
    u2.ap.ap2 = u1.ap.ap2;
    u2.ap.ap3 = u1.ap.ap3;
    u2.ap.ap4 = u1.ap.ap4;
    u2.ap.ap5 = u1.ap.ap5;
    u2.ap.ap6 = u1.ap.ap6;
    u2.ap.ap7 = u1.ap.ap7;

    // apply using extended-format
    writeAccessExtARMv5(arm, isData, u2.u32);
}

//
// Try mapping memory at the passed address for the specified access type and
// return a status code indicating whether the mapping succeeded
//
static armVMAction mpuMiss(
    armP    arm,
    memPriv requiredPriv,
    Uns32   address,
    Uns32  *lowVAP,
    Uns32  *highVAP,
    Bool    complete
) {
    Uns32       MVA       = translateFCSE(arm, address);
    Bool        isData    = !(requiredPriv & MEM_PRIV_X);
    protRegionP pacRegs   = getPACRegs(arm, isData);
    protRegionP match     = 0;
    Uns32       regionNum = mpuRegionNum(arm, isData);
    Uns32       low       = 0;
    Uns32       high      = 0;
    Uns32       i;

    // scan regions in lowest-to-highest priority order
    for(i=0; i<regionNum; i++) {

        Uns32 rlow, rhigh;

        // determine if the address lies in the passed area
        if(getProtectionEntryRange(arm, pacRegs+i, &rlow, &rhigh)==PEC_ENABLED) {
            if((rlow<=MVA) && (rhigh>=MVA)) {
                // match in this region
                match = pacRegs+i;
                low   = rlow;
                high  = rhigh;
            } else if(!match) {
                // no further action unless a match found
            } else if((rlow>MVA) && (rlow<high)) {
                // remove part of region ABOVE matching address
                high = rlow-1;
            } else if((rhigh<MVA) && (rhigh>low)) {
                // remove part of region BELOW matching address
                low = rhigh+1;
            }
        }
    }

    // map in region if a match was found
    if(match) {

        // get access permissions applicable to the range
        Uns8    AP       = match->AP;
        Bool    XN       = match->XN;
        memPriv priv     = getPrivMPU(arm, AP, XN);
        Uns32   delta    = MVA - address;
        Uns32   lowVA    = low  - delta;
        Uns32   highVA   = high - delta;
        Uns32   highFCSE = getFCSELimit();

        if(delta) {

            // if this is an address in an FCSE-mapped page, the page is limited
            // to the size of the FCSE-mapped page, even if the region size is
            // bigger
            if(low<delta) {
                Uns32 adjust = delta-low;
                lowVA  += adjust;
                low    += adjust;
            }

            if(highFCSE<highVA) {
                Uns32 adjust = highVA-highFCSE;
                highVA -= adjust;
                high   -= adjust;
            }

        } else if(enabledFCSE(arm) && (lowVA<=highFCSE)) {

            // if this is not an address in an FCSE-mapped page but the page
            // overlaps the FCSE page, don't map the section in the FCSE page
            Uns32 adjust = highFCSE-lowVA+1;
            lowVA += adjust;
            low   += adjust;
        }

        // if debug mode is enabled, report the range being mapped
        if(ARM_DEBUG_MMU(arm)) {
            vmiPrintf(
                "MPU%s MAP VA 0x%08x:0x%08x PA 0x%08x:0x%08x AP %u%s\n",
                getMPUName(arm, isData),
                lowVA, highVA,
                low, high,
                AP,
                XN ? "-x" : ""
            );
        }

        // does the entry have sufficient permissions?
        if((priv & requiredPriv) == requiredPriv) {

            // map MPU entry memory using non-FCSE-translated address
            mapDomainPairRange(
                arm, requiredPriv, priv, low, high, lowVA, True, 0
            );

            // indicate the address range that has been mapped
            *lowVAP  = lowVA;
            *highVAP = highVA;

            // MPU entry permissions are ok, but access may still not be
            // possible if no physical memory exists at the physical address
            return accessOK(arm, requiredPriv, address) ? MA_OK : MA_BAD;
        }
    }

    // invalid access, either because no matching entry found or existing entry
    // has insufficient permissions
    handleInvalidAccess(arm, 0, MVA, requiredPriv, complete);

    return MA_EXCEPTION;
}

//
// Free an MPU data structure
//
static void freeMPU(protRegionP *mpuHandle) {

    protRegionP mpu = *mpuHandle;

    STYPE_FREE(mpu);

    *mpuHandle = 0;
}

//
// Free an MPU data structure
//
static protRegionP newMPU(armP arm, Bool isData) {

    Uns32 regionNum = mpuRegionNum(arm, isData);

    return regionNum ? STYPE_CALLOC_N(protRegion, regionNum) : 0;
}

//
// Reset an MPU data structure
//
static void resetMPU(armP arm, Bool isData) {

    Uns32 regionNum = mpuRegionNum(arm, isData);
    Uns32 i;

    for(i=0; i<regionNum; i++) {
        updateRegionBaseSizeE(arm, i, isData, 0, 0, 0);
    }
}

//
// Free MPU structures for the passed processor
//
static void freeMPUs(armP arm) {

    // free instruction/unified MPU
    if(arm->impu) {
        freeMPU(&arm->impu);
    }

    // free data MPU if required
    if(arm->dmpu) {
        freeMPU(&arm->dmpu);
    }
}

//
// Allocate MPU structures for the passed processor
//
static void newMPUs(armP arm) {

    // create instruction/unified MPU
    arm->impu = newMPU(arm, False);

    // create data MPU if required
    if(!MPU_UNIFIED(arm)) {
        arm->dmpu = newMPU(arm, True);
    }
}

//
// Reset MPU contents
//
static void resetMPUs(armP arm) {

    // reset instruction/unified MPU
    if(arm->impu) {
        resetMPU(arm, False);
    }

    // reset data MPU if required
    if(arm->dmpu) {
        resetMPU(arm, True);
    }
}


////////////////////////////////////////////////////////////////////////////////
// TCM FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// This type enables fields in a protection area to be extracted
//
typedef union tcmRegionU {
    Uns32        reg;
    armTCMRegion fields;
} tcmRegion, *tcmRegionP;

//
// Return size in bytes for 'Size' field in TCM region
//
inline static Uns32 tcmSizeToBytes(Uns32 size) {
    return size ? (1 << (size+9)) : 0;
}

//
// Allocate TCM for the passed domain set
//
static void newTCM(armDomainSetP domainSet, const char *name, Uns32 size) {
    domainSet->tcm = vmirtNewDomain(name, ARM_GPR_BITS);
    vmirtMapMemory(domainSet->tcm, 0, size-1, MEM_RAM);
}

//
// Allocate ITCM
//
static void newITCM(armP arm) {
    Uns32 size = tcmSizeToBytes(CP_FIELD(arm, ITCMRR, Size));
    newTCM(getDomainSetI(arm), "code TCM", size);
}

//
// Allocate DTCM
//
static void newDTCM(armP arm) {
    Uns32 size = tcmSizeToBytes(CP_FIELD(arm, DTCMRR, Size));
    newTCM(getDomainSetD(arm), "data TCM", size);
}

//
// Update TCM region register
//
static void updateTCMRegion(
    armP          arm,
    armDomainSetP domainSet,
    memDomainP    tcm,
    Uns32         oldValue,
    Uns32         newValue,
    Uns32         bank
) {
    tcmRegion oldValueS = {oldValue};
    tcmRegion newValueS = {newValue};
    tcmRegion maskS     = {0};
    Uns32     size      = tcmSizeToBytes(oldValueS.fields.Size);
    Uns32     delta     = size - 1;

    // remove any existing mapping
    if(oldValueS.fields.E) {

        // get old base address, clipped to TCM size
        maskS.reg    = ~delta;
        Uns64 lowPA  = maskS.reg & oldValueS.reg;
        Uns64 highPA = lowPA + delta;

        // if debug mode is enabled, report the range being unmapped
        if(ARM_DEBUG_MMU(arm)) {
            vmiPrintf("TCM UNMAP PA 0x"FMT_6408x":0x"FMT_6408x"\n", lowPA, highPA);
        }

        // restore mapping to physical memory
        vmirtAliasMemory(
            domainSet->external,
            domainSet->postTCM,
            lowPA,
            highPA,
            lowPA,
            0
        );
    }

    // create new mapping if required
    if(newValueS.fields.E) {

        // get old base address, clipped to TCM size
        maskS.reg      = ~delta;
        Uns64 lowPA    = maskS.reg & newValueS.reg;
        Uns64 highPA   = lowPA + delta;
        Uns32 lowTCMA  = bank * size;
        Uns32 highTCMA = lowTCMA + delta;

        // if debug mode is enabled, report the range being mapped
        if(ARM_DEBUG_MMU(arm)) {
            vmiPrintf(
                "TCM MAP PA 0x"FMT_6408x":0x"FMT_6408x" (bank %u, size %u)\n",
                lowPA, highPA, bank, size
            );
        }

        // create mapping to TCM
        vmirtAliasMemory(
            tcm,
            domainSet->postTCM,
            lowTCMA,
            highTCMA,
            lowPA,
            0
        );
    }
}

//
// Update ITCM region register
//
static void updateITCMRR(armP arm, Uns32 newValue) {

    armDomainSetP domainSetI = getDomainSetI(arm);
    armDomainSetP domainSetD = getDomainSetD(arm);
    memDomainP    tcm        = domainSetI->tcm;
    Uns32         oldValue   = CP_REG_UNS32(arm, ITCMRR);
    Uns32         bank       = 0;

    // ITCM is visible in code and data space
    updateTCMRegion(arm, domainSetI, tcm, oldValue, newValue, bank);
    updateTCMRegion(arm, domainSetD, tcm, oldValue, newValue, bank);

    CP_REG_UNS32(arm, ITCMRR) = newValue;
}

//
// Update DTCM region register
//
static void updateDTCMRR(armP arm, Uns32 newValue) {

    armDomainSetP domainSetD = getDomainSetD(arm);
    memDomainP    tcm        = domainSetD->tcm;
    Uns32         oldValue   = CP_REG_UNS32(arm, DTCMRR);
    Uns32         bank       = 0;

    // DTCM is visible in data space only
    updateTCMRegion(arm, domainSetD, tcm, oldValue, newValue, bank);

    CP_REG_UNS32(arm, DTCMRR) = newValue;
}


////////////////////////////////////////////////////////////////////////////////
// DMA FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Return the number of DMA units implemented
//
static Uns32 getDMAUnitNum(armP arm) {

    Uns32 mask  = CP_REG_UNS32(arm, DMAPresent);
    Uns32 count = 0;

    while(mask) {
        mask >>= 1;
        count++;
    }

    return count;
}

//
// Is the passed DMA unit in operation?
//
static Bool inOperation(armDMAUnitP unit) {

    switch(unit->DMAStatus.Status) {

        case ADS_Queued:
        case ADS_Running:
            // these states always indicate the channel is running
            return True;

        case ADS_Complete:
            // in this state the channel is running if it is in an error state
            // (NOTE: behavior here differs from ARM spec)
            return (
                unit->DMAStatus.BP ||
                unit->DMAStatus.IS ||
                unit->DMAStatus.ES
            );

        default:
            return False;
    }
}

//
// Is the passed DMA unit running or queued?
//
static Bool isRunningOrQueued(armDMAUnitP unit) {

    switch(unit->DMAStatus.Status) {

        case ADS_Queued:
        case ADS_Running:
            return True;

        default:
            return False;
    }
}

//
// Write DMAControl register value
//
static void writeDMAControl(armP arm, Uns32 newValue) {

    armDMAUnitP unit = getCurrentDMAUnit(arm);

    if(unit) {
        unit->DMAControlU32 = newValue;
    }
}

//
// Read DMAControl register value
//
static Uns32 readDMAControl(armP arm) {

    armDMAUnitP unit = getCurrentDMAUnit(arm);

    return unit ? unit->DMAControlU32 : 0;
}

//
// Write DMAInternalStart register value
//
static void writeDMAInternalStart(armP arm, Uns32 newValue) {

    armDMAUnitP unit = getCurrentDMAUnit(arm);

    if(unit && !isRunningOrQueued(unit)) {
        unit->internalStart = newValue;
    }
}

//
// Read DMAInternalStart register value
//
static Uns32 readDMAInternalStart(armP arm) {

    armDMAUnitP unit = getCurrentDMAUnit(arm);

    return unit ? unit->internalStart : 0;
}

//
// Write DMAExternalStart register value
//
static void writeDMAExternalStart(armP arm, Uns32 newValue) {

    armDMAUnitP unit = getCurrentDMAUnit(arm);

    if(unit && !isRunningOrQueued(unit)) {
        unit->externalStart = newValue;
    }
}

//
// Read DMAExternalStart register value
//
static Uns32 readDMAExternalStart(armP arm) {

    armDMAUnitP unit = getCurrentDMAUnit(arm);

    return unit ? unit->externalStart : 0;
}

//
// Write DMAInternalEnd register value
//
static void writeDMAInternalEnd(armP arm, Uns32 newValue) {

    armDMAUnitP unit = getCurrentDMAUnit(arm);

    if(unit && !isRunningOrQueued(unit)) {
        unit->internalEnd = newValue;
    }
}

//
// Read DMAInternalEnd register value
//
static Uns32 readDMAInternalEnd(armP arm) {

    armDMAUnitP unit = getCurrentDMAUnit(arm);

    return unit ? unit->internalEnd : 0;
}

//
// Read DMAStatus register value
//
static Uns32 readDMAStatus(armP arm) {

    armDMAUnitP unit = getCurrentDMAUnit(arm);

    return unit ? unit->DMAStatusU32 : 0;
}

//
// Write DMAContextID register value
//
static void writeDMAContextID(armP arm, Uns32 newValue) {

    armDMAUnitP unit = getCurrentDMAUnit(arm);

    if(unit) {
        unit->DMAContextIDU32 = newValue;
    }
}

//
// Read DMAContextID register value
//
static Uns32 readDMAContextID(armP arm) {

    armDMAUnitP unit = getCurrentDMAUnit(arm);

    return unit ? unit->DMAContextIDU32 : 0;
}

//
// Return DMA unit transaction size
//
inline static Uns32 getTransactionSize(armDMAUnitP unit) {
    return 1<<unit->DMAControl.TS;
}

//
// Are DMA unit parameters valid?
//
static Bool validateDMAParameters(armP arm, armDMAUnitP unit) {

    Uns32 internalStart = unit->internalStart;
    Uns32 internalEnd   = unit->internalEnd;
    Uns32 externalStart = unit->externalStart;
    Uns32 TS            = getTransactionSize(unit);
    Uns32 TSM1          = TS-1;
    Uns32 stride        = unit->DMAControl.ST;

    // do alignment checks
    if(internalStart&TSM1) {
        return False;
    } else if(internalEnd&TSM1) {
        return False;
    } else if(externalStart&TSM1) {
        return False;
    } else if(stride&TSM1) {
        return False;
    } else {
        return True;
    }
}

//
// Does the passed virtual address lie within the enabled TCM referred to by the
// passed DMA unit?
//
static Bool addressInTCM(
    armP        arm,
    armDMAUnitP unit,
    Uns32       address,
    memRegionPP cachedRegion,
    memPriv     priv
) {
    memPriv       privTCM       = unit->DMAControl.TR ? MEM_PRIV_X : MEM_PRIV_R;
    armDomainSetP set           = getDomainSetPriv(arm, privTCM);
    memDomainP    virtualDomain = getVirtualDataDomain(arm);
    memDomainP    physicalDomain;

    // get physical domain for any currently-established mapping
    vmirtMapVAToPA(virtualDomain, address, &physicalDomain, cachedRegion);

    // no currently-established mapping: try establishing the mapping
    if(!physicalDomain) {
        armVMMiss(arm, priv, address, 1, True);
        vmirtMapVAToPA(virtualDomain, address, &physicalDomain, cachedRegion);
    }

    // determine whether the address is in the TCM domain
    return (physicalDomain == set->tcm);
}

//
// Validate that the passed virtual address is either in or not in the TCM
// region, depending on whether it is an external address. If the address is
// invalid, update DMAStatus.ES or DMAStatus.IS with the required value and
// abort the DMA operation.
//
static Bool validateAddressDMA(
    armP        arm,
    armDMAUnitP unit,
    Uns32       address,
    memRegionPP cachedRegion,
    memPriv     priv
) {
    memDomainP virtualDomain = getVirtualDataDomain(arm);
    Bool       isExternal    = unit->isExternal;
    Bool       inTCM         = addressInTCM(arm, unit, address, cachedRegion, priv);

    if(!(vmirtGetDomainPrivileges(virtualDomain, address) & priv)) {

        // no access at the required address (access permission failures are
        // higher priority than TCM address failures)
        return True;

    } else if(inTCM==isExternal) {

        // external address required but TCM address found, or vice-versa
        Uns32 faultStatusD = isExternal ? AFS_DMA_Unshared : AFS_DMA_TCM_Range;
        handleInvalidAccessDMA(arm, faultStatusD, unit);
        return False;

    } else {

        // permissions OK and addressed units OK
        return True;
    }
}

//
// Do DMA transfer using the passed unit
//
static void doDMA(armP arm, armDMAUnitP unit) {

    // the DMA transaction is performed immediately as an atomic operation,
    // so set the DMAStatus to Complete or Error
    unit->DMAStatus.Status = ADS_Complete;

    // validate the arguments
    unit->DMAStatus.BP = !validateDMAParameters(arm, unit);

    // only do the transfer if arguments are valid
    if(!unit->DMAStatus.BP) {

        Bool        DT        = unit->DMAControl.DT;
        Uns32       TS        = getTransactionSize(unit);
        Uns32       stride    = unit->DMAControl.ST;
        Bool        userMode  = IN_USER_MODE(arm) || unit->DMAControl.UM;
        armCPSRMode oldMode   = GET_MODE(arm);
        armCPSRMode newMode   = userMode ? ARM_CPSR_USER : oldMode;
        memRegionP  srcRegion = 0;
        memRegionP  dstRegion = 0;

        // force the processor into user mode if required during the DMA
        armForceMode(arm, newMode);

        // get the memory domain to use for transfers (*after* forcing the
        // processor into the required mode)
        memDomainP domain = getVirtualDataDomain(arm);

        // indicate that this DMA unit is active
        arm->dmaActive = unit;
        unit->abort    = False;

        // do each iteration
        while(unit->internalStart<unit->internalEnd) {

            memEndian endian  = MEM_ENDIAN_LITTLE;
            Uns32     intAddr = unit->internalStart;
            Uns32     extAddr = unit->externalStart;
            Uns32     srcAddr = DT ? intAddr : extAddr;
            Uns32     dstAddr = DT ? extAddr : intAddr;
            Uns64     value;

            // set isExternal for the source value access
            unit->isExternal = !DT;

            // read the source value
            if(!validateAddressDMA(arm, unit, srcAddr, &srcRegion, MEM_PRIV_R)) {
                value = 0;
            } else if(TS==1) {
                value = vmirtRead1ByteDomain(domain, srcAddr, True);
            } else if(TS==2) {
                value = vmirtRead2ByteDomain(domain, srcAddr, endian, True);
            } else if(TS==4) {
                value = vmirtRead4ByteDomain(domain, srcAddr, endian, True);
            } else {
                value = vmirtRead8ByteDomain(domain, srcAddr, endian, True);
            }

            // terminate the transfer on error
            if(unit->abort) {
                break;
            }

            // set isExternal for the destination value access
            unit->isExternal = DT;

            // write the destination value
            if(!validateAddressDMA(arm, unit, dstAddr, &dstRegion, MEM_PRIV_W)) {
                // no action
            } else if(TS==1) {
                vmirtWrite1ByteDomain(domain, dstAddr, value, True);
            } else if(TS==2) {
                vmirtWrite2ByteDomain(domain, dstAddr, endian, value, True);
            } else if(TS==4) {
                vmirtWrite4ByteDomain(domain, dstAddr, endian, value, True);
            } else {
                vmirtWrite8ByteDomain(domain, dstAddr, endian, value, True);
            }

            // terminate the DMA
            if(unit->abort) {
                break;
            }

            // increment for next iteration
            unit->internalStart = intAddr + TS;  // transaction size
            unit->externalStart = extAddr + stride;
        }

        // signal interrupt request on completion if required
        if(!unit->abort && unit->DMAControl.IC) {
            writeDMAIRQ(arm, 1);
        }

        // indicate that this DMA unit is no longer active
        arm->dmaActive = 0;

        // force the processor back into the correct mode
        armForceMode(arm, oldMode);
    }
}

//
// Write DMAStop register value
//
static void writeDMAStop(armP arm) {

    armDMAUnitP unit = getCurrentDMAUnit(arm);

    if(unit->DMAStatus.Status == ADS_Queued) {
        unit->DMAStatus.Status = ADS_Idle;
    }
}

//
// Initiate a DMA transfer
//
static void writeDMAStart(armP arm) {

    armDMAUnitP unit = getCurrentDMAUnit(arm);

    if(unit->DMAStatus.Status == ADS_Idle) {

        Uns32 unitNum = getDMAUnitNum(arm);
        Bool  queue   = False;
        Uns32 i;

        // is any other DMA unit currently in use?
        for(i=0; i<unitNum; i++) {

            armDMAUnitP other = arm->dmaUnits + i;

            if((other!=unit) && inOperation(other)) {
                queue = True;
            }
        }

        // either queue this unit or run now
        if(queue) {
            unit->DMAStatus.Status = ADS_Queued;
        } else {
            doDMA(arm, unit);
        }
    }
}

//
// Write DMAClear register
//
static void writeDMAClear(armP arm) {

    armDMAUnitP unit    = getCurrentDMAUnit(arm);
    Uns32       unitNum = getDMAUnitNum(arm);
    Uns32       i;

    // clear all error bits for the current channel and mark it as idle
    unit->DMAStatusU32 = 0;

    // clear DMAIRQ if required
    writeDMAIRQ(arm, 0);

    // restart any other queued DMA unit
    for(i=0; i<unitNum; i++) {

        armDMAUnitP other = arm->dmaUnits + i;

        if((other!=unit) && (other->DMAStatus.Status==ADS_Queued)) {
            doDMA(arm, other);
        }
    }
}

//
// Allocate DMA units if required
//
static void newDMAUnits(armP arm) {

    Uns32 unitNum = getDMAUnitNum(arm);

    if(unitNum) {
        arm->dmaUnits = STYPE_CALLOC_N(armDMAUnit, unitNum);
        arm->nDMAIRQ  = vmirtGetNetPortHandle((vmiProcessorP)arm, "nDMAIRQ");
    }
}

//
// Free DMA unit structures
//
static void freeDMAUnits(armP arm) {

    if(arm->dmaUnits) {
        STYPE_FREE(arm->dmaUnits);
    }
}


////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Try mapping memory at the passed address for the specified access type and
// return a status code indicating whether the mapping succeeded
//
armVMAction armVMMiss(
    armP    arm,
    memPriv requiredPriv,
    Uns32   address,
    Uns32   bytes,
    Bool    complete
) {
    armVMAction action = MA_BAD;
    Uns32       lastVA = address+bytes-1;
    Uns32       highVA = address-1;
    Uns32       lowVA;

    // iterate over all address ranges straddled by this access
    do {
        address = highVA+1;

        if(MMU_PRESENT(arm)) {
            return tlbMiss(arm, requiredPriv, address, &lowVA, &highVA, complete);
        } else if(MPU_PRESENT(arm)) {
            return mpuMiss(arm, requiredPriv, address, &lowVA, &highVA, complete);
        }

    } while((action==MA_OK) && ((lastVA<lowVA) || (lastVA>highVA)));

    return action;
}

//
// Perform VA to PA mapping
//
void armVMTranslateVAtoPA(armP arm, Uns32 VA, Bool isUser, memPriv priv) {
    translateVAtoPA(arm, VA, isUser, priv);
}

//
// Invalidate entire TLB
//
void armVMInvalidate(armP arm, memPriv priv) {
    invalidateRange(arm, priv, 0, ARM_MAX_ADDR, MM_ANY_UNLOCKED, 0);
}

//
// Invalidate TLB entry by matching MVA (and ASID if ARMv6)
//
void armVMInvalidateEntryMVA(armP arm, Uns32 MVA, Uns32 ASID, memPriv priv) {
    invalidateRange(arm, priv, MVA, MVA, MM_ASID_GLOBAL, ASID);
}

//
// Invalidate TLB entries by matching ASID (ARMv6)
//
void armVMInvalidateEntryASID(armP arm, Uns32 ASID, memPriv priv) {
    invalidateRange(arm, priv, 0, ARM_MAX_ADDR, MM_ASID, ASID);
}

//
// Invalidate TLB entry by matching MVA, ignoring ASID
//
void armVMInvalidateEntryMVAA(armP arm, Uns32 MVA, memPriv priv) {
    invalidateRange(arm, priv, MVA, MVA, MM_ANY_UNLOCKED, 0);
}

//
// Perform any required memory mapping updates on a PID or mode switch
//
void armVMSetPIDOrModeFCSE(armP arm) {

    Uns8      PID = CP_FIELD(arm, FCSEIDR, PID);
    Uns32     DAC = CP_REG_UNS32(arm, DACR);
    armPIDSet set = getPIDSet(arm);

    if(downgradeDACPriv(arm, arm->dacs[set], DAC)) {

        // remove all access permissions if Domain Access Control implies
        // reduced permissions than when we were originally in this mode
        removePermissionsCurrent(arm, 0, ARM_MAX_ADDR);

    } else if(arm->pids[set] != PID) {

        // delete lowest 32Mb page mappings if PID has changed since we were
        // originally in this mode
        fcseVA highVAS = {fields:{PID:0, low:-1}};
        Uns32  lowVA   = 0;
        Uns32  highVA  = highVAS.raw;

        if(set!=APS_PHYS) {

            // VIRTUAL ADDRESS MODE: remove all permissions in the lowest 32Mb
            // page (later calls to armVMMiss will reinstate them if required)
            removePermissionsCurrent(arm, lowVA, highVA);

        } else if(!arm->configInfo.fcseRequiresMMU) {

            // PHYSICAL ADDRESS MODE, FCSE ENABLED SEPARATELY TO MMU/MPU: the
            // lowest 32Mb page must be re-aliased to the correct 32Mb portion
            // in the true physical memDomain (determined by PID)
            fcseVA lowPAS  = {fields:{PID:PID, low: 0}};
            fcseVA highPAS = {fields:{PID:PID, low:-1}};
            Uns64  lowPA   = lowPAS.raw;
            Uns64  highPA  = highPAS.raw;

            // map code and data memDomains
            mapFCSE(getDomainSetI(arm), lowPA, highPA, lowVA);
            mapFCSE(getDomainSetD(arm), lowPA, highPA, lowVA);
        }
    }

    // maintain state
    arm->pids[set] = PID;
    arm->dacs[set] = DAC;
}

//
// Perform any required memory mapping updates on an ASID change
//
void armVMSetASID(armP arm) {
    vmirtSetProcessorASID((vmiProcessorP)arm, getASID(arm));
}

//
// Perform any required memory mapping updates on domain access control change
//
void armVMSetDomainAccessControl(armP arm) {

    Uns32     DAC = CP_REG_UNS32(arm, DACR);
    armPIDSet set = getPIDSet(arm);

    // report if the DAC value contains undefined values
    validateDAC(arm, DAC);

    // if permissions have been downgraded, remove all permissions (later calls
    // to armVMMiss will reinstate them if required)
    if(downgradeDACPriv(arm, arm->dacs[set], DAC)) {
        removePermissionsCurrent(arm, 0, ARM_MAX_ADDR);
    }

    // maintain state
    arm->dacs[set] = DAC;
}

//
// Set the privileged mode data domain to the user domain (for LDRT, STRT)
//
void armVMSetUserPrivilegedModeDataDomain(armP arm) {
    pushVirtualDataDomain(arm);
}

//
// Restore the normal data domain for the current mode
//
void armVMRestoreNormalDataDomain(armP arm) {
    restoreVirtualDataDomain(arm);
}

//
// Write ARMv5 MPU Access Permissions register
//
void armVMWriteAccessARMv5(armP arm, Bool isData, Uns32 newValue) {
    writeAccessARMv5(arm, isData, newValue);
}

//
// Read ARMv5 MPU Access Permissions register
//
Uns32 armVMReadAccessARMv5(armP arm, Bool isData) {
    return readAccessARMv5(arm, isData);
}

//
// Write ARMv5 MPU Extended Access Permissions register
//
void armVMWriteAccessExtARMv5(armP arm, Bool isData, Uns32 newValue) {
    writeAccessExtARMv5(arm, isData, newValue);
}

//
// Read ARMv5 MPU Extended Access Permissions register
//
Uns32 armVMReadAccessExtARMv5(armP arm, Bool isData) {
    return readAccessExtARMv5(arm, isData);
}

//
// Write the indexed protection area control register (ARMv4/ARMv5 only)
//
void armVMWriteRegionARMv5(armP arm, Uns32 index, Bool isData, Uns32 newValue) {

    if(validateRegionNum(arm, isData, index)) {

        // initialize result with base address
        union {Uns32 u32; CP_REG_DECL(DMRR0);} u     = {newValue};
        union {CP_REG_DECL(DMRR0); Uns32 u32;} uMask = {{Base:-1}};

        // extract fields from raw value
        Uns32 Base = u.u32 & uMask.u32;
        Uns32 Size = u.DMRR0.Size;
        Uns32 E    = u.DMRR0.E;

        // update the required register
        updateRegionBaseSizeE(arm, index, isData, Base, Size, E);
    }
}

//
// Read the indexed protection area control register (ARMv4/ARMv5 only)
//
Uns32 armVMReadRegionARMv5(armP arm, Uns32 index, Bool isData) {

    if(validateRegionNum(arm, isData, index)) {

        protRegionP pacReg = getPACRegs(arm, isData) + index;

        // initialize result with base address
        union {Uns32 u32; CP_REG_DECL(DMRR0);} u = {pacReg->Base};

        // include Size and Enabled fields
        u.DMRR0.Size = pacReg->Size;
        u.DMRR0.E    = pacReg->E;

        // return composed result
        return u.u32;

    } else {

        return 0;
    }
}

//
// Write the indexed MPU RegionBase register value (ARMv6 and later)
//
void armVMWriteRegionBase(armP arm, Uns32 index, Bool isData, Uns32 newValue) {

    if(validateRegionNum(arm, isData, index)) {

        protRegionP pacReg = getPACRegs(arm, isData) + index;

        // prepare Base, Size and E arguments
        Uns32 Base = newValue & CP_WRITE_MASK_DRBAR;
        Uns32 Size = pacReg->Size;
        Uns32 E    = pacReg->E;

        // update the required register
        updateRegionBaseSizeE(arm, index, isData, Base, Size, E);
    }
}

//
// Read the indexed MPU RegionBase register value (ARMv6 and later)
//
Uns32 armVMReadRegionBase(armP arm, Uns32 index, Bool isData) {

    if(validateRegionNum(arm, isData, index)) {

        protRegionP pacReg = getPACRegs(arm, isData) + index;

        // create mask to select Base field
        union {CP_REG_DECL(DRBAR); Uns32 u32;} uMask = {{Base:-1}};

        // return masked result
        return pacReg->Base & uMask.u32;

    } else {

        return 0;
    }
}

//
// Write the indexed MPU RegionSizeE register value (ARMv6 and later)
//
void armVMWriteRegionSizeE(armP arm, Uns32 index, Bool isData, Uns32 newValue) {

    if(validateRegionNum(arm, isData, index)) {

        protRegionP pacReg = getPACRegs(arm, isData) + index;

        // create union to enable field extraction
        union {Uns32 u32; CP_REG_DECL(DRSR);} u = {newValue};

        // prepare Base, Size and E arguments
        Uns32 Base = pacReg->Base;
        Uns32 Size = u.DRSR.Size;
        Uns32 E    = u.DRSR.E;

        // update the required register
        updateRegionBaseSizeE(arm, index, isData, Base, Size, E);
    }
}

//
// Read the indexed MPU RegionSizeE register value (ARMv6 and later)
//
Uns32 armVMReadRegionSizeE(armP arm, Uns32 index, Bool isData) {

    if(validateRegionNum(arm, isData, index)) {

        protRegionP pacReg = getPACRegs(arm, isData) + index;

        // initialize result
        union {Uns32 u32; CP_REG_DECL(DRSR);} u = {0};

        // extract required fields
        u.DRSR.Size = pacReg->Size;
        u.DRSR.E    = pacReg->E;

        // return composed result
        return u.u32;

    } else {

        return 0;
    }
}

//
// Write the indexed MPU RegionAccess register value (ARMv6 and later)
//
void armVMWriteRegionAccess(armP arm, Uns32 index, Bool isData, Uns32 newValue) {

    if(validateRegionNum(arm, isData, index)) {

        protRegionP pacReg = getPACRegs(arm, isData) + index;

        // create union to enable field extraction
        union {Uns32 u32; CP_REG_DECL(DRACR);} u = {newValue};

        // update fields with no simulation effect
        pacReg->B   = u.DRACR.B;
        pacReg->C   = u.DRACR.C;
        pacReg->S   = u.DRACR.S;
        pacReg->TEX = u.DRACR.TEX;

        // extract AP and XN fields
        Uns8 AP = u.DRACR.AP;
        Bool XN = u.DRACR.XN;

        // update the required register
        updateRegionAPXN(arm, index, isData, AP, XN);
    }
}

//
// Read the indexed MPU RegionAccess register value (ARMv6 and later)
//
Uns32 armVMReadRegionAccess(armP arm, Uns32 index, Bool isData) {

    if(validateRegionNum(arm, isData, index)) {

        protRegionP pacReg = getPACRegs(arm, isData) + index;

        // initialize result
        union {Uns32 u32; CP_REG_DECL(DRACR);} u = {0};

        // extract required fields
        u.DRACR.B   = pacReg->B;
        u.DRACR.C   = pacReg->C;
        u.DRACR.S   = pacReg->S;
        u.DRACR.TEX = pacReg->TEX;
        u.DRACR.AP  = pacReg->AP;
        u.DRACR.XN  = pacReg->XN;

        // return composed result
        return u.u32;

    } else {

        return 0;
    }
}

//
// Write TLBLockdown register
//
void armVMWriteLockdown(armP arm, Bool isData, Uns32 newValue) {

    armTLBP tlb = isData ? getDTLB(arm) : getITLB(arm);

    tlb->TLBLRu32 = newValue & tlb->ldMask;
}

//
// Read TLBLockdown register
//
Uns32 armVMReadLockdown(armP arm, Bool isData) {

    armTLBP tlb = isData ? getDTLB(arm) : getITLB(arm);

    return tlb->TLBLRu32;
}

//
// Read VA of the indexed TLB lockdown entry
//
Uns32 armVMReadLockdownVA(armP arm, Bool isData) {

    armTLBP   tlb   = isData ? getDTLB(arm) : getITLB(arm);
    tlbEntryP entry = getLockDownEntry(tlb, CP_REG_UNS32(arm, TLBRI));

    // initialize result
    union {Uns32 u32; CP_REG_DECL(TLBLDVA);} u = {entry->lowVA};

    // extract required fields
    u.TLBLDVA.ASID = entry->ASID;
    u.TLBLDVA.G    = entry->G;
    u.TLBLDVA.NS   = entry->NS;

    // return composed result
    return u.u32;
}

//
// Write VA of the indexed TLB lockdown entry
//
void armVMWriteLockdownVA(armP arm, Bool isData, Uns32 newValue) {

    armTLBP   tlb    = isData ? getDTLB(arm) : getITLB(arm);
    tlbEntryP entry  = getLockDownEntry(tlb, CP_REG_UNS32(arm, TLBWI));
    Uns32     sizeM1 = getTLBEntrySize(entry)-1;

    // delete the entry if it is mapped (NOTE: this does not affect entry
    // validity)
    deleteTLBEntry(arm, tlb, entry);

    // create union to enable field extraction
    union {Uns32 u32; CP_REG_DECL(TLBLDVA);} u = {newValue};

    // update fields
    entry->lowVA  = newValue & ~sizeM1;
    entry->highVA = entry->lowVA + sizeM1;
    entry->ASID   = u.TLBLDVA.ASID;
    entry->G      = u.TLBLDVA.G;
    entry->NS     = u.TLBLDVA.NS;

    // if the entry is valid, delete any overlapping entries (in case this
    // supersedes them)
    if(entry->LDV) {
        invalidateOverlappingTLBEntries(arm, tlb, entry);
    }
}

//
// Read PA of the indexed TLB lockdown entry
//
Uns32 armVMReadLockdownPA(armP arm, Bool isData) {

    armTLBP   tlb   = isData ? getDTLB(arm) : getITLB(arm);
    tlbEntryP entry = getLockDownEntry(tlb, CP_REG_UNS32(arm, TLBRI));
    Uns32     size  = getTLBEntrySize(entry);

    // initialize result
    union {Uns32 u32; CP_REG_DECL(TLBLDPA);} u = {entry->lowPA};

    // extract required fields
    u.TLBLDPA.V  = entry->LDV;
    u.TLBLDPA.AP = entry->APsp0;
    u.TLBLDPA.SZ = sizeToSZ(size);

    // return composed result
    return u.u32;
}

//
// Write PA of the indexed TLB lockdown entry
//
void armVMWriteLockdownPA(armP arm, Bool isData, Uns32 newValue) {

    armTLBP   tlb   = isData ? getDTLB(arm) : getITLB(arm);
    tlbEntryP entry = getLockDownEntry(tlb, CP_REG_UNS32(arm, TLBWI));

    // delete the entry if it is mapped (NOTE: this does not affect entry
    // validity)
    deleteTLBEntry(arm, tlb, entry);

    // create union to enable field extraction
    union {Uns32 u32; CP_REG_DECL(TLBLDPA);} u = {newValue};

    // derive entry size from TLBLDPA.SZ;
    Uns32 sizeM1 = szToSize(u.TLBLDPA.SZ)-1;

    // update fields (NOTE: virtual address range depends on size)
    entry->lowVA  = entry->lowVA & ~sizeM1;
    entry->highVA = entry->lowVA + sizeM1;
    entry->LDV    = u.TLBLDPA.V;
    entry->lowPA  = newValue & ~sizeM1;
    entry->APsp0  = u.TLBLDPA.AP;

    // if the entry is valid, delete any overlapping entries (in case this
    // supersedes them)
    if(entry->LDV) {
        invalidateOverlappingTLBEntries(arm, tlb, entry);
    }
}

//
// Read attributes of the indexed TLB lockdown entry
//
Uns32 armVMReadLockdownAttr(armP arm, Bool isData) {

    armTLBP   tlb   = isData ? getDTLB(arm) : getITLB(arm);
    tlbEntryP entry = getLockDownEntry(tlb, CP_REG_UNS32(arm, TLBRI));

    // initialize result
    union {Uns32 u32; CP_REG_DECL(TLBLDATTR);} u = {0};

    // extract required fields
    u.TLBLDATTR.S      = entry->S;
    u.TLBLDATTR.TEXCB  = entry->TEXCB;
    u.TLBLDATTR.XN     = entry->XN;
    u.TLBLDATTR.Domain = entry->domain;
    u.TLBLDATTR.NS     = entry->NS;

    // return composed result
    return u.u32;
}

//
// Write attributes of the indexed TLB lockdown entry
//
void armVMWriteLockdownAttr(armP arm, Bool isData, Uns32 newValue) {

    armTLBP   tlb   = isData ? getDTLB(arm) : getITLB(arm);
    tlbEntryP entry = getLockDownEntry(tlb, CP_REG_UNS32(arm, TLBWI));

    // delete the entry if it is mapped (NOTE: this does not affect entry
    // validity)
    deleteTLBEntry(arm, tlb, entry);

    // create union to enable field extraction
    union {Uns32 u32; CP_REG_DECL(TLBLDATTR);} u = {newValue};

    // update fields
    entry->S      = u.TLBLDATTR.S;
    entry->TEXCB  = u.TLBLDATTR.TEXCB;
    entry->XN     = u.TLBLDATTR.XN;
    entry->domain = u.TLBLDATTR.Domain;
    entry->NS     = u.TLBLDATTR.NS;
}

//
// Update ITCM region register
//
void armVMUpdateITCMRegion(armP arm, Uns32 newValue) {
    updateITCMRR(arm, newValue);
}

//
// Update DTCM region register
//
void armVMUpdateDTCMRegion(armP arm, Uns32 newValue) {
    updateDTCMRR(arm, newValue);
}

//
// Write DMAControl register value
//
void armVMWriteDMAControl(armP arm, Uns32 newValue) {
    writeDMAControl(arm, newValue);
}

//
// Read DMAControl register value
//
Uns32 armVMReadDMAControl(armP arm) {
    return readDMAControl(arm);
}

//
// Write DMAInternalStart register value
//
void armVMWriteDMAInternalStart(armP arm, Uns32 newValue) {
    writeDMAInternalStart(arm, newValue);
}

//
// Read DMAInternalStart register value
//
Uns32 armVMReadDMAInternalStart(armP arm) {
    return readDMAInternalStart(arm);
}

//
// Write DMAExternalStart register value
//
void armVMWriteDMAExternalStart(armP arm, Uns32 newValue) {
    writeDMAExternalStart(arm, newValue);
}

//
// Read DMAExternalStart register value
//
Uns32 armVMReadDMAExternalStart(armP arm) {
    return readDMAExternalStart(arm);
}

//
// Write DMAInternalEnd register value
//
void armVMWriteDMAInternalEnd(armP arm, Uns32 newValue) {
    writeDMAInternalEnd(arm, newValue);
}

//
// Read DMAInternalEnd register value
//
Uns32 armVMReadDMAInternalEnd(armP arm) {
    return readDMAInternalEnd(arm);
}

//
// Read DMAStatus register value
//
Uns32 armVMReadDMAStatus(armP arm) {
    return readDMAStatus(arm);
}

//
// Write DMAContextID register value
//
void armVMWriteDMAContextID(armP arm, Uns32 newValue) {
    writeDMAContextID(arm, newValue);
}

//
// Read DMAContextID register value
//
Uns32 armVMReadDMAContextID(armP arm) {
    return readDMAContextID(arm);
}

//
// Write DMAStop register value
//
void armVMWriteDMAStop(armP arm) {
    writeDMAStop(arm);
}

//
// Write DMAStart register
//
void armVMWriteDMAStart(armP arm) {
    writeDMAStart(arm);
}

//
// Write DMAClear register
//
void armVMWriteDMAClear(armP arm) {
    writeDMAClear(arm);
}

//
// Dump current TLB contents
//
void armVMDumpTLB(armP arm) {

    dumpTLB(arm, getITLB(arm));

    if(!TLB_UNIFIED(arm)) {
        dumpTLB(arm, getDTLB(arm));
    }
}

//
// Virtual memory constructor
//
VMI_VMINIT_FN(armVMInit) {

    armP       arm           = (armP)processor;
    Uns32      bits          = ARM_GPR_BITS;
    memDomainP extCodeDomain = codeDomains[0];
    memDomainP extDataDomain = dataDomains[0];
    memDomainP postTCMCodeDomain;
    memDomainP postTCMDataDomain;
    memDomainP postFCSECodeDomain;
    memDomainP postFCSEDataDomain;
    memDomainP codeDomain32;
    memDomainP dataDomain32;

    // if multiprocessing extensions are implemented, create alias of physical
    // memory domain with private memory region instantiated
    extDataDomain = armMPVMInit(arm, extDataDomain);

    // create post-TCM physical code domain if required
    if(CP_FIELD(arm, TCMTR, ITCM) && CP_FIELD(arm, ITCMRR, Size)) {
        newITCM(arm);
        postTCMCodeDomain = makeDomainAlias(extCodeDomain, "code post-TCM");
    } else {
        postTCMCodeDomain = extCodeDomain;
    }

    // create post-TCM physical data domain if required
    if(CP_FIELD(arm, TCMTR, DTCM) && CP_FIELD(arm, DTCMRR, Size)) {
        newDTCM(arm);
        postTCMDataDomain = makeDomainAlias(extDataDomain, "data post-TCM");
    } else {
        postTCMDataDomain = extDataDomain;
    }

    // create post-FCSE physical domains
    postFCSECodeDomain = makeDomainAlias(postTCMCodeDomain, "code post-FCSE");
    postFCSEDataDomain = makeDomainAlias(postTCMDataDomain, "data post-FCSE");

    // save physical memDomains on processor structure
    arm->ids.external = extCodeDomain;
    arm->ids.postTCM  = postTCMCodeDomain;
    arm->ids.postFCSE = postFCSECodeDomain;
    arm->dds.external = extDataDomain;
    arm->dds.postTCM  = postTCMDataDomain;
    arm->dds.postFCSE = postFCSEDataDomain;

    // create 32-bit alias of postFCSECodeDomain if required
    if(vmirtGetDomainAddressBits(postFCSECodeDomain)>bits) {
        codeDomain32 = makeDomainAliasBits(
            postFCSECodeDomain, "32-bit code", bits
        );
    } else {
        codeDomain32 = postFCSECodeDomain;
    }

    // create 32-bit alias of postFCSEDataDomain if required
    if(vmirtGetDomainAddressBits(postFCSEDataDomain)>bits) {
        dataDomain32 = makeDomainAliasBits(
            postFCSEDataDomain, "32-bit data", bits
        );
    } else {
        dataDomain32 = postFCSEDataDomain;
    }

    // set 32-bit physical code memDomains for each mode
    codeDomains[ARM_MODE_PRIV] = codeDomain32;
    codeDomains[ARM_MODE_USER] = codeDomain32;

    // set 32-bit physical data memDomains for each mode
    dataDomains[ARM_MODE_PRIV] = dataDomain32;
    dataDomains[ARM_MODE_USER] = dataDomain32;

    // initialize MMU/MPU data structures if required
    if(MMU_MPU_PRESENT(arm)) {

        // create MMU/MPU-managed memDomains
        arm->ids.vmPriv = vmirtNewDomain("priv MMU/MPU Code", bits);
        arm->ids.vmUser = vmirtNewDomain("user MMU/MPU Code", bits);
        arm->dds.vmPriv = vmirtNewDomain("priv MMU/MPU Data", bits);
        arm->dds.vmUser = vmirtNewDomain("user MMU/MPU Data", bits);

        // set MMU/MPU code memDomains for each mode
        codeDomains[ARM_MODE_PRIV_MMU_MPU] = arm->ids.vmPriv;
        codeDomains[ARM_MODE_USER_MMU_MPU] = arm->ids.vmUser;

        // set MMU/MPU data memDomains for each mode
        dataDomains[ARM_MODE_PRIV_MMU_MPU] = arm->dds.vmPriv;
        dataDomains[ARM_MODE_USER_MMU_MPU] = arm->dds.vmUser;

        // initialize TLBs if required
        if(MMU_PRESENT(arm)) {
            newTLBs(arm);
        } else if(MPU_PRESENT(arm)) {
            newMPUs(arm);
        }
    }

    // allocate DMA units if required
    newDMAUnits(arm);
}

//
// Reset VM structures
//
void armVMReset(armP arm) {
    if(MPU_PRESENT(arm)) {
        resetMPUs(arm);
    }
}

//
// Free structures used for virtual memory management
//
void armVMFree(armP arm) {

    if(MMU_PRESENT(arm)) {
        freeTLBs(arm);
    } else if(MPU_PRESENT(arm)) {
        freeMPUs(arm);
    }

    // free DMA unit structures
    freeDMAUnits(arm);
}


