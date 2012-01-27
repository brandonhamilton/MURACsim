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

// VMI header files
#include "vmi/vmiCxt.h"

// model header files
#include "armDecodeARM.h"
#include "armDecodeThumb.h"
#include "armDecodeTypes.h"
#include "armStructure.h"

//
// Null decoder for trivial Jazelle extension
//
static void decodeJazelle(armP arm, Uns32 thisPC, armInstructionInfoP info) {

    // most fields are zeroed out
    armInstructionInfo jazelleInfo = {
        thisPC      : thisPC,
        type        : ARM_IT_LAST,
        cond        : ARM_C_AL,
        instruction : vmicxtFetch1Byte((vmiProcessorP)arm, thisPC),
        bytes       : 1,
        opcode      : "Jazelle",
        format      : "(unknown)"
    };

    *info = jazelleInfo;
}

//
// Decode the instruction at the passed address. The 'info' structure is filled
// with details of the instruction.
//
ARM_DECODER_FN(armDecode) {

    // record current PC in decoded structure
    info->thisPC = thisPC;

    if(IN_JAZELLE_MODE(arm)) {
        decodeJazelle(arm, thisPC, info);
    } else if(IN_THUMB_MODE(arm)) {
        armDecodeThumb(arm, thisPC, info, IN_THUMB_EE_MODE(arm));
    } else {
        armDecodeARM(arm, thisPC, info);
    }
}

//
// Return the size of the instruction at the passed address and the mode
//
ARM_ISIZE_FN(armGetInstructionSizeMode) {
    return isThumb ? armGetThumbInstructionSize(arm, thisPC) : 4;
}

//
// Return the size of the instruction at the passed address
//
Uns32 armGetInstructionSize(armP arm, Uns32 thisPC) {
    if(IN_JAZELLE_MODE(arm)) {
        return 1;
    } else {
        return armGetInstructionSizeMode(arm, thisPC, IN_THUMB_MODE(arm));
    }
}

