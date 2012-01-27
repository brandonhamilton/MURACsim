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

// Standard header files
#include "string.h"
#include "stdio.h"

// Imperas header files
#include "hostapi/impAlloc.h"

// VMI header files
#include "vmi/vmiAttrs.h"
#include "vmi/vmiDbg.h"
#include "vmi/vmiMessage.h"
#include "vmi/vmiOSLib.h"
#include "vmi/vmiRt.h"

// model header files
#include "armRegisters.h"
#include "armStructure.h"
#include "armSIMDVFP.h"
#include "armUtils.h"
#include "armCP.h"


//
// Prefix for messages from this module
//
#define CPU_PREFIX "ARM_DEBUG"


////////////////////////////////////////////////////////////////////////////////
// REGISTER GROUPS
////////////////////////////////////////////////////////////////////////////////

//
// This describes the register groups in the processor
//
typedef enum armRegGroupIdE {
    ARM_RG_CORE,        // Core group
    ARM_RG_GDB_FPR,     // FPU register views required for gdb (unimplemented)
    ARM_RG_CONTROL,     // control register group
    ARM_RG_USER,        // user mode register group
    ARM_RG_FIQ,         // FIQ mode register group
    ARM_RG_IRQ,         // IRQ mode register group
    ARM_RG_SUPERVISOR,  // supervisor mode register group
    ARM_RG_UNDEFINED,   // undefined mode register group
    ARM_RG_ABORT,       // abort mode register group
    ARM_RG_FPR,         // SIMD/VFP register group
    ARM_RG_CP,          // coprocessor register group
    ARM_RG_LAST         // KEEP LAST: for sizing
} armRegGroupId;

//
// This provides information about each group
//
static const vmiRegGroup groups[ARM_RG_LAST+1] = {
    [ARM_RG_CORE]       = {name: "Core"                                 },
    [ARM_RG_GDB_FPR]    = {name: "FPU registers for GDB (unimplemented)"},
    [ARM_RG_CONTROL]    = {name: "Control"                              },
    [ARM_RG_USER]       = {name: "User"                                 },
    [ARM_RG_FIQ]        = {name: "FIQ"                                  },
    [ARM_RG_IRQ]        = {name: "IRQ"                                  },
    [ARM_RG_SUPERVISOR] = {name: "Supervisor"                           },
    [ARM_RG_UNDEFINED]  = {name: "Undefined"                            },
    [ARM_RG_ABORT]      = {name: "Abort"                                },
    [ARM_RG_FPR]        = {name: "SIMD/VFP"                             },
    [ARM_RG_CP]         = {name: "Coprocessor"                          }
};

//
// Macro to specify a the group for a register
//
#define ARM_GROUP(_G) &groups[ARM_RG_##_G]


////////////////////////////////////////////////////////////////////////////////
// MACROS FOR REGISTER ACCESS
////////////////////////////////////////////////////////////////////////////////

//
// Macro to specify a register that can be accessed using raw read/write
// callbacks
//
#define ARM_RAW_REG(_R, _G) \
    VMI_REG_RAW_READ_CB,        \
    VMI_REG_RAW_WRITE_CB,       \
    (void *)ARM_CPU_OFFSET(_R), \
    _G

//
// Macro to specify a banked register
//
#define ARM_ACCESS_BANK(_M) \
    readBank, writeBank, (void *)ARM_CPSR_##_M, ARM_GROUP(_M)

//
// Macro to specify core registers
//
#define ARM_CORE_REG(_I) ARM_RAW_REG(regs[_I], ARM_GROUP(CORE))

//
// Macro to specify the PC accessible for read/write
//
#define ARM_PC_RW readPC, writePC, 0, ARM_GROUP(CORE)

//
// Macro to specify the archaic GDB floating point registers
//
#define ARM_GDB_FPR_RW readFP, writeFP, 0, ARM_GROUP(GDB_FPR)

//
// Macro to specify control registers
//
#define ARM_CONTROL_RW(_ID) read##_ID, write##_ID, 0, ARM_GROUP(CONTROL)

