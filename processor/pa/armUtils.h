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

#ifndef ARM_UTILS_H
#define ARM_UTILS_H

// model header files
#include "armTypeRefs.h"

//
// Set the initial endianness for the model
//
void armSetInitialEndian(armP arm, Bool isBigEndian);

//
// Return the name of a GPR
//
const char *armGetGPRName(armP arm, Uns32 index);

//
// Return the name of a CPR
//
const char *armGetCPRName(armP arm, Uns32 index);

//
// If register 'base' is in the GPR group, set 'gprIndex' to its index number
// and return True; otherwise, return False
//
Bool armGetGPRIndex(armMorphStateP state, vmiReg base, Uns32 *gprIndex);

//
// Return effective prcessor mode for the passed effective mode (translates
// invalid modes to user mode)
//
armCPSRMode armGetCPSRMode(armCPSRMode raw);

//
// Update processor block mask
//
void armSetBlockMask(armP arm);

//
// Switch banked registers on switch to the passed mode
//
void armSwitchRegs(armP arm, armCPSRMode oldCPSRMode, armCPSRMode newCPSRMode);

//
// Set up read and write masks for PSRs based on configuration options
//
void armSetPSRMasks(armP arm);

//
// Return CPSR/APSR read mask for the current mode
//
Uns32 armGetReadMaskCPSR(armP arm);

//
// Return CPSR/APSR write mask for the current mode
//
Uns32 armGetWriteMaskCPSR(armP arm, Bool affectExecState);

//
// Return SPSR write mask
//
Uns32 armGetWriteMaskSPSR(armP arm);

//
// Read CPSR register
//
Uns32 armReadCPSR(armP arm);

//
// Write CPSR register
//
void armWriteCPSR(armP arm, Uns32 value, Uns32 mask);

//
// Read SPSR register
//
Uns32 armReadSPSR(armP arm);

//
// Write SPSR register
//
void armWriteSPSR(armP arm, Uns32 value, Uns32 mask);

//
// Read PC register
//
Uns32 armReadPC(armP arm);

//
// Write PC register
//
void armWritePC(armP arm, Uns32 value);

//
// Switch processor mode if required
//
void armSwitchMode(armP arm);

//
// Force the processor temporarily into the passed mode
//
void armForceMode(armP arm, armCPSRMode mode);

//
// Abort any active exclusive access
//
void armAbortExclusiveAccess(armP arm);

#endif
