/**
 * MURAC Test Application - AES 128 bit cryptography
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 *
 */
#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <iostream>
#include <systemc.h>
#include "../../../framework/murac.h"

#include "aes.h"

//static aes_transactor *tr;
//static adapter *ad1;
//static sc_clock clk("clk");

class aes_invoker {

public:
    sc_clock aes_clock;
    sc_signal<bool> reset;
    sc_signal<bool> do_load;
    sc_signal<bool> mode_decrypt;
    sc_signal<sc_biguint<128> > data_in;
    sc_signal<sc_biguint<128> > key;
    sc_signal<sc_biguint<128> > data_out;
    aes *crypt_obj;

    sc_signal<bool> ready;

    aes_invoker(): aes_clock("aes_clock", 1, SC_MS) {

        crypt_obj = new aes("aes");
        crypt_obj->clk(aes_clock);
        crypt_obj->reset(reset);
        crypt_obj->load_i(do_load);
        crypt_obj->decrypt_i(mode_decrypt);
        crypt_obj->data_i(data_in);
        crypt_obj->key_i(key);
        crypt_obj->data_o(data_out);
        crypt_obj->ready_o(ready);
    }
};

static aes_invoker *invoker;
static BusInterface* bus;


MURAC_AA_INIT(aes128) {
    ::bus = bus;
    invoker = new aes_invoker();
    return 0;
}

void reset(void) {
    invoker->reset.write(0);
    wait(invoker->aes_clock.posedge_event());
    invoker->reset.write(1);
}

void encrypt(sc_biguint<128> k, sc_biguint<128> d) {
    wait(invoker->aes_clock.posedge_event());
    invoker->do_load.write(1);
    invoker->data_in.write(d);
    invoker->key.write(k);
    invoker->mode_decrypt.write(0);
    cout << "@" << sc_time_stamp() << " Loading data" << endl;
    wait(invoker->aes_clock.posedge_event());
    cout << "@" << sc_time_stamp() << " Encrypting data" << endl;
    invoker->do_load.write(0);
    wait(invoker->ready.posedge_event());
}

int run_aes_simulation(unsigned long int stack) {
    
    /* Read the stack */
    unsigned int variables[4];
    printf("Reading addresses from bus at 0x%x\n", stack);
    if (bus->read(stack, (unsigned char*) variables, 4*sizeof(unsigned int))) {
        printf("Error reading fom bus 0x%x\n", stack);
        return -1;
    }
    for (int i = 0; i < 4; i++) {
        printf("   Memory address[%d] = 0x%x\n", i, variables[i]);
    }

    /* Key */
    char key_data[16];
    if (bus->read(variables[0], (unsigned char*) key_data, 16)) {
        printf("Error reading fom bus 0x%x\n", variables[0]);
        return -1;
    }

    /* Input */
    char input_data[16];
    printf("Reading 16 bytes from bus at 0x%x\n", variables[1]);
    if (bus->read(variables[1], (unsigned char*) key_data, 16)) {
        printf("Error reading fom bus 0x%x\n", variables[1]);
        return -1;
    }

    for (int i = 0; i < 16; i++) {
        printf(" 0x%x", (int) input_data[i] & 0xFF);
    }
    printf("\n");

    sc_biguint<128> test_key = ( 
        *((unsigned int*) &key_data[0]),
        *((unsigned int*) &key_data[4]),
        *((unsigned int*) &key_data[8]),
        *((unsigned int*) &key_data[12]) );
    sc_biguint<128> test_data = ( 
        *((unsigned int*) &input_data[0]),
        *((unsigned int*) &input_data[4]),
        *((unsigned int*) &input_data[8]),
        *((unsigned int*) &input_data[12]) );
    
    cout << "@" << sc_time_stamp() << " Running encrypt" << endl;



    reset();
    encrypt(test_key, test_data);
    
    cout << "@" << sc_time_stamp() << " Writing result to memory" << endl;

    sc_biguint<128> test_output = invoker->data_out.read();
    char output_data[16];

    *((unsigned int *) &output_data[0]) = test_output.range(0,31).to_uint();
    *((unsigned int *) &output_data[4]) = test_output.range(32,63).to_uint();
    *((unsigned int *) &output_data[8]) = test_output.range(64,95).to_uint();
    *((unsigned int *) &output_data[12]) = test_output.range(96,127).to_uint();

    for (int i = 0; i < 16; i++) {
        printf(" 0x%x", (int) output_data[i] & 0xFF);
    }
    printf("\n");

    /* Output */
    if (bus->write(variables[2], (unsigned char*) output_data, 16)) {
        printf("Error writing to bus 0x%x\n", variables[2]);
        return -1;
    }
}