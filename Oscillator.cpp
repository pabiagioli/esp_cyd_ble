#include "Oscillator.hpp"
#include <cmath>
#include <limits>

Oscillator::Oscillator(uint32_t sample_rate, uint32_t table_len)
: m_sampleRate(sample_rate),
  m_tableLen(table_len),
  m_sineTable(),
  m_phaseAcc(0),
  m_phaseInc(0),
  m_amplitude(1.0f),
  m_waveform(static_cast<uint8_t>(Waveform::Sine)),
  m_enabled(false)
{
    if (m_tableLen < 8) m_tableLen = 8;
    // Ensure power-of-two length for fast lookup
    uint32_t p2 = 1;
    while (p2 < m_tableLen) p2 <<= 1;
    m_tableLen = p2;
    buildSineTable();
}

void Oscillator::buildSineTable() {
    m_sineTable.resize(m_tableLen);
    for (uint32_t i = 0; i < m_tableLen; ++i) {
        double phase = (double)i / (double)m_tableLen;
        m_sineTable[i] = static_cast<float>(std::sin(phase * 2.0 * M_PI));
    }
}

void Oscillator::setFrequency(float hz) {
    if (hz < 0.0f) hz = 0.0f;
    // phase_inc = freq * (2^32 / sample_rate)
    double factor = (double(1ULL << 32)) / double(m_sampleRate);
    uint32_t inc = static_cast<uint32_t>(hz * factor);
    m_phaseInc.store(inc, std::memory_order_release);
}

void Oscillator::setAmplitude(float amp) {
    if (amp < 0.0f) amp = 0.0f;
    if (amp > 1.0f) amp = 1.0f;
    m_amplitude.store(amp, std::memory_order_release);
}

void Oscillator::setWaveform(Waveform wf) {
    m_waveform.store(static_cast<uint8_t>(wf), std::memory_order_release);
}
/*
void Oscillator::setEnabled(bool en) {
    m_enabled.store(en, std::memory_order_release);
}
*/
float Oscillator::tableLookup(uint32_t phase) const {
    // map top bits of 32-bit phase to table index
    // index = phase >> (32 - log2(table_len))
    uint32_t l2 = 0;
    uint32_t tmp = m_tableLen;
    while (tmp > 1) { tmp >>= 1; ++l2; }
    uint32_t shift = 32 - l2;
    uint32_t idx = phase >> shift;
    return m_sineTable[idx & (m_tableLen - 1)];
}

float Oscillator::nextSample() {
    if (!m_enabled.load(std::memory_order_acquire)) return 0.0f;

    // advance phase
    uint32_t inc = m_phaseInc.load(std::memory_order_acquire);
    m_phaseAcc += inc;
    uint32_t phase = m_phaseAcc;

    float out = 0.0f;
    Waveform wf = static_cast<Waveform>(m_waveform.load(std::memory_order_acquire));
    switch (wf) {
        case Waveform::Sine:
            out = tableLookup(phase);
            break;
        case Waveform::Square:
            out = (phase & 0x80000000U) ? 1.0f : -1.0f;
            break;
        case Waveform::Triangle: {
            // triangle from phase: map to -1..1
            // produce saw and fold
            uint32_t p = phase;
            // normalized 0..1
            float v = static_cast<float>(p) / static_cast<float>(std::numeric_limits<uint32_t>::max());
            if (v < 0.25f) out = v * 4.0f;
            else if (v < 0.75f) out = 2.0f - v * 4.0f;
            else out = v * 4.0f - 4.0f;
            out = (out * 2.0f) - 1.0f; // map 0..1->-1..1 (approx)
            break;
        }
        case Waveform::Saw: {
            float v = static_cast<float>(phase) / static_cast<float>(std::numeric_limits<uint32_t>::max());
            out = 2.0f * v - 1.0f;
            break;
        }
        default:
            out = 0.0f;
    }

    float amp = m_amplitude.load(std::memory_order_acquire);
    out *= amp;
    return out;
}
