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

#ifndef ARM_MPCORE_REGISTERS_H
#define ARM_MPCORE_REGISTERS_H

// VMI header files
#include "vmi/vmiTypes.h"

// model header files
#include "armTypeRefs.h"


// construct global enumeration member name from register name
#define MPG_ID(_R)                      MPG_ID_##_R

// access an entire global MPCore register as an Uns32
#define MPG_REG_UNS32(_P, _R)           ((_P)->mpGlobals->regValues.regs[MPG_ID(_R)])

// access an entire global MPCore register as a structure
#define MPG_REG_STRUCT(_P, _R)          ((_P)->mpGlobals->regValues.fields._R)

// access a field in a global MPCore register
#define MPG_FIELD(_P, _R, _F)           MPG_REG_STRUCT(_P, _R)._F

// construct local enumeration member name from register name
#define MPL_ID(_R)                      MPL_ID_##_R

// access an entire local MPCore register as an Uns32
#define MPL_REG_UNS32(_P, _R)           ((_P)->mpLocals->regValues.regs[MPL_ID(_R)])

// access an entire local MPCore register as a structure
#define MPL_REG_STRUCT(_P, _R)          ((_P)->mpLocals->regValues.fields._R)

// access a field in a local MPCore register
#define MPL_FIELD(_P, _R, _F)           MPL_REG_STRUCT(_P, _R)._F

// access default value for an entire MPCore register as a structure
#define MP_REG_STRUCT_DEFAULT(_P, _R)   ((_P)->configInfo.mpRegDefaults._R)

