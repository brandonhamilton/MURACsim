/**
 * Morac FPGA peripheral
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 */


#define PREFIX "MoracFPGA"

#include "peripheral/impTypes.h"
#include "peripheral/bhm.h"
#include "peripheral/ppm.h"
#include "moracFPGA.h"

#define REGISTER_SIZE 4

static MoracFPGAState fpgaState;
static Uns32 finishOnReset = 0;
Uns32 diagnosticLevel;

/**************
  Utilities
 **************/
static inline Uns32 byteSwap(Uns32 data){
#ifdef ENDIANBIG
    return
        ((data & 0xff000000) >> 24) |
        ((data & 0x00ff0000) >>  8) |
        ((data & 0x0000ff00) <<  8) |
        ((data & 0x000000ff) << 24);
#else
    return data;
#endif
}

PPM_NET_CB(brArchIRQ) {
    if ( ppmReadNet(fpgaState.brarch) == 1) {
        if (diagnosticLevel >= 3) {
            bhmMessage("I", "moracFPGA", "Branch Architecture interrupt received\n");
        }
        if(!fpgaState.busy) {
            bhmTriggerEvent(fpgaState.start);
        }
    }
}

static void setDiagLevel(Uns32 new) {
    diagnosticLevel = new;
}

static void auxiliaryArchitectureThread(void *user) {
    Uns32 pc;
    Uns32 instSize;
    Uns32 instCount;
    for (;;) {
        if (diagnosticLevel >= 3) {
            bhmMessage("I", "moracFPGA", "auxiliaryArchitectureThread waiting\n");
        }
        fpgaState.busy = False;
        bhmWaitEvent(fpgaState.start);
        if (diagnosticLevel >= 3) {
            bhmMessage("I", "moracFPGA", "auxiliaryArchitectureThread processing\n");
        }
        fpgaState.busy = True;

        ppmReadAddressSpace(fpgaState.readHandle, MORAC_PC_ADDRESS, 4, &pc);
        bhmMessage("I", "moracFPGA", "PC is 0x%x\n", pc);

        ppmReadAddressSpace(fpgaState.readHandle, MORAC_PC_ADDRESS + 4, 4, &instSize);
        bhmMessage("I", "moracFPGA", "AA Instruction size is %d\n", instSize);

        ppmReadAddressSpace(fpgaState.readHandle, MORAC_PC_ADDRESS + 8, 4, &instCount);
        bhmMessage("I", "moracFPGA", "AA Instruction count is %d\n", instCount);

        // Trigger RetArch
        ppmWriteNet(fpgaState.intRetArch, 1);
        ppmWriteNet(fpgaState.intRetArch, 0);
    }
}

int main(int argc, char **argv) {

 	bhmEventHandle wait_event;

    diagnosticLevel = 0;
    bhmInstallDiagCB(setDiagLevel);

    /* Create the Bus Mastes */
    fpgaState.readHandle  = ppmOpenAddressSpace("fpga_memread");
    fpgaState.writeHandle = ppmOpenAddressSpace("fpga_memwrite");

    /* Create the interrupt lines */
    fpgaState.brarch = ppmOpenNetPort("fpga_brarch");
    ppmInstallNetCallback(fpgaState.brarch, brArchIRQ, (void*)0);

    fpgaState.intRetArch = ppmOpenNetPort("fpga_retarch");

    if (BHM_DIAG_LOW) 
        bhmMessage("I", PREFIX,"main\n");

    bhmIntegerAttribute("stoponsoftreset", &finishOnReset);

    /* Create thread */
    fpgaState.start = bhmCreateEvent();
    fpgaState.busy = False;
    fpgaState.thread = bhmCreateThread(auxiliaryArchitectureThread, (void*)0, "moracFPGAthread", &fpgaState.stack[THREAD_STACK] );

    /* Idle until triggered */
    wait_event = bhmCreateEvent();
    bhmWaitEvent(wait_event);

	return 0;
}
