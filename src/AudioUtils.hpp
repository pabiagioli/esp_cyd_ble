#pragma once
#include <cmath>

/**
* Compute alpha from cutoff frequency + sample rate
*/
float onePoleAlpha(float cutoffHz, float sampleRate) {
    if (cutoffHz <= 0.0f) return 0.0f;
    float x = expf(-2.0f * M_PI * cutoffHz / sampleRate);
    return 1.0f - x;
}