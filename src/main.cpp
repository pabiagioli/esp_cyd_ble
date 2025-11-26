#include <Arduino.h>
// Including this library will also include lvgl.h and TFT_eSPI.h
#include <memory>
#include "pampanet_config.h"
//#include <FS.h>
//#include <LittleFS.h>
#include <lvgl.h>
#include "FPOscillator.hpp"
#include "RingBuffer.hpp"
#include "AudioTask.hpp"
#include "UITask.hpp"
#include "BLETask.hpp"

static std::shared_ptr<FPOscillator> globalOsc = nullptr;
static std::shared_ptr<RingBuffer<float, cfg::AUDIO_BUFFER_SIZE>> audioBuffer = nullptr;
static std::shared_ptr<RingBuffer<float, cfg::BLE_BUFFER_SIZE>> bleBuffer = nullptr;
// allocate the UI task once and keep it alive
static std::shared_ptr<AudioTask> audioTask = nullptr;
static std::shared_ptr<UITask> uiTask = nullptr;
static std::shared_ptr<BLETask> bleTask = nullptr;

void setup() {
  // put your setup code here, to run once:
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(cfg::SERIAL_BAUD_RATE);
  Serial.println(LVGL_Arduino);

  globalOsc = std::make_shared<FPOscillator>(cfg::SAMPLE_RATE);
  audioBuffer = std::make_shared<RingBuffer<float, cfg::AUDIO_BUFFER_SIZE>>();
  bleBuffer = std::make_shared<RingBuffer<float, cfg::BLE_BUFFER_SIZE>>();
  audioTask = std::make_shared<AudioTask>("Audio Task", cfg::AUDIO_CORE, globalOsc, audioBuffer, bleBuffer);
  // create persistent task object (pin to core 1 for LVGL if you want)
  uiTask = std::make_shared<UITask>("UI Task", cfg::UI_CORE, globalOsc, audioBuffer);
  //delay(500);
  bleTask = std::make_shared<BLETask>("BLE Task", cfg::BLE_CORE, bleBuffer);
  uiTask->start();
  delay(500);
  bleTask->start();
  delay(500);
}

void loop() {
  // No more main loop
  vTaskDelete(NULL); //delete this task
}
