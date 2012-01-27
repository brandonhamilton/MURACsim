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

#ifndef ARM_EXCEPTIONS_H
#define ARM_EXCEPTIONS_H

// VMI header files
#include "vmi/vmiTypes.h"

// model header files
#include "armTypeRefs.h"
#include "armExceptionTypes.h"

//
// Macros for construction of faultStatusD argument to armDataAbort
//
#define INVALID_DOMAIN          0xff
#define ARM_DOMAIN_SHIFT        4
#define ARM_FAULT_STATUS_MASK   ((1<<ARM_DOMAIN_SHIFT)-1)
#define FAULT_STATUS(_S, _D)    ((_S) | ((_D)<<ARM_DOMAIN_SHIFT))

//
// Exception levels
//
#define FIQ_MASK 0x1
#define IRQ_MASK 0x2

//
// Create new input port specification
//
void armAddNetInputPort(
    armP           arm,
    const char    *name,
    vmiNetChangeFn cb,
    Uns32          userData,
    const char    *desc
);

//
// Create new output port specification
//
void armAddNetOutputPort(
    armP        arm,
    const char *name,
    Uns32      *handle,
    const char *desc
);

//
// Free port specifications
//
void armFreePortSpecs(armP arm);

//
// Is there a pending interrupt on this processor?
//
Bool armInterruptPending(armP arm);

//
// Perform actions on a possible change to IRQ/FIQ state
//
void armRefreshInterrupt(armP arm);

//
// Send event to all processors in this cluster
//
void armDoSEV(armP arm);

//
// Do breakpoint exception
//
void armBKPT(armP arm, Uns32 thisPC);

//
// Do software exception
//
void armSWI(armP arm, Uns32 thisPC);

//
// Do undefined instruction exception
//
void armUndefined(armP arm, Uns32 thisPC, Uns32 setDEX);

//
// Do data/prefetch abort exception
//
void armMemoryAbort(
    armP    arm,
    Uns32   faultStatusD,
    Uns32   faultAddress,
    memPriv priv
);

//
// Do external memory abort exception
//
void armExternalMemoryAbort(
    armP    arm,
    Uns32   faultAddress,
    memPriv priv
);

//
// Do reset exception
//
void armReset(armP arm, Bool intMask);

//
// This is called to perform an exception-like branch to the null handler
//
void armCheckHandlerNull(armP arm);

//
// This is called to perform an exception-like branch to the array handler
//
void armCheckHandlerArray(armP arm);

//
// Connect up CPU nets
//
void armConnectCPUNets(armP cpu);

#endif
