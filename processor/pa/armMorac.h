/**
 * MORAC
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 */

#ifndef ARM_MORAC_H
#define ARM_MORAC_H

#include "armStructure.h"

#define MORAC_PC_ADDRESS 0xCF000000

void vmic_branchAuxiliaryArchitecture(armP arm, Uns32 aa_block_size);

#endif
