#pragma once
#include <cstdint>
#include <vector>
#include <atomic>
#include <memory>

enum class Waveform : uint8_t {
  Sine = 0,
  Square,
  Triangle,
  Saw
};

class Oscillator {
public:
  explicit Oscillator(uint32_t sample_rate = 44100, uint32_t table_len = 1024);
  ~Oscillator() = default;

  // Thread-safe setters (safe to call from LVGL UI thread)
  void setFrequency(float hz);
  void setAmplitude(float amp);  // 0.0 .. 1.0
  void setWaveform(Waveform wf);
  bool isEnabled() const noexcept {
    return m_enabled.load(std::memory_order_acquire);
  }

  // Atomically toggle and return the new state.
  bool toggleEnabled() noexcept {
    bool prev = m_enabled.load(std::memory_order_acquire);
    while (!m_enabled.compare_exchange_weak(prev, !prev, std::memory_order_acq_rel)) {
      // compare_exchange_weak updates prev on failure, loop until success
    }
    return !prev;  // new value
  }
  void setEnabled(bool en) noexcept { m_enabled.store(en, std::memory_order_release); }

  // Called from audio thread / ISR - returns sample -1..1
  float nextSample();

  uint32_t sampleRate() const {
    return m_sampleRate;
  }

private:
  uint32_t m_sampleRate;
  uint32_t m_tableLen;
  std::vector<float> m_sineTable;

  // DDS state
  uint32_t m_phaseAcc;               // only modified in audio thread
  std::atomic<uint32_t> m_phaseInc;  // updated atomically by UI
  std::atomic<float> m_amplitude;
  std::atomic<uint8_t> m_waveform;  // stores Waveform as integer
  std::atomic<bool> m_enabled;

  void buildSineTable();
  float tableLookup(uint32_t phase) const;
};
