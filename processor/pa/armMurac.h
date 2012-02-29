/**
 * MURAC
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 */

#ifndef ARM_MURAC_H
#define ARM_MURAC_H

#include "armStructure.h"

#define MURAC_PC_ADDRESS 0xCF000000

void vmic_branchAuxiliaryArchitecture(armP arm, Uns32 aa_block_size);

#endif