// construct 4 global enumeration member names from register name
#define MPG_IDx4(_R) \
    MPG_ID(_R##0), MPG_ID(_R##1), MPG_ID(_R##2), MPG_ID(_R##3)

// construct 8 global enumeration member names from register name
#define MPG_IDx8(_R) \
    MPG_ID(_R##0), MPG_ID(_R##1), MPG_ID(_R##2), MPG_ID(_R##3), \
    MPG_ID(_R##4), MPG_ID(_R##5), MPG_ID(_R##6), MPG_ID(_R##7)

// construct 7 global enumeration member names from register name
#define MPG_IDx7(_R) \
    MPG_ID(_R##0), MPG_ID(_R##1), MPG_ID(_R##2), MPG_ID(_R##3), \
    MPG_ID(_R##4), MPG_ID(_R##5), MPG_ID(_R##6)

// construct 16 global enumeration member names from register name
#define MPG_IDx16(_R) \
    MPG_ID(_R##0), MPG_ID(_R##1), MPG_ID(_R##2), MPG_ID(_R##3), \
    MPG_ID(_R##4), MPG_ID(_R##5), MPG_ID(_R##6), MPG_ID(_R##7), \
    MPG_ID(_R##8), MPG_ID(_R##9), MPG_ID(_R##A), MPG_ID(_R##B), \
    MPG_ID(_R##C), MPG_ID(_R##D), MPG_ID(_R##E), MPG_ID(_R##F)

// construct 64 global enumeration member names from register name
#define MPG_IDx64(_R) \
    MPG_IDx16(_R##0), MPG_IDx16(_R##1), MPG_IDx16(_R##2), MPG_IDx16(_R##3)

// this is the number of bits used to implement priorities
#define MP_PRIORITY_BITS 8

// this is the idle priority
#define MP_IDLE_PRIORITY ((1<<MP_PRIORITY_BITS)-1)


//
// Identifiers for each implemented MPCore register
//
typedef enum armMPRegIdE {

    ////////////////////////////////////////////////////////////////////////////
    // GLOBAL REGISTERS
    ////////////////////////////////////////////////////////////////////////////

    // this code defines an invalid MPCore register specification
    MPG_ID(INVALID)=-1,

    // SCU register entries
    MPG_ID(SCUControl) = 0,
    MPG_ID(SCUConfiguration),
    MPG_ID(SCUCPUPowerStatus),
    MPG_ID(SCUFilteringStart),
    MPG_ID(SCUFilteringEnd),
    MPG_ID(SCUSAC),
    MPG_ID(SCUNSAC),

    // interrupt distributor register entries
    MPG_ID(ICDDCR),
    MPG_ID(ICDICTR),
    MPG_ID(ICDIIDR),
    MPG_ID(PPI_STATUS),
    MPG_ID(ICDSGIR),
    MPG_IDx8(PERIPH_ID),
    MPG_IDx4(COMPONENT_ID),

    // marker for pseudo-registers (not represented in processor structure)
    MPG_ID(FirstPseudoReg),

    // SCU register entries
    MPG_ID(SCUInvalidateSecure) = MPG_ID(FirstPseudoReg),

    // interrupt distributor register entries
    MPG_IDx8(ICDGRPR),
    MPG_IDx8(ICDISER),
    MPG_IDx8(ICDICER),
    MPG_IDx8(ICDISPR),
    MPG_IDx8(ICDICPR),
    MPG_IDx8(ICDABR),
    MPG_IDx64(ICDIPR),
    MPG_IDx64(ICDIPTR),
    MPG_IDx16(ICDICFR),
    MPG_IDx7(SPI_STATUS),

    // keep last (used to define size of the global enumeration)
    MPG_ID(Size),

    ////////////////////////////////////////////////////////////////////////////
    // LOCAL REGISTERS
    ////////////////////////////////////////////////////////////////////////////

    // interrupt controller register entries
    MPL_ID(ICCICR) = 0,
    MPL_ID(ICCPMR),
    MPL_ID(ICCBPR),
    MPL_ID(ICCIAR),
    MPL_ID(ICCEOIR),
    MPL_ID(ICCRPR),
    MPL_ID(ICCHPIR),
    MPL_ID(ICCABPR),
    MPL_ID(ICCIDR),

    // marker for pseudo-registers (not represented in processor structure)
    MPL_ID(FirstPseudoReg),

    // keep last (used to define size of the enumeration)
    MPL_ID(Size) = MPL_ID(FirstPseudoReg),

    // local timer register entries
    MPL_ID(PTLoad),
    MPL_ID(WTLoad),
    MPL_ID(PTCounter),
    MPL_ID(WTCounter),
    MPL_ID(PTControl),
    MPL_ID(WTControl),
    MPL_ID(PTInterruptStatus),
    MPL_ID(WTInterruptStatus),
    MPL_ID(PTResetStatus),  // dummy entry
    MPL_ID(WTResetStatus),
    MPL_ID(PTDisable),      // dummy entry
    MPL_ID(WTDisable),

    // global timer register entries
    MPL_ID(GTCounter),
    MPL_ID(GTControl),
    MPL_ID(GTInterruptStatus),
    MPL_ID(GTComparator),
    MPL_ID(GTAutoIncrement),

} armMPRegId;

// use this to declare a register structure below
#define MP_REG_STRUCT_DECL(_N) armMPReg_##_N


// -----------------------------------------------------------------------------
// SCUControl
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 SCU_enable                       :  1;
    Uns32 Address_filtering_enable         :  1;
    Uns32 SCU_RAMs_parity_enable           :  1;
    Uns32 SCU_speculative_linefill_enable  :  1;
    Uns32 Force_all_Device_to_port0_enable :  1;
    Uns32 SCU_standby_enable               :  1;
    Uns32 IC_standby_enable                :  1;
    Uns32 _u1                              : 25;
} MP_REG_STRUCT_DECL(SCUControl);

#define MP_WRITE_MASK_SCUControl 0x00000003
#define MP_ADDRESS_SCUControl    0x00

// -----------------------------------------------------------------------------
// SCUConfiguration
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Number_of_CPUs         :  2;
    Uns32 _u1                    :  2;
    Uns32 CPUs_in_coherency_mode :  4;
    Uns32 Tag_RAM_Sizes          :  8;
    Uns32 _u2                    : 16;
} MP_REG_STRUCT_DECL(SCUConfiguration);

#define MP_WRITE_MASK_SCUConfiguration 0x00000000
#define MP_ADDRESS_SCUConfiguration    0x04

// -----------------------------------------------------------------------------
// SCUCPUPowerStatus
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 CPU0status : 2;
    Uns32 _u1        : 6;
    Uns32 CPU1status : 2;
    Uns32 _u2        : 6;
    Uns32 CPU2status : 2;
    Uns32 _u3        : 6;
    Uns32 CPU3status : 2;
    Uns32 _u4        : 6;
} MP_REG_STRUCT_DECL(SCUCPUPowerStatus);

#define MP_WRITE_MASK_SCUCPUPowerStatus 0x03030303
#define MP_ADDRESS_SCUCPUPowerStatus    0x08

// -----------------------------------------------------------------------------
// SCUFilteringStart, SCUFilteringEnd
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 _u1     : 20;
    Uns32 Address : 12;
} MP_REG_STRUCT_DECL(SCUFilteringStart), MP_REG_STRUCT_DECL(SCUFilteringEnd);

#define MP_WRITE_MASK_SCUFilteringStart 0xfff00000
#define MP_WRITE_MASK_SCUFilteringEnd   0xfff00000
#define MP_ADDRESS_SCUFilteringStart    0x40
#define MP_ADDRESS_SCUFilteringEnd      0x44

// -----------------------------------------------------------------------------
// SCUSAC
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 CPU0 :  1;
    Uns32 CPU1 :  1;
    Uns32 CPU2 :  1;
    Uns32 CPU3 :  1;
    Uns32 _u1  : 28;
} MP_REG_STRUCT_DECL(SCUSAC);

#define MP_WRITE_MASK_SCUSAC 0x0000000f
#define MP_ADDRESS_SCUSAC    0x50

// -----------------------------------------------------------------------------
// SCUNSAC
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 CPU0_component     :  1;
    Uns32 CPU1_component     :  1;
    Uns32 CPU2_component     :  1;
    Uns32 CPU3_component     :  1;
    Uns32 CPU0_private_timer :  1;
    Uns32 CPU1_private_timer :  1;
    Uns32 CPU2_private_timer :  1;
    Uns32 CPU3_private_timer :  1;
    Uns32 CPU0_global_timer  :  1;
    Uns32 CPU1_global_timer  :  1;
    Uns32 CPU2_global_timer  :  1;
    Uns32 CPU3_global_timer  :  1;
    Uns32 _u1                : 20;
} MP_REG_STRUCT_DECL(SCUNSAC);

#define MP_WRITE_MASK_SCUNSAC 0x00000fff
#define MP_ADDRESS_SCUNSAC    0x54

// -----------------------------------------------------------------------------
// SCUInvalidateSecure
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 CPU0ways :  4;
    Uns32 CPU1ways :  4;
    Uns32 CPU2ways :  4;
    Uns32 CPU3ways :  4;
    Uns32 _u1      : 16;
} MP_REG_STRUCT_DECL(SCUInvalidateSecure);

#define MP_WRITE_MASK_SCUInvalidateSecure 0x0000ffff
#define MP_ADDRESS_SCUInvalidateSecure    0x0c

// -----------------------------------------------------------------------------
// ICDDCR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 EnableGrp0 :  1;
    Uns32 EnableGrp1 :  1;
    Uns32 _u1        : 30;
} MP_REG_STRUCT_DECL(ICDDCR);

#define MP_WRITE_MASK_ICDDCR 0x00000001
#define MP_ADDRESS_ICDDCR    0x1000

// -----------------------------------------------------------------------------
// ICDICTR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 ITLines      :  5;
    Uns32 CPUNumber    :  3;
    Uns32 _u1          :  2;
    Uns32 SecurityExtn :  1;
    Uns32 LSPI         :  5;
    Uns32 _u2          : 16;
} MP_REG_STRUCT_DECL(ICDICTR);

#define MP_WRITE_MASK_ICDICTR 0x00000000
#define MP_ADDRESS_ICDICTR    0x1004

// -----------------------------------------------------------------------------
// ICDIIDR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Implementer : 12;
    Uns32 Revision    : 12;
    Uns32 Version     :  8;
} MP_REG_STRUCT_DECL(ICDIIDR);

