/**
 *
 * Morphable Runtime Architecture Computer
 * Primary Architecture (PA) enhanced instruction set
 *
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 *
 */

 // standard includes
#include <stdio.h>

// VMI area includes
#include "vmi/vmiCxt.h"
#include "vmi/vmiDecode.h"
#include "vmi/vmiMessage.h"
#include "vmi/vmiOSAttrs.h"
#include "vmi/vmiOSLib.h"
#include "vmi/vmiRt.h"
#include "vmi/vmiVersion.h"

/**
  BAA Instruction Format
   
  31  ____ ________ ____________ ____ ____  0
     |1110|00010010|............|0100| Rn |
     |____|________|____________|____|____|

  e.g. 11100001001011111111111101000001
       = E12FFF41
 */
   
#define WIDTH(_W, _ARG) ((_ARG) & ((1<<(_W))-1))
#define EXRACT_B(_I) WIDTH(5,(_I))

typedef struct vmiosObjectS {
    // Enhanced instruction decode table
    vmidDecodeTableP table;
} vmiosObject;

#define DECODE_ENTRY(_PRIORITY, _NAME, _FMT) \
    vmidNewEntryFmtBin(     \
        table,              \
        #_NAME,             \
        ARM_MPA_##_NAME,    \
        _FMT,               \
        _PRIORITY           \
    )

//
// Primary Architecture instruction type enumeration
//
typedef enum armMURACInstrTypeE {

    // Branch Architecture instruction
    ARM_MPA_BAA,

    // KEEP LAST: for sizing the array
    ARM_MPA_LAST

} armMURACInstrType;

//
// Create the MURAC PA decode table
//
static vmidDecodeTableP createDecodeTable(void) {

    vmidDecodeTableP table = vmidNewDecodeTable(32, ARM_MPA_LAST);

    DECODE_ENTRY(0, BAA, "1110|00010010|............|0100|....");

    return table;
}


//
// Constructor
//
static VMIOS_CONSTRUCTOR_FN(constructor) {
    object->table = createDecodeTable();
}

//
// Morpher callback implementing Primary Architecture instructions
//
static VMIOS_INTERCEPT_FN(doBrArch) {
    // TODO: Implement the BrArch instruction here
}

//
// Morpher callback implementing Primary Architecture instructions
//
static VMIOS_MORPH_FN(muracPAMorph) {
    Uns32             instruction = vmicxtFetch4Byte(processor, thisPC);
    armMURACInstrType type        = vmidDecode(object->table, instruction);

    if ( type == ARM_MPA_BAA ) {
        // Handle the BAA instruction
        *opaque   = True;
        *userData = (void *)instruction;
        return doBrArch;
    } else {
        return 0;
    }
}

//
// Return instruction address after passed program counter
//
static VMIOS_NEXT_PC_FN(muracPANextPC) {
    Uns32             instruction = vmicxtFetch4Byte(processor, thisPC);
    armMURACInstrType type        = vmidDecode(object->table, instruction);

    if ( type == ARM_MPA_BAA ) {
        *nextPC = thisPC + 4;
        return True;
    } else {
        return False;
    }
}

//
// Disassembler callback disassembling Primary Architecture instructions
//
static VMIOS_DISASSEMBLE_FN(muracPADisassemble) {
    Uns32             instruction = vmicxtFetch4Byte(processor, thisPC);
    armMURACInstrType type        = vmidDecode(object->table, instruction);

    if ( type == ARM_MPA_BAA ) {
        // instruction is BranchArchitexture
        static char buffer[256];
        // extract instruction fields
        Uns32 ra = EXRACT_B(instruction);
        sprintf(buffer, "%-8s r%u", "baa", ra);
        return buffer;
    } else {
        return 0;
    }
}

////////////////////////////////////////////////////////////////////////////////
// INTERCEPT ATTRIBUTES
////////////////////////////////////////////////////////////////////////////////

vmiosAttr modelAttrs = {

    ////////////////////////////////////////////////////////////////////////
    // VERSION
    ////////////////////////////////////////////////////////////////////////

    VMI_VERSION,            // version string (THIS MUST BE FIRST)
    VMI_INTERCEPT_LIBRARY,  // model type
    "murac_pa",             // description
    sizeof(vmiosObject),    // size in bytes of OSS object

    ////////////////////////////////////////////////////////////////////////
    // CONSTRUCTOR/DESTRUCTOR ROUTINES
    ////////////////////////////////////////////////////////////////////////

    constructor,            // object constructor
    0,                      // object destructor

    ////////////////////////////////////////////////////////////////////////
    // INSTRUCTION INTERCEPT ROUTINES
    ////////////////////////////////////////////////////////////////////////

    muracPAMorph,          // morph callback
    muracPANextPC,         // get next instruction address
    muracPADisassemble,    // disassemble instruction

    ////////////////////////////////////////////////////////////////////////
    // ADDRESS INTERCEPT DEFINITIONS
    ////////////////////////////////////////////////////////////////////////

    {{0}}
};
