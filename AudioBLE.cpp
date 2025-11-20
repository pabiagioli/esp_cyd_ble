#pragma once
#include <Arduino.h>
#include "AudioBLE.hpp"

void AudioBLE::begin() {
  const char* SERVICE_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";
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

  BLEService* svc = server->createService(SERVICE_UUID);

  m_char = svc->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);

  // Optional: ensure characteristic buffer is large enough
  m_char->setValue((uint8_t*)"init", 4);

  svc->start();

  // Start advertising AFTER service is up
  BLEAdvertising* adv = BLEDevice::getAdvertising();
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

void AudioBLE::start() {
  m_running = true;
  //m_osc->setEnabled(true);
  Serial.println("--- BLEServer started correctly ---");
}

void AudioBLE::stop() {
  m_running = false;
  //m_osc->setEnabled(false);
  Serial.println("--- BLEServer stopped correctly ---");
}

void AudioBLE::update() {
  if (!m_running || !m_char) return;

  static uint32_t last = 0;
  uint32_t now = millis();
  if (now - last < 20) return;
  last = now;

  // 160 PCM samples @ 8kHz for 20ms audio frame
  constexpr int N = RAW_SAMPLES;
  static int16_t pcm[N];
  static uint8_t out[N / 2];

  // Generate PCM block
  for (int i = 0; i < N; i++) {
    float s = m_osc->nextSample();
    s = constrain(s, -1.0f, 1.0f);
    pcm[i] = (int16_t)(s * 32767);
  }

  // ADPCM encode -> out[]
  uint8_t* p = out;
  for (int i = 0; i < N; i += 2) {
    uint8_t hi = adpcm_encode_sample(pcm[i], m_state);
    uint8_t lo = adpcm_encode_sample(pcm[i + 1], m_state);
    *p++ = (hi << 4) | lo;
  }

  // Send BLE notification (safe)
  m_char->setValue(out, N / 2);
  if (clientSubscribed(m_char) > 0) {
    m_char->notify();
  }
}
