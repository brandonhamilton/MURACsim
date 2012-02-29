
#include "mem_access.hpp"
#include "../../../framework/murac.h"

using std::cout;
using std::endl;

class _mem_access {
public:
    _mem_access(): bus(0) {  }
    sc_signal<bool>   clock;
    sc_signal<bool>   reset;
    sc_signal<bool>   enable;
    //sc_signal<sc_uint<4> > counter_out;
    //test_counter counter;
    BusInterface *bus;
};

static _mem_access mem_access;

MURAC_AA_INIT(mem_access_init) {
    mem_access.bus = bus;
}

int run_mem_access_simulation(unsigned long int stack) {
    cout << "[AA] Executing memory access simulation" << endl;
    return 1;
}