//
// SIMD/VFP registers - must check for existence in the variant
//
#define ARM_SDFP0_INDEX     700
#define ARM_SDFP31_INDEX    731
#define IS_SDFP_REG(_I)     (((_I)>=ARM_SDFP0_INDEX) && ((_I)<=ARM_SDFP31_INDEX))
#define ARM_SDFP_INDEX(_I)  ((_I)-ARM_SDFP0_INDEX)
#define ARM_FPR_REG(_I)     ARM_RAW_REG(vregs.d[_I], ARM_GROUP(FPR))

//
// Some registers are hidden in gdb, but we allow access to them
//
#define ARM_GDB_HIDDEN_INDEX  99
#define IS_GDB_HIDDEN_REG(_I) ((_I)>=ARM_GDB_HIDDEN_INDEX)

//
// coprocessor registers do not have access
//
#define ARM_CP_INDEX        0x100000
#define IS_CP_REG(_I)       ((_I)>=ARM_CP_INDEX)


////////////////////////////////////////////////////////////////////////////////
// DEBUGGER REGISTER INTERFACE
////////////////////////////////////////////////////////////////////////////////

//
// Return current vmiRegInfoCP structure for the passed banked vmiRegInfoCP
//
vmiRegInfoCP getCurrentInfo(vmiRegInfoCP reg);

//
// Return coprocessor register id for vmiRegInfoCP
//
static armCPRegId getCpId(vmiRegInfoCP reg) {
    if(!IS_CP_REG(reg->gdbIndex)) {
        return CP_INVALID;
    } else {
        return (armCPRegId)reg->userData;
    }
}

//
// Read callback for pc
//
static VMI_REG_READ_FN(readPC) {
    *(Uns32*)buffer = armReadPC((armP)processor);
    return True;
}

//
// Write callback for pc
//
static VMI_REG_WRITE_FN(writePC) {
    armWritePC((armP)processor, *(Uns32*)buffer);
    return True;
}

//
// Write callback for floating point register (currently unimplemented)
//
static VMI_REG_WRITE_FN(writeFP) {
    return True;
}

//
// Read callback for floating point register (currently unimplemented)
//
static VMI_REG_READ_FN(readFP) {
    Uns32 i;
    for(i=0; i<(96/32); i++) {
        ((Uns32*)buffer)[i] = 0;
    }
    return True;
}

//
// Check if the double word SIMD/VFP register index is present and in range
//
static Bool sdfpDIndexValid(armP arm, Uns32 index) {

    if (!ADVSIMD_PRESENT(arm) && !VFP_PRESENT(arm)) return False;

    Uns32 simdRegisters = SDFP_FIELD(arm, MVFR0, A_SIMD_Registers);

    // simdRegisters = 0 means no SIMD/VFP regs are implemented
    if(!simdRegisters) return False;

    // simdRegisters = 1 means only 16 64-bit regs are implemented
    if(index>15 && simdRegisters==1) return False;

    // Max registers is 32
    if(index > 31) return False;

    return True;
}

//
// Read callback for floating point status register
//
static VMI_REG_READ_FN(readFPSCR) {
    *(Uns32*)buffer = armReadFPSCR((armP)processor);
    return True;
}

//
// Write callback for floating point status register
//
static VMI_REG_WRITE_FN(writeFPSCR) {
    armP arm = (armP)processor;
    armWriteFPSCR(arm, *(Uns32*)buffer);
    return True;
}

//
// Read callback for CPSR
//
static VMI_REG_READ_FN(readCPSR) {
    *(Uns32*)buffer = armReadCPSR((armP)processor);
    return True;
}

//
// Write callback for CPSR
//
static VMI_REG_WRITE_FN(writeCPSR) {
    armP arm = (armP)processor;
    armWriteCPSR(arm, *(Uns32*)buffer, arm->wMaskCPSR);
    return True;
}

//
// Read callback for SPSR
//
static VMI_REG_READ_FN(readSPSR) {
    *(Uns32*)buffer = armReadSPSR((armP)processor);
    return True;
}

