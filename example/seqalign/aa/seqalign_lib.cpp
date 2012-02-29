/**
 * MURAC Test Application - Smith-Waterman Sequence Alignment
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 *
 * Adapted from the DNA Sequence Alignment Accelerator project on OpenCores.org
 * http://opencores.org/project,seqalign
 *
 * To operate the block, a query value (a packed array containing encoded 2-bit
 * nucleotides) and length (in nucleotides) should be set as inputs. Then, a reset
 * pulse should be sent down the array. Then, the i_vld (valid) signal should be 
 * held high as the comparison string is clocked in, two bits at a time, until 
 * the string is completed and the i_vld signal is lowered. When the o_vld signal
 * goes low, the array is done processing and the result is ready. If the "local" 
 * bit is high, the result will be on the "o_high" output of the last element. If
 * the "local" bit is low (global alignment mode), the valid result will be the 
 * higher of the two "o_right_*" outputs of the last element.
 *
 */
#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <iostream>
#include <systemc.h>
#include "../../../framework/murac.h"

#include "sw_gen_affine.h"

#define LOG_LEN 3
#define LEN 4
#define SCORE_WIDTH 11

#define N_A 0        //nucleotide "A"
#define N_G 1        //nucleotide "G"
#define N_T 2        //nucleotide "T"
#define N_C 3        //nucleotide "C"

#define WAIT_CYCLES(x) for (int i = 0; i < x; i++) wait(invoker->seqalign_clock.posedge_event());

class seqalign_invoker {

public:

    sc_clock seqalign_clock;
    sc_signal<bool> reset;
    sc_signal<sc_uint<LOG_LEN> > query_length;
    sc_signal<bool> local;
    sc_signal<sc_uint<(LEN * 2)> > query;
    sc_signal<bool> input_valid;
    sc_signal<sc_uint<2> > data;
    sc_signal<bool> output_valid;
    sc_signal<sc_uint<(SCORE_WIDTH)> > result;

    sw_gen_affine<LOG_LEN,LEN>* seq_obj;

    seqalign_invoker(): seqalign_clock("seqalign_clock", 1, SC_MS) {

        seq_obj = new sw_gen_affine<LOG_LEN,LEN>("seqalign");
        seq_obj->clk(seqalign_clock);
        seq_obj->rst(reset);
        seq_obj->i_query_length(query_length);
        seq_obj->i_local(local);
        seq_obj->query(query);
        seq_obj->i_vld(input_valid);
        seq_obj->i_data(data);
        seq_obj->o_vld(output_valid);
        seq_obj->m_result(result);
    }
};

static BusInterface* bus;
static seqalign_invoker* invoker;

MURAC_AA_INIT(seqalign) {
    printf("[AA] Initialising Sequence Alignment example\n");
    ::bus = bus;
    ::invoker = new seqalign_invoker();
    return 0;
}

unsigned char do_sequence() {

    printf("[AA] Starting Sequence Alignment\n");
    invoker->reset.write(0);
    WAIT_CYCLES(1)
    printf("[AA] Reseting PEs\n");
    invoker->reset.write(1);
    invoker->input_valid.write(0);
    invoker->data.write(0);
    invoker->local.write(1);
    WAIT_CYCLES(2)
    invoker->reset.write(0);
    WAIT_CYCLES(10)
    printf("[AA] Writing data\n");
    invoker->data = N_G;
    invoker->input_valid = 1;
    
    WAIT_CYCLES(2)
    invoker->data = N_G;
    WAIT_CYCLES(2)
    invoker->data = N_G;
    WAIT_CYCLES(2)
    invoker->data = N_C;

    WAIT_CYCLES(2)

    printf("[AA] Waiting for result\n");
    invoker->input_valid = 0;
    WAIT_CYCLES(20)

    int timeout = 20;
    while (timeout > 0 && invoker->output_valid.read() > 0) {
        WAIT_CYCLES(1)
        timeout--;
    }

    if (timeout <= 0) {
        printf("[AA] Error, sequence did not complete...\n");
        return 0;
    }

    return invoker->result.read();
}

#define NUMBER_OF_INPUT_VARS 2

int run_seqalign_simulation(unsigned long int stack) {

    /* Read the stack*/
    unsigned int variables[NUMBER_OF_INPUT_VARS];
    printf("[AA] Reading addresses from bus at 0x%x\n", stack);
    if (bus->read(stack, (unsigned char*) variables, NUMBER_OF_INPUT_VARS*sizeof(unsigned int))) {
        printf("[AA] Error reading fom bus 0x%x\n", stack);
        return -1;
    }
    for (int i = 0; i < NUMBER_OF_INPUT_VARS; i++) {
        printf("[AA]    Memory address[%d] = 0x%x\n", i, variables[i]);
    }

    invoker->query_length = LOG_LEN;
    invoker->query = (N_G, N_C, N_C, N_C);

    unsigned char result = do_sequence();

    /* Output */
    if (bus->write(variables[1], &result, 1)) {
        printf("[AA] Error writing to bus 0x%x\n", variables[1]);
        return -1;
    } 
    return 0;
}
