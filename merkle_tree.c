#include <stdio.h>
#include "sha3_224.c"

// #define DATA_SIZE 1433600000 // ~1.4Gb - this is 1000000 sets of 32 blocks
// #define DATA_SIZE 1433600 // ~1.4Mb - this is 100 sets of 32 blocks
#define DATA_SIZE 1792 // this fits perfectly into 32 blocks
// #define LEAF_COUNT 32
// #define LEAF_COUNT 16
#define LEAF_COUNT 8
#define BLOCK_SIZE 56 // each block stores up to 448 bits (56 bytes)

// With a data size of 1792, all the block sizes respond correctly to changing the sentinel value
// None of them seem to respond correctly to changes in the sentinel value for a bigger data size
// Need to debug this! Why is this not responding to data of bigger sizes?

int main() {
    static unsigned char raw_data[DATA_SIZE] = {0};
    raw_data[DATA_SIZE - 1] = 1; // sentinel value to check we're hashing everything
    static unsigned char hashed_data[DATA_SIZE/2];
    unsigned long int data_processed = 0;

    // Read in all the data to leaf nodes and hash them
    while (data_processed != DATA_SIZE) {
        for (int i = 0; i < LEAF_COUNT; i++) {
            SHA3_224(&raw_data[data_processed + i * BLOCK_SIZE], &hashed_data[data_processed/2 + i * BLOCK_SIZE/2]);
        }

        for (int j = 0; j < BLOCK_SIZE/2; j++) {
            printf("%02x", hashed_data[j]); // TODO - wonder if my indexing maths is correct...
        }

        data_processed += (LEAF_COUNT * BLOCK_SIZE);
    }

    // We now have to pair adjacent nodes and hash them
    int blocks_to_process = (data_processed / BLOCK_SIZE) / 2;
    
    while (blocks_to_process != 0) {
        for (int i = 0; i < blocks_to_process; i++) {
            SHA3_224(&hashed_data[i*BLOCK_SIZE], &hashed_data[i*(BLOCK_SIZE/2)]);
        }

        blocks_to_process = blocks_to_process/2;
    }

    // print out hashed data
    for (int j = 0; j < BLOCK_SIZE/2; j++) {
        printf("%02x", hashed_data[j]);
    }
    printf("\n");
}

struct hash_t {
    uint8_t bytes[28];
};

// leaf_nodes == total number in the tree is data_size
// make an upper bound --> 56 * 16^n --> 6 levels in the tree
// Break everything into separate functions to be able to separate concerns (no return in any subfunction -- void)
// Inputs and outputs must be streams -- do this after



hash_t merkle_tree(hls::stream<ap_uint<224>> &in, uint32_t leaf_nodes) {

}

/*
    For testing this ^^

    C++ testbench
    Make stream object in test file --> pass that to th efunction call
    We write to the input stream in the c++ file (whole data set)

*/