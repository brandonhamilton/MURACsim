/**
 *
 * Morphable Runtime Architecture Computer
 * Platform Configuration
 *
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 *
 */

#include <stdio.h>
#include <string.h>

#include "icm/icmCpuManager.h"

#define SIMULATION_FLAGS ( ICM_ATTR_DEFAULT | \
                           ICM_ATTR_RELAXED_SCHED )

static    Bool  finishOnSoftReset = False;

// Platform entry point
int main(int argc, char **argv) {

    // Get the name of the application to run from the command line
    if (argc != 2) {
        icmPrintf("usage: %s <pa application>\n", argv[0]);
        return -1;
    }

    // Initialize OVPsim, enabling verbose mode to get statistics at end of execution
    icmInit(ICM_VERBOSE | ICM_STOP_ON_CTRLC | ICM_ENABLE_IMPERAS_INTERCEPTS, NULL, 0);

    /***************************************************************************************************
       MORAC Primary Architecture
         - ARM processor
     ***************************************************************************************************/
#ifdef INTECEPT_OBJECT_SUPPORTED
    const char *armModel    = icmGetVlnvString(NULL, "arm.ovpworld.org", "processor", "arm", "1.0", "model");
#endif
    const char *armSemihost = icmGetVlnvString(NULL, "arm.ovpworld.org", "semihosting", "armNewlib", "1.0", "model");

    icmAttrListP armUserAttr = icmNewAttrList();
    icmAddStringAttr(armUserAttr, "compatibility", "gdb");
    icmAddStringAttr(armUserAttr, "variant", "Cortex-A8");
    icmAddStringAttr(armUserAttr, "UAL", "1");

    // Create the Primary Architecture (PA) processor
    icmProcessorP pa = icmNewProcessor(
        "pa",                // CPU name
        "arm",               // CPU type
        0,                   // CPU cpuId
        0,                   // CPU model flags
        32,                  // address bits
#ifdef INTECEPT_OBJECT_SUPPORTED
        armModel,            // model file
#else

        MORAC_PA_MODEL_FILE, // model file
#endif
        "modelAttrs",        // model attributes
        SIMULATION_FLAGS,    // simulation flags
        armUserAttr,         // user-defined attributes
        armSemihost,         // semi-hosting file
        "modelAttrs"         // semi-hosting attributes
    );

#ifdef INTECEPT_OBJECT_SUPPORTED
    // Add intercept libarary for MORAC Primary Architecture instructions
    icmAddInterceptObject(pa, "morac_pa", MORAC_PA_INSTRUCTIONS_FILE, "modelAttrs", 0);
#endif

    /***************************************************************************************************
       System Memory
     ***************************************************************************************************/
    // Local memory for the PA
    // the ARM processor toolchain sites code in lower memory and stack in higher memory
    // so we will use two memories
    // NOTE: this is just a consequence of the default linker script used
    icmMemoryP codeMemory = icmNewMemory("code_memory", ICM_PRIV_RWX, 0x9fffffff);
    icmMemoryP localPA    = icmNewMemory("local_pa_memory", ICM_PRIV_RWX, 0x0fffffff);

    // Local memory for the AA
    icmMemoryP localAA    = icmNewMemory("local_aa_memory", ICM_PRIV_RWX, 0x0fffffff);

    // Processor state memory (shared)
    icmMemoryP prStateMemory = icmNewMemory("pr_state_memory", ICM_PRIV_RWX, 0x100);

    /***************************************************************************************************
       System Bus connections
     ***************************************************************************************************/
    icmBusP busPA = icmNewBus("pa_bus", 32);
    icmBusP busAA = icmNewBus("aa_bus", 32);

    // Connect Primary Architecture to the pa bus
    icmConnectProcessorBusses(pa, busPA, busPA);

    // Connect memories to the busses
    icmConnectMemoryToBus(busPA, "pa_code_memory_port", codeMemory, 0x00000000);
    icmConnectMemoryToBus(busPA, "pa_local_memory_port", localPA, 0xf0000000);
    icmConnectMemoryToBus(busPA, "pa_pr_state_memory_port", prStateMemory, 0xcf000000);

    icmConnectMemoryToBus(busAA, "aa_code_memory_port", codeMemory, 0x00000000);
    icmConnectMemoryToBus(busAA, "aa_local_memory_port", localAA, 0xf0000000);
    icmConnectMemoryToBus(busAA, "aa_pr_state_memory_port", prStateMemory, 0xcf000000);

    /***************************************************************************************************
       MORAC Peripherals (Auxiliary Architecture)
     ***************************************************************************************************/
    icmAttrListP fpgaAttrs = icmNewAttrList();
    if(finishOnSoftReset) {
        icmAddUns64Attr(fpgaAttrs, "stoponsoftreset",  1);
    }
    icmPseP moracFpga = icmNewPSE("moracFpga", MORAC_AA_FPGA_PSE_FILE, fpgaAttrs, 0, 0);

    // Connect memory access ports to the bus
    icmConnectPSEBus(moracFpga, busAA, "fpga_memread", True, 0x00000000, 0xffffffff);
    icmConnectPSEBus(moracFpga, busAA, "fpga_memwrite", True, 0x00000000, 0xffffffff);
    
    /***************************************************************************************************
       System Interrupt connections
     ***************************************************************************************************/
    icmNetP intBrArch = icmNewNet("brArch");
    icmNetP intRetArch = icmNewNet("retArch");

    // connect the processor interrupt port to the net
    icmConnectProcessorNet(pa, intBrArch, "brarch", ICM_OUTPUT);
    // connect the FPGA interrupt port to the net
    icmConnectPSENet(moracFpga, intBrArch, "fpga_brarch", ICM_INPUT);

    // connect the processor interrupt port to the net
    icmConnectProcessorNet(pa, intRetArch, "fiq", ICM_INPUT);
    // connect the FPGA interrupt port to the net
    icmConnectPSENet(moracFpga, intRetArch, "fpga_retarch", ICM_OUTPUT);

    /***************************************************************************************************
       Information
     ***************************************************************************************************/

    // Show the bus connections
    icmPrintf("\nPrimary Architecture Bus Connections\n");
    icmPrintBusConnections(busPA);
    icmPrintf("\nAuxiliary Architecture Bus Connections\n");
    icmPrintBusConnections(busAA);
    icmPrintf("\nNet Connections\n");
    icmPrintNetConnections();
    icmPrintf("\n");

    /***************************************************************************************************
       MORAC Simulation
     ***************************************************************************************************/

    // Load the application code into the primary architecture memory
    icmLoadProcessorMemory(pa, argv[1], False, False, True);

    // Run the simulation until done (no instruction limit)
    icmProcessorP final = icmSimulatePlatform();
    if ( final && (icmGetStopReason(final) == ICM_SR_INTERRUPT ) ) {
        icmPrintf("*** MORAC simulation interrupted\n");
    }

    // report the total number of instructions executed
    icmPrintf("MORAC Primary Architecture has executed " FMT_64u " instructions\n", icmGetProcessorICount(pa));

    icmTerminate();
    
    return 0;

}
