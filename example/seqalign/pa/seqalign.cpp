/**
 * MURAC Test Application - Smith-Waterman Sequence Alignment
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "../aa/embed/seqalign.h"
#include "../../../framework/murac.h"

void printdata(const char *data) {
    for (int i = 0; i < 16; i++) {
        printf(" 0x%x", data[i]);
    }
    printf("\n");
}
int main(void) {

    printf("[PA] Sequence Aligment example...\n");
    
    unsigned char *input = (unsigned char*) malloc(4);
    unsigned char *output = (unsigned char*) malloc(1);

    unsigned int *vars = (unsigned int *) malloc(2*sizeof(unsigned int));
    vars[0] = (unsigned int) input;
    vars[1] = (unsigned int) output;

    MURAC_SET_PTR(vars);

    EXECUTE_SEQALIGN
    
    printf("[PA] Sequence Alignment result: ");
    printf(*output > 0 ? "MATCH\n" : "NO MATCH\n");
   // printdata(encrypt_output);
    printf("[PA] Example finished...\n");

    free(input);
    free(output);
    free(vars);

    return 0; 
}