//
// Write callback for SPSR
//
static VMI_REG_WRITE_FN(writeSPSR) {
    armP arm = (armP)processor;
    armWriteSPSR(arm, *(Uns32*)buffer, arm->wMaskSPSR);
    return True;
}

//
// Read callback for banked register
//
static VMI_REG_READ_FN(readBank) {

    armP        arm          = (armP)processor;
    armCPSRMode trueCPSRMode = arm->CPSR.fields.mode;
    armCPSRMode tempCPSRMode = (armCPSRMode)reg->userData;

    armSwitchRegs(arm, trueCPSRMode, tempCPSRMode);
    Bool result = vmiosRegRead(processor, getCurrentInfo(reg), buffer);
    armSwitchRegs(arm, tempCPSRMode, trueCPSRMode);

    return result;
}

//
// Write callback for banked register
//
static VMI_REG_WRITE_FN(writeBank) {

    armP        arm          = (armP)processor;
    armCPSRMode trueCPSRMode = arm->CPSR.fields.mode;
    armCPSRMode tempCPSRMode = (armCPSRMode)reg->userData;

    armSwitchRegs(arm, trueCPSRMode, tempCPSRMode);
    Bool result = vmiosRegWrite(processor, getCurrentInfo(reg), buffer);
    armSwitchRegs(arm, tempCPSRMode, trueCPSRMode);

    return result;
}

//
// Read callback for coprocessor register
//
static VMI_REG_READ_FN(readCP) {

    armP       arm = (armP)processor;
    armCPRegId id  = getCpId(reg);

    if(!armReadCpRegPriv(id, arm, (Uns32*)buffer)) {

        return False;

    } else if(id!=CP_ID(MIDR)) {

        return True;

    } else {

        union {Uns32 u32; CP_REG_DECL(MIDR);} u = {*(Uns32*)buffer};

        if(!u.u32) {
            armArchitecture variant = arm->configInfo.arch;
            u.MIDR.postArm7.Architecture = ARM_VARIANT_ARCH(variant);
            *(Uns32*)buffer = u.u32;
        }

        return True;
    }
}

//
// Write callback for coprocessor register
//
static VMI_REG_WRITE_FN(writeCP) {
    return armWriteCpRegPriv(getCpId(reg), (armP)processor, *(Uns32*)buffer);
}

