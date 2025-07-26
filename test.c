#include <stdio.h>
#include <stdlib.h>

#include "sha3_224.c"
#include "sha3_opt_224.c"

// Want to optimise the code for sha3 and remove anything that won't work in vivado
// Need to check that the optimised version outputs the same hash as FIPS and our original version

int main() {

    const unsigned char* input = (const unsigned char*) "12345678901234567890123456789012345678901234567890123456";
    unsigned char* fips_output = calloc(28, sizeof(char));
    unsigned char* optimised_output = calloc(28, sizeof(char));

    FIPS202_SHA3_224(input, 56, fips_output);
    SHA3_OPT_224(input, optimised_output);

    for (int i = 0; i < 28; i++) {
        if (fips_output[i] != optimised_output[i]) {
            printf("ERROR!!! OPTIMISED OUTPUT PRODUCING WRONG RESULT\n");
            return -1; 
        }
    }

    printf("SUCCESS!!!!\n");
    printf("HASH OUTPUT: ");
    for (int i = 0; i < 28; i++) {
        printf("%X", optimised_output[i]);
    }
    printf("\n");
    
    return 0;
}