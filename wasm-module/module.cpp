#include "Phhhsrrr/gen_exported.cpp"
#include <string.h>

#define SAMPLE_RATE 48000
#define MAX_BLOCK_SIZE 256

// Static internal buffers: these have non-zero WASM addresses, so gen~'s
// null-pointer guard (__in1 == 0) won't fire even when the host places the
// external I/O at WASM address 0.
static t_sample s_input[MAX_BLOCK_SIZE];
static t_sample s_output[MAX_BLOCK_SIZE];

extern "C" {

void process(float* input, float* output, int num_samples) {
    static CommonState* state = nullptr;
    if (!state) {
        state = (CommonState*)gen_exported::create(SAMPLE_RATE, num_samples);
    }

    // Copy host-provided data into static buffers (safe non-null addresses)
    memcpy(s_input, input, num_samples * sizeof(float));

    t_sample* ins[2] = {s_input, s_input};
    t_sample* outs[1] = {s_output};
    gen_exported::perform(state, ins, 2, outs, 1, num_samples);

    // Copy results back to host-provided output pointer
    memcpy(output, s_output, num_samples * sizeof(float));
}

}