#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <cstdint>
#include <cstdlib>

extern void sha3_opt_256(const unsigned char input[64], unsigned char output[32]);

int main() {
    const std::string input_str = "0123456789012345678901234567890123456789012345678901234567890123";

    // Prepare input and output buffers
    std::vector<unsigned char> input(input_str.begin(), input_str.end());
    std::vector<unsigned char> output(32, 0);

    // Call the HLS function
    sha3_opt_256(input.data(), output.data());

    // Write output to file
    std::ofstream fout("output.dat");
    if (!fout) {
        std::cerr << "Failed to open output.dat for writing.\n";
        return 1;
    }

    for (const auto& byte : output) {
        fout << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }

    fout.close();

    // Compare output with golden output
    int diff_result = std::system("diff -w output.dat golden.dat");

    if (diff_result != 0) {
        std::cout << "*******************************************\n";
        std::cout << "FAIL: Output does NOT match golden output\n";
        std::cout << "*******************************************\n";
        return 1;
    } else {
        std::cout << "*******************************************\n";
        std::cout << "PASS: Output matches golden output!\n";
        std::cout << "*******************************************\n";
        return 0;
    }
}