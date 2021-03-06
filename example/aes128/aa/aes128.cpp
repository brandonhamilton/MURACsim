/**
 * MURAC Test Application - AES 128 bit cryptography
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 *
 */

#include <iostream>
#include <systemc.h>
#include "../../../framework/murac.h"

using std::cout;
using std::endl;

extern int run_aes_simulation(unsigned long int stack);

MURAC_AA_EXECUTE(aes128) {
    cout << "[AA] Running AES AA simulation" << endl;
    run_aes_simulation(stack);
    return 1;
}