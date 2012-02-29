/**
 * MURAC Memory Access Application
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "../aa/embed/mem_access.h"
#include "../../../framework/murac.h"

int main(void) {

    printf("[PA] Starting application\n");

    EXECUTE_MEM_ACCESS

    printf("[PA] Terminating application\n");
    return 0; 
}
