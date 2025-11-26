#pragma once
#include <atomic>
#include <cmath>
#include "Waveform.hpp"

class FPOscillator {
public:
  explicit FPOscillator(uint32_t sample_rate = 44100)
    : m_sampleRate(sample_rate),
      m_phase(0.0f),
      m_phaseInc(0.0f),
      m_targetFreq(0.0f),
      m_targetAmp(1.0f),
      m_curFreq(0.0f),
      m_curAmp(1.0f),
      m_waveform(Waveform::Sine),
      m_enabled(false) {
  }

  // ===== UI-thread safe setters =====
  void setFrequency(float hz) {
    if (hz < 0) hz = 0;
    m_targetFreq.store(hz, std::memory_order_release);
  }

  void setAmplitude(float amp) {
    if (amp < 0) amp = 0;
    if (amp > 1) amp = 1;
    m_targetAmp.store(amp, std::memory_order_release);
  }

  void setWaveform(Waveform wf) {
    m_waveform.store(wf, std::memory_order_release);
  }

  bool isEnabled() const noexcept {
    return m_enabled.load(std::memory_order_relaxed);
  }

  bool toggleEnabled() noexcept {
    bool old = m_enabled.load(std::memory_order_relaxed);
    bool now = !old;
    m_enabled.store(now, std::memory_order_release);
    return now;
  }

  void setEnabled(bool e) noexcept {
    m_enabled.store(e, std::memory_order_release);
  }

  // ===== Audio thread: glitch-free, smoothed =====
  float nextSample() {
    if (!m_enabled.load(std::memory_order_acquire))
      return 0.0f;

    // --- Smooth frequency ---
    float targetHz = m_targetFreq.load(std::memory_order_acquire);
    m_curFreq += 0.001f * (targetHz - m_curFreq);

    m_phaseInc = m_curFreq / m_sampleRate;

    // --- Smooth amplitude ---
    float targetAmp = m_targetAmp.load(std::memory_order_acquire);
    m_curAmp += 0.002f * (targetAmp - m_curAmp);

    // --- Advance phase ---
    m_phase += m_phaseInc;
    if (m_phase >= 1.0f)
      m_phase -= 1.0f;

    float x = m_phase;
    Waveform wf = m_waveform.load(std::memory_order_acquire);

    float s = 0.0f;
    switch (wf) {
      case Waveform::Sine:
        s = sinf(2.f * M_PI * x);
        break;

      case Waveform::Square:
        s = x < 0.5f ? 1.f : -1.f;
        break;

      case Waveform::Triangle:
        if (x < 0.25f) s = 4.f * x;
        else if (x < 0.75f) s = 2.f - 4.f * x;
        else s = -4.f + 4.f * x;
        break;

      case Waveform::Saw:
        s = 2.f * x - 1.f;
        break;
    }

    return s * m_curAmp;
  }

  uint32_t sampleRate() const {
    return m_sampleRate;
  }

private:
  uint32_t m_sampleRate;

  // audio thread only
  float m_phase;
  float m_phaseInc;

  // smoothed runtime parameters (audio thread)
  float m_curFreq;
  float m_curAmp;

  // UI thread updated
  std::atomic<float> m_targetFreq;
  std::atomic<float> m_targetAmp;
  std::atomic<Waveform> m_waveform;
  std::atomic<bool> m_enabled;
};
