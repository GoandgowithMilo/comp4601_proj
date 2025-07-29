#include <iostream>
#include <vector>

#include "sha3_256.cpp"
#include "../Keccak-readable-and-compact.c"

int main() {
    std::string input = "0123456789012345678901234567890123456789012345678901234567890123";
    unsigned char * fips_output = new unsigned char[32];
    unsigned char * output = new unsigned char[32];

    FIPS202_SHA3_256(reinterpret_cast<const unsigned char*>(input.c_str()), 64, fips_output);
    sha3_opt_256(reinterpret_cast<const unsigned char*>(input.c_str()), output);

    for (int i = 0; i < 32; i++) {
        if (fips_output[i] != output[i]) {
            printf("ERROR!!! OPTIMISED OUTPUT PRODUCING WRONG RESULT\n");
            return -1; 
        }
    }

    printf("SUCCESS!!!!\n");
    printf("HASH OUTPUT: ");
    for (int i = 0; i < 32; i++) {
        printf("%X", output[i]);
    }
    printf("\n");
    
    return 0;
}