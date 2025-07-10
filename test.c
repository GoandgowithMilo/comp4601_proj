#include <stdio.h>
#include <stdlib.h>

#include "sha3_224.c"

int main() {

    const unsigned char* input = (const unsigned char*) "AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHHIIIIJJJJKKKKLLLLMMMMNNNN";
    unsigned char* fips_output = calloc(28, sizeof(char));
    unsigned char* output = calloc(28, sizeof(char));

    FIPS202_SHA3_224(input, 56, fips_output);
    SHA3_224(input, output);

    printf("FIPS hash: ");
    for (int i = 0; i < 28; i++) {
        printf("%X", fips_output[i]);
    }
    printf("\n");

    printf("Our hash: ");
    for (int i = 0; i < 28; i++) {
        printf("%X", output[i]);
    }
    printf("\n");
    
    return 0;
}