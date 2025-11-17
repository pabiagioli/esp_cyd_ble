// Including this library will also include lvgl.h and TFT_eSPI.h
#include <memory>
//#include <FS.h>
//#include <LittleFS.h>
#include <lvgl.h>
#include "Oscillator.hpp"
#include "UITask.hpp"
#include "BLETask.hpp"

static std::shared_ptr<Oscillator> globalOsc = nullptr; //std::make_shared<Oscillator>(44100, 1024);

// allocate the UI task once and keep it alive
static std::shared_ptr<UITask> uiTask = nullptr;
static std::shared_ptr<BLETask> bleTask = nullptr;

void setup() {
  // put your setup code here, to run once:
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.println(LVGL_Arduino);

  globalOsc = std::make_shared<Oscillator>(44100, 1024);
  // create persistent task object (pin to core 1 for LVGL if you want)
  uiTask = std::make_shared<UITask>("LVGL Task", 1, globalOsc);
  //delay(500);
  bleTask = std::make_shared<BLETask>("BLE Task", 0, globalOsc);
  uiTask->start();
  delay(500);
  bleTask->start();
  delay(500);
}

void loop() {
  // No more main loop
}
