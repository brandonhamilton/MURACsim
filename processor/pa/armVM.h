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

#ifndef ARM_VM_H
#define ARM_VM_H

// VMI header files
#include "vmi/vmiTypes.h"

// model header files
#include "armTypeRefs.h"


//
// This is the minimum page size (1Kb)
//
#define MIN_PAGE_SIZE 1024

//
// Enumation describing actions performed by armVMMiss
//
typedef enum armVMActionE {
    MA_OK,          // memory was mapped, access can proceed
    MA_EXCEPTION,   // memory was not mapped, exception was triggered
    MA_BAD          // memory was not mapped, invalid access
} armVMAction;

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
);

//
// Perform VA to PA mapping
//
void armVMTranslateVAtoPA(armP arm, Uns32 VA, Bool isUser, memPriv priv);

//
// Invalidate entire TLB
//
void armVMInvalidate(armP arm, memPriv priv);

//
// Invalidate TLB entry by matching MVA and ASID (ARMv6)
//
void armVMInvalidateEntryMVA(armP arm, Uns32 MVA, Uns32 ASID, memPriv priv);

//
// Invalidate TLB entry by matching ASID (ARMv6)
//
void armVMInvalidateEntryASID(armP arm, Uns32 ASID, memPriv priv);

//
// Invalidate TLB entry by matching MVA, ignoring ASID
//
void armVMInvalidateEntryMVAA(armP arm, Uns32 MVA, memPriv priv);

//
// Perform any required memory mapping updates on a PID or mode switch
//
void armVMSetPIDOrModeFCSE(armP arm);

//
// Perform any required memory mapping updates on an ASID change
//
void armVMSetASID(armP arm);

//
// Perform any required memory mapping updates on domain access control change
//
void armVMSetDomainAccessControl(armP arm);

//
// Set the privileged mode data domain to the user domain (for LDRT, STRT)
//
void armVMSetUserPrivilegedModeDataDomain(armP arm);

//
// Restore the normal data domain for the current mode (for LDRT, STRT)
//
void armVMRestoreNormalDataDomain(armP arm);

//
// Write MPU Access Permissions register (ARMv4/ARMv5 only)
//
void armVMWriteAccessARMv5(armP arm, Bool isData, Uns32 newValue);

//
// Read MPU Access Permissions register (ARMv4/ARMv5 only)
//
Uns32 armVMReadAccessARMv5(armP arm, Bool isData);

//
// Write MPU Extended Access Permissions register (ARMv4/ARMv5 only)
//
void armVMWriteAccessExtARMv5(armP arm, Bool isData, Uns32 newValue);

//
// Read MPU Extended Access Permissions register (ARMv4/ARMv5 only)
//
Uns32 armVMReadAccessExtARMv5(armP arm, Bool isData);

//
// Write the indexed protection area control register (ARMv4/ARMv5 only)
//
void armVMWriteRegionARMv5(armP arm, Uns32 index, Bool isData, Uns32 newValue);

//
// Read the indexed protection area control register (ARMv4/ARMv5 only)
//
Uns32 armVMReadRegionARMv5(armP arm, Uns32 index, Bool isData);

//
// Write the indexed MPU RegionBase register value (ARMv6 and later)
//
void armVMWriteRegionBase(armP arm, Uns32 index, Bool isData, Uns32 newValue);

//
// Read the indexed MPU RegionBase register value (ARMv6 and later)
//
Uns32 armVMReadRegionBase(armP arm, Uns32 index, Bool isData);

//
// Write the indexed MPU RegionSizeE register value (ARMv6 and later)
//
void armVMWriteRegionSizeE(armP arm, Uns32 index, Bool isData, Uns32 newValue);

//
// Read the indexed MPU RegionSizeE register value (ARMv6 and later)
//
Uns32 armVMReadRegionSizeE(armP arm, Uns32 index, Bool isData);

//
// Write the indexed MPU RegionAccess register value (ARMv6 and later)
//
void armVMWriteRegionAccess(armP arm, Uns32 index, Bool isData, Uns32 newValue);

//
// Read the indexed MPU RegionAccess register value (ARMv6 and later)
//
Uns32 armVMReadRegionAccess(armP arm, Uns32 index, Bool isData);

//
// Write TLBLockdown register
//
void armVMWriteLockdown(armP arm, Bool isData, Uns32 newValue);

//
// Read TLBLockdown register
//
Uns32 armVMReadLockdown(armP arm, Bool isData);

//
// Read VA of the indexed TLB lockdown entry
//
Uns32 armVMReadLockdownVA(armP arm, Bool isData);

//
// Write VA of the indexed TLB lockdown entry
//
void armVMWriteLockdownVA(armP arm, Bool isData, Uns32 newValue);

//
// Read PA of the indexed TLB lockdown entry
//
Uns32 armVMReadLockdownPA(armP arm, Bool isData);

//
// Write PA of the indexed TLB lockdown entry
//
void armVMWriteLockdownPA(armP arm, Bool isData, Uns32 newValue);

//
// Read attributes of the indexed TLB lockdown entry
//
Uns32 armVMReadLockdownAttr(armP arm, Bool isData);

//
// Write attributes of the indexed TLB lockdown entry
//
void armVMWriteLockdownAttr(armP arm, Bool isData, Uns32 newValue);

//
// Update ITCM region register
//
void armVMUpdateITCMRegion(armP arm, Uns32 newValue);

//
// Update DTCM region register
//
void armVMUpdateDTCMRegion(armP arm, Uns32 newValue);

//
// Write DMAControl register value
//
void armVMWriteDMAControl(armP arm, Uns32 newValue);

//
// Read DMAControl register value
//
Uns32 armVMReadDMAControl(armP arm);

//
// Write DMAInternalStart register value
//
void armVMWriteDMAInternalStart(armP arm, Uns32 newValue);

//
// Read DMAInternalStart register value
//
Uns32 armVMReadDMAInternalStart(armP arm);

//
// Write DMAExternalStart register value
//
void armVMWriteDMAExternalStart(armP arm, Uns32 newValue);

//
// Read DMAExternalStart register value
//
Uns32 armVMReadDMAExternalStart(armP arm);

//
// Write DMAInternalEnd register value
//
void armVMWriteDMAInternalEnd(armP arm, Uns32 newValue);

//
// Read DMAInternalEnd register value
//
Uns32 armVMReadDMAInternalEnd(armP arm);

//
// Read DMAStatus register value
//
Uns32 armVMReadDMAStatus(armP arm);

//
// Write DMAContextID register value
//
void armVMWriteDMAContextID(armP arm, Uns32 newValue);

//
// Read DMAContextID register value
//
Uns32 armVMReadDMAContextID(armP arm);

//
// Write DMAStop register value
//
void armVMWriteDMAStop(armP arm);

//
// Write DMAStart register
//
void armVMWriteDMAStart(armP arm);

//
// Write DMAClear register
//
void armVMWriteDMAClear(armP arm);

//
// Dump current TLB contents
//
void armVMDumpTLB(armP arm);

//
// Reset VM structures
//
void armVMReset(armP arm);

//
// Free structures used for virtual memory management
//
void armVMFree(armP arm);

#endif

