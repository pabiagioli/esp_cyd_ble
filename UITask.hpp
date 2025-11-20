#pragma once
#include <memory>
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

#include "ITask.hpp"
#include "GraphView.hpp"
#include "SettingsView.hpp"

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

class UITask : public ITask {
private:
  lv_obj_t* main_gui = nullptr;
  std::shared_ptr<SettingsView> oscUI;
  std::shared_ptr<GraphView> chartUI;
  std::shared_ptr<FPOscillator> m_osc;

  void startUIFramework();
  void setUITheme();

public:
  // public UI Config stuff
  tft_t touch_coords = { 0, 0, 0 };
  uint8_t draw_buf[DRAW_BUF_SIZE];
  TFT_eSPI tft = TFT_eSPI();
  SPIClass touchscreenSPI = SPIClass(VSPI);
  XPT2046_Touchscreen touchscreen = XPT2046_Touchscreen(XPT2046_CS, XPT2046_IRQ);

  explicit UITask(const char* name, uint32_t core, std::shared_ptr<FPOscillator> osc)
    : ITask(name, core, 1024 * 48), m_osc(osc) {}

  /// public static methods

  static void touchscreen_read(lv_indev_t* indev, lv_indev_data_t* data);
  static void log_print(lv_log_level_t level, const char* buf);
  
  /*--- inherited methods ---*/
  
  void setup() override {
    startUIFramework();
    setUITheme();

    main_gui = lv_tabview_create(NULL);
    lv_obj_t *setupTab = lv_tabview_add_tab(main_gui, "Setup");
    lv_obj_t *viewerTab = lv_tabview_add_tab(main_gui, "Viewer");

    //m_osc = std::make_shared<Oscillator>(44100, 1024);
    oscUI = std::make_shared<SettingsView>(m_osc);
    oscUI->create(setupTab);
    
    chartUI = std::make_shared<GraphView>(m_osc);
    chartUI->begin(viewerTab);

    lv_screen_load(main_gui);
  }

  void loop() override {
    // lvgl needs this to be called in a loop to run the interface
    lv_task_handler();  // let the GUI do its work
    lv_tick_inc(5);     // tell LVGL how much time has passed
    vTaskDelay(pdMS_TO_TICKS(5));
  }

};