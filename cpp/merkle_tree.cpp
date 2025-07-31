
#include <ap_int.h>
#include <hls_stream.h>

#include "sha3_256.cpp"

struct hash_t {
    unsigned char to_hash[64];
    unsigned char hashed[32];
    int hash_layer;
};

void input_manager(hls::stream<ap_uint<512>> &input, hls::stream<hash_t> &feedback, hls::stream<hash_t> &output) {
    while (true) {
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
}

void sha3_pipeline(hls::stream<hash_t> &input, hls::stream<hash_t> &output) {
    while (true) {
        if (!input.empty()) {
            hash_t data = input.read();

            sha3_opt_256(data.to_hash, data.hashed);

            output.write(data);
        }
    }
}

#define TREE_LAYERS 14
void output_manager(hls::stream<hash_t> &input, hls::stream<hash_t> &feedback, hls::stream<ap_uint<256>> &output) {
    static hash_t layers[TREE_LAYERS]; // For 16,384 leaves we have log_2(16384) = 14 layers of the tree
    static bool layer_in_use[TREE_LAYERS];

    while(true) {
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
}

// Takes as input the data from the dataloader as a stream?
void controller(hls::stream<ap_uint<512>> &in) {
    hls::stream<hash_t> feedback; // output_manager => input_manager
    hls::stream<hash_t> in_sha3; // input_manager => sha_3
    hls::stream<hash_t> sha3_out; // sha_3 => output_manager
    hls::stream<ap_uint<256>> result; // output_manager => ???

    input_manager(in, feedback, in_sha3);
    sha3_pipeline(in_sha3, out_sha3);
    output_manager(sha3_out, feedback, result);

    ap_uint<256> = result.read(); // What do we do with the output here? Is this blocking?
}