//
// Static const array holding information about the registers in the cpu,
// used for debugger interaction
//
static const vmiRegInfo basicRegisters[] = {

    // current mode registers (visible in gdb)
    {"r0",            0, vmi_REG_NONE, 32, False, ARM_CORE_REG(0)      },
    {"r1",            1, vmi_REG_NONE, 32, False, ARM_CORE_REG(1)      },
    {"r2",            2, vmi_REG_NONE, 32, False, ARM_CORE_REG(2)      },
    {"r3",            3, vmi_REG_NONE, 32, False, ARM_CORE_REG(3)      },
    {"r4",            4, vmi_REG_NONE, 32, False, ARM_CORE_REG(4)      },
    {"r5",            5, vmi_REG_NONE, 32, False, ARM_CORE_REG(5)      },
    {"r6",            6, vmi_REG_NONE, 32, False, ARM_CORE_REG(6)      },
    {"r7",            7, vmi_REG_NONE, 32, False, ARM_CORE_REG(7)      },
    {"r8",            8, vmi_REG_NONE, 32, False, ARM_CORE_REG(8)      },
    {"r9",            9, vmi_REG_NONE, 32, False, ARM_CORE_REG(9)      },
    {"r10",          10, vmi_REG_NONE, 32, False, ARM_CORE_REG(10)     },
    {"r11",          11, vmi_REG_FP,   32, False, ARM_CORE_REG(11)     },
    {"r12",          12, vmi_REG_NONE, 32, False, ARM_CORE_REG(12)     },
    {"sp",           13, vmi_REG_SP,   32, False, ARM_CORE_REG(13)     },
    {"lr",           14, vmi_REG_NONE, 32, False, ARM_CORE_REG(14)     },
    {"pc",           15, vmi_REG_PC,   32, False, ARM_PC_RW            },
    {"f0",           16, vmi_REG_NONE, 96, False, ARM_GDB_FPR_RW       },
    {"f1",           17, vmi_REG_NONE, 96, False, ARM_GDB_FPR_RW       },
    {"f2",           18, vmi_REG_NONE, 96, False, ARM_GDB_FPR_RW       },
    {"f3",           19, vmi_REG_NONE, 96, False, ARM_GDB_FPR_RW       },
    {"f4",           20, vmi_REG_NONE, 96, False, ARM_GDB_FPR_RW       },
    {"f5",           21, vmi_REG_NONE, 96, False, ARM_GDB_FPR_RW       },
    {"f6",           22, vmi_REG_NONE, 96, False, ARM_GDB_FPR_RW       },
    {"f7",           23, vmi_REG_NONE, 96, False, ARM_GDB_FPR_RW       },
    {"fps",          24, vmi_REG_NONE, 32, False, ARM_CONTROL_RW(FPSCR)},
    {"cpsr",         25, vmi_REG_NONE, 32, False, ARM_CONTROL_RW(CPSR) },

    // current mode spsr register (not visible in gdb)
    {"spsr",         99, vmi_REG_NONE, 32, False, ARM_CONTROL_RW(SPSR) },

    // usr/sys mode registers (not visible in gdb)
    {"r8_usr",      108, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(USER)},
    {"r9_usr",      109, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(USER)},
    {"r10_usr",     110, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(USER)},
    {"r11_usr",     111, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(USER)},
    {"r12_usr",     112, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(USER)},
    {"sp_usr",      113, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(USER)},
    {"lr_usr",      114, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(USER)},

    // fiq mode registers (not visible in gdb)
    {"r8_fiq",      208, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(FIQ)},
    {"r9_fiq",      209, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(FIQ)},
    {"r10_fiq",     210, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(FIQ)},
    {"r11_fiq",     211, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(FIQ)},
    {"r12_fiq",     212, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(FIQ)},
    {"sp_fiq",      213, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(FIQ)},
    {"lr_fiq",      214, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(FIQ)},
    {"spsr_fiq",    299, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(FIQ)},

    // irq mode registers (not visible in gdb)
    {"sp_irq",      313, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(IRQ)},
    {"lr_irq",      314, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(IRQ)},
    {"spsr_irq",    399, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(IRQ)},

    // svc mode registers (not visible in gdb)
    {"sp_svc",      413, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(SUPERVISOR)},
    {"lr_svc",      414, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(SUPERVISOR)},
    {"spsr_svc",    499, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(SUPERVISOR)},

    // undef mode registers (not visible in gdb)
    {"sp_undef",    513, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(UNDEFINED)},
    {"lr_undef",    514, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(UNDEFINED)},
    {"spsr_undef",  599, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(UNDEFINED)},

    // abt mode registers (not visible in gdb)
    {"sp_abt",      613, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(ABORT)},
    {"lr_abt",      614, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(ABORT)},
    {"spsr_abt",    699, vmi_REG_NONE, 32, False, ARM_ACCESS_BANK(ABORT)},

    // SIMD/VFP registers - double word view only (not visible in gdb)
    {"d0",          700, vmi_REG_NONE, 64, False, ARM_FPR_REG(0) },
    {"d1",          701, vmi_REG_NONE, 64, False, ARM_FPR_REG(1) },
    {"d2",          702, vmi_REG_NONE, 64, False, ARM_FPR_REG(2) },
    {"d3",          703, vmi_REG_NONE, 64, False, ARM_FPR_REG(3) },
    {"d4",          704, vmi_REG_NONE, 64, False, ARM_FPR_REG(4) },
    {"d5",          705, vmi_REG_NONE, 64, False, ARM_FPR_REG(5) },
    {"d6",          706, vmi_REG_NONE, 64, False, ARM_FPR_REG(6) },
    {"d7",          707, vmi_REG_NONE, 64, False, ARM_FPR_REG(7) },
    {"d8",          708, vmi_REG_NONE, 64, False, ARM_FPR_REG(8) },
    {"d9",          709, vmi_REG_NONE, 64, False, ARM_FPR_REG(9) },
    {"d10",         710, vmi_REG_NONE, 64, False, ARM_FPR_REG(10)},
    {"d11",         711, vmi_REG_NONE, 64, False, ARM_FPR_REG(11)},
    {"d12",         712, vmi_REG_NONE, 64, False, ARM_FPR_REG(12)},
    {"d13",         713, vmi_REG_NONE, 64, False, ARM_FPR_REG(13)},
    {"d14",         714, vmi_REG_NONE, 64, False, ARM_FPR_REG(14)},
    {"d15",         715, vmi_REG_NONE, 64, False, ARM_FPR_REG(15)},
    {"d16",         716, vmi_REG_NONE, 64, False, ARM_FPR_REG(16)},
    {"d17",         717, vmi_REG_NONE, 64, False, ARM_FPR_REG(17)},
    {"d18",         718, vmi_REG_NONE, 64, False, ARM_FPR_REG(18)},
    {"d19",         719, vmi_REG_NONE, 64, False, ARM_FPR_REG(19)},
    {"d20",         720, vmi_REG_NONE, 64, False, ARM_FPR_REG(20)},
    {"d21",         721, vmi_REG_NONE, 64, False, ARM_FPR_REG(21)},
    {"d22",         722, vmi_REG_NONE, 64, False, ARM_FPR_REG(22)},
    {"d23",         723, vmi_REG_NONE, 64, False, ARM_FPR_REG(23)},
    {"d24",         724, vmi_REG_NONE, 64, False, ARM_FPR_REG(24)},
    {"d25",         725, vmi_REG_NONE, 64, False, ARM_FPR_REG(25)},
    {"d26",         726, vmi_REG_NONE, 64, False, ARM_FPR_REG(26)},
    {"d27",         727, vmi_REG_NONE, 64, False, ARM_FPR_REG(27)},
    {"d28",         728, vmi_REG_NONE, 64, False, ARM_FPR_REG(28)},
    {"d29",         729, vmi_REG_NONE, 64, False, ARM_FPR_REG(29)},
    {"d30",         730, vmi_REG_NONE, 64, False, ARM_FPR_REG(30)},
    {"d31",         731, vmi_REG_NONE, 64, False, ARM_FPR_REG(31)},

    {0},
};

