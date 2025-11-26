#pragma once
#include <stdint.h>

struct ADPCMState {
    int16_t predicted = 0;
    int8_t index = 0;
};

uint8_t adpcm_encode_sample(int16_t pcm, ADPCMState &state);