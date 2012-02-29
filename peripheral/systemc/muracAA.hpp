/**
 * Murac FPGA peripheral HDL integration framework
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 */

#ifndef MURAC_AA_H
#define MURAC_AA_H

#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "../../framework/murac.h"

#define MURAC_PC_ADDRESS 0xCF000000

class muracAA;

class muracAAInterupt: public tlm::tlm_analysis_if<int> {
  public:
      muracAAInterupt(const char *name, muracAA *aa);
      void write(const int &value);

  private:
      muracAA      *m_aa;
      const char   *m_name;
};

class muracAA: public sc_core::sc_module, BusInterface {
    public:
        muracAA (sc_core::sc_module_name  name);
        tlm_utils::simple_initiator_socket<muracAA> aa_bus;
        
        /* BrArch interrupt from the PA */
        muracAAInterupt brarch;

        /* RetArch interrupt to signal return to PA */
        tlm::tlm_analysis_port<int>  intRetArch;

        /* Handle BrArch interrupt */
        void onBrArch(const int &value);
    
        /* Bus interface */
        int read(unsigned long int addr, unsigned char*data, unsigned int len);
        int write(unsigned long int addr, unsigned char*data, unsigned int len);

        int loadLibrary(const char *library);
        
    private:

        /* Bus transport payload */
        tlm::tlm_generic_payload bus_payload;

        /* Read from the bus */
        int busRead (unsigned long int  addr,
                     unsigned char      rdata[],
                     int                dataLen);

        /* Write to the bus */
        int busWrite (unsigned long int  addr,
                      unsigned char      wdata[],
                      int                dataLen);

        /* Initiate bus transfer */
        void busTransfer(tlm::tlm_generic_payload &trans);

        int invokePluginSimulation(const char* path, unsigned long int ptr);
};

#endif  // MURAC_AA_H