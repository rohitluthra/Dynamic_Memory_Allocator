#include <stdio.h>
#include "sfmm.h"
#include "debug.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    double* ptr = sf_malloc(400);
    double* ptr1 = sf_malloc(88);
    double* ptr2 = sf_malloc(880);

    *ptr = 320320320e-320;
    *ptr1 = 320320320e-320;

    printf("%f\n", *ptr);

    sf_free(ptr1);
    sf_free(ptr2);
    sf_free (ptr);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
