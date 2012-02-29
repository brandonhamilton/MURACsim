/**
 *
 * Morphable Runtime Architecture Computer
 * Platform Configuration
 *
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 *
 */

#include <iostream>

#include "tlm.h"
#include "ovpworld.org/modelSupport/tlmPlatform/1.0/tlm2.0/tlmPlatform.hpp"
#include "ovpworld.org/modelSupport/tlmDecoder/1.0/tlm2.0/tlmDecoder.hpp"
#include "ovpworld.org/memory/ram/1.0/tlm2.0/tlmMemory.hpp"
#include "../peripheral/systemc/muracAA.hpp"
#ifdef INTECEPT_OBJECT_SUPPORTED
#include "arm.ovpworld.org/processor/arm/1.0/tlm2.0/processor.igen.hpp"
#else
#include "../processor/tlm2.0/processor.igen.hpp"
#endif

#ifndef SYSTEMC_LIB
    #define SYSTEMC_LIB 0
#endif

#define SC_INCLUDE_DYNAMIC_PROCESSES 1

class MuracPlatform : public sc_core::sc_module {
  public:
    MuracPlatform (sc_core::sc_module_name name);
    icmTLMPlatform  platform;
    
    decoder<2,3>    pa_bus;      // PA bus
    decoder<1,3>    aa_bus;      // AA bus
    decoder<4,2>    shared_bus;  // Shared memory bridge

    ram             pa_memory;     // Local memory for PA
    ram             aa_memory;     // Local memory for AA
    ram             shared_memory; // Shared memory exposed to all processors
    ram             murac_memory;  // Shared memory used for murac specific signalling

    muracAA         aa;       // Murac Auxiliary architecture

#ifdef INTECEPT_OBJECT_SUPPORTED
    arm             pa;       // Murac Primary architecture
#else
    murac_arm       pa;       // Murac Primary architecture
#endif

    icmAttrListObject *attributesForPA() {
        icmAttrListObject *userAttrs = new icmAttrListObject;
        userAttrs->addAttr("showHiddenRegs", "0");
        userAttrs->addAttr("compatibility", "ISA");
        userAttrs->addAttr("variant", "Cortex-A8");
        userAttrs->addAttr("override_debugMask",0);
        return userAttrs;
    }
};


MuracPlatform::MuracPlatform (sc_core::sc_module_name name)
    : sc_core::sc_module (name),
      platform ("icm", ICM_VERBOSE | ICM_STOP_ON_CTRLC | ICM_ENABLE_IMPERAS_INTERCEPTS | ICM_WALLCLOCK),
      pa_bus("pa_bus"),
      aa_bus("aa_bus"),
      shared_bus("shared_bus"),
      pa_memory("mem_pa", "sp1", 0x100000),
      aa_memory("mem_aa", "sp1", 0x100000),
      shared_memory("mem_shared", "sp1", 0x1000000),
      murac_memory("mem_murac", "sp1", 0x1000000),
      aa("aa"),
#ifdef INTECEPT_OBJECT_SUPPORTED
      pa ( "pa", 0, ICM_ATTR_SIMEX | ICM_ATTR_TRACE_ICOUNT | ICM_ATTR_RELAXED_SCHED, attributesForPA() )
#else
      pa ( "pa", 0, MURAC_PA_MODEL_FILE, ICM_ATTR_SIMEX | ICM_ATTR_TRACE_ICOUNT | ICM_ATTR_RELAXED_SCHED , attributesForPA() )
#endif
{
#ifdef INTECEPT_OBJECT_SUPPORTED
    pa.addInterceptObject("pa", MURAC_PA_INSTRUCTIONS_FILE, "modelAttrs", 0);
#endif

    // PA bus master
    pa.INSTRUCTION.socket(pa_bus.target_socket[0]);
    pa.DATA.socket(pa_bus.target_socket[1]);

    // PA bus slaves
    pa_bus.initiator_socket[0](pa_memory.sp1);
    pa_bus.setDecode(0, 0xFFF00000, 0xFFFFFFFF);

    pa_bus.initiator_socket[1](shared_bus.target_socket[0]);
    pa_bus.setDecode(1, 0x00000000, 0x00FFFFFF);

    pa_bus.initiator_socket[2](shared_bus.target_socket[1]);
    pa_bus.setDecode(2, 0xCF000000, 0xCFFFFFFF);

    // AA bus master
    aa.aa_bus(aa_bus.target_socket[0]);

    // AA bus slaves
    aa_bus.initiator_socket[0](aa_memory.sp1);
    aa_bus.setDecode(0, 0xFFF00000, 0xFFFFFFFF);

    aa_bus.initiator_socket[1](shared_bus.target_socket[2]);
    aa_bus.setDecode(1, 0x00000000, 0x00FFFFFF);

    aa_bus.initiator_socket[2](shared_bus.target_socket[3]);
    aa_bus.setDecode(2, 0xCF000000, 0xCFFFFFFF);

    // Share memory bridge
    shared_bus.initiator_socket[0](shared_memory.sp1);
    shared_bus.setDecode(0, 0x00000000, 0x00FFFFFF);

    shared_bus.initiator_socket[1](murac_memory.sp1);
    shared_bus.setDecode(1, 0xCF000000, 0xCFFFFFFF);

    // Interrupts
    pa.brarch( aa.brarch );
    aa.intRetArch( pa.fiq );
}

int sc_main (int argc, char *argv[]) {

    const char *pa_exe = "application/pa/murac_test.ARM7.elf";
    const char *aa_lib = 0;//SYSTEMC_LIB;
    sc_time stop(10000,SC_MS);

    if (argc > 1) {
        pa_exe = argv[1];
        if (argc > 2) {
            aa_lib = argv[2];
        }
    } else {
        cout << endl << "Usage: " << argv[0] << " <pa application> [<aa library>]" << endl;
        cout << "       Please specify application and library for simulation" << endl;
        return 0;
    }

    sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);

    // Ignore some of the Warning messages
    icmIgnoreMessage ("ICM_NPF");

    cout << "Running MURAC TLM platform simulator" << endl;

    MuracPlatform murac("murac");

    murac.pa.setIPS(1000);

    // Load the PA application into memory
    unsigned char *targetPtr = murac.shared_memory.getMemory()->get_mem_ptr();
    murac.pa.loadNativeMemory(targetPtr, 0x1000000, 0x00000000, "mem_shared", pa_exe, 0, 1, 1);

    // Load the AA library
    if (aa_lib) {
        murac.aa.loadLibrary(aa_lib);
    }

    // Specify the debug processor.
    murac.pa.debugThisProcessor();

    sc_report_handler::set_actions (SC_ID_MORE_THAN_ONE_SIGNAL_DRIVER_, SC_DO_NOTHING);
    // Start the simulation
    cout << "Starting sc_main." << endl;
    sc_core::sc_start();
    cout << "Finished sc_main." << endl;
    return 0;
}