#define MP_WRITE_MASK_ICDIIDR 0x00000000
#define MP_ADDRESS_ICDIIDR    0x1008

// -----------------------------------------------------------------------------
// ICDGRPR0...ICDGRPR7
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_ICDGRPR 0xffffffff
#define MP_ADDRESS_ICDGRPR    0x1080

// -----------------------------------------------------------------------------
// ICDISER0...ICDISER7
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_ICDISER 0xffffffff
#define MP_ADDRESS_ICDISER    0x1100

// -----------------------------------------------------------------------------
// ICDICER0...ICDICER7
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_ICDICER 0xffffffff
#define MP_ADDRESS_ICDICER    0x1180

// -----------------------------------------------------------------------------
// ICDISPR0...ICDISPR7
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_ICDISPR 0xffffffff
#define MP_ADDRESS_ICDISPR    0x1200

// -----------------------------------------------------------------------------
// ICDICPR0...ICDICPR7
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_ICDICPR 0xffffffff
#define MP_ADDRESS_ICDICPR    0x1280

// -----------------------------------------------------------------------------
// ICDABR0...ICDABR7
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_ICDABR 0x00000000
#define MP_ADDRESS_ICDABR    0x1300

// -----------------------------------------------------------------------------
// ICDIPR00...ICDIPR3F
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_ICDIPR 0xffffffff
#define MP_ADDRESS_ICDIPR    0x1400

