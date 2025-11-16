// Including this library will also include lvgl.h and TFT_eSPI.h
#include <memory>
//#include <FS.h>
//#include <LittleFS.h>
#include <lvgl.h>
#include "Oscillator.hpp"
#include "UITask.hpp"

static std::shared_ptr<Oscillator> globalOsc = std::make_shared<Oscillator>(44100, 1024);

// allocate the UI task once and keep it alive
static std::shared_ptr<UITask> uiTask = nullptr;

void setup() {
  // put your setup code here, to run once:
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.println(LVGL_Arduino);

  // create persistent task object (pin to core 1 for LVGL if you want)
  uiTask = std::make_shared<UITask>("LVGL Task", 1, globalOsc);
  uiTask->start();
}

void loop() {
  // No more main loop
}
