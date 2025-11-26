#pragma once
#include <memory>
#include <BLEDevice.h>
#include "RingBuffer.hpp"
#include "ADPCM.hpp"

class AudioBLE {
public:

  explicit AudioBLE(std::shared_ptr<RingBuffer<float, 2048>> buffer)
    : audioBuffer(buffer) {}

  void begin();
  void start();
  void stop();
  void update();  // call every loop()
  static bool clientSubscribed(BLECharacteristic* chr) {
    auto* desc = chr->getDescriptorByUUID((uint16_t)0x2902);
    if (!desc) return false;

    uint8_t* val = desc->getValue();
    return (val && val[0] == 0x01);
  }

private:
  std::shared_ptr<RingBuffer<float, 2048>> audioBuffer;
  BLEServer* server;
  BLECharacteristic* m_char = nullptr;
  bool m_running = false;

  ADPCMState m_state;

  static const int RAW_SAMPLES = 360;   // 360 PCM samples
  static const int PACKED_BYTES = 180;  // 4:1 compression
};
