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

#ifndef ARM_MODE_H
#define ARM_MODE_H

//
// Dictionary modes
//
typedef enum armModeE {

    // BITMASKS
    ARM_MODE_U            = 0x1, // user mode bitmask
    ARM_MODE_MMU_MPU      = 0x2, // MMU/MPU enabled mode bitmask

    // MODES
    ARM_MODE_PRIV         = (0 | 0          | 0               ),
    ARM_MODE_USER         = (0 | ARM_MODE_U | 0               ),
    ARM_MODE_PRIV_MMU_MPU = (0 | 0          | ARM_MODE_MMU_MPU),
    ARM_MODE_USER_MMU_MPU = (0 | ARM_MODE_U | ARM_MODE_MMU_MPU),

    // KEEP LAST: for array sizing
    ARM_MODE_LAST

} armMode;

//
// ARM CPSR modes
//
typedef enum armCPSRModeE {
    ARM_CPSR_USER       = 0x10,
    ARM_CPSR_FIQ        = 0x11,
    ARM_CPSR_IRQ        = 0x12,
    ARM_CPSR_SUPERVISOR = 0x13,
    ARM_CPSR_ABORT      = 0x17,
    ARM_CPSR_UNDEFINED  = 0x1b,
    ARM_CPSR_SYSTEM     = 0x1f,
    ARM_CPSR_LAST       = 0x20
} armCPSRMode;

//
// Block mask entries
//
typedef enum armBlockMaskE {

    // Thumb/ThumbEE mode control
    ARM_BM_THUMB    = 0x00001,
    ARM_BM_JAZ_EE   = 0x00002,
    ARM_BM_THUMB_EE = ARM_BM_THUMB | ARM_BM_JAZ_EE,

    // other simulator control settings
    ARM_BM_BIG_ENDIAN   = 0x00004,
    ARM_BM_UNALIGNED    = 0x00008,
    ARM_BM_L4           = 0x00010,
    ARM_BM_SDFP_EN      = 0x00020,
    ARM_BM_CP10         = 0x00040,
    ARM_BM_ASEDIS       = 0x00080,
    ARM_BM_D32DIS       = 0x00100,
    ARM_BM_SWP          = 0x00200,

    // register set selections
    ARM_BM_R8_R12_BASE      = 0x00400,
    ARM_BM_R8_R12_FIQ       = 0x00800,
    ARM_BM_R13_R14_BASE     = 0x01000,
    ARM_BM_R13_R14_SPSR_FIQ = 0x02000,
    ARM_BM_R13_R14_SPSR_IRQ = 0x04000,
    ARM_BM_R13_R14_SPSR_SVC = 0x08000,
    ARM_BM_R13_R14_SPSR_ABT = 0x10000,
    ARM_BM_R13_R14_SPSR_UND = 0x20000,

    // register set selections per mode
    ARM_BM_USER       = ARM_BM_R8_R12_BASE | ARM_BM_R13_R14_BASE,
    ARM_BM_SYSTEM     = ARM_BM_R8_R12_BASE | ARM_BM_R13_R14_BASE,
    ARM_BM_FIQ        = ARM_BM_R8_R12_FIQ  | ARM_BM_R13_R14_SPSR_FIQ,
    ARM_BM_IRQ        = ARM_BM_R8_R12_BASE | ARM_BM_R13_R14_SPSR_IRQ,
    ARM_BM_SUPERVISOR = ARM_BM_R8_R12_BASE | ARM_BM_R13_R14_SPSR_SVC,
    ARM_BM_ABORT      = ARM_BM_R8_R12_BASE | ARM_BM_R13_R14_SPSR_ABT,
    ARM_BM_UNDEFINED  = ARM_BM_R8_R12_BASE | ARM_BM_R13_R14_SPSR_UND,

    // FPSCR LEN and STRIDE fields
    ARM_BM_STRIDE0 = 0x040000,
    ARM_BM_STRIDE1 = 0x080000,
    ARM_BM_LEN0    = 0x100000,
    ARM_BM_LEN1    = 0x200000,
    ARM_BM_LEN2    = 0x400000,
    ARM_BM_STRIDE  = ARM_BM_STRIDE0 | ARM_BM_STRIDE1,
    ARM_BM_LEN     = ARM_BM_LEN0 | ARM_BM_LEN1 | ARM_BM_LEN2,

} armBlockMask;

//
// Processor disable reasons (bitmask)
//
typedef enum armDisableReasonE {
    AD_ACTIVE = 0x0,    // processor running
    AD_WFE    = 0x1,    // processor halted in WFE
    AD_WFI    = 0x2,    // processor halted in WFI
    AD_RESET  = 0x4,    // processor halted in reset
} armDisableReason;

#endif
