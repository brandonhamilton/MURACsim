/**
 * MORAC processor generator
 * Author: Brandon Hamilton
 */

#ifndef PROCESSOR_MORAC_ARM_1_0
#define PROCESSOR_MORAC_ARM_1_0
#include "ovpworld.org/modelSupport/tlmProcessor/1.0/tlm2.0/tlmProcessor.hpp"
using namespace sc_core;

class morac_arm : public icmCpu
{
  private:
    const char *getModel() {
        return icmGetVlnvString (NULL, "arm.ovpworld.org", "processor", "arm", "1.0", "model");
    }

    const char *getSHL() {
        return icmGetVlnvString (NULL, 0, 0, "armNewlib", 0, "model");
    }

  public:
    icmCpuMasterPort     INSTRUCTION;
    icmCpuMasterPort     DATA;
    icmCpuInterrupt      fiq;
    icmCpuInterrupt      irq;
    icmCpuInterrupt      reset;
    icmCpuInterrupt      pabort;
    icmCpuInterrupt      dabort;
    icmCpuOutputNetPort  nDMAIRQ;
    icmCpuOutputNetPort  brarch;

    morac_arm(
        sc_module_name        name,
        const unsigned int    ID,
        const char           *model_file,
        icmNewProcAttrs       attrs    = ICM_ATTR_DEFAULT,
        icmAttrListObject    *attrList = NULL,
        const char           *semiHost = NULL
     )
    : icmCpu(name, ID, "morac_arm", model_file ? model_file : getModel(), "modelAttrs", semiHost ? semiHost : getSHL(), attrs, attrList)
    , INSTRUCTION (this, "INSTRUCTION", 32)
    , DATA (this, "DATA", 32)
    , fiq("fiq", this)
    , irq("irq", this)
    , reset("reset", this)
    , pabort("pabort", this)
    , dabort("dabort", this)
    , nDMAIRQ("nDMAIRQ", this)
    , brarch("brarch", this)
    {
        INSTRUCTION.busError(&pabort);
        DATA.busError(&dabort);
    }

}; /* class arm */

#endif
