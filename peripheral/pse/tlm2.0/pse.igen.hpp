// MODEL IO:
//    Slave Port busPort1
//    Slave Port busPort2

#ifndef PERIPHERAL_MURACFPGA_1_0
#define PERIPHERAL_MURACFPGA_1_0
#include "ovpworld.org/modelSupport/tlmPeripheral/1.0/tlm2.0/tlmPeripheral.hpp"
using namespace sc_core;

class muracFPGA : public icmPeripheral
{
  private:
    const char *getModel() {
        return icmGetVlnvString (NULL, "mips.ovpworld.org", "peripheral", "MaltaFPGA", "1.0", "pse");
    }

  public:
    icmMasterPort    memory_read;
    icmMasterPort    memory_write;
    icmInputNetPort  brarch;
    icmOutputNetPort intRetArch;

    muracFPGA(sc_module_name name, const char *model_file, icmAttrListObject *initialAttrs = 0 )
        : icmPeripheral(name, model_file ? model_file : getModel(), 0, initialAttrs)
        , memory_read(this, "fpga_memread", 32)
        , memory_write(this, "fpga_memwrite", 32)
        , brarch(this, "fpga_brarch")
        , intRetArch(this, "fpga_retarch")
    {
    }

}; /* class MuracFPGA */

#endif
