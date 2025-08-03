#include <iostream>
#include <ap_int.h>
#include <hls_stream.h>

// Declare the top-level function
void merkle_tree(hls::stream<ap_uint<512>> &in, hls::stream<ap_uint<256>> &out);

int main() {
    hls::stream<ap_uint<512>, 64> in_stream;
    hls::stream<ap_uint<256>, 64> out_stream;

    // Example input: Fill 64 bytes (512 bits) with some known pattern
    ap_uint<512> input_block = 0;
    for (int i = 0; i < 64; i++) {
        input_block.range(8*(i+1)-1, 8*i) = i;  // 0x00, 0x01, ..., 0x3F
    }

    // You can duplicate input blocks if you want a larger Merkle tree
    const int num_leaves = 16384;  // Change this for more layers (must be power of 2)

    for (int i = 0; i < num_leaves; ++i) {
        in_stream.write(input_block);
    }

    // Call top-level function (runs as tasks internally)
    merkle_tree(in_stream, out_stream);

    ap_uint<256> final_hash = out_stream.read();

    std::cout << "Merkle Root Hash: 0x";
    for (int i = 0; i < 32; i++) {
        unsigned char byte = final_hash.range(8 * (31 - i + 1) - 1, 8 * (31 - i));
        printf("%02x", byte);
    }
    std::cout << std::endl;

    return 0;
}