//
// Return a string for the given cp field value
//
static const char *cpFieldStr(Int8 field) {

    static const char *numStrings[] = {
            " 0", " 1", " 2", " 3", " 4", " 5", " 6", " 7",
            " 8", " 9", "10", "11", "12", "13", "14", "15"
    };

    if (field >= 0 && field <= 15)
        return (numStrings[field]);
    else
        return "--";
}

//
// Callback used to sort CP register entries in cpNum/CRn/op1/CRm/op2 order
//
//static Int32 compareCpReg(const void *va, const void *vb) {
//
//    const vmiRegInfo *a = va;
//    const vmiRegInfo *b = vb;
//    union {
//        Uns32 u32;
//        struct {
//            Uns32 cpNum  :  4;
//            Uns32 CRm    :  4;
//            Uns32 CRn    :  4;
//            Uns32 op1    :  4;
//            Uns32 op2    :  4;
//        } f;
//    } ua = {a->gdbIndex}, ub = {b->gdbIndex};
//
//    if      (ua.f.cpNum != ub.f.cpNum) return ua.f.cpNum < ub.f.cpNum ? -1 : 1;
//    else if (ua.f.CRn   != ub.f.CRn)   return ua.f.CRn   < ub.f.CRn   ? -1 : 1;
//    else if (ua.f.op1   != ub.f.op1)   return ua.f.op1   < ub.f.op1   ? -1 : 1;
//    else if (ua.f.CRm   != ub.f.CRm)   return ua.f.CRm   < ub.f.CRm   ? -1 : 1;
//    else if (ua.f.op2   != ub.f.op2)   return ua.f.op2   < ub.f.op2   ? -1 : 1;
//    else return 0;
//
//}

