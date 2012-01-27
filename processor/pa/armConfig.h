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

#ifndef ARM_CONFIG_H
#define ARM_CONFIG_H

// basic number types
#include "hostapi/impTypes.h"

// model header files
#include "armCPRegisters.h"
#include "armMPCoreRegisters.h"
#include "armSIMDVFPRegisters.h"
#include "armTypeRefs.h"
#include "armVariant.h"

//
// Use this to define a coprocessor write mask entry in the structure below
//
#define CP_MASK_DECL(_N) union {      \
    Uns32                  value32;   \
    CP_REG_STRUCT_DECL(_N) fields;    \
} _N

//
// Use this to define a SIMD/VFP write mask entry in the structure below
//
#define SDFP_MASK_DECL(_N) union {      \
    Uns32                    value32;   \
    SDFP_REG_STRUCT_DECL(_N) fields;    \
} _N

//
// This structure hold configuration information about an ARM variant
//
typedef struct armConfigS {

    // name of configuration
    const char *name;

    // configuration not held in coprocessor registers
    armArchitecture arch           :16; // specific ISA supported
    Uns32           ERGpreV7       : 4; // reservation granule (pre ARMv7)
    Uns32           ICCPMRBits     : 4; // writable bits in ICCPMR (0-8)
    Uns32           minICCBPR      : 4; // minimum value of ICCBPR
    Bool            fcsePresent    : 1; // is FCSE present?
    Bool            fcseRequiresMMU: 1; // does FCSE require enabled MMU/MPU?
    Bool            rotateUnaligned: 1; // rotate unaligned LDR/LDRT/SWP?
    Bool            align64as32    : 1; // align 64-bit load/store on 32-bit
    Bool            STRoffsetPC12  : 1; // STR/STM store PC with offset 12?
    Bool            ignoreBadCp15  : 1; // ignore invalid Cp15 accesses?
    Bool            mpuV5ExtAP     : 1; // ARMv5 extended access permissions?
    Bool            IFARPresent    : 1; // is IFAR register present?
    Bool            PCRPresent     : 1; // is PCR register present?
    Bool            NEONBPresent   : 1; // is NEONB register present?
    Bool            CBARPresent    : 1; // is CBAR register present?
    Bool            TLBLDPresent   : 1; // are Cp15/CRn15 TLBLD registers present?
    Bool            SGIDisable     : 1; // is SGI disable possible using ICDICER0?
    Bool            TLBHRPresent   : 1; // is TLBHR (TLB Hitmap) register present?
    Bool            fpexcDexPresent: 1; // is FPEXC.DEX register field present?
    Bool            advSIMDPresent : 1; // are Advanced SIMD (NEON) exetensions  present?
    Bool            vfpPresent     : 1; // are VFP exetensions  present?
    Uns32           numCPUs;            // number of MP CPUs

    // default values for coprocessor registers
    struct {
        CP_REG_DECL(MIDR);
        CP_REG_DECL(CTR);
        CP_REG_DECL(TCMTR);
        CP_REG_DECL(TLBTR);
        CP_REG_DECL(MPUIR);
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
        CP_REG_DECL(CLIDR);
        CP_REG_DECL(AIDR);
        CP_REG_DECL(CCSIDR)[2][ARM_NUM_CCSIDR];
        CP_REG_DECL(SCTLR);
        CP_REG_DECL(ACTLR);
        CP_REG_DECL(DTCMRR);
        CP_REG_DECL(ITCMRR);
        CP_REG_DECL(DMAPresent);
        CP_REG_DECL(PCR);
        CP_REG_DECL(CBAR);
        CP_REG_DECL(CPACR);
    } cpRegDefaults;

    // write masks for coprocessor registers
    struct {
        CP_MASK_DECL(SCTLR);
        CP_MASK_DECL(CPACR);
    } cpRegMasks;

    // default values for SIMD/VFP registers
    struct {
        SDFP_REG_DECL(FPSID);
        SDFP_REG_DECL(MVFR0);
        SDFP_REG_DECL(MVFR1);
        SDFP_REG_DECL(FPEXC);
    } sdfpRegDefaults;

    // write masks for SIMD/VFP registers
    struct {
        SDFP_MASK_DECL(FPSCR);
        SDFP_MASK_DECL(FPEXC);
    } sdfpRegMasks;

    // default values for MPCore registers
    struct {
        MP_REG_DECL(SCUConfiguration);
        MP_REG_DECL(ICDICTR);
        MP_REG_DECL(ICDIIDR);
        MP_UNS32_DECL(PERIPH_ID)[8];
        MP_UNS32_DECL(COMPONENT_ID)[4];
        MP_REG_DECL(ICCIDR);
        MP_UNS32_DECL(PERIPH_CLK);
    } mpRegDefaults;

} armConfig;

