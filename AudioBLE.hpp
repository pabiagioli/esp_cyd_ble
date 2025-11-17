#pragma once
#include <memory>
#include "Oscillator.hpp"
#include "ADPCM.hpp"
#include <BLEDevice.h>

class AudioBLE {
public:
    
    explicit AudioBLE(std::shared_ptr<Oscillator> osc): m_osc(osc){}

    void begin();
    void start();
    void stop();
    void update(); // call every loop()

private:
    std::shared_ptr<Oscillator> m_osc;
    BLEServer *server;
    BLECharacteristic *m_char = nullptr;
    bool m_running = false;

    ADPCMState m_state;

    static const int RAW_SAMPLES = 360;     // 360 PCM samples
    static const int PACKED_BYTES = 180;    // 4:1 compression
};
