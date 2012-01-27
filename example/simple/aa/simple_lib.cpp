
#include "simple.hpp"
#include "../../../framework/morac.h"

using std::cout;
using std::endl;

SC_MODULE (test_counter) {
    sc_in_clk     clock ;      // Clock input of the design
    sc_in<bool>   reset ;      // active high, synchronous Reset input
    sc_in<bool>   enable;      // Active high enable signal for counter
    sc_out<sc_uint<4> > counter_out; // 4 bit vector output of the counter

    sc_uint<4>  count;

    void incr_count () {
        // At every rising edge of clock we check if reset is active
        // If active, we load the counter output with 4'b0000
        if (reset.read() == 1) {
            count =  0;
            counter_out.write(count);
        // If enable is active, then we increment the counter
        } else if (enable.read() == 1) {
            count = count + 1;
            counter_out.write(count);
            cout << "@" << sc_time_stamp() << " :: Incremented Counter " << counter_out.read() << endl;
        }
    } // End of function incr_count

    SC_CTOR(test_counter) {
        cout << "Executing new" << endl;
        SC_METHOD(incr_count);
            sensitive << reset;
            sensitive << clock.pos();
    }
};

class _simple {
public:
    _simple(): counter("COUNTER"), bus(0) {  }
    sc_signal<bool>   clock;
    sc_signal<bool>   reset;
    sc_signal<bool>   enable;
    sc_signal<sc_uint<4> > counter_out;
    test_counter counter;
    BusInterface *bus;
};

static _simple simple;

MORAC_AA_INIT(simple_init) {

    simple.bus = bus;
    
    // Connect the DUT
    simple.counter.clock(simple.clock);
    simple.counter.reset(simple.reset);
    simple.counter.enable(simple.enable);
    simple.counter.counter_out(simple.counter_out);
}


int run_simple_simulation(unsigned long int stack) {
    int i = 0;
    cout << "Executing simple simulation" << endl;

    sc_time step(100,SC_MS);

    simple.reset = 0;       // initial value of reset
    simple.enable = 1;      // initial value of enable
    for (i=0;i<5;i++) {
        simple.clock = 0; 
        wait(step);
        //sc_start(step);
        simple.clock = 1; 
        //sc_start(step);
        wait(step);
    }
    /*simple.reset = 1;    // Assert the reset
    cout << "@" << sc_time_stamp() <<" Asserting reset\n" << endl;
    for (i=0;i<10;i++) {
        simple.clock = 0; 
        //sc_start(step);
        wait(step);
        simple.clock = 1; 
        //sc_start(step);
        wait(step);
    }
    cout << "@" << sc_time_stamp() <<" De-Asserting Enable\n" << endl;
    simple.enable = 0; // De-assert enable*/
    unsigned int counter_value = simple.counter_out.read().value();
    cout << "@" << sc_time_stamp() <<" Terminating simulation of counter\n" << endl;
    //sc_start();

    if (simple.bus->write(stack, (unsigned char*) &counter_value, sizeof(unsigned int))) {
        printf("Error writing to bus 0x%x\n", stack);
        return -1;
    }

    return 1;
}