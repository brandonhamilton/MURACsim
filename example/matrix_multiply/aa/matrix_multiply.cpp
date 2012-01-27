#include <iostream>
#include <systemc.h>
#include "../../../framework/morac.h"

using std::cout;
using std::endl;

#define N 3

static BusInterface *bus = 0;

MORAC_AA_INIT(matrix_multiply_init) {
    ::bus = bus;
}

MORAC_AA_EXECUTE(matrix_multiply_exec) {

    // Read paramters from stack
    unsigned int* vars = (unsigned int*) malloc(3*sizeof(unsigned int));
    if (bus->read(stack, (unsigned char*) vars, 3*sizeof(unsigned int))) {
        printf("Error reading from bus: 0x%x\n", stack);
        return -1;
    }

    int *m1 = (int *) malloc(N*N*sizeof(int));
    if (bus->read(vars[0], (unsigned char*) m1, N*N*sizeof(int))) {
        printf("Error reading from bus 0x%x\n", vars[0]);
        return -1;
    }

    int *m2 = (int *) malloc(N*N*sizeof(int));
    if (bus->read(vars[1], (unsigned char*) m2, N*N*sizeof(int))) {
        printf("Error reading from bus 0x%x\n", vars[1]);
        return -1;
    }
    int *r  = (int *) malloc(N*N*sizeof(int));
    memset(r, 0, N*N*sizeof(int));

    int i = 0;
    int j = 0;
    int k = 0;
    for(i = 0; i < N; i++) 
       for( j = 0; j < N; j++)
           for( k = 0; k < N; k++) {
               r[i + j*N] +=  m1[i + k*N] * m2[k + j*N];
            }

    if (bus->write(vars[2], (unsigned char*) r, N*N*sizeof(int))) {
        printf("Error writing to bus 0x%x\n", vars[2]);
        return -1;
    }

    free(m1);
    free(m2);
    free(r);
    free(vars);

    return 1;
}