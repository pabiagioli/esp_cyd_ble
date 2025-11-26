#pragma once
#include "FPOscillator.hpp"
#include <lvgl.h>
#include <memory>

class SettingsView {
public:
    explicit SettingsView(std::shared_ptr<FPOscillator> osc);
    ~SettingsView() = default;

    // create widgets on given parent (usually lv_scr_act())
    void create(lv_obj_t *parent);

private:
    std::shared_ptr<FPOscillator> m_osc;

    // lv objects
    lv_obj_t *m_freqLabel = nullptr;
    lv_obj_t *m_freqSlider = nullptr;
    lv_obj_t *m_ampLabel = nullptr;
    lv_obj_t *m_ampSlider = nullptr;
    lv_obj_t *m_waveDropdown = nullptr;
    lv_obj_t *m_startBtn = nullptr;

    // instance handlers
    void onFreqChanged(lv_event_t *e);
    void onAmpChanged(lv_event_t *e);
    void onWaveChanged(lv_event_t *e);
    void onBtnClicked(lv_event_t *e);

    // static trampolines
    static void s_freqChanged(lv_event_t *e);
    static void s_ampChanged(lv_event_t *e);
    static void s_waveChanged(lv_event_t *e);
    static void s_btnClicked(lv_event_t *e);

    void updateFreqLabel(int v);
};