//
// Return ARM register descriptions
//
static vmiRegInfoCP getRegisters(void) {

    static vmiRegInfo *allRegisters;

    if(!allRegisters) {

        armCpRegDetails details;
        Uns32           basicNum = 0;
        Uns32           cpNum  = 0;
        Uns32           i;

        // count basic registers
        while(basicRegisters[basicNum].name) {
            basicNum++;
        }

        // count coprocessor registers
        details.name = 0;
        while(armGetCpRegisterDetails(&details)) {
            cpNum++;
        }

        // allocate full register information, including terminating NULL entry
        // TODO: This is never freed! (description field should be freed too)
        allRegisters = STYPE_CALLOC_N(vmiRegInfo, basicNum+cpNum+1);

        // fill basic entries
        for(i=0; i<basicNum; i++) {
            allRegisters[i] = basicRegisters[i];
        }

        // fill coprocessor entries
        for(details.name=0, i=0; armGetCpRegisterDetails(&details); i++) {

            vmiRegInfo *reg = &allRegisters[basicNum+i];

            // fill basic fields
            reg->name     = details.name;
            reg->usage    = vmi_REG_NONE;
            reg->bits     = 32;
            reg->readonly = False;
            reg->readCB   = readCP;
            reg->writeCB  = writeCP;
            reg->userData = (void *)details.id;
            reg->group    = ARM_GROUP(CP);

            // synthesize a description
            char desc[64];
            snprintf(desc, 64,
                "CP%2d CRn:%s Op1:%s CRn:%s Op2:%s Priv:%s User:%s",
                details.cpNum,
                cpFieldStr(details.crn),
                cpFieldStr(details.op1),
                cpFieldStr(details.crm),
                cpFieldStr(details.op2),
                details.privRW,
                details.userRW
            );
            reg->description = strdup(desc);

            // manufacture pseudo-index
            reg->gdbIndex = (
                ARM_CP_INDEX        |
                (details.cpNum<< 0) |
                (((Uns32) details.crm) << 4) |
                (((Uns32) details.crn) << 8) |
                (((Uns32) details.op1) <<12) |
                (((Uns32) details.op2) <<16)
            );
        }

        // sort CP register descriptions
//        if (i > 0) {
//            qsort(allRegisters+basicNum, i, sizeof(allRegisters[0]), compareCpReg);
//        }
    }

    // return register set
    return allRegisters;
}

//
// Return current vmiRegInfoCP structure for the passed banked vmiRegInfoCP
//
vmiRegInfoCP getCurrentInfo(vmiRegInfoCP reg) {

    Uns32        index = reg->gdbIndex % 100;
    vmiRegInfoCP info;

    for(info=getRegisters(); info->name; info++) {
        if(info->gdbIndex == index) {
            return info;
        }
    }

    return 0;
}

//
// Is the passed register supported on this processor?
//
static Bool isRegSupported(armP arm, vmiRegInfoCP reg, Bool gdbFrame) {

    if(gdbFrame && IS_GDB_HIDDEN_REG(reg->gdbIndex)) {

        // if this is a GDB frame request then registers that should be hidden
        // from GDB should be ignored
        return False;

    } else if(!gdbFrame && (reg->group==ARM_GROUP(GDB_FPR))) {

        // if not a GDB frame request, ignore the archaic FPRs
        return False;

    } else if(IS_CP_REG(reg->gdbIndex)) {

        // coprocessor registers are supported only if the associated unit is
        // present
        return armGetCpRegSupported(getCpId(reg), arm);

    } else if(IS_SDFP_REG(reg->gdbIndex)) {

        // SIMD/VFP registers are supported if in variant and index is in range
        return sdfpDIndexValid(arm, ARM_SDFP_INDEX(reg->gdbIndex));

    } else {

        // other registers are always supported
        return True;
    }
}

