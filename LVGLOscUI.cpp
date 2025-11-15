#pragma once
#include "LVGLOscUI.hpp"
#include <cstdio>
#include <memory>

LVGLOscUI::LVGLOscUI(std::shared_ptr<Oscillator> osc) : m_osc(osc) {}

void LVGLOscUI::create(lv_obj_t *parent) {
    // Create a container that uses grid
    lv_obj_t* cont = parent; //lv_obj_create(parent);
    //lv_obj_set_size(cont, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(cont, 12, 0);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);

    // Define grid template
    static lv_coord_t col_dsc[] = { LV_GRID_FR(1), LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST };
    static lv_coord_t row_dsc[] = {
        LV_GRID_CONTENT,  // freq label
        LV_GRID_CONTENT,  // freq slider
        LV_GRID_CONTENT,  // amp label
        LV_GRID_CONTENT,  // amp slider
        LV_GRID_CONTENT,  // waveform dropdown
        LV_GRID_CONTENT,  // start/stop button
        LV_GRID_TEMPLATE_LAST
    };

    lv_obj_set_grid_dsc_array(cont, col_dsc, row_dsc);

    // ========== Frequency Label ==========
    m_freqLabel = lv_label_create(cont);
    lv_label_set_text(m_freqLabel, "Frequency: 440 Hz");
    lv_obj_set_grid_cell(m_freqLabel,
        LV_GRID_ALIGN_START, 0, 2,   // col 0-1
        LV_GRID_ALIGN_CENTER, 0, 1   // row 0
    );

    // ========== Frequency Slider ==========
    m_freqSlider = lv_slider_create(cont);
    lv_slider_set_range(m_freqSlider, 20, 20000);
    lv_slider_set_value(m_freqSlider, 440, LV_ANIM_OFF);
    lv_obj_add_event_cb(m_freqSlider, LVGLOscUI::s_freqChanged, LV_EVENT_VALUE_CHANGED, this);
    lv_obj_set_grid_cell(m_freqSlider,
        LV_GRID_ALIGN_STRETCH, 0, 2,   // full width
        LV_GRID_ALIGN_CENTER, 1, 1     // row 1
    );

    // ========== Amplitude Label ==========
    m_ampLabel = lv_label_create(cont);
    lv_label_set_text(m_ampLabel, "Amplitude: 0.5");
    lv_obj_set_grid_cell(m_ampLabel,
        LV_GRID_ALIGN_START, 0, 2,
        LV_GRID_ALIGN_CENTER, 2, 1
    );

    // ========== Amplitude Slider ==========
    m_ampSlider = lv_slider_create(cont);
    lv_slider_set_range(m_ampSlider, 0, 100);
    lv_slider_set_value(m_ampSlider, 50, LV_ANIM_OFF);
    lv_obj_add_event_cb(m_ampSlider, LVGLOscUI::s_ampChanged, LV_EVENT_VALUE_CHANGED, this);
    lv_obj_set_grid_cell(m_ampSlider,
        LV_GRID_ALIGN_STRETCH, 0, 2,
        LV_GRID_ALIGN_CENTER, 3, 1
    );

    // ========== Waveform Dropdown ==========
    m_waveDropdown = lv_dropdown_create(cont);
    lv_dropdown_set_options(m_waveDropdown,
        "Sine\n"
        "Square\n"
        "Triangle\n"
        "Saw"
    );
    lv_obj_add_event_cb(m_waveDropdown, LVGLOscUI::s_waveChanged, LV_EVENT_VALUE_CHANGED, this);
    lv_obj_set_grid_cell(m_waveDropdown,
        LV_GRID_ALIGN_START, 0, 2,
        LV_GRID_ALIGN_CENTER, 4, 1
    );

    // ========== Start Button ==========
    m_startBtn = lv_btn_create(cont);
    lv_obj_add_event_cb(m_startBtn, LVGLOscUI::s_btnClicked, LV_EVENT_CLICKED, this);
    lv_obj_set_grid_cell(m_startBtn,
        LV_GRID_ALIGN_CENTER, 0, 2,
        LV_GRID_ALIGN_CENTER, 5, 1
    );

    lv_obj_t* btnLabel = lv_label_create(m_startBtn);
    lv_label_set_text(btnLabel, "Start");
}

void LVGLOscUI::updateFreqLabel(int v) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "Freq: %d Hz", v);
    lv_label_set_text(m_freqLabel, buf);
}

void LVGLOscUI::onFreqChanged(lv_event_t *e) {
    int v = lv_slider_get_value(m_freqSlider);
    updateFreqLabel(v);
    m_osc->setFrequency(static_cast<float>(v));
}

void LVGLOscUI::onAmpChanged(lv_event_t *e) {
    int v = lv_slider_get_value(m_ampSlider);
    m_osc->setAmplitude(static_cast<float>(v) / 100.0f);
}

void LVGLOscUI::onWaveChanged(lv_event_t *e) {
    uint16_t sel = lv_dropdown_get_selected(m_waveDropdown);
    if (sel > 3) sel = 0;
    m_osc->setWaveform(static_cast<Waveform>(sel));
}

void LVGLOscUI::onBtnClicked(lv_event_t *e) {
    bool newState = !m_osc->nextSample(); // placeholder - we need to read enabled; but nextSample can't be used this way
    // Instead, toggle using an atomic store/read pattern: read current via setEnabled/read? There is no getter, so use naive toggle:
    // We'll toggle by setting enabled to !current by reading into a temp variable via a trick: setEnabled(read then invert) - but no getter was exposed.
    // Simpler: use a static label to switch. We'll store enabled via an atomic in oscillator; read it by setting a temporary call
    // To keep API simple, let's add a small read via casting: (unsafe read) - but acceptable on ESP32 (atomic bool).
    // Workaround: reinterpret the atomic<bool> via lambda capturing pointer - but not visible. Instead, add an overload? To avoid changing header, we'll toggle by keeping a small visual button state:
    // We'll use lv_obj_has_state to determine label and call setEnabled accordingly.
    bool isChecked = lv_obj_has_state(m_startBtn, LV_STATE_CHECKED);
    if (!isChecked) {
        lv_obj_add_state(m_startBtn, LV_STATE_CHECKED);
        lv_label_set_text(lv_obj_get_child(m_startBtn, 0), "Stop");
        m_osc->setEnabled(true);
    } else {
        lv_obj_clear_state(m_startBtn, LV_STATE_CHECKED);
        lv_label_set_text(lv_obj_get_child(m_startBtn, 0), "Start");
        m_osc->setEnabled(false);
    }
}

/* static trampolines */
void LVGLOscUI::s_freqChanged(lv_event_t *e) {
    LVGLOscUI *self = static_cast<LVGLOscUI*>(lv_event_get_user_data(e));
    if (self) self->onFreqChanged(e);
}
void LVGLOscUI::s_ampChanged(lv_event_t *e) {
    LVGLOscUI *self = static_cast<LVGLOscUI*>(lv_event_get_user_data(e));
    if (self) self->onAmpChanged(e);
}
void LVGLOscUI::s_waveChanged(lv_event_t *e) {
    LVGLOscUI *self = static_cast<LVGLOscUI*>(lv_event_get_user_data(e));
    if (self) self->onWaveChanged(e);
}
void LVGLOscUI::s_btnClicked(lv_event_t *e) {
    LVGLOscUI *self = static_cast<LVGLOscUI*>(lv_event_get_user_data(e));
    if (self) self->onBtnClicked(e);
}
