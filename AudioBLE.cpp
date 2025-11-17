#include "BLEDevice.h"
#include "AudioBLE.hpp"
#include <Arduino.h>

void AudioBLE::begin()
{
    const char* SERVICE_UUID        = "beb5483e-36e1-4688-b7f5-ea07361b26a8";
    const char* CHARACTERISTIC_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";

    BLEDevice::init("ESP32-ADPCM");

    server = BLEDevice::createServer();

    // Prevent hidden watchdog disconnects
    class ServerCallbacks : public BLEServerCallbacks {
      void onConnect(BLEServer* s) override {
        Serial.println("BLE: Client connected.");
      }
      void onDisconnect(BLEServer* s) override {
        Serial.println("BLE: Client disconnected. Restarting advertising.");
        BLEDevice::startAdvertising();
      }
    };

    server->setCallbacks(new ServerCallbacks());

    BLEService *svc = server->createService(SERVICE_UUID);

    m_char = svc->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );

    // Optional: ensure characteristic buffer is large enough
    m_char->setValue((uint8_t*)"init", 4);

    svc->start();

    // Start advertising AFTER service is up
    BLEAdvertising *adv = BLEDevice::getAdvertising();
    adv->addServiceUUID(SERVICE_UUID);
    adv->setScanResponse(true);

    // Fixes iPhone “connect then drop” bug
    adv->setMinPreferred(0x06);
    adv->setMinPreferred(0x12);

    // Delay avoids ESP32 display driver collision with BT stack
    delay(200);

    BLEDevice::startAdvertising();

    Serial.println("--- BLEAudio service started ---");
}

void AudioBLE::start()
{
    m_running = true;
    //m_osc->setEnabled(true);
    Serial.println("--- BLEServer started correctly ---");
}

void AudioBLE::stop()
{
    m_running = false;
    //m_osc->setEnabled(false);
    Serial.println("--- BLEServer stopped correctly ---");
}

void AudioBLE::update()
{
    if (!m_running || !m_char) return;

    uint8_t out[PACKED_BYTES];
    uint8_t *p = out;

    for (int i = 0; i < RAW_SAMPLES; i += 2) {

        // get float sample
        float s = m_osc->nextSample();
        if (s < -1.0f) s = -1.0f;
        if (s >  1.0f) s =  1.0f;

        // convert to 16-bit PCM
        int16_t pcm = (int16_t)(s * 32767);

        uint8_t hi = adpcm_encode_sample(pcm, m_state);

        // encode next sample
        float s2 = m_osc->nextSample();
        if (s2 < -1.0f) s2 = -1.0f;
        if (s2 >  1.0f) s2 =  1.0f;

        int16_t pcm2 = (int16_t)(s2 * 32767);
        uint8_t lo = adpcm_encode_sample(pcm2, m_state);

        // pack two nibbles into one byte
        *p++ = (hi << 4) | lo;
    }
    portENTER_CRITICAL(&bleMux);
    m_char->setValue(out, PACKED_BYTES);
    m_char->notify();
    portEXIT_CRITICAL(&bleMux);
    //Serial.println("--- BLEServer notified correctly ---");
}
