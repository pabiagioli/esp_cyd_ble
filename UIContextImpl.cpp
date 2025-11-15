#pragma once
#include "UIContext.h"

void UIContext::popup_cb(lv_event_t *e) {
    lv_obj_t *mbox = (lv_obj_t *)lv_event_get_user_data(e);
    lv_obj_t *btn = lv_event_get_target_obj(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    LV_UNUSED(label);
    LV_LOG_USER("Button %s clicked", lv_label_get_text(label));
    lv_msgbox_close(mbox);
  }

  void UIContext::lv_popup(const char *title, const char *message) {
    lv_obj_t *mbox1 = lv_msgbox_create(NULL);

    lv_msgbox_add_title(mbox1, title);

    lv_msgbox_add_text(mbox1, message);
    lv_msgbox_add_close_button(mbox1);

    lv_obj_t *btn;
    btn = lv_msgbox_add_footer_button(mbox1, "OK");

    lv_obj_add_event_cb(btn, UIContext::popup_cb, LV_EVENT_CLICKED, mbox1);
    return;
  }
  void UIContext::touchscreen_read(lv_indev_t *indev, lv_indev_data_t *data) {
    UIContext *self = static_cast<UIContext *>(indev->user_data);
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

  void UIContext::log_print(lv_log_level_t level, const char *buf) {
    LV_UNUSED(level);
    Serial.println(buf);
    Serial.flush();
  }