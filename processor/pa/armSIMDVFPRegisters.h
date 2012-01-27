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

#ifndef ARM_SIMD_VFP_REGISTERS_H
#define ARM_SIMD_VFP_REGISTERS_H

// VMI header files
#include "vmi/vmiTypes.h"

// model header files
#include "armCPRegisters.h"
#include "armRegisters.h"


// construct enumeration member name from register name
#define SDFP_ID(_R)                     SDFP_ID_##_R

// morph-time macro to access a field in a SIMD/VFP register
#define ARM_SDFP_REG(_ID)               ARM_CPU_REG(sdfp.regs[_ID])

// access an entire SIMD/VFP register as an Uns32
#define SDFP_REG_UNS32(_P, _R)          ((_P)->sdfp.regs[SDFP_ID(_R)])

// access an entire SIMD/VFP register as a structure
#define SDFP_REG_STRUCT(_P, _R)         ((_P)->sdfp.fields._R)

// access a field in a SIMD/VFP register
#define SDFP_FIELD(_P, _R, _F)          (SDFP_REG_STRUCT(_P, _R)._F)

// get mask to use when accessing a SIMD/VFP register mask
#define SDFP_MASK_UNS32(_P, _R)         ((_P)->configInfo.sdfpRegMasks._R.value32)

// get mask to use when accessing a SIMD/VFP register mask field
#define SDFP_MASK_FIELD(_P, _R, _F)     ((_P)->configInfo.sdfpRegMasks._R.fields._F)

// access default value for an entire SIMD/VFP register as a structure
#define SDFP_REG_STRUCT_DEFAULT(_P, _R) ((_P)->configInfo.sdfpRegDefaults._R)

// access default value for a field in a SIMD/VFP register
#define SDFP_FIELD_DEFAULT(_P, _R, _F)  (SDFP_REG_STRUCT_DEFAULT(_P, _R)._F)

// access to MVFR field
#define ARM_MVFR(_N, _F)                SDFP_FIELD(arm, MVFR##_N, _F)


//
// Identifiers for each implemented SIMD/VFP register
//
typedef enum armSDFPRegIdE {
    // SIMD/VFP register entries
    SDFP_ID(FPSID),
    SDFP_ID(FPSCR),
    SDFP_ID(MVFR0),
    SDFP_ID(MVFR1),
    SDFP_ID(FPEXC),
    // keep last (used to define size of the enumeration)
    SDFP_ID(Size),
} armSDFPRegId;

// use this to declare a register structure below
#define SDFP_REG_STRUCT_DECL(_N) armSDFPReg_##_N


// -----------------------------------------------------------------------------
// SIMD/VFP FPSID register
// -----------------------------------------------------------------------------

typedef struct {
    Uns32          Revision       : 4;
    Uns32          Variant        : 4;
    Uns32          PartNumber     : 8;
    Uns32          Subarchitecture: 7;
    Uns32          SW             : 1;
    armImplementor Implementor    : 8;
} SDFP_REG_STRUCT_DECL(FPSID);

// -----------------------------------------------------------------------------
// SIMD/VFP FPSCR register
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 IOC    : 1;
    Uns32 DZC    : 1;
    Uns32 OFC    : 1;
    Uns32 UFC    : 1;
    Uns32 IXC    : 1;
    Uns32 _u1    : 2;
    Uns32 IDC    : 1;
    Uns32 IOE    : 1;
    Uns32 DZE    : 1;
    Uns32 OFE    : 1;
    Uns32 UFE    : 1;
    Uns32 IXE    : 1;
    Uns32 _u2    : 2;
    Uns32 IDE    : 1;
    Uns32 Len    : 3;
    Uns32 _u3    : 1;
    Uns32 Stride : 2;
    Uns32 RMode  : 2;
    Uns32 FZ     : 1;
    Uns32 DN     : 1;
    Uns32 AHP    : 1;
    Uns32 QC     : 1;
    Uns32 V      : 1;
    Uns32 C      : 1;
    Uns32 Z      : 1;
    Uns32 N      : 1;
} SDFP_REG_STRUCT_DECL(FPSCR);

// -----------------------------------------------------------------------------
// SIMD/VFP MVFR0 register
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 A_SIMD_Registers  : 4;
    Uns32 SinglePrecision   : 4;
    Uns32 DoublePrecision   : 4;
    Uns32 VFP_ExceptionTrap : 4;
    Uns32 Divide            : 4;
    Uns32 SquareRoot        : 4;
    Uns32 ShortVectors      : 4;
    Uns32 VFP_RoundingModes : 4;
} SDFP_REG_STRUCT_DECL(MVFR0);

// -----------------------------------------------------------------------------
// SIMD/VFP MVFR1 register
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 FlushToZeroMode        : 4;
    Uns32 DefaultNaNMode         : 4;
    Uns32 A_SIMD_LoadStore       : 4;
    Uns32 A_SIMD_Integer         : 4;
    Uns32 A_SIMD_SinglePrecision : 4;
    Uns32 A_SIMD_HalfPrecision   : 4;
    Uns32 VFP_HalfPrecision      : 4;
    Uns32 VFP_FusedMAC           : 4;
} SDFP_REG_STRUCT_DECL(MVFR1);

// -----------------------------------------------------------------------------
// SIMD/VFP FPEXC register (only model synchronous exception fields)
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 _u1 : 29;
    Uns32 DEX : 1;
    Uns32 EN  : 1;
    Uns32 EX  : 1;      // This bit is read-only 0 since asynch FP exceptions not modeled
} SDFP_REG_STRUCT_DECL(FPEXC);

// -----------------------------------------------------------------------------
// CONTAINER
// -----------------------------------------------------------------------------

// use this to define a field-based register entry in armSDFPRegsU below
#define SDFP_REG_DECL(_N) SDFP_REG_STRUCT_DECL(_N) _N

// use this to define a plain register entry in armSDFPRegsU below
#define SDFP_UNS32_DECL(_N) Uns32 _N

//
// This type defines the entire implemented SIMD/VFP register set
//
typedef union armSDFPRegsU {

    Uns32 regs[SDFP_ID(Size)];          // use this for by-register access

    struct {                            // use this for by-field access
        SDFP_REG_DECL(FPSID);
        SDFP_REG_DECL(FPSCR);
        SDFP_REG_DECL(MVFR0);
        SDFP_REG_DECL(MVFR1);
        SDFP_REG_DECL(FPEXC);
    } fields;

} armSDFPRegs, *armSDFPRegsP;

#endif
