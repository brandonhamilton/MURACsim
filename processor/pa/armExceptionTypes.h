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

#ifndef ARM_EXCEPTION_TYPES_H
#define ARM_EXCEPTION_TYPES_H

//
// This enumerates exception codes
//
typedef enum armExceptionE {
    AE_Reset,           // reset
    AE_Undefined,       // undefined instruction
    AE_SupervisorCall,  // supervisor call
    AE_PrefetchAbort,   // prefetch abort
    AE_DataAbort,       // data abort
    AE_IRQ,             // interrupt
    AE_FIQ,             // fast interrupt
    AE_LAST             // for sizing (keep last)
} armException;

//
// This enumerates fault status codes
//
typedef enum armFaultStatusE {
    AFS_OK                  = 0x00, // used by MMU routines to indicate success
    AFS_VectorException     = 0x00,
    AFS_Alignment1          = 0x01, // ARMv4 and ARMv5 only
    AFS_Alignment           = 0x01, // ARMv6 and later
    AFS_DebugEvent          = 0x02,
    AFS_Alignment3          = 0x03, // ARMv4 and ARMv5 only
    AFS_AccessFlagSection   = 0x03, // ARMv6 and later
    AFS_CacheMaintenence    = 0x04, // ARMv6 and later
    AFS_TranslationSection  = 0x05,
    AFS_AccessFlagPage      = 0x06, // ARMv6 and later
    AFS_TranslationPage     = 0x07,
    AFS_PreciseExternal     = 0x08, // ARMv6 and later
    AFS_DomainSection       = 0x09,
    AFS_DomainPage          = 0x0b,
    AFS_TranslationLevel1   = 0x0c,
    AFS_PermissionSection   = 0x0d,
    AFS_TranslationLevel2   = 0x0e,
    AFS_PermissionPage      = 0x0f,
    AFS_SynchronousExternal = 0x08,
    AFS_DMA_TCM_Range       = 0x08, // ARMv6 and later - supplemental DMA code
    AFS_DMA_Unshared        = 0x09, // ARMv6 and later - supplemental DMA code
    AFS_DMA                 = 0x10  // ARMv6 and later - supplemental DMA code
} armFaultStatus;

#endif
