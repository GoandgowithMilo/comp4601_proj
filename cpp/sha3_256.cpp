#include <cstdint>
#include "sha3_256.h"

typedef uint64_t tKeccakLane;

// Rotation left for 64-bit words
tKeccakLane opt_ROL64(tKeccakLane a, unsigned int offset) {
    return ((a << offset) | (a >> (64 - offset)));
}

// Linear Feedback Shift Register (for round constants)
int opt_LFSR86540(uint8_t *LFSR) {
    int result = (*LFSR & 0x01) != 0;
    if (*LFSR & 0x80)
        *LFSR = (*LFSR << 1) ^ 0x71;
    else
        *LFSR <<= 1;
    return result;
}

void opt_KeccakF1600_StatePermute(tKeccakLane state[5][5]) {
    uint8_t LFSRstate = 0x01;
    unsigned int round, x, y, j, t;

    for (round = 0; round < 24; round++) {
        #pragma HLS pipeline

        // === θ step ===
        {
            tKeccakLane C[5], D;
            for (x = 0; x < 5; x++) {
                #pragma HLS pipeline off
                C[x] = state[x][0] ^ state[x][1] ^ state[x][2] ^ state[x][3] ^ state[x][4];
            }

            for (x = 0; x < 5; x++) {
                #pragma HLS pipeline off
                unsigned int xp1 = (x + 1) >= 5 ? 0 : x + 1;
                unsigned int xm1 = (x + 4) >= 5 ? x - 1 : x + 4; // (x - 1 + 5) % 5
                D = C[xm1] ^ opt_ROL64(C[xp1], 1);
                for (y = 0; y < 5; y++) {
                    #pragma HLS pipeline off
                    state[x][y] ^= D;
                }
            }
        }

        // === ρ and π steps ===
        {
            tKeccakLane current, temp;
            x = 1; y = 0;
            current = state[x][y];

            for (t = 0; t < 24; t++) {
                #pragma HLS pipeline off
                unsigned int r = ((t + 1) * (t + 2) / 2) % 64;
                unsigned int newX = y;
                unsigned int newY = (2 * x + 3 * y) % 5;
                temp = state[newX][newY];
                state[newX][newY] = opt_ROL64(current, r);
                current = temp;
                x = newX;
                y = newY;
            }
        }

        // === χ step ===
        {
            tKeccakLane temp[5];
            for (y = 0; y < 5; y++) {
                #pragma HLS pipeline off
                for (x = 0; x < 5; x++) {
                    #pragma HLS pipeline off
                    temp[x] = state[x][y];
                }

                for (x = 0; x < 5; x++) {
                    #pragma HLS pipeline off
                    unsigned int xp1 = (x + 1 >= 5) ? 0 : x + 1;
                    unsigned int xp2 = (x + 2 >= 5) ? x - 3 : x + 2;
                    state[x][y] = temp[x] ^ ((~temp[xp1]) & temp[xp2]);
                }
            }
        }

        // === ι step ===
        {
            for (j = 0; j < 7; j++) {
                #pragma HLS pipeline off
                unsigned int bitPosition = (1 << j) - 1;
                if (opt_LFSR86540(&LFSRstate)) {
                    state[0][0] ^= ((tKeccakLane)1 << bitPosition);
                }
            }
        }
    }
}

void sha3_opt_256(const unsigned char input[64], unsigned char output[32]) {
    unsigned int rate = 1088;
    unsigned int inputByteLen = 64;
    unsigned char delimitedSuffix = 0x06;
    unsigned int outputByteLen = 32;
    unsigned int rateInBytes = rate / 8;

    tKeccakLane state[5][5];
    unsigned int i, j;

    // === Initialize the state ===
    for (j = 0; j < 5; j++) {
        #pragma HLS pipeline off
        for (i = 0; i < 5; i++) {
            #pragma HLS pipeline off
            state[i][j] = 0;
        }
    }

    // === Absorb the input ===
    for (i = 0; i < inputByteLen; i++) {
        #pragma HLS pipeline off
        unsigned int laneIndex = i / 8;
        unsigned int byteIndex = i % 8;
        ((uint8_t*)&state[laneIndex % 5][laneIndex / 5])[byteIndex] ^= input[i];
    }

    // === Padding ===
    unsigned int padIndex = inputByteLen;
    unsigned int padLane = padIndex / 8;
    unsigned int padOffset = padIndex % 8;
    ((uint8_t*)&state[padLane % 5][padLane / 5])[padOffset] ^= delimitedSuffix;

    ((uint8_t*)&state[(rateInBytes - 1) / 8 % 5][(rateInBytes - 1) / 8 / 5])[((rateInBytes - 1) % 8)] ^= 0x80;

    // === Permutation ===
    opt_KeccakF1600_StatePermute(state);

    // === Squeeze output ===
    for (j = 0; j < outputByteLen; j++) {
        #pragma HLS pipeline off
        unsigned int laneIndex = j / 8;
        unsigned int byteIndex = j % 8;
        output[j] = ((uint8_t*)&state[laneIndex % 5][laneIndex / 5])[byteIndex];
    }
}
