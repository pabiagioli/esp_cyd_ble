#include <optional>
#include <functional>
#include <memory>
#include <lvgl.h>
#include <FS.h>
#include <LittleFS.h>
#include "UIContext.h"
#include "AudioBLE.hpp"

std::unique_ptr<UIContext> uiContext;
// Create oscillator and subsystems
std::shared_ptr<Oscillator> osc = std::make_shared<Oscillator>(44100, 1024);
std::shared_ptr<AudioBLE> audioBLE;

hw_timer_t* lvglTickTimer = nullptr;

void IRAM_ATTR lvglTickISR() {
    lv_tick_inc(1);  // tell LVGL that 1ms passed
}

void setupLVGLTick() {
    // 1000Hz → 1ms per tick
    lvglTickTimer = timerBegin(1000);  

    timerAttachInterrupt(lvglTickTimer, &lvglTickISR);

    // Fire every 1 timer tick (1ms)
    timerAlarm(
        lvglTickTimer,
        1,          // alarm value
        true,       // autoreload
        0           // reload value unused in autoreload
    );

    timerStart(lvglTickTimer);

    Serial.println("LVGL tick timer started");
}

void setup() {
  // put your setup code here, to run once:
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.println(LVGL_Arduino);
  LittleFS.begin(true);

  std::unique_ptr<UIContext> ctx = std::make_unique<UIContext>(osc);
  uiContext = std::move(ctx);
  audioBLE = std::make_shared<AudioBLE>(osc);

  // Start LVGL
  lv_init();
  // Register print function for debugging
  lv_log_register_print_cb(UIContext::log_print);

  setupLVGLTick();

  // Start the SPI for the touchscreen and init the touchscreen
  uiContext->touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  uiContext->touchscreen.begin(uiContext->touchscreenSPI);
  // Set the Touchscreen rotation in landscape mode
  // Note: in some displays, the touchscreen might be upside down, so you might need to set the rotation to 3: touchscreen.setRotation(3);
  uiContext->touchscreen.setRotation(2);
  // Create a display object
  lv_display_t *disp;
  // Initialize the TFT display using the TFT_eSPI library
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, uiContext->draw_buf, sizeof(uiContext->draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
  lv_display_set_resolution(disp, SCREEN_WIDTH, SCREEN_HEIGHT);

  // Initialize an LVGL input device object (Touchscreen)
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  // Set the callback function to read Touchscreen input
  lv_indev_set_read_cb(indev, UIContext::touchscreen_read);
  lv_indev_set_user_data(indev, uiContext.get());

  lv_disp_t *dispp = lv_disp_get_default();
  //lv_theme_simple_init(dispp);
  
  lv_theme_t *theme = lv_theme_default_init(dispp, 
    lv_palette_main(LV_PALETTE_RED), 
    lv_palette_main(LV_PALETTE_NONE), 
    true, 
    LV_FONT_DEFAULT);
  lv_disp_set_theme(dispp, theme);
  audioBLE->begin();
  //audioBLE->start();
  uiContext->updateScreen();
}

void loop() {
  lv_task_handler();
  delay(2);            // Allows BLE stack to respond
  audioBLE->update();  // Very lightweight only
}
