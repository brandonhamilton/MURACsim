/**
 * MURAC Test Application - AES 128 bit cryptography
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../aa/embed/aes128.h"
#include "../../../framework/murac.h"

void printdata(const char *data) {
    for (int i = 0; i < 16; i++) {
        printf(" 0x%x", data[i]);
    }
    printf("\n");
}
int main(void) {

    printf("[PA] MURAC AES-128 example...\n");
    
    char *key   = (char *) malloc(16);
    sprintf(key, "mysimpletestkey!");
    char *input = (char *) malloc(16);
    sprintf(input, "random inputdata");
    char *encrypt_output = (char *) malloc(16);
    char *decrypt_output = (char *) malloc(16);
    memset(decrypt_output, 0, 16);

    /*printf("Input data is located at: 0x%x\n", input);

    printf("Encryption key: ");
    printdata(key);
    printf("Encryption input: ");
    printdata(input);
    */

    unsigned int *vars = (unsigned int *) malloc(4*sizeof(unsigned int));
    vars[0] = (unsigned int) key;
    vars[1] = (unsigned int) input;
    vars[2] = (unsigned int) encrypt_output;
    vars[3] = (unsigned int) decrypt_output;

    MURAC_SET_PTR(vars)

    EXECUTE_AES128
    
    printf("[PA] Encryption output: ");
    printdata(encrypt_output);
    printf("[PA] Example finished...\n");

    free(key);
    free(input);
    free(encrypt_output);
    free(decrypt_output);
    free(vars);

    return 0; 
}
