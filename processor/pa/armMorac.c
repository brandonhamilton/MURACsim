/**
 * MORAC
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 */

#include "vmi/vmiMessage.h"
#include "vmi/vmiRt.h"

#include "armMorac.h"
#include "stdio.h"

void vmic_branchAuxiliaryArchitecture(armP arm, Uns32 aa_block_size) {
    /* Set the PC */
    Uns32 simPC = vmirtGetPC((vmiProcessorP)arm);
    int toAlign = (aa_block_size % 4 > 0) ? 4 - aa_block_size % 4 : 0;
    simPC += 4 + aa_block_size + toAlign;
    vmirtSetPC((vmiProcessorP)arm, simPC);
    /* Trigger the interrupt */
    vmirtWriteNetPort((vmiProcessorP)arm, arm->brarch, 1);
    vmirtWriteNetPort((vmiProcessorP)arm, arm->brarch, 0);    
}
