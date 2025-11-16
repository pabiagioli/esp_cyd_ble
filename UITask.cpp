#pragma once
#include "UITask.hpp"

void UITask::touchscreen_read(lv_indev_t* indev, lv_indev_data_t* data) {
  UITask* self = static_cast<UITask*>(indev->user_data);
  // Checks if Touchscreen was touched, and prints X, Y and Pressure (Z)
  if (self->touchscreen.tirqTouched() && self->touchscreen.touched()) {
    // Get Touchscreen points
    TS_Point p = self->touchscreen.getPoint();
    // Calibrate Touchscreen points with map function to the correct width and height
    self->touch_coords.x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    self->touch_coords.y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    self->touch_coords.z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;

    // Set the coordinates
    data->point.x = self->touch_coords.x;
    data->point.y = self->touch_coords.y;

  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void UITask::log_print(lv_log_level_t level, const char* buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

void UITask::startUIFramework() {
  // Start LVGL
  lv_init();

  // Register print function for debugging
  lv_log_register_print_cb(log_print);

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  // Set the Touchscreen rotation in landscape mode
  // Note: in some displays, the touchscreen might be upside down, so you might need to set the rotation to 3 : touchscreen.setRotation(3);
  touchscreen.setRotation(2);
  // Create a display object
  lv_display_t* disp;
  // Initialize the TFT display using the TFT_eSPI library
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
  lv_display_set_resolution(disp, SCREEN_WIDTH, SCREEN_HEIGHT);
  // Initialize an LVGL input device object (Touchscreen)
  lv_indev_t* indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  // Set the callback function to read Touchscreen input
  lv_indev_set_read_cb(indev, touchscreen_read);
  lv_indev_set_user_data(indev, this);
}

void UITask::setUITheme() {
  lv_disp_t* dispp = lv_disp_get_default();
  lv_theme_t* theme = lv_theme_default_init(dispp,
                                            lv_palette_main(LV_PALETTE_RED),
                                            lv_palette_main(LV_PALETTE_RED),
                                            true,
                                            LV_FONT_DEFAULT);
  lv_disp_set_theme(dispp, theme);
}