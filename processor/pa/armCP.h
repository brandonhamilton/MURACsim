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

#ifndef ARM_CP_H
#define ARM_CP_H

// VMI header files
#include "vmi/vmiTypes.h"

// model header files
#include "armTypeRefs.h"
#include "armCPRegisters.h"

//
// Reset all coprocessor invalid access warnings
//
void armCpResetWarnings(armP arm);

//
// Emit coprocessor read (one register)
//
Bool armEmitCpRead(armMorphStateP state, vmiReg rd);

//
// Emit coprocessor write (one register)
//
Bool armEmitCpWrite(armMorphStateP state, vmiReg rs);

//
// Emit coprocessor read (two registers)
//
Bool armEmitCpReadPair(armMorphStateP state, vmiReg rd1, vmiReg rd2);

//
// Emit coprocessor write (two registers)
//
Bool armEmitCpWritePair(armMorphStateP state, vmiReg rs1, vmiReg rs2);

//
// Call on initialization
//
void armCpInitialize(armP arm, Uns32 index);

//
// Call on reset
//
void armCpReset(armP arm);

//
// Is the indicated coprocessor register supported on this processor?
//
Bool armGetCpRegSupported(armCPRegId id, armP arm);

//
// Perform a privileged-mode read of the coprocessor register
//
Bool armReadCpRegPriv(armCPRegId id, armP arm, Uns32 *result);

//
// Perform a privileged-mode write of the coprocessor register
//
Bool armWriteCpRegPriv(armCPRegId id, armP arm, Uns32 value);

//
// Add programmer's view of coprocessor register
//
void armAddCpRegisterView(
    armCPRegId     id,
    armP           arm,
    vmiViewObjectP baseObject,
    const char    *name
);

//
// Structure filled with coprocessor register details by armGetCpRegisterDetails
//
typedef struct armCpRegDetailsS {
    const char *name;
    armCPRegId  id;
    Int8        cpNum;
    Int8        op1;
    Int8        op2;
    Int8        crn;
    Int8        crm;
    const char  *privRW;
    const char  *userRW;
} armCpRegDetails, *armCpRegDetailsP;

//
// Iterator filling 'details' with the next coprocessor register details -
// 'details.name' should be initialized to NULL prior to the first call
//
Bool armGetCpRegisterDetails(armCpRegDetailsP details);

#endif

