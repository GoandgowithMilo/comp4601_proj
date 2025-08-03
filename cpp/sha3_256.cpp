#include <cstdint>
#include "sha3_256.h"

typedef uint64_t tKeccakLane;

/*
================================================================
A readable and compact implementation of the Keccak-f[1600] permutation.
================================================================
*/
#define opt_ROL64(a, offset) ((((uint64_t)a) << offset) ^ (((uint64_t)a) >> (64-offset)))
#define opt_i(x, y) ((x)+5*(y))
#define opt_readLane(x, y)          (((tKeccakLane*)state)[opt_i(x, y)])
#define opt_writeLane(x, y, lane)   (((tKeccakLane*)state)[opt_i(x, y)]) = (lane)
#define opt_XORLane(x, y, lane)     (((tKeccakLane*)state)[opt_i(x, y)]) ^= (lane)

/**
  * Function that computes the linear feedback shift register (LFSR) used to
  * define the round constants (see [Keccak Reference, Section 1.2]).
  */
int opt_LFSR86540(uint8_t *LFSR)
{
    int result = ((*LFSR) & 0x01) != 0;
    if (((*LFSR) & 0x80) != 0)
        /* Primitive polynomial over GF(2): x^8+x^6+x^5+x^4+1 */
        (*LFSR) = ((*LFSR) << 1) ^ 0x71;
    else
        (*LFSR) <<= 1;
    return result;
}

/**
 * Function that computes the Keccak-f[1600] permutation on the given state.
 */
void opt_KeccakF1600_StatePermute(void *state)
{
    unsigned int round, x, y, j, t;
    uint8_t LFSRstate = 0x01;

    for(round=0; round<24; round++) {
        #pragma HLS pipeline
        {   /* === θ step (see [Keccak Reference, Section 2.3.2]) === */
            tKeccakLane C[5], D;

            /* Compute the parity of the columns */
            for(x=0; x<5; x++) {
                #pragma HLS pipeline off
                C[x] = opt_readLane(x, 0) ^ opt_readLane(x, 1) ^ opt_readLane(x, 2) ^ opt_readLane(x, 3) ^ opt_readLane(x, 4);
            }

            for(x=0; x<5; x++) {
                #pragma HLS pipeline off
                /* Compute the θ effect for a given column */
                D = C[(x+4)%5] ^ opt_ROL64(C[(x+1)%5], 1);
                /* Add the θ effect to the whole column */
                for (y=0; y<5; y++) {
                    #pragma HLS pipeline off
                    opt_XORLane(x, y, D);
                }
            }
        }

        {   /* === �? and π steps (see [Keccak Reference, Sections 2.3.3 and 2.3.4]) === */
            tKeccakLane current, temp;
            /* Start at coordinates (1 0) */
            x = 1; y = 0;
            current = opt_readLane(x, y);
            /* Iterate over ((0 1)(2 3))^t * (1 0) for 0 ≤ t ≤ 23 */
            for(t=0; t<24; t++) {
                #pragma HLS pipeline off
                /* Compute the rotation constant r = (t+1)(t+2)/2 */
                unsigned int r = ((t+1)*(t+2)/2)%64;
                /* Compute ((0 1)(2 3)) * (x y) */
                unsigned int Y = (2*x+3*y)%5; x = y; y = Y;
                /* Swap current and state(x,y), and rotate */
                temp = opt_readLane(x, y);
                opt_writeLane(x, y, opt_ROL64(current, r));
                current = temp;
            }
        }

        {   /* === χ step (see [Keccak Reference, Section 2.3.1]) === */
            tKeccakLane temp[5];
            for(y=0; y<5; y++) {
                #pragma HLS pipeline off
                /* Take a copy of the plane */
                for(x=0; x<5; x++) {
                    #pragma HLS pipeline off
                    temp[x] = opt_readLane(x, y);
                }
                    
                /* Compute χ on the plane */
                for(x=0; x<5; x++) {
                    #pragma HLS pipeline off
                    opt_writeLane(x, y, temp[x] ^((~temp[(x+1)%5]) & temp[(x+2)%5]));   
                }
            }
        }

        {   /* === ι step (see [Keccak Reference, Section 2.3.5]) === */
            for(j=0; j<7; j++) {
                #pragma HLS pipeline off
                unsigned int bitPosition = (1<<j)-1; /* 2^j-1 */
                if (opt_LFSR86540(&LFSRstate))
                    opt_XORLane(0, 0, (tKeccakLane)1<<bitPosition);
            }
        }
    }
}

/**
 * This is a stripped back version that removes the generalities from the FIPS implementation
 */
void sha3_opt_256(const unsigned char input[64], unsigned char output[32])
{
    unsigned int rate = 1088; // Thus capacity = 512
    unsigned int inputByteLen = 64;
    unsigned char delimitedSuffix = 0x06;
    unsigned int outputByteLen = 32;
    uint8_t state[200];

    unsigned int rateInBytes = rate/8;
    unsigned int i;
    unsigned int j;

    /* === Initialize the state === */
    for (j = 0; j < 200; j++) {
        #pragma HLS pipeline off
        state[j] = 0;
    }

    /* === Absorb all the input blocks === */
    for(i=0; i < inputByteLen; i++) {
        #pragma HLS pipeline off
        state[i] ^= input[i]; // XOR the input with the state
    }

    /* === Do the padding and switch to the squeezing phase === */
    /* Add padding (which coincides with the delimiter in delimitedSuffix) */
    state[inputByteLen] ^= delimitedSuffix;
    /* Add the second bit of padding */
    state[rateInBytes-1] ^= 0x80;
    /* Switch to the squeezing phase */
    opt_KeccakF1600_StatePermute(state);

    /* === Squeeze out all the output blocks === */
    for (j = 0; j < outputByteLen; j++) {
        #pragma HLS pipeline off
        output[j] = state[j];
    }
}
