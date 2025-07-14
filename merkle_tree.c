#include <stdio.h>
#include <assert.h>
#include "sha3_224.c"

/*
TODO -- stopped at working on how to update this to take in variable data sizes

// save back into memory
// 32 leaf nodes > 1 32*32 > 32 nodes hsah back 1

// Read up to block_size * leaf_block_count data into a buffer from the file
// Hash that using merkle tree structure to produce a single 224 string
// Append that to 

32 * 32 -> 1
32 * 32 -> 2
32 * 32 * 32 > 1
Maximum possible hash level --> choose a number --> store this in Bram

Stream in data into the leaf nodes
 >> load file into ram first --> loading from disk very slow
 >> data stream into leaf nodes at some rate (as fast as we can -- this is likely to be limiting factor instead of the compute). Probably max is 56 bytes per cycle but unlikely to get there with streaming.
*/

/* Start with the assumption that we have 32 blocks worth of data */
// TODO - replace divides with shifts
// TODO - seperate write/read buffers
#define BLOCKS 32
#define DATA_IN_BYTES 1792

int main() {
    const unsigned char data_to_hash[DATA_IN_BYTES] = {0}; // unhashed data
    unsigned char hashed_data[DATA_IN_BYTES / 2] = {0}; // each hash takes 56 bytes and produces 28 so at most this only needs to be half as big in this implementation
    int block_size_in_bytes = DATA_IN_BYTES / BLOCKS;
    int blocks_to_hash = BLOCKS;

    assert(block_size_in_bytes == 56); // this should always be 56 (448 bits) for our purposes

    // Hash all the lower blocks
    for (int i = 0; i < blocks_to_hash; i++) { // for each of the data blocks
        SHA3_224(&data_to_hash[i*block_size_in_bytes], &hashed_data[i*(block_size_in_bytes/2)]);
    }

    // // Test printing
    // for (int i = 0; i < blocks_to_hash; i++) {
    //     for (int j = 0; j < block_size_in_bytes/2; j++) {
    //         printf("%02x", hashed_data[(i * block_size_in_bytes/2) + j]);
    //     }

    //     printf("\n");
    // }

    while ((blocks_to_hash = blocks_to_hash/2) != 1) {
        for (int i = 0; i < blocks_to_hash; i++) {
            SHA3_224(&hashed_data[i*block_size_in_bytes], &hashed_data[i*(block_size_in_bytes/2)]);
        }
    }

    // print out hashed data
    for (int j = 0; j < block_size_in_bytes/2; j++) {
        printf("%02x", hashed_data[j]);
    }
    printf("\n");
}