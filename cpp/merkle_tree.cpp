
#include <ap_int.h>
#include <hls_stream.h>
#include <hls_task.h>

#include "sha3_256.h"

struct hash_t {
    unsigned char to_hash[64];
    unsigned char hashed[32];
    int hash_layer;
};

void input_manager(hls::stream<ap_uint<512>> &input, hls::stream<hash_t> &feedback, hls::stream<hash_t> &output) {
    hash_t out;

    if (!feedback.empty()) {
        hash_t in1 = feedback.read();
        hash_t in2 = feedback.read();
        out.hash_layer = in1.hash_layer;

        for (int i = 0; i < 32; i++) {
            out.to_hash[i]      = in1.hashed[i];
            out.to_hash[i + 32] = in2.hashed[i];
        }

    } else {
        ap_uint<512> in_val = input.read();
        out.hash_layer = 0;
        
        for (int i = 0; i < 64; i++) {
            out.to_hash[i] = in_val.range(8 * (i + 1) - 1, 8 * i);
        }
    }

    output.write(out);
}

void sha3_pipeline(hls::stream<hash_t> &input, hls::stream<hash_t> &output) {
    if (!input.empty()) {
        hash_t data = input.read();

        sha3_opt_256(data.to_hash, data.hashed);

        output.write(data);
    }
}

#define TREE_LAYERS 3
void output_manager(hls::stream<hash_t> &input, hls::stream<hash_t> &feedback, hls::stream<ap_uint<256>> &output) {
    static hash_t layers[TREE_LAYERS]; // For 16,384 leaves we have log_2(16384) = 14 layers of the tree
    static bool layer_in_use[TREE_LAYERS];

    if (!input.empty()) {
        hash_t in = input.read();
        int layer = in.hash_layer;

        if (layer_in_use[layer]) { // we have an existing element at that layer so pair up and rehash
            hash_t combined;
            combined.hash_layer = layer + 1;

            for (int i = 0; i < 32; i++) {
                combined.to_hash[i] = layers[layer].hashed[i];
                combined.to_hash[i + 32] = in.hashed[i];
            }

            layer_in_use[layer] = false;

            if (combined.hash_layer < TREE_LAYERS) {
                feedback.write(combined);
            } else { // we've hit the end and want to write back the resulting hash
                ap_uint<256> final_hash;
                for (int i = 0; i < 32; i++) {
                    final_hash.range(8 * (i + 1) - 1, 8 * i) = combined.hashed[i];
                }
                output.write(final_hash);
            }
        } else { // we store the hash until its pair arrives
            layers[layer] = in;
            layer_in_use[layer] = true;
        }

    }
}

void merkle_tree(hls::stream<ap_uint<512>> &in, hls::stream<ap_uint<256>> &out) {
    hls_thread_local hls::stream<hash_t, 32> feedback; // output_manager => input_manager
    hls_thread_local hls::stream<hash_t, 32> in_sha3; // input_manager => sha_3
    hls_thread_local hls::stream<hash_t, 32> sha3_out; // sha_3 => output_manager

    hls::task t_input_manager(input_manager, in, feedback, in_sha3);
    hls::task t_sha3_pipeline(sha3_pipeline, in_sha3, sha3_out);
    hls::task t_output_manager(output_manager, sha3_out, feedback, out);
}

// Create a testbench for this and just use CSIM to verify output
// To prevent deadlocks we want queues between copmonents to be large enough hence hls::stream<hash_t, 32>
// Run CSIM, then run COSIM to check for deadlocks (size is ignored for csim)
// verify and check no deadlocks!

/* Need to come up with a way to produce an expected value... Could maybe use chatgpt to write a python program or modify
my C++ program to do it? */

/*
    Create input and output streams
    Generate dataset
    create instance of controller with input and ouput streams
    write data to input stream
    read from the output stream

*/