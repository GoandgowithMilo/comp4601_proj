// Reads from this ap_uint 512 stream object
// Store the result in some hash struct and return it (could we just replace it with a stream?)

#include <ap_int.h>
#include <hls_stream.h>

#include "sha3_256.cpp"

void merkle_tree_8(hls::stream<ap_uint<256>> &in , unsigned char output[32]) {
    int i, j;
    unsigned char input[64];
    
    unsigned char hashed_leaves[256];

    for (i = 0; i < 8; i++) {
        // We read in a pair of data
        ap_uint<256> left  = in.read();
        ap_uint<256> right = in.read();
        ap_uint<512> combined;

        // Check this works like I think it does...?
        combined = (( (ap_uint<512>) right << 256 ) | (ap_uint<512>) left);
        
        // Convert to a byte array for sha3
        for (j = 0; j < 64; j++) {
            input[j] = combined.range(8 * (j + 1) - 1, 8 * j);
        }

        // Pass the 64 byte input and store the 32 byte hashed result
        sha3_opt_256(input, &hashed_leaves[i*32]);
    }

    unsigned char hashed_layer_1[128];
    for (i = 0; i < 4; i++) {
        // Pass the 64 byte input and store the 32 byte hashed result
        sha3_opt_256(&hashed_leaves[i*64], &hashed_layer_1[i*32]);
    }

    unsigned char hashed_layer_2[64];
    for (i = 0; i < 2; i++) {
        // Pass the 64 byte input and store the 32 byte hashed result
        sha3_opt_256(&hashed_layer_1[i*64], &hashed_layer_2[i*32]);
    }

    sha3_opt_256(hashed_layer_2, output);
}