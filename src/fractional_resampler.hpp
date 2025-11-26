#pragma once
#include <vector>

class FractionalResampler {
public:
    FractionalResampler(float inputRate, float outputRate)
        : ratio(inputRate / outputRate), t(0.0f) {}

    // Feed ONE sample at inputRate
    // Return >=0 samples at outputRate
    // (normally returns 0 or 1)
    int process(float x, float& out) {
        input.push_back(x);

        int produced = 0;

        while (t <= input.size() - 2) {
            int i = (int)t;
            float frac = t - i;

            float y = input[i] * (1.0f - frac) +
                      input[i + 1] * frac;

            out = y;
            produced = 1;
            t += ratio;
        }

        // Clean consumed samples to avoid unbounded growth
        if ((int)t > 1) {
            int drop = (int)t - 1;
            input.erase(input.begin(), input.begin() + drop);
            t -= drop;
        }

        return produced;
    }

private:
    float ratio;
    float t;
    std::vector<float> input;
};