DEFINE_CS(armConfig);

//
// This specifies configuration information for each supported variant
//
extern const struct armConfigS armConfigTable[];

//
// Predicates for system features
//

// is a generic feature present (not necessarily currently enabled)?
#define CP_FEATURE_PRESENT(_A, _R, _F) \
    (CP_FIELD(_A, _R, _F) || CP_MASK_FIELD(_A, _R, _F))

// is MMU/MPU enabled?
#define MMU_MPU_ENABLED(_A) CP_FIELD(_A, SCTLR, M)

// are TLB/MPU unified?
#define TLB_UNIFIED(_A)     (!CP_FIELD(_A, TLBTR, nU))
#define MPU_UNIFIED(_A)     (!CP_FIELD(_A, MPUIR, nU))

// get TLB lockdown region counts
#define TLDD_SIZE(_A)       CP_FIELD(_A, TLBTR, DLsize)
#define TLDI_SIZE(_A)       CP_FIELD(_A, TLBTR, ILsize)

// is MMU/MPU present?
#define MMU_MPU_PRESENT(_A) CP_FEATURE_PRESENT(_A, SCTLR, M)
#define SELECT_MPU(_A)      CP_FIELD(_A, MPUIR, DRegion)
#define MMU_PRESENT(_A)     (MMU_MPU_PRESENT(_A) && !SELECT_MPU(_A))
#define MMUS_PRESENT(_A)    (MMU_PRESENT(_A) && !TLB_UNIFIED(_A))
#define MPU_PRESENT(_A)     (MMU_MPU_PRESENT(_A) &&  SELECT_MPU(_A))
#define MPUS_PRESENT(_A)    (MPU_PRESENT(_A) && !MPU_UNIFIED(_A))

// is alignment checking enabled/present?
#define ALIGN_ENABLED(_A)   CP_FIELD(_A, SCTLR, A)
#define ALIGN_PRESENT(_A)   CP_FEATURE_PRESENT(_A, SCTLR, A)

// is unaligned access enabled?
#define DO_UNALIGNED(_A)    (!ALIGN_ENABLED(_A) && (CP_FIELD(_A, SCTLR, U)))

// are ARMv5 MPU extended access permission registers present?
#define EXT_AP_PRESENT(_A)  ((_A)->configInfo.mpuV5ExtAP)

// is FCSE present?
#define FCSE_PRESENT(_A)    ((_A)->configInfo.fcsePresent)

// is Jazelle present?
#define JAZELLE_PRESENT(_A) ARM_SUPPORT((_A)->configInfo.arch, ARM_J)

// are DTCM/ITCM present?
#define DTCM_PRESENT(_A)    CP_FIELD(_A, TCMTR, DTCM)
#define ITCM_PRESENT(_A)    CP_FIELD(_A, TCMTR, ITCM)

// are miscellaneous registers present?
#define IFAR_PRESENT(_A)      ((_A)->configInfo.IFARPresent)
#define PCR_PRESENT(_A)       ((_A)->configInfo.PCRPresent)
#define NEONB_PRESENT(_A)     ((_A)->configInfo.NEONBPresent)
#define CBAR_PRESENT(_A)      ((_A)->configInfo.CBARPresent)
#define TLBLD_PRESENT(_A)     ((_A)->configInfo.TLBLDPresent)
#define TLBHR_PRESENT(_A)     ((_A)->configInfo.TLBHRPresent)
#define FPEXC_DEX_PRESENT(_A) ((_A)->configInfo.fpexcDexPresent)

// is DMA present?
#define DMA_PRESENT(_A)     CP_FIELD(_A, DMAPresent, CH0)

// is NEON present?
#define ADVSIMD_PRESENT(_A)    ((_A)->configInfo.advSIMDPresent)

// is VFP present?
#define VFP_PRESENT(_A)    ((_A)->configInfo.vfpPresent)

#endif

