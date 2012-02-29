/**
 * MURAC Test Application
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../aa/embed/matrix_multiply.h"
#include "../../../framework/murac.h"

#define N 3

int main(void) {

    printf("[PA] Matrix multiplication example...\n");
    
    int *m1 = (int *) malloc(N*N*sizeof(int));//0xCF0FF000;//
    int *m2 = (int *) malloc(N*N*sizeof(int));//0xCF0FF024;//malloc(N*N*sizeof(int));
    int *r  = (int *) malloc(N*N*sizeof(int));//0xCF0FF048;//malloc(N*N*sizeof(int));
    memset(r, 0, N*N*sizeof(int));

    unsigned int* stack = (unsigned int*) malloc(N*N*sizeof(int));//0xCF0FF100;//malloc(3*sizeof(unsigned int));
    stack[0] = (unsigned int) m1;
    stack[1] = (unsigned int) m2;
    stack[2] = (unsigned int) r;

    for (int i = 0; i < N*N; i++) {
        m1[i] = i + 1;
        m2[i] = N - i;
    }

    MURAC_SET_PTR(stack)

    EXECUTE_MATRIX_MULTIPLY

    printf("Matrix multiplication result...\n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d ", r[j + i*N]);
        }
        printf("\n");
    }
    fflush(stdout);

    free(m1);
    free(m2);
    free(r);
    free(stack);
    return 0; 
}
