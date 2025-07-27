#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Declare the top-level HLS function
extern void SHA3_OPT_224(const unsigned char input[56], unsigned char output[28]);

int main() {
    unsigned char input[] = "12345678901234567890123456789012345678901234567890123456";
    unsigned char output[28];
    int k;

    // Call the HLS function
    SHA3_OPT_224(input, output);

    // Write output to file
    FILE *fp_out = fopen("output.dat", "w");
    if (!fp_out) {
        perror("Failed to open output file");
        return 1;
    }

    for (k = 0; k < 28; k++) {
        fprintf(fp_out, "%X", output[k]);
    }

    fclose(fp_out);

    // Compare against golden output file
    int diff_result = system("diff -w output.dat golden.dat");

    if (diff_result != 0) {
        printf("*******************************************\n");
        printf("FAIL: Output does NOT match golden output\n");
        printf("*******************************************\n");
        return 1;
    } else {
        printf("*******************************************\n");
        printf("PASS: Output matches golden output!\n");
        printf("*******************************************\n");
        return 0;
    }
}
