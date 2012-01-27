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

// Imperas header files
#include "hostapi/impAlloc.h"

// VMI header files
#include "vmi/vmiAttrs.h"
#include "vmi/vmiParameters.h"
#include "vmi/vmiMessage.h"

#include "armFunctions.h"
#include "armConfig.h"
#include "armParameters.h"
#include "armStructure.h"
#include "armVariant.h"

static vmiEnumParameter compatTable[] = {
    {"ISA",       COMPAT_ISA},
    {"gdb",       COMPAT_GDB},
    {"nopSVC",    COMPAT_CODE_SOURCERY},
    { 0, 0 }
};

//
// Table of parameter specs
//
static vmiParameter formals[] = {

    VMI_ENUM_PARAM_SPEC(  armParamValues, variant,          NULL, "Selects variant (either a generic ISA or a specific model)"),
    VMI_ENDIAN_PARAM_SPEC(armParamValues, endian,           "Model endian"),
    VMI_ENUM_PARAM_SPEC(  armParamValues, compatibility,    compatTable, "Specify compatibility mode"),

    VMI_BOOL_PARAM_SPEC(  armParamValues, verbose,          1, "Specify verbosity of output" ),
    VMI_BOOL_PARAM_SPEC(  armParamValues, showHiddenRegs,   0, "Show hidden registers during register tracing" ),
    VMI_BOOL_PARAM_SPEC(  armParamValues, UAL,              1, "Disassemble using UAL syntax" ),
    VMI_BOOL_PARAM_SPEC(  armParamValues, enableVFPAtReset, 0, "Enable vector floating point (SIMD and VFP) instructions at reset. (Enables cp10/11 in CPACR and sets FPEXC.EN)" ),

    VMI_UNS32_PARAM_SPEC( armParamValues, override_MainId                , 0, 0, VMI_MAXU32, "Coprocessor 15 MainId register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_debugMask             , 0, 0, VMI_MAXU32, "Specifies debug mask, enabling debug output for model components"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_CacheType             , 0, 0, VMI_MAXU32, "Override coprocessor 15 CacheType register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_TLBType               , 0, 0, VMI_MAXU32, "Override coprocessor 15 TLBType register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_MPUType               , 0, 0, VMI_MAXU32, "Override coprocessor 15 MPUType register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_InstructionAttributes0, 0, 0, VMI_MAXU32, "Override coprocessor 15 InstructionAttributes0 register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_InstructionAttributes1, 0, 0, VMI_MAXU32, "Override coprocessor 15 InstructionAttributes1 register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_InstructionAttributes2, 0, 0, VMI_MAXU32, "Override coprocessor 15 InstructionAttributes2 register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_InstructionAttributes3, 0, 0, VMI_MAXU32, "Override coprocessor 15 InstructionAttributes3 register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_InstructionAttributes4, 0, 0, VMI_MAXU32, "Override coprocessor 15 InstructionAttributes4 register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_InstructionAttributes5, 0, 0, VMI_MAXU32, "Override coprocessor 15 InstructionAttributes5 register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_CLIDR                 , 0, 0, VMI_MAXU32, "Override coprocessor 15 CLIDR register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_AIDR                  , 0, 0, VMI_MAXU32, "Override coprocessor 15 AIDR register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_CBAR                  , 0, 0, VMI_MAXU32, "Override coprocessor 15 CBAR register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_FPSID                 , 0, 0, VMI_MAXU32, "Override SIMD/VFP FPSID register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_MVFR0                 , 0, 0, VMI_MAXU32, "Override SIMD/VFP MVFR0 register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_MVFR1                 , 0, 0, VMI_MAXU32, "Override SIMD/VFP MVFR1 register"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_Control_V             , 0, 0, VMI_MAXU32, "Override Control/V bit with the passed value (enables high vectors)"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_rotateUnaligned       , 0, 0, 1,          "Specifies that data from unaligned loads by LDR, LDRT or SWP should be rotated (if 1)"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_align64as32           , 0, 0, 1,          "Specifies that 64:bit loads and stores are aligned to 32:bit boundaries (if 1)"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_STRoffsetPC12         , 0, 0, 1,          "Specifies that STR/STR of PC should do so with 12:byte offset from the current instruction (if 1), otherwise an 8:byte offset is used"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_fcsePresent           , 0, 0, 1,          "Specifies that FCSE is present (if 1)"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_mpuV5ExtAP            , 0, 0, 1,          "Specifies that ARMv5 MPU extended access permissions implemented (cp15/5/2 and cp15/5/2)"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_fcseRequiresMMU       , 0, 0, 1,          "Specifies that FCSE is active only when MMU is enabled (if 1)"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_ERG                   , 3, 3, 11,         "Specifies exclusive reservation granule"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_ignoreBadCp15         , 0, 0, 1,          "Specifies whether invalid coprocessor 15 access should be ignored (if 1) or cause Invalid Instruction exceptions (if 0)"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_fpexcDexPresent       , 0, 0, 1,          "Specifies whether the FPEXC.DEX register field is implemented (if 1)"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_advSIMDPresent        , 0, 0, 1,          "Specifies whether Advanced SIMD extensions are present  (1=present, 0=not present)"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_vfpPresent            , 0, 0, 1,          "Specifies whether VFP extensions are present  (1=present, 0=not present)"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_numCPUs               , 0, 0, 255,        "Specify the number of cores in a multiprocessor"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_ICCPMRBits            , 0, 0, 8,          "Specify the number of writable bits in ICCPMR"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_minICCBPR             , 0, 0, 7,          "Specify the minimum possible value for ICCBPR"),
    VMI_UNS32_PARAM_SPEC( armParamValues, override_ICCIDR                , 0, 0, VMI_MAXU32, "Override GIC Id register ICCIDR"),
    VMI_BOOL_PARAM_SPEC ( armParamValues, override_SGIDisable            , 0,                "Override whether SGI disable possible using ICDICER0" ),

    VMI_END_PARAM
};

static Uns32 countVariants(void) {
    armConfigCP cfg = armConfigTable;
    Uns32         i   = 0;
    while(cfg->name) {
        cfg++;
        i++;
    }
    return i;
}

//
// First time through, malloc and fill the variant list from the config table
//
static vmiEnumParameterP getVariantList() {
    static vmiEnumParameterP list = NULL;
    if (!list) {
        Uns32 v = 1 + countVariants();
        list = STYPE_CALLOC_N(vmiEnumParameter, v);
        vmiEnumParameterP prm;
        armConfigCP    cfg;
        Uns32 i;
        for (i = 0, cfg = armConfigTable, prm = list;
             cfg->name;
             i++, cfg++, prm++) {
            prm->name = cfg->name;
            prm->value = i;
        }

    }
    return list;
}

//
// First time through, fill the formals table
//
static vmiParameterP getFormals(void) {
    static Bool first = True;
    if(first) {
        first = False;
        formals[0].u.enumParam.legalValues = getVariantList();
    }
    return formals;
}


//
// Function to iterate the parameter specs
//
VMI_PROC_PARAM_SPECS_FN(armGetParamSpec) {
    if(!prev) {
        return getFormals();
    } else {
        prev++;
        if (prev->name)
            return prev;
        else
            return 0;
    }
}

//
// Get the size of the parameter values table
//
VMI_PROC_PARAM_TABLE_SIZE_FN(armParamValueSize) {
    return sizeof(armParamValues);
}