//
// Is the processor an MPCore container?
//
inline static Bool isMPCore(armP arm) {
    return vmirtGetSMPChild((vmiProcessorP)arm) && True;
}

//
// Return next supported register on this processor
//
static vmiRegInfoCP getNextRegister(armP arm, vmiRegInfoCP reg, Bool gdbFrame) {

    do {
        if(!reg) {
            reg = isMPCore(arm) ? 0 : getRegisters();
        } else if((reg+1)->name) {
            reg = reg+1;
        } else {
            reg = 0;
        }
    } while(reg && !isRegSupported(arm, reg, gdbFrame));

    return reg;
}

//
// Is the passed register group supported on this processor?
//
static Bool isGroupSupported(armP arm, vmiRegGroupCP group) {

    vmiRegInfoCP info = 0;

    while((info=getNextRegister(arm, info, False))) {
        if(info->group == group) {
            return True;
        }
    }

    return False;
}

//
// Return next supported group on this processor
//
static vmiRegGroupCP getNextGroup(armP arm, vmiRegGroupCP group) {

    do {
        if(!group) {
            group = groups;
        } else if((group+1)->name) {
            group = group+1;
        } else {
            group = 0;
        }
    } while(group && !isGroupSupported(arm, group));

    return group;
}

//
// Register structure iterator
//
VMI_REG_INFO_FN(armRegInfo) {
    return getNextRegister((armP)processor, prev, gdbFrame);
}

//
// Register group iterator
//
VMI_REG_GROUP_FN(armRegGroup) {
    return getNextGroup((armP)processor, prev);
}

//
// Return processor description
//
VMI_PROC_DESC_FN(armProcessorDescription) {
    return isMPCore((armP)processor) ? "MPCORE" : "CPU";
}


////////////////////////////////////////////////////////////////////////////////
// REGISTER DUMP INTERFACE
////////////////////////////////////////////////////////////////////////////////

//
// Dump processor registers
//
VMI_DEBUG_FN(armDumpRegisters) {

    armP         arm            = (armP)processor;
    Bool         showHiddenRegs = arm->showHiddenRegs;
    Uns32        nameWidth      = showHiddenRegs ? 10 : 7;
    vmiRegInfoCP info           = 0;

    while((info=getNextRegister(arm, info, False))) {

        if(IS_CP_REG(info->gdbIndex)) {

            // ignore coprocessor registers

        } else if(IS_SDFP_REG(info->gdbIndex)) {

            // print SDFP regs if enabled
            if(ARM_DUMP_SDFP_REG(arm)) {

                const char *fmt = "        %-*s 0x" FMT_640Nx "\n";
                Uns64 value;

                // read and print register value
                vmiosRegRead(processor, info, &value);
                vmiPrintf(fmt, nameWidth, info->name, value);
            }

        } else if(!IS_GDB_HIDDEN_REG(info->gdbIndex) || showHiddenRegs) {

            const char *fmt;
            Uns32 value;

            // read register value
            vmiosRegRead(processor, info, &value);

            // select appropriate format string
            if((info->usage==vmi_REG_SP) || (info->usage==vmi_REG_PC)) {
                fmt = "        %-*s 0x%-8x 0x%x\n";
            } else {
                fmt = "        %-*s 0x%-8x %u\n";
            }

            // print register using selected format
            vmiPrintf(fmt, nameWidth, info->name, value, value);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
// COPROCESSOR PROGRAMMER'S VIEW
////////////////////////////////////////////////////////////////////////////////

//
// Add programmer's view of all coprocessor registers
//
void armAddCpRegistersView(armP arm, vmiViewObjectP processorObject) {

    vmiRegInfoCP info = 0;

    // create coprocessor child object
    vmiViewObjectP baseObject = vmirtAddViewObject(processorObject, "CP15", 0);

    while((info=getNextRegister(arm, info, False))) {

        if(IS_CP_REG(info->gdbIndex)) {

            const char *name = info->name;
            armCPRegId  id   = getCpId(info);

            armAddCpRegisterView(id, arm, baseObject, name);
        }
    }
}

