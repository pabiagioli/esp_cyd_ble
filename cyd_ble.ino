// Including this library will also include lvgl.h and TFT_eSPI.h
#include <memory>
//#include <FS.h>
//#include <LittleFS.h>
#include <lvgl.h>
#include "FPOscillator.hpp"
#include "RingBuffer.hpp"
#include "AudioTask.hpp"
#include "UITask.hpp"
#include "BLETask.hpp"

#define SERIAL_BAUD_RATE 115200
#define SAMPLE_RATE 44100

#define UI_CORE 1
#define AUDIO_CORE 1
#define BLE_CORE 0

static std::shared_ptr<FPOscillator> globalOsc = nullptr;
static std::shared_ptr<RingBuffer<float, 2048>> buffer = nullptr;
// allocate the UI task once and keep it alive
static std::shared_ptr<AudioTask> audioTask = nullptr;
static std::shared_ptr<UITask> uiTask = nullptr;
static std::shared_ptr<BLETask> bleTask = nullptr;

void setup() {
  // put your setup code here, to run once:
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println(LVGL_Arduino);

  globalOsc = std::make_shared<FPOscillator>(SAMPLE_RATE);
  buffer = std::make_shared<RingBuffer<float, 2048>>();
  audioTask = std::make_shared<AudioTask>("Audio Task", AUDIO_CORE, globalOsc, buffer);
  // create persistent task object (pin to core 1 for LVGL if you want)
  uiTask = std::make_shared<UITask>("UI Task", UI_CORE, globalOsc);
  //delay(500);
  bleTask = std::make_shared<BLETask>("BLE Task", BLE_CORE, globalOsc);
  uiTask->start();
  delay(500);
  bleTask->start();
  delay(500);
}

void loop() {
  // No more main loop
}
