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
#include <string.h>
#include <stdio.h>

// VMI header files
#include "vmi/vmiAttrs.h"
#include "vmi/vmiCommand.h"
#include "vmi/vmiMessage.h"
#include "vmi/vmiRt.h"

// model header files
#include "armConfig.h"
#include "armCP.h"
#include "armCPRegisters.h"
#include "armDoc.h"
#include "armDecode.h"
#include "armDebug.h"
#include "armExceptions.h"
#include "armFunctions.h"
#include "armMode.h"
#include "armMPCore.h"
#include "armMPCoreRegisters.h"
#include "armStructure.h"
#include "armSIMDVFP.h"
#include "armUtils.h"
#include "armVM.h"


//
// Prefix for messages from this module
//
#define CPU_PREFIX "ARM_MAIN"


////////////////////////////////////////////////////////////////////////////////
// PARSING CONFIGURATION OPTIONS
////////////////////////////////////////////////////////////////////////////////

//
// Return processor configuration variant argument
//
static armConfigCP getConfigVariantArg(armP arm, armParamValuesP params) {

    armConfigCP match;

    if(params->SETBIT(variant)) {

        match = &armConfigTable[params->variant];

    } else {

        match = armConfigTable;

        if(arm->verbose) {

            // report the value specified for an option in verbose mode
            vmiMessage("I", CPU_PREFIX"_ANS1",
                "Attribute '%s' not specified; defaulting to '%s'",
                "variant",
                match->name
            );
        }
    }

    // return matching configuration
    return match;
}


////////////////////////////////////////////////////////////////////////////////
// USER COMMANDS
////////////////////////////////////////////////////////////////////////////////

//
// Dump the contents of the TLBs
//
static VMIRT_COMMAND_FN(dumpTLBCommand) {

	armP        arm     = (armP) processor;
	const char *cmdName = argc > 0 ? argv[0] : "dumpTLB";

    if ((argc != 1) || (argc > 1 && strcmp(argv[1], "-help") == 0)) {
        vmiPrintf("%s: Shows the TLB contents (no arguments accepted).\n", cmdName);
        return "";
    } else if (!MMU_PRESENT(arm)){
        vmiPrintf("%s: No TLB present on processor '%s'\n", cmdName, vmirtProcessorName(processor));
        return "";
    } else {
        armVMDumpTLB(arm);
    }
    return "1";
}


////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTOR AND DESTRUCTOR
////////////////////////////////////////////////////////////////////////////////

// specify default value for entire coprocessor register
#define SET_CP_REG_DEFAULT(_R, _V) \
    union {Uns32 u32; CP_REG_DECL(_R);} _U = {_V};      \
    CP_REG_STRUCT_DEFAULT(arm, _R) = _U._R

// specify default value for entire SIMD/VFP register
#define SET_SDFP_REG_DEFAULT(_R, _V) \
    union {Uns32 u32; SDFP_REG_DECL(_R);} _U = {_V};    \
    SDFP_REG_STRUCT_DEFAULT(arm, _R) = _U._R

// specify default value for entire MPCore register
#define SET_MP_REG_DEFAULT(_R, _V) \
    union {Uns32 u32; MP_REG_DECL(_R);} _U = {_V};      \
    MP_REG_STRUCT_DEFAULT(arm, _R) = _U._R

//
// Is the processor a leaf processor?
//
inline static Bool isLeaf(armP arm) {
    return !vmirtGetSMPChild((vmiProcessorP)arm);
}

//
// Return any parent of the passed processor
//
inline static armP getParent(armP arm) {
    return (armP)vmirtGetSMPParent((vmiProcessorP)arm);
}

//
// Initialize processor exclusiveTagMask
//
static void setExclusiveTagMask(armP arm) {

    Uns32 ERG;

    // get ERG
    if(CP_FIELD(arm, CTR.postV7, format)==CT_POST_V7) {
        ERG = CP_FIELD(arm, CTR.postV7, ERG);
    } else {
        ERG = arm->configInfo.ERGpreV7;
    }

    // set exclusiveTagMask from ERG
    arm->exclusiveTagMask = (-1 << (ERG+2));
}

//
// Give each sub-processor a unique name
//
static void setName(armP arm, armP parent) {

    const char   *baseName = vmirtProcessorName((vmiProcessorP)parent);
    vmiProcessorP proc     = (vmiProcessorP)arm;
    const char   *type     = armProcessorDescription(proc);
    Uns32         index    = vmirtGetSMPIndex(proc);
    char          tmp[strlen(baseName)+strlen(type)+10];

    sprintf(tmp, "%s_%s%u", baseName, type, index);
    vmirtSetProcessorName(proc, tmp);
}

