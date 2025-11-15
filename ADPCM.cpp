#include "ADPCM.hpp"

// Standard IMA ADPCM step table
static const int stepTable[89] = {
     7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31,
     34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
     130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371,
     408, 449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166,
     1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
     3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845,
     8630, 9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500,
     20350, 22385, 24623, 27086, 29794, 32767
};

// Index adjust table
static const int indexAdjust[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8
};

uint8_t adpcm_encode_sample(int16_t pcm, ADPCMState &s)
{
    int step = stepTable[s.index];
    int diff = pcm - s.predicted;
    uint8_t nibble = 0;

    if (diff < 0) {
        nibble = 8;
        diff = -diff;
    }

    int temp = step;
    if (diff >= temp) {
        nibble |= 4;
        diff -= temp;
    }
    temp >>= 1;
    if (diff >= temp) {
        nibble |= 2;
        diff -= temp;
    }
    temp >>= 1;
    if (diff >= temp) {
        nibble |= 1;
    }

    int delta = 0;
    if (nibble & 4) delta += step;
    if (nibble & 2) delta += step >> 1;
    if (nibble & 1) delta += step >> 2;
    delta += step >> 3;

    if (nibble & 8) delta = -delta;

    s.predicted += delta;
    if (s.predicted > 32767) s.predicted = 32767;
    if (s.predicted < -32768) s.predicted = -32768;

    s.index += indexAdjust[nibble];
    if (s.index < 0) s.index = 0;
    if (s.index > 88) s.index = 88;

    return nibble;
}
