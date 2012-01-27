/**
 * Morac FPGA peripheral
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 */

#ifndef MORACFPGA_H
#define MORACFPGA_H

extern Uns32 diagnosticLevel;

/****
 * PC -> register set by the PA
 * BrArch -> Interrupt to enable AA
 * RetArch -> Interrupt to return to PA
 */

#define THREAD_STACK      (8*1024)
#define MORAC_PC_ADDRESS 0xcf000000

typedef struct {
	char  name[128]; // Device description    
    ppmAddressSpaceHandle readHandle;  // Memory Read Address Space Handle
    ppmAddressSpaceHandle writeHandle; // Memory Write Address Space Handle
    ppmNetHandle          brarch;      // Interrupt to indicate BrArch
    ppmNetHandle          intRetArch;  // Interrupt to return to Primary Architecture

    bhmThreadHandle       thread;
    bhmEventHandle        start;
    Bool                  busy;
    char                  stack[THREAD_STACK];

} MoracFPGAState;

#endif
