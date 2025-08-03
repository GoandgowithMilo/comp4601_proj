#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>
#include <cstring>

extern void sha3_opt_256(const unsigned char input[64], unsigned char output[32]);

#define NUM_LEAVES 16384  // Must be power of 2
#define TREE_LAYERS 14

int main() {
    // Generate leaf nodes with fixed input pattern
    std::vector<std::vector<unsigned char>> hashes;

    unsigned char data_block[64];
    for (int i = 0; i < 64; i++) {
        data_block[i] = i;  // 0x00, 0x01, ..., 0x3F
    }

    for (int i = 0; i < NUM_LEAVES; ++i) {
        unsigned char hash_output[32];
        sha3_opt_256(data_block, hash_output);
        hashes.emplace_back(hash_output, hash_output + 32);
    }

    // Build tree layer by layer
    for (int layer = 0; layer < TREE_LAYERS; ++layer) {
        std::vector<std::vector<unsigned char>> next_layer;

        for (size_t i = 0; i < hashes.size(); i += 2) {
            unsigned char combined[64];
            memcpy(combined, &hashes[i][0], 32);
            memcpy(combined + 32, &hashes[i + 1][0], 32);

            unsigned char hash_output[32];
            sha3_opt_256(combined, hash_output);

            next_layer.emplace_back(hash_output, hash_output + 32);
        }

        hashes = std::move(next_layer);  // Move up to the next layer
    }

    // Final root hash
    const std::vector<unsigned char>& root_hash = hashes[0];

    std::cout << "Golden Merkle Root Hash: 0x";
    for (int i = 0; i < 32; i++) {
        printf("%02x", root_hash[i]);
    }
    std::cout << std::endl;

    // Write root hash to file for comparison
    std::ofstream fout("golden.dat");
    for (int i = 0; i < 32; i++) {
        fout << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(root_hash[i]);
    }
    fout.close();

    return 0;
}
