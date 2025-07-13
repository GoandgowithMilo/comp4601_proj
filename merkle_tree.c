// typedef struct { 
//     char *hashed_data;
//     char *raw_data;
// } merkle_tree_node;

/*
    > Read data from a file into an array.
start:
    > Read up to 16 blocks of data from that array of size 448 bits (56 bytes)
    > iterate through each block, hashing it
    > Store these hashed blocks in result array
    > Go back to start until whole file has been processed and stored in result array

DFS MIGHT BE EASIEST HERE
    > Read in data from a file into an array
Retrieve:
    > Retrieve up to 16 blocks worth of data from the array
    > Hash each of those blocks 
    > Combine pairs and has those 8 blocks
    > Combine pairs and hash those 4 blocks
    > Combine pairs and hash those 2 blocks
    > Store the last block in a results array
    > If data from file still hasn't been processed, repeat

*/

#include <stdio.h>
#include <assert.h>
#include "sha3_224.c"

/* Start with the assumption that we have 32 blocks worth of data */
// TODO - replace divides with shifts
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