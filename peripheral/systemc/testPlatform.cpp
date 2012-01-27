#include "systemc.h"
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "moracAA.hpp"
#include "tlm_utils/simple_target_socket.h"

struct Memory: sc_module
{
    tlm_utils::simple_target_socket<Memory> socket;

    enum { SIZE = 256 };

    SC_CTOR(Memory) : socket("socket")
    {
        // Register callback for incoming b_transport interface method call
        socket.register_b_transport(this, &Memory::b_transport);

        // Initialize memory with random data
        for (int i = 0; i < SIZE; i++)
            mem[i] = 0xAA000000 | (rand() % 256);
    }

    // TLM-2 blocking transport method
    virtual void b_transport( tlm::tlm_generic_payload& trans, sc_time& delay )
    {
        tlm::tlm_command cmd = trans.get_command();
        sc_dt::uint64    adr = trans.get_address();
        unsigned char*   ptr = trans.get_data_ptr();
        unsigned int     len = trans.get_data_length();
        unsigned char*   byt = trans.get_byte_enable_ptr();

        adr -= MORAC_PC_ADDRESS;

        // Obliged to check address range and check for unsupported features,
        //   i.e. byte enables, streaming, and bursts
        // Can ignore DMI hint and extensions
        // Using the SystemC report handler is an acceptable way of signalling an error

        if (adr >= sc_dt::uint64(SIZE) || byt != 0 || len > 4)
          SC_REPORT_ERROR("TLM-2", "Target does not support given generic payload transaction");

        // Obliged to implement read and write commands
        if ( cmd == tlm::TLM_READ_COMMAND )
          memcpy(ptr, &mem[adr], len);
        else if ( cmd == tlm::TLM_WRITE_COMMAND )
          memcpy(&mem[adr], ptr, len);

        // Obliged to set response status to indicate successful completion
        trans.set_response_status( tlm::TLM_OK_RESPONSE );
    }

    int mem[SIZE];
};

struct DummyPA: sc_module
{
    class returnTrigger: public tlm::tlm_analysis_if<int> {
        public:
            returnTrigger(const char *name, DummyPA *pa):
                m_pa(pa), m_name(name) {
                
            }
            void write(const int &value) {
                cout << "@" << sc_time_stamp() << " returnTrigger::write - " << value << endl;  
            }

        private:
            DummyPA      *m_pa;
            const char   *m_name;
    };

    tlm::tlm_analysis_port<int>  triggerAA;
    returnTrigger                triggerPA;

    SC_CTOR(DummyPA):
        triggerPA("ret", this)
    {
        SC_THREAD(thread_process);
    }

    void thread_process()
    {
        sc_time delay = sc_time(10, SC_NS);
        cout << "@ " << sc_time_stamp()
           << " waiting for " << delay << endl;
        wait(delay);

        triggerAA.write(1);
    }
};

SC_MODULE(Top)
{
    moracAA aa;
    DummyPA pa;
    Memory  mem;

    SC_CTOR(Top):
        aa("morac_aa"),
        pa("morac_pa"),
        mem("memory")
    {
        aa.aa_bus.bind( mem.socket );
        pa.triggerAA( aa.intBrArch );
        aa.intRetArch( pa.triggerPA );
    }
};

int sc_main(int argc, char* argv[])
{
    Top top("top");
    sc_start();
    return 0;
}