// -----------------------------------------------------------------------------
// ICDIPTR00...ICDIPTR3F
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_ICDIPTR 0xffffffff
#define MP_ADDRESS_ICDIPTR    0x1800

// -----------------------------------------------------------------------------
// ICDICFR0...ICDICFRF
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_ICDICFR 0xffffffff
#define MP_ADDRESS_ICDICFR    0x1c00

// -----------------------------------------------------------------------------
// PPI_STATUS
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 _u1  : 11;
    Uns32 PPI0 :  1;
    Uns32 FIQ  :  1;
    Uns32 PPI2 :  1;
    Uns32 PPI3 :  1;
    Uns32 IRQ  :  1;
    Uns32 _u2  : 16;
} MP_REG_STRUCT_DECL(PPI_STATUS);

#define MP_WRITE_MASK_PPI_STATUS 0x00000000
#define MP_ADDRESS_PPI_STATUS    0x1d00

// -----------------------------------------------------------------------------
// SPI_STATUS0...SPI_STATUS6
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_SPI_STATUS 0x00000000
#define MP_ADDRESS_SPI_STATUS    0x1d04

// -----------------------------------------------------------------------------
// ICDSGIR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 SGIINTID         :  4;
    Uns32 _u1              : 11;
    Uns32 NSATT            :  1;
    Uns32 CPUTargetList    :  8;
    Uns32 TargetListFilter :  2;
    Uns32 _u2              :  6;
} MP_REG_STRUCT_DECL(ICDSGIR);

#define MP_WRITE_MASK_ICDSGIR 0xffffffff
#define MP_ADDRESS_ICDSGIR    0x1f00

// -----------------------------------------------------------------------------
// PERIPH_ID0...PERIPH_ID7
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_PERIPH_ID 0x00000000
#define MP_ADDRESS_PERIPH_ID    0x1fd0

// -----------------------------------------------------------------------------
// COMPONENT_ID0...COMPONENT_ID3
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_COMPONENT_ID 0x00000000
#define MP_ADDRESS_COMPONENT_ID    0x1ff0

