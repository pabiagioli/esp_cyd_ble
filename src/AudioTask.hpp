#pragma once
#include <Arduino.h>
#include "pampanet_config.h"
#include "ITask.hpp"
#include "FPOscillator.hpp"
#include "RingBuffer.hpp"

class AudioTask : public ITask {
private:
  static constexpr int blockSize = 64;  // ~1.45 ms at 44.1 kHz
  std::shared_ptr<FPOscillator> m_osc;
  std::shared_ptr<RingBuffer<float, cfg::AUDIO_BUFFER_SIZE>> m_uiBuffer;
  std::shared_ptr<RingBuffer<float, 2048>> m_bleBuffer;
  /// ticking period
  ///- sampleRate = 44100 Hz, blockSize = 64 → period ≈ 1.45 ms
  ///- blockSize = 48 → period = 1.088 ms
  ///- blockSize = 32 → period = 0.726 ms
  TickType_t period;

public:
  explicit AudioTask(const char* name, uint32_t core,
                     std::shared_ptr<FPOscillator> osc,
                     std::shared_ptr<RingBuffer<float, cfg::AUDIO_BUFFER_SIZE>> uiBuffer,
                     std::shared_ptr<RingBuffer<float, 2048>> bleBuffer)
    : ITask(name, core, 1024 * 8, configMAX_PRIORITIES - 2),
      m_osc(osc), m_uiBuffer(uiBuffer), m_bleBuffer(bleBuffer) {}

  void setup() override {
    //period = pdMS_TO_TICKS(1);
    float_t sampleRate = m_osc->sampleRate();
    float_t ms = (1000.0f * blockSize) / sampleRate;
    period = pdMS_TO_TICKS(ms);
    Serial.printf("--- Audio Task configured with sample rate = %.2f hz and period = %.2f ms / %d ticks ---\n", sampleRate, ms, period);
  }

  void loop() override {
    TickType_t last = xTaskGetTickCount();
    vTaskDelayUntil(&last, period);
    for (int i = 0; i < blockSize; i++) {
      float s = m_osc->nextSample();
      m_uiBuffer->push(s);
      m_bleBuffer->push(s);
    }
  }
};