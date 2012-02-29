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
#include "../peripheral/pse/tlm2.0/pse.igen.hpp"
#ifdef INTECEPT_OBJECT_SUPPORTED
#include "arm.ovpworld.org/processor/arm/1.0/tlm2.0/processor.igen.hpp"
#else
#include "../processor/tlm2.0/processor.igen.hpp"
#endif

class MuracPlatform : public sc_core::sc_module {
  public:
    MuracPlatform (sc_core::sc_module_name name);
    icmTLMPlatform  platform;

    decoder<2,2>    pa_bus;      // PA bus
    decoder<2,2>    aa_bus;      // AA bus
    decoder<2,1>    shared_bus;  // Shared memory bridge

    ram             pa_memory;
    ram             aa_memory;
    ram             shared_memory;

    muracFPGA       aa;       // Murac Auxilliary Architecture

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
      platform ("icm", ICM_VERBOSE | ICM_STOP_ON_CTRLC| ICM_ENABLE_IMPERAS_INTERCEPTS | ICM_WALLCLOCK),
      pa_bus("pa_bus"),
      aa_bus("aa_bus"),
      shared_bus("shared_bus"),
      pa_memory("mem_pa", "sp1", 0x100000),
      aa_memory("mem_aa", "sp1", 0x100000),
      shared_memory("mem_shared", "sp1", 0x100000),
      aa ( "aa", MURAC_AA_FPGA_PSE_FILE, 0 ),
#ifdef INTECEPT_OBJECT_SUPPORTED
      pa ( "pa", 0, ICM_ATTR_SIMEX | ICM_ATTR_TRACE_ICOUNT | ICM_ATTR_RELAXED_SCHED, attributesForPA() )
#else
      pa ( "pa", 0, MURAC_PA_MODEL_FILE, ICM_ATTR_SIMEX | ICM_ATTR_TRACE_ICOUNT | ICM_ATTR_RELAXED_SCHED, attributesForPA() )
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
    pa_bus.setDecode(1, 0x00000000, 0x000FFFFF);

    // AA bus master
    aa.memory_read.socket(aa_bus.target_socket[0]);
    aa.memory_write.socket(aa_bus.target_socket[1]);

    // AA bus slaves
    aa_bus.initiator_socket[0](aa_memory.sp1);
    aa_bus.setDecode(0, 0xFFF00000, 0xFFFFFFFF);

    aa_bus.initiator_socket[1](shared_bus.target_socket[1]);
    aa_bus.setDecode(1, 0x00000000, 0x000FFFFF);

    // Share memory bridge
    shared_bus.initiator_socket[0](shared_memory.sp1);
    shared_bus.setDecode(0, 0x00000000, 0x000FFFFF);

    // Interrupts
    pa.brarch( aa.brarch );
    aa.intRetArch( pa.fiq );
}

int sc_main (int argc, char *argv[]) {

    const char *exe = "defaultFile";
    sc_time stop(1000,SC_MS);


    if (argc == 2) {
        exe = argv[1];
    } else {
        cout << "Usage: murac_tlm <application>" << endl;
        cout << "       Please specify application" << endl;
        return 0;
    }

    sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);

    // Ignore some of the Warning messages
    icmIgnoreMessage ("ICM_NPF");

    cout << "Running MURAC TLM platform simulator" << endl;

    MuracPlatform murac("murac");
    unsigned char *targetPtr = murac.shared_memory.getMemory()->get_mem_ptr();
    murac.pa.loadNativeMemory(targetPtr, 0x100000, 0x00000000, "mem_shared", exe, 0, 1, 1);

    // Specify the debug processor.
    murac.pa.debugThisProcessor();

    // Start the simulation
    cout << "Starting sc_main." << endl;
    sc_core::sc_start(stop);
    cout << "Finished sc_main." << endl;
    return 0;
}