// -----------------------------------------------------------------------------
// ICCICR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 EnableGrp0 :  1;
    Uns32 EnableGrp1 :  1;
    Uns32 AckCtl     :  1;
    Uns32 FIQEn      :  1;
    Uns32 CBPR       :  1;
    Uns32 _u1        : 27;
} MP_REG_STRUCT_DECL(ICCICR);

#define MP_WRITE_MASK_ICCICR 0x0000001f
#define MP_ADDRESS_ICCICR    0x100

// -----------------------------------------------------------------------------
// ICCPMR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Priority : MP_PRIORITY_BITS;
    Uns32 _u1      : 32-MP_PRIORITY_BITS;
} MP_REG_STRUCT_DECL(ICCPMR);

#define MP_WRITE_MASK_ICCPMR ((1<<MP_PRIORITY_BITS)-1)
#define MP_ADDRESS_ICCPMR    0x104

// -----------------------------------------------------------------------------
// ICCBPR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 BinaryPoint :  3;
    Uns32 _u1         : 29;
} MP_REG_STRUCT_DECL(ICCBPR);

#define MP_WRITE_MASK_ICCBPR 0x00000007
#define MP_ADDRESS_ICCBPR    0x108

// -----------------------------------------------------------------------------
// ICCIAR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 InterruptID : 10;
    Uns32 CPUID       :  3;
    Uns32 _u1         : 19;
} MP_REG_STRUCT_DECL(ICCIAR);

#define MP_WRITE_MASK_ICCIAR 0x00000000
#define MP_ADDRESS_ICCIAR    0x10c

// -----------------------------------------------------------------------------
// ICCEOIR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 InterruptID : 10;
    Uns32 CPUID       :  3;
    Uns32 _u1         : 19;
} MP_REG_STRUCT_DECL(ICCEOIR);

#define MP_WRITE_MASK_ICCEOIR 0x000003ff
#define MP_ADDRESS_ICCEOIR    0x110

// -----------------------------------------------------------------------------
// ICCRPR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Priority : MP_PRIORITY_BITS;
    Uns32 _u1      : 32-MP_PRIORITY_BITS;
} MP_REG_STRUCT_DECL(ICCRPR);

#define MP_WRITE_MASK_ICCRPR 0x00000000
#define MP_ADDRESS_ICCRPR    0x114

// -----------------------------------------------------------------------------
// ICCHPIR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 InterruptID : 10;
    Uns32 CPUID       :  3;
    Uns32 _u1         : 19;
} MP_REG_STRUCT_DECL(ICCHPIR);

#define MP_WRITE_MASK_ICCHPIR 0x00000000
#define MP_ADDRESS_ICCHPIR    0x118

// -----------------------------------------------------------------------------
// ICCABPR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 BinaryPoint :  3;
    Uns32 _u1         : 29;
} MP_REG_STRUCT_DECL(ICCABPR);

#define MP_WRITE_MASK_ICCABPR 0x00000007
#define MP_ADDRESS_ICCABPR    0x11c

// -----------------------------------------------------------------------------
// ICCIDR
// -----------------------------------------------------------------------------

typedef struct {
    Uns32 Implementer  : 12;
    Uns32 Revision     :  4;
    Uns32 Architecture :  4;
    Uns32 Part         : 12;
} MP_REG_STRUCT_DECL(ICCIDR);

#define MP_WRITE_MASK_ICCIDR 0x00000000
#define MP_ADDRESS_ICCIDR    0x1fc

// -----------------------------------------------------------------------------
// PTLoad, WTLoad
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_PTLoad 0xffffffff
#define MP_ADDRESS_PTLoad    0x600
#define MP_WRITE_MASK_WTLoad 0xffffffff
#define MP_ADDRESS_WTLoad    0x620

// -----------------------------------------------------------------------------
// PTCounter, WTCounter
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_PTCounter 0xffffffff
#define MP_ADDRESS_PTCounter    0x604
#define MP_WRITE_MASK_WTCounter 0xffffffff
#define MP_ADDRESS_WTCounter    0x624

