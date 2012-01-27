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

#ifndef ARM_SIMD_VFP_H
#define ARM_SIMD_VFP_H

// VMI header files
#include "vmi/vmiTypes.h"

// model header files
#include "armTypeRefs.h"


//
// This enumeration specifies the current mode of operation of the floating
// point unit - it can be in either SIMD or VFP mode
//
typedef enum armFPModeE {
    ARM_FPM_NONE = 0x0, // no current mode
    ARM_FPM_SIMD = 0x1, // SIMD mode is active
    ARM_FPM_VFP  = 0x2  // VFP mode is active
} armFPMode;

//
// Set VFP control word
//
void armSetVFPControlWord(armP arm);

//
// Set SIMD control word
//
void armSetSIMDControlWord(armP arm);

//
// Write SIMD/VFP FPSCR register
//
void armWriteFPSCR(armP arm, Uns32 newValue);

//
// Read SIMD/VFP FPSCR register
//
Uns32 armReadFPSCR(armP arm);

//
// Write SIMD/VFP FPEXC register
//
void armWriteFPEXC(armP arm, Uns32 newValue);

//
// Call on initialization
//
void armFPInitialize(armP arm);

//
// Call on reset
//
void armFPReset(armP arm);

//
// Convert from half-precision to single-precision
//
Uns32 armFPHalfToSingle(armP arm, Uns16 half);

//
// Convert from single-precision to half-precision
//
Uns16 armFPSingleToHalf(armP arm, Uns32 single);

//
// Do reciprocal estimate
//
Uns32 armFPRecipEstimate(armP arm, Uns32 op);

//
// Do unsigned reciprocal estimate
//
Uns32 armFPUnsignedRecipEstimate(armP arm, Uns32 op);

//
// Do reciprocal square root estimate
//
Uns32 armFPRSqrtEstimate(armP arm, Uns32 op);

//
// Do unsigned reciprocal square root estimate
//
Uns32 armFPUnsignedRsqrtEstimate(armP arm, Uns32 op);

//
// Return True if the single-rrecision floating point values op1 and op2 are 0
// and infinity (in either order), setting the denormal sticky bit if so
//
Bool armFPInfinityAndZero(armP arm, Uns32 op1, Uns32 op2);

#endif
