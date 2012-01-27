/**
 * MORAC Test Application
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "../aa/embed/simple.h"
#include "../../../framework/morac.h"

int main(void) {

    printf("Branching to another architecture...\n");
    
    unsigned int* counter = (unsigned int*) malloc(sizeof(unsigned int));

    *counter = 0;

    MORAC_SET_PTR(counter)

    EXECUTE_SIMPLE

    printf("Final counter value = %d\n", *counter);

    printf("Exiting from application...\n");
    return 0; 
}