//
// ARM processor constructor
//
VMI_CONSTRUCTOR_FN(armConstructor) {

    armP            arm    = (armP)processor;
    armParamValuesP params = parameterValues;

    // save the parent if this is a cluster member
    armP parent = arm->parent = getParent(arm);

    // set callbacks (for intercept libraries)
    arm->decoderCB = armDecode;
    arm->isizeCB   = armGetInstructionSizeMode;

    if(parent) {

        // copy configuration from the parent
        arm->flags          = parent->flags;
        arm->simEx          = parent->simEx;
        arm->verbose        = parent->verbose;
        arm->compatMode     = parent->compatMode;
        arm->showHiddenRegs = parent->showHiddenRegs;
        arm->UAL            = parent->UAL;
        arm->configInfo     = parent->configInfo;

        // set the name
        setName(arm, parent);

    } else {

        // save flags on processor structure
        arm->flags = vmirtProcessorFlags(processor);

        // get string configuration options
        arm->simEx          = simulateExceptions;
        arm->verbose        = params->verbose;
        arm->compatMode     = params->compatibility;
        arm->showHiddenRegs = params->showHiddenRegs;
        arm->UAL            = params->UAL;

        // get default variant information
        arm->configInfo = *getConfigVariantArg(arm, params);

        // override other configuration values
        if(params->SETBIT(override_debugMask)) {
            arm->flags = params->override_debugMask;
        }
        if(params->SETBIT(override_MainId)) {
            SET_CP_REG_DEFAULT(MIDR, params->override_MainId);
        }
        if(params->SETBIT(override_CacheType)) {
            SET_CP_REG_DEFAULT(CTR, params->override_CacheType);
        }
        if(params->SETBIT(override_TLBType)) {
            SET_CP_REG_DEFAULT(TLBTR, params->override_TLBType);
        }
        if(params->SETBIT(override_MPUType)) {
            SET_CP_REG_DEFAULT(MPUIR, params->override_MPUType);
        }
        if(params->SETBIT(override_InstructionAttributes0)) {
            SET_CP_REG_DEFAULT(ID_ISAR0, params->override_InstructionAttributes0);
        }
        if(params->SETBIT(override_InstructionAttributes1)) {
            SET_CP_REG_DEFAULT(ID_ISAR1, params->override_InstructionAttributes1);
        }
        if(params->SETBIT(override_InstructionAttributes2)) {
            SET_CP_REG_DEFAULT(ID_ISAR2, params->override_InstructionAttributes2);
        }
        if(params->SETBIT(override_InstructionAttributes3)) {
            SET_CP_REG_DEFAULT(ID_ISAR3, params->override_InstructionAttributes3);
        }
        if(params->SETBIT(override_InstructionAttributes4)) {
            SET_CP_REG_DEFAULT(ID_ISAR4, params->override_InstructionAttributes4);
        }
        if(params->SETBIT(override_InstructionAttributes5)) {
            SET_CP_REG_DEFAULT(ID_ISAR5, params->override_InstructionAttributes5);
        }
        if(params->SETBIT(override_CLIDR)) {
            SET_CP_REG_DEFAULT(CLIDR, params->override_CLIDR);
        }
        if(params->SETBIT(override_AIDR)) {
            SET_CP_REG_DEFAULT(AIDR, params->override_AIDR);
        }
        if(params->SETBIT(override_CBAR)) {
            SET_CP_REG_DEFAULT(CBAR, params->override_CBAR);
        }
        if(params->SETBIT(override_FPSID)) {
            SET_SDFP_REG_DEFAULT(FPSID, params->override_FPSID);
        }
        if(params->SETBIT(override_MVFR0)) {
            SET_SDFP_REG_DEFAULT(MVFR0, params->override_MVFR0);
        }
        if(params->SETBIT(override_MVFR1)) {
            SET_SDFP_REG_DEFAULT(MVFR1, params->override_MVFR1);
        }
        if(params->SETBIT(override_Control_V)) {
            CP_FIELD_DEFAULT(arm, SCTLR, V) = params->override_Control_V;
        }
        if(params->SETBIT(override_rotateUnaligned)) {
            arm->configInfo.rotateUnaligned = params->override_rotateUnaligned;
        }
        if(params->SETBIT(override_align64as32)) {
            arm->configInfo.align64as32 = params->override_align64as32;
        }
        if(params->SETBIT(override_STRoffsetPC12)) {
            arm->configInfo.STRoffsetPC12 = params->override_STRoffsetPC12;
        }
        if(params->SETBIT(override_fcsePresent)) {
            arm->configInfo.fcsePresent = params->override_fcsePresent;
        }
        if(params->SETBIT(override_mpuV5ExtAP)) {
            arm->configInfo.mpuV5ExtAP = params->override_mpuV5ExtAP;
        }
        if(params->SETBIT(override_fcseRequiresMMU)) {
            arm->configInfo.fcseRequiresMMU = params->override_fcseRequiresMMU;
        }
        if(params->SETBIT(override_ERG)) {
            if(CP_FIELD_DEFAULT(arm, CTR.postV7, format)==CT_POST_V7) {
                CP_FIELD_DEFAULT(arm, CTR.postV7, ERG) = params->override_ERG;
            } else {
                arm->configInfo.ERGpreV7 = params->override_ERG;
            }
        }
        if(params->SETBIT(override_ignoreBadCp15)) {
            arm->configInfo.ignoreBadCp15 = params->override_ignoreBadCp15;
        }
        if(params->SETBIT(override_numCPUs)) {
            if(arm->configInfo.numCPUs) {
                arm->configInfo.numCPUs = params->override_numCPUs;
            } else {
                vmiMessage("W", CPU_PREFIX"_NCI",
                    "Number of CPUs for variant %s is not configurable",
                    arm->configInfo.name
                );
            }
        }
        if(params->SETBIT(override_fpexcDexPresent)) {
            arm->configInfo.fpexcDexPresent = params->override_fpexcDexPresent;
        }
        if(params->SETBIT(override_advSIMDPresent)) {
            arm->configInfo.advSIMDPresent = params->override_advSIMDPresent;
        }
        if(params->SETBIT(override_vfpPresent)) {
            arm->configInfo.vfpPresent = params->override_vfpPresent;
        }
        if(params->SETBIT(override_ICCPMRBits)) {
            arm->configInfo.ICCPMRBits = params->override_ICCPMRBits;
        }
        if(params->SETBIT(override_minICCBPR)) {
            arm->configInfo.minICCBPR = params->override_minICCBPR;
        }
        if(params->SETBIT(override_ICCIDR)) {
            SET_MP_REG_DEFAULT(ICCIDR, params->override_ICCIDR);
        }
        if(params->SETBIT(override_SGIDisable)) {
            arm->configInfo.SGIDisable = params->override_SGIDisable;
        }

        // Enable SIMD/VFP instructions at reset?
        if(params->enableVFPAtReset) {
            SDFP_FIELD_DEFAULT(arm, FPEXC, EN) = 1;
            CP_FIELD_DEFAULT(arm, CPACR, cp10) = 3;
            CP_FIELD_DEFAULT(arm, CPACR, cp11) = 3;
        }

        // install documentation
        armDoc(processor, parameterValues);
    }

    // is this a multicore processor container?
    if(!parent && arm->configInfo.numCPUs) {

        // allocate MPCore global structures
        armMPAllocGlobal(arm);

        // supply SMP configuration properties
        smpContext->isContainer = True;
        smpContext->numChildren = arm->configInfo.numCPUs;

        vmirtConceal(processor);

    } else {

        // override endianness
        armSetInitialEndian(arm, params->endian);

        // set read-only registers to their initial state (AFTER applying
        // overrides)
        armCpInitialize(arm, smpContext->index);
        armFPInitialize(arm);

        // allocate MPCore global structures
        armMPAllocLocal(arm);

        // connect CPU-level nets
        armConnectCPUNets(arm);

        // register view objects and event handlers
        vmiViewObjectP viewObject = vmirtGetProcessorViewObject(processor);
        armAddCpRegistersView(arm, viewObject);
        arm->mmuEnableEvent  = vmirtAddViewEvent(viewObject, "mmuEnable", 0);

        // add debug commands to the command interpreter
        vmirtAddCommand(processor, "dumpTLB", NULL, dumpTLBCommand);

        // initialize exclusiveTagMask
        setExclusiveTagMask(arm);

        // set up read and write masks for PSRs based on configuration options
        armSetPSRMasks(arm);

        // reset the processor - interrupt masks are 0 if in gdb compatibility
        // mode, otherwise 1
        armReset(arm, arm->compatMode!=COMPAT_GDB);

        // reset stack pointer (and banked variants) in gdb compatibility mode
        if(arm->compatMode==COMPAT_GDB) {
            arm->regs[ARM_REG_SP] = 0x800;
            arm->bank.R13_svc     = 0x800;
            arm->bank.R13_abt     = 0x800;
            arm->bank.R13_und     = 0x800;
        }

        // force into Thumb/ThumbEE mode if required
        if(ARM_THUMB(arm))  {arm->blockMask |= ARM_BM_THUMB; }
        if(ARM_JAZ_EE(arm)) {arm->blockMask |= ARM_BM_JAZ_EE;}
    }
}

//
// Processor destructor callback
//
static VMI_SMP_ITER_FN(destructorCB) {

    armP arm = (armP)processor;

    if(isLeaf(arm)) {
        armCpResetWarnings(arm);
        armVMFree(arm);
    }

    // free local MPCCore structures
    armMPFreeLocal(arm);
}

//
// ARM processor destructor
//
VMI_DESTRUCTOR_FN(armDestructor) {

    armP arm = (armP)processor;

    // apply destructor to root processor
    destructorCB(processor, 0);

    // apply destructor to all descendants
    vmirtIterAllDescendants(processor, destructorCB, 0);

    // free global MPCCore structures
    armMPFreeGlobal(arm);

    // free port specifications
    armFreePortSpecs(arm);
}