// -----------------------------------------------------------------------------
// PTControl, WTControl
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_PTControl 0x0000ff07
#define MP_ADDRESS_PTControl    0x608
#define MP_WRITE_MASK_WTControl 0x0000ff0f
#define MP_ADDRESS_WTControl    0x628

// -----------------------------------------------------------------------------
// PTInterruptStatus, WTInterruptStatus
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_PTInterruptStatus 0x00000001
#define MP_ADDRESS_PTInterruptStatus    0x60c
#define MP_WRITE_MASK_WTInterruptStatus 0x00000001
#define MP_ADDRESS_WTInterruptStatus    0x62c

// -----------------------------------------------------------------------------
// WTResetStatus
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_WTResetStatus 0x00000001
#define MP_ADDRESS_WTResetStatus    0x630

// -----------------------------------------------------------------------------
// WTDisable
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_WTDisable 0xffffffff
#define MP_ADDRESS_WTDisable    0x634

// -----------------------------------------------------------------------------
// GTCounter
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_GTCounter 0xffffffff
#define MP_ADDRESS_GTCounter    0x200

// -----------------------------------------------------------------------------
// GTControl
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_GTControl 0x0000ff0f
#define MP_ADDRESS_GTControl    0x208

// -----------------------------------------------------------------------------
// GTInterruptStatus
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_GTInterruptStatus 0x00000001
#define MP_ADDRESS_GTInterruptStatus    0x20c

// -----------------------------------------------------------------------------
// GTComparator
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_GTComparator 0xffffffff
#define MP_ADDRESS_GTComparator    0x210

// -----------------------------------------------------------------------------
// GTAutoIncrement
// -----------------------------------------------------------------------------

#define MP_WRITE_MASK_GTAutoIncrement 0xffffffff
#define MP_ADDRESS_GTAutoIncrement    0x218


// -----------------------------------------------------------------------------
// CONTAINERS
// -----------------------------------------------------------------------------

// use this to define a field-based register entry in armMP[GL]RegsU below
#define MP_REG_DECL(_N) MP_REG_STRUCT_DECL(_N) _N

// use this to define a single plain register in armMP[GL]RegsU below
#define MP_UNS32_DECL(_N) Uns32 _N

//
// This type defines the entire implemented global register set
//
typedef union armMPGRegsU {

    Uns32 regs[MPG_ID(FirstPseudoReg)]; // use this for by-register access

    struct {                            // use this for by-field access
        // SCU register entries
        MP_REG_DECL(SCUControl);
        MP_REG_DECL(SCUConfiguration);
        MP_REG_DECL(SCUCPUPowerStatus);
        MP_REG_DECL(SCUFilteringStart);
        MP_REG_DECL(SCUFilteringEnd);
        MP_REG_DECL(SCUSAC);
        MP_REG_DECL(SCUNSAC);
        // interrupt distributor register entries
        MP_REG_DECL(ICDDCR);
        MP_REG_DECL(ICDICTR);
        MP_REG_DECL(ICDIIDR);
        MP_REG_DECL(PPI_STATUS);
        MP_REG_DECL(ICDSGIR);
        MP_UNS32_DECL(PERIPH_ID)[8];
        MP_UNS32_DECL(COMPONENT_ID)[4];
    } fields;

} armMPGRegs, *armMPGRegsP;

//
// This type defines the entire implemented local register set
//
typedef union armMPLRegsU {

    Uns32 regs[MPL_ID(FirstPseudoReg)]; // use this for by-register access

    struct {                            // use this for by-field access
        // Interrupt controller register entries
        MP_REG_DECL(ICCICR);
        MP_REG_DECL(ICCPMR);
        MP_REG_DECL(ICCBPR);
        MP_REG_DECL(ICCIAR);
        MP_REG_DECL(ICCEOIR);
        MP_REG_DECL(ICCRPR);
        MP_REG_DECL(ICCHPIR);
        MP_REG_DECL(ICCABPR);
        MP_REG_DECL(ICCIDR);
    } fields;

} armMPLRegs, *armMPLRegsP;

#endif

