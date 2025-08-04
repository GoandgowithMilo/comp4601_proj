
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
        hash_t in = feedback.read();
        out.hash_layer = in.hash_layer;

        for (int i = 0; i < 64; i++) {
            out.to_hash[i] = in.to_hash[i];
        }

        output.write(out);

    } else if (!input.empty()) {
        ap_uint<512> in_val = input.read();
        out.hash_layer = 0;
        
        for (int i = 0; i < 64; i++) {
            out.to_hash[i] = in_val.range(8 * (i + 1) - 1, 8 * i);
        }
        
        output.write(out);
    }
}

void sha3_pipeline(hls::stream<hash_t> &input, hls::stream<hash_t> &output) {
    hash_t data = input.read();

    sha3_opt_256(data.to_hash, data.hashed);

    output.write(data);
}

#define TREE_LAYERS 1
void output_manager(hls::stream<hash_t> &input, hls::stream<hash_t> &feedback, hls::stream<ap_uint<256>> &output) {
    static hash_t layers[TREE_LAYERS]; // For 16,384 leaves we have log_2(16384) = 14 layers of the tree
    static bool layer_in_use[TREE_LAYERS] = {};

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

        feedback.write(combined);

    } else if (layer == (TREE_LAYERS - 1)) { // We're at the final hash
            ap_uint<256> final_hash;

            for (int i = 0; i < 32; i++) {
                final_hash.range(8 * (31 - i + 1) - 1, 8 * (31 - i)) = in.hashed[i];
            }

            output.write(final_hash);

    } else { // We store this for pairing up later
        layers[layer] = in;
        layer_in_use[layer] = true;
    }
}

void merkle_tree(hls::stream<ap_uint<512>> &in, hls::stream<ap_uint<256>> &out) {
    #pragma HLS INTERFACE mode=ap_ctrl_none port=return
    hls_thread_local hls::stream<hash_t, 64> feedback; // output_manager => input_manager
    hls_thread_local hls::stream<hash_t, 64> in_sha3; // input_manager => sha_3
    hls_thread_local hls::stream<hash_t, 64> sha3_out; // sha_3 => output_manager

    hls::task t_input_manager(input_manager, in, feedback, in_sha3);
    hls::task t_sha3_pipeline(sha3_pipeline, in_sha3, sha3_out);
    hls::task t_output_manager(output_manager, sha3_out, feedback, out);
}