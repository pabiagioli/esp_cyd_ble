#pragma once
#include <cstddef>
#include <optional>

#include <SPI.h>

/*  Install the "lvgl" library version 9.2 by kisvegabor to interface with the TFT Display - https://lvgl.io/
    *** IMPORTANT: lv_conf.h available on the internet will probably NOT work with the examples available at Random Nerd Tutorials ***
    *** YOU MUST USE THE lv_conf.h FILE PROVIDED IN THE LINK BELOW IN ORDER TO USE THE EXAMPLES FROM RANDOM NERD TUTORIALS ***
    FULL INSTRUCTIONS AVAILABLE ON HOW CONFIGURE THE LIBRARY: https://RandomNerdTutorials.com/cyd-lvgl/ or https://RandomNerdTutorials.com/esp32-tft-lvgl/   */
#include <lvgl.h>

/*  Install the "TFT_eSPI" library by Bodmer to interface with the TFT Display - https://github.com/Bodmer/TFT_eSPI
    *** IMPORTANT: User_Setup.h available on the internet will probably NOT work with the examples available at Random Nerd Tutorials ***
    *** YOU MUST USE THE User_Setup.h FILE PROVIDED IN THE LINK BELOW IN ORDER TO USE THE EXAMPLES FROM RANDOM NERD TUTORIALS ***
    FULL INSTRUCTIONS AVAILABLE ON HOW CONFIGURE THE LIBRARY: https://RandomNerdTutorials.com/cyd/ or https://RandomNerdTutorials.com/esp32-tft/   */
#include <TFT_eSPI.h>

// Install the "XPT2046_Touchscreen" library by Paul Stoffregen to use the Touchscreen - https://github.com/PaulStoffregen/XPT2046_Touchscreen
// Note: this library doesn't require further configuration
#include <XPT2046_Touchscreen.h>
#include <memory>
#include <FS.h>
#include "Oscillator.hpp"
#include "LVGLOscUI.hpp"

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define FONT_SIZE 2

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))

// Touchscreen coordinates: (x, y) and pressure (z)
typedef struct _tft_t {
  int x, y, z;
} tft_t;

class UIContext {
private:
  lv_obj_t *main_gui, *startTab, *bleTab, *aboutTab;
  static void popup_cb(lv_event_t *e);
public:
  explicit UIContext(std::shared_ptr<Oscillator> osc): m_osc(osc){}

  tft_t touch_coords = { 0, 0, 0 };
  uint32_t draw_buf[DRAW_BUF_SIZE / 4];
  TFT_eSPI tft = TFT_eSPI();
  SPIClass touchscreenSPI = SPIClass(VSPI);
  XPT2046_Touchscreen touchscreen = XPT2046_Touchscreen(XPT2046_CS, XPT2046_IRQ);
  
  std::shared_ptr<LVGLOscUI> oscUI;
  std::shared_ptr<Oscillator> m_osc;
  
  /// public static methods

  static void touchscreen_read(lv_indev_t *indev, lv_indev_data_t *data);
  static void log_print(lv_log_level_t level, const char *buf);
  
  /// instance methods
  
  void lv_popup(const char *title, const char *message);

  void updateScreen(void) {
    main_gui = lv_tabview_create(NULL);
    //lv_tabview_set_tab_bar_size(main_gui, 40);
    startTab = lv_tabview_add_tab(main_gui, "Start");
    bleTab = lv_tabview_add_tab(main_gui, "Settings");
    aboutTab = lv_tabview_add_tab(main_gui, "About");

    m_osc->setFrequency(440.0f);
    m_osc->setAmplitude(0.8f);
    m_osc->setWaveform(Waveform::Sine);
    m_osc->setEnabled(false);
    oscUI = std::make_shared<LVGLOscUI>(m_osc);
    oscUI->create(startTab);
    lv_screen_load(main_gui);
  }
};