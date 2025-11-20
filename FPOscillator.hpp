#pragma once
#include <cstdint>
#include <atomic>
#include <cmath>
#include "Waveform.hpp"

class FPOscillator {
public:
    explicit FPOscillator(uint32_t sample_rate = 44100)
        : m_sampleRate(sample_rate),
          m_phase(0.0f),
          m_phaseInc(0.0f),
          m_amplitude(1.0f),
          m_waveform(static_cast<uint8_t>(Waveform::Sine)),
          m_enabled(true)
    {}

    ~FPOscillator() = default;

    void setFrequency(float hz) {
        float inc = hz / float(m_sampleRate);
        if (inc < 0.f) inc = 0.f;
        m_phaseInc.store(inc, std::memory_order_relaxed);
    }

    void setAmplitude(float amp) {
        if (amp < 0.f) amp = 0.f;
        if (amp > 1.f) amp = 1.f;
        m_amplitude.store(amp, std::memory_order_relaxed);
    }

    void setWaveform(Waveform wf) {
        m_waveform.store(static_cast<uint8_t>(wf), std::memory_order_relaxed);
    }

    bool isEnabled() const noexcept {
        return m_enabled.load(std::memory_order_relaxed);
    }

    bool toggleEnabled() noexcept {
        bool old = m_enabled.load();
        bool now = !old;
        m_enabled.store(now);
        return now;
    }

    void setEnabled(bool en) noexcept {
        m_enabled.store(en);
    }

    float nextSample() {
        if (!m_enabled.load(std::memory_order_relaxed))
            return 0.0f;

        // Load atomics once
        float phaseInc   = m_phaseInc.load(std::memory_order_relaxed);
        float amplitude  = m_amplitude.load(std::memory_order_relaxed);
        uint8_t wf       = m_waveform.load(std::memory_order_relaxed);

        // Advance phase 0..1
        m_phase += phaseInc;
        m_phase = (m_phase - floorf(m_phase));
        //if (m_phase >= 1.f)
        //    m_phase -= 1.f;

        float s;
        switch (static_cast<Waveform>(wf)) {
        case Waveform::Sine:
            s = sinf(2.0f * M_PI * m_phase);
            break;

        case Waveform::Square:
            s = m_phase < 0.5f ? 1.0f : -1.0f;
            break;

        case Waveform::Triangle:
            // 0..1 phase → triangle: -1..1
            if (m_phase < 0.25f)
                s = 4.f * m_phase;
            else if (m_phase < 0.75f)
                s = 2.f - 4.f * m_phase;
            else
                s = -4.f + 4.f * m_phase;
            break;

        case Waveform::Saw:
            s = 2.f * m_phase - 1.f;  // simple saw
            break;

        default:
            s = 0.f;
            break;
        }

        return s * amplitude;
    }

    uint32_t sampleRate() const { return m_sampleRate; }

private:
    uint32_t m_sampleRate;

    // Phase in normalized 0..1 range
    float m_phase;

    // Atomics touched by UI/UI tasks
    std::atomic<float>   m_phaseInc;
    std::atomic<float>   m_amplitude;
    std::atomic<uint8_t> m_waveform;
    std::atomic<bool>    m_enabled;
};
