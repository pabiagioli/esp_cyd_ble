#pragma once
#include "FPOscillator.hpp"
#include <Arduino.h>
#include "SettingsView.hpp"
#include <cstdio>
#include <memory>

SettingsView::SettingsView(std::shared_ptr<FPOscillator> osc) : m_osc(osc) {}

void SettingsView::create(lv_obj_t *parent) {
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
    m_osc->setFrequency(440);
    lv_obj_add_event_cb(m_freqSlider, SettingsView::s_freqChanged, LV_EVENT_VALUE_CHANGED, this);
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
    m_osc->setAmplitude(0.5);
    lv_obj_add_event_cb(m_ampSlider, SettingsView::s_ampChanged, LV_EVENT_VALUE_CHANGED, this);
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
    lv_obj_add_event_cb(m_waveDropdown, SettingsView::s_waveChanged, LV_EVENT_VALUE_CHANGED, this);
    lv_obj_set_grid_cell(m_waveDropdown,
        LV_GRID_ALIGN_START, 0, 2,
        LV_GRID_ALIGN_CENTER, 4, 1
    );
    m_osc->setWaveform(Waveform::Sine);

    // ========== Start Button ==========
    m_startBtn = lv_btn_create(cont);
    lv_obj_add_event_cb(m_startBtn, SettingsView::s_btnClicked, LV_EVENT_CLICKED, this);
    lv_obj_set_grid_cell(m_startBtn,
        LV_GRID_ALIGN_CENTER, 0, 2,
        LV_GRID_ALIGN_CENTER, 5, 1
    );

    lv_obj_t* btnLabel = lv_label_create(m_startBtn);
    lv_label_set_text(btnLabel, "Start");
}

void SettingsView::updateFreqLabel(int v) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "Freq: %d Hz", v);
    lv_label_set_text(m_freqLabel, buf);
}

void SettingsView::onFreqChanged(lv_event_t *e) {
    int v = lv_slider_get_value(m_freqSlider);
    updateFreqLabel(v);
    m_osc->setFrequency(static_cast<float>(v));
}

void SettingsView::onAmpChanged(lv_event_t *e) {
    int v = lv_slider_get_value(m_ampSlider);
    float result = static_cast<float>(v) / 100.0f;
    char buf[32];
    std::snprintf(buf, sizeof(buf), "Amplitude: %.1f", result);
    lv_label_set_text(m_ampLabel, buf);
    m_osc->setAmplitude(result);
}

void SettingsView::onWaveChanged(lv_event_t *e) {
    uint16_t sel = lv_dropdown_get_selected(m_waveDropdown);
    if (sel > 3) sel = 0;
    m_osc->setWaveform(static_cast<Waveform>(sel));
}

void SettingsView::onBtnClicked(lv_event_t *e) {
    Serial.printf("UIButton clicked — m_osc=%p, m_btnToggle=%p\n", (void*)m_osc.get(), (void*)m_startBtn);
    
    auto self = this; // instance method
    auto osc = m_osc; // shared_ptr copy
    if (!osc) {
        Serial.println("UI: no oscillator");
        return;
    }
    

    bool newState = osc->toggleEnabled();
    lv_obj_t *label = lv_obj_get_child(self->m_startBtn, 0);
    if (newState) {
        lv_obj_add_state(self->m_startBtn, LV_STATE_CHECKED);
        if(label) lv_label_set_text(label, "Stop");
    } else {
        lv_obj_clear_state(self->m_startBtn, LV_STATE_CHECKED);
        if(label) lv_label_set_text(label, "Start");
    }
}

/* static trampolines */
void SettingsView::s_freqChanged(lv_event_t *e) {
    SettingsView *self = static_cast<SettingsView*>(lv_event_get_user_data(e));
    if (self) self->onFreqChanged(e);
}
void SettingsView::s_ampChanged(lv_event_t *e) {
    SettingsView *self = static_cast<SettingsView*>(lv_event_get_user_data(e));
    if (self) self->onAmpChanged(e);
}
void SettingsView::s_waveChanged(lv_event_t *e) {
    SettingsView *self = static_cast<SettingsView*>(lv_event_get_user_data(e));
    if (self) self->onWaveChanged(e);
}
void SettingsView::s_btnClicked(lv_event_t *e) {
    //Serial.printf("--- UIButton clicked — e=%p ---\n", (void*)e);
    SettingsView *self = static_cast<SettingsView*>(lv_event_get_user_data(e));
    if (self) self->onBtnClicked(e);
}
