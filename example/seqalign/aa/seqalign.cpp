/**
 * MORAC Test Application - Smith-Waterman Sequence Alignment
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 *
 */

#include <iostream>
#include <systemc.h>
#include "../../../framework/morac.h"

using std::cout;
using std::endl;

extern int run_seqalign_simulation(unsigned long int stack);

MORAC_AA_EXECUTE(seqalign) {
    cout << "Running Sequence Alignment AA simulation" << endl;
    int r = run_seqalign_simulation(stack);
    return 1;
}