/**
 * Murac Auxilliary Architecture integration framework
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 */

#include <systemc.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "muracAA.hpp"


typedef int (*murac_init_func)(BusInterface*);
typedef int (*murac_exec_func)(unsigned long int);

using std::cout;
using std::endl;
using std::dec;
using std::hex;

SC_HAS_PROCESS( muracAA );

muracAAInterupt::muracAAInterupt(const char *name, muracAA *aa):
  m_aa(aa),
  m_name(name) {
  
}

void muracAAInterupt::write(const int &value) {
    if (value == 1) {
        m_aa->onBrArch(value);
    }
}

/** 
 * Constructor
 */
muracAA::muracAA( sc_core::sc_module_name  name) :
  sc_module( name ),
  brarch("brarch", this) {
  
}

int muracAA::loadLibrary(const char *library) {
    cout << "Loading murac library: " << library << endl;
    void* handle = dlopen(library, RTLD_NOW | RTLD_GLOBAL); 
    if (!handle) {
      cout << dlerror() << endl;
      return -1;
    }
    dlerror();
    cout << "Initializing murac library: " << library << endl;
    murac_init_func m_init = (murac_init_func) dlsym(handle, "murac_init");
    int result = m_init(this);
    return result;
}

int muracAA::invokePluginSimulation(const char* path, unsigned long int ptr) {
    char *error;
    void* handle = dlopen(path, RTLD_LAZY | RTLD_GLOBAL); 
    if (!handle) {
      cout << dlerror() << endl;
      return -1;
    }

    dlerror();

    murac_exec_func m_exec = (murac_exec_func) dlsym(handle, "murac_execute");
    if ((error = dlerror()) != NULL) {
      fprintf(stderr, "%s\n", error);
      return -1;
    }

    int result = -1;
   
    sc_process_handle h = sc_spawn(&result, sc_bind(m_exec, ptr)  );
    wait(h.terminated_event());

    //dlclose(handle);

    return result;
}

/**
 * Handle the BrArch interrupt from the PA
 */
void muracAA::onBrArch(const int &value) {
    cout << "@" << sc_time_stamp() << " onBrArch" << endl;

    int ret = -1;
    int fd;
    unsigned long int pc = 0;
    unsigned int instruction_size = 0;
    unsigned long int ptr = 0;
    unsigned char* fmap;
    char *tmp_file_name = strdup("/tmp/murac_AA_XXXXXX");

    if (busRead(MURAC_PC_ADDRESS, (unsigned char*) &pc, 4) < 0) {
      cout << "@" << sc_time_stamp() << " Memory read error !" << endl;
      goto trigger_return_interrupt;
    }

    cout << "@" << sc_time_stamp() << " PC: 0x" << hex << pc << endl;

    if (busRead(MURAC_PC_ADDRESS + 4, (unsigned char*) &instruction_size, 4) < 0) {
      cout << "@" << sc_time_stamp() << " Memory read error !" << endl;
      goto trigger_return_interrupt;
    }
    cout << "@" << sc_time_stamp() << " Instruction size: " << dec << instruction_size << endl;

    if (busRead(MURAC_PC_ADDRESS + 8, (unsigned char*) &ptr, 4) < 0) {
      cout << "@" << sc_time_stamp() << " Memory read error !" << endl;
      goto trigger_return_interrupt;
    }
    cout << "@" << sc_time_stamp() << " Ptr : " << hex << ptr << dec << endl;

    cout << "@" << sc_time_stamp() << " Reading embedded AA simulation file " << endl;

    fd = mkostemp (tmp_file_name, O_RDWR | O_CREAT | O_TRUNC);
    if (fd == -1) {
      cout << "Error: Cannot open temporary file for writing." << endl;
      goto trigger_return_interrupt;
    }
    ret = lseek(fd, instruction_size-1, SEEK_SET);
    if (ret == -1) {
      close(fd);
      cout << "Error: Cannot call lseek() on temporary file." << endl;
      goto trigger_return_interrupt;
    }
    ret = ::write(fd, "", 1);
    if (ret != 1) {
      close(fd);
      cout << "Error: Error writing last byte of the temporary file." << endl;
      goto trigger_return_interrupt;
    }

    fmap = (unsigned char*) mmap(0, instruction_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (fmap == MAP_FAILED) {
      close(fd);
      cout << "Error: Error mmapping the temporary file." << endl;
      goto trigger_return_interrupt;
    }

    // Write the file
    if (busRead(pc, fmap, instruction_size) < 0) {
      cout << "@" << sc_time_stamp() << " Memory read error !" << endl;
      munmap(fmap, instruction_size);
      close(fd);
      goto trigger_return_interrupt;
    }

    if (munmap(fmap, instruction_size) == -1) {
      close(fd);
      cout << "Error: Error un-mmapping the temporary file." << endl;
      goto trigger_return_interrupt;
    }

    close(fd);

    cout << "@" << sc_time_stamp() << " Running murac AA simulation " << endl;
    ret = invokePluginSimulation(tmp_file_name, ptr);
    cout << "@" << sc_time_stamp() << " Simulation result = " << ret << endl;

    remove(tmp_file_name);
    
  trigger_return_interrupt:

    free(tmp_file_name);

    cout << "@" << sc_time_stamp() << " Returning to PA " << endl;

    // Trigger interrupt for return to PA
    intRetArch.write(1);
    intRetArch.write(0);
}

/**
 * Read from the bus
 */
int muracAA::busRead (unsigned long int  addr,
                      unsigned char      rdata[],
                      int                dataLen) {

    bus_payload.set_read ();
    bus_payload.set_address ((sc_dt::uint64) addr);

    bus_payload.set_byte_enable_length ((const unsigned int) dataLen);
    bus_payload.set_byte_enable_ptr (0);

    bus_payload.set_data_length ((const unsigned int) dataLen);
    bus_payload.set_data_ptr ((unsigned char *) rdata);

    busTransfer(bus_payload);
    return bus_payload.is_response_ok () ? 0 : -1;
}

/**
 * Write to the bus
 */
int muracAA::busWrite (unsigned long int  addr,
                       unsigned char      wdata[],
                       int                dataLen) {

    bus_payload.set_write ();
    bus_payload.set_address ((sc_dt::uint64) addr);

    bus_payload.set_byte_enable_length ((const unsigned int) dataLen);
    bus_payload.set_byte_enable_ptr (0);

    bus_payload.set_data_length ((const unsigned int) dataLen);
    bus_payload.set_data_ptr ((unsigned char *) wdata);

    busTransfer(bus_payload);
    return bus_payload.is_response_ok () ? 0 : -1;
}

/**
 * Initiate bus transfer
 */
void muracAA::busTransfer( tlm::tlm_generic_payload &trans ) {
    sc_core::sc_time dummyDelay = sc_core::SC_ZERO_TIME;
    aa_bus->b_transport( trans, dummyDelay );
}


int muracAA::read(unsigned long int addr, unsigned char*data, unsigned int len) {
    return busRead(addr, data, len);
}

int muracAA::write(unsigned long int addr, unsigned char*data, unsigned int len) {
    return busWrite(addr, data, len);  
}
