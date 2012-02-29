/**
 * MURAC Test Application
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "../aa/embed/simple.h"
#include "../../../framework/murac.h"

int main(void) {

    printf("[PA] Starting Simple example...\n");
    printf("[PA] Branching to another architecture...\n");
    
    unsigned int* counter = (unsigned int*) malloc(sizeof(unsigned int));

    *counter = 0;

    MURAC_SET_PTR(counter)

    EXECUTE_SIMPLE

    printf("[PA] Final counter value = %d\n", *counter);
    printf("[PA] Exiting from application...\n");

    return 0; 
}
