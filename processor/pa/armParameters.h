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

#ifndef ARM_PARAMETERS_H
#define ARM_PARAMETERS_H

// VMI header files
#include "vmi/vmiAttrs.h"
#include "vmi/vmiParameters.h"

#include "armTypeRefs.h"

//
// Define the attributes value structure
//
typedef struct armParamValuesS {
    VMI_BOOL_PARAM(verbose);
    VMI_BOOL_PARAM(showHiddenRegs);
    VMI_BOOL_PARAM(UAL);
    VMI_BOOL_PARAM(enableVFPAtReset);
    VMI_ENDIAN_PARAM(endian);
    VMI_ENUM_PARAM(variant);
    VMI_ENUM_PARAM(compatibility);
    VMI_UNS32_PARAM(override_MainId);
    VMI_UNS32_PARAM(override_debugMask);
    VMI_UNS32_PARAM(override_CacheType);
    VMI_UNS32_PARAM(override_TLBType);
    VMI_UNS32_PARAM(override_MPUType);
    VMI_UNS32_PARAM(override_InstructionAttributes0);
    VMI_UNS32_PARAM(override_InstructionAttributes1);
    VMI_UNS32_PARAM(override_InstructionAttributes2);
    VMI_UNS32_PARAM(override_InstructionAttributes3);
    VMI_UNS32_PARAM(override_InstructionAttributes4);
    VMI_UNS32_PARAM(override_InstructionAttributes5);
    VMI_UNS32_PARAM(override_CLIDR);
    VMI_UNS32_PARAM(override_AIDR);
    VMI_UNS32_PARAM(override_CBAR);
    VMI_UNS32_PARAM(override_FPSID);
    VMI_UNS32_PARAM(override_MVFR0);
    VMI_UNS32_PARAM(override_MVFR1);
    VMI_UNS32_PARAM(override_Control_V);
    VMI_UNS32_PARAM(override_rotateUnaligned);
    VMI_UNS32_PARAM(override_align64as32);
    VMI_UNS32_PARAM(override_STRoffsetPC12);
    VMI_UNS32_PARAM(override_fcsePresent);
    VMI_UNS32_PARAM(override_mpuV5ExtAP);
    VMI_UNS32_PARAM(override_fcseRequiresMMU);
    VMI_UNS32_PARAM(override_ERG);
    VMI_UNS32_PARAM(override_ignoreBadCp15);
    VMI_UNS32_PARAM(override_fpexcDexPresent);
    VMI_UNS32_PARAM(override_advSIMDPresent);
    VMI_UNS32_PARAM(override_vfpPresent);
    VMI_UNS32_PARAM(override_numCPUs);
    VMI_UNS32_PARAM(override_ICCPMRBits);
    VMI_UNS32_PARAM(override_minICCBPR);
    VMI_UNS32_PARAM(override_ICCIDR);
    VMI_BOOL_PARAM(override_SGIDisable);

} armParamValues, *armParamValuesP;

#endif
