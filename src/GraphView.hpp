#pragma once
#include <memory>
#include <vector>
#include <lvgl.h>
#include "RingBuffer.hpp"
#include "pampanet_config.h"


class GraphView {
public:
  GraphView(std::shared_ptr<RingBuffer<float, cfg::AUDIO_BUFFER_SIZE>> buffer,
            uint16_t sampleCount = 200,
            uint32_t updateMs = 10);

  ~GraphView() = default;

  void begin(lv_obj_t* parent) {
    initChart(parent);

    // Create an LVGL timer that calls update()
    timer = lv_timer_create([](lv_timer_t* t) {
      GraphView* scope =
        static_cast<GraphView*>(t->user_data);
      scope->update();
    },
                            m_updateMs, this);
    lv_timer_resume(timer);
  }
  // optional: external refresh call if needed
  void update();

private:
  lv_timer_t* timer;
  void initChart(lv_obj_t* parent);

  std::shared_ptr<RingBuffer<float, cfg::AUDIO_BUFFER_SIZE>> audioBuffer;

  lv_obj_t* m_chart = nullptr;
  lv_chart_series_t* m_series = nullptr;

  uint16_t m_sampleCount;
  uint32_t m_updateMs;
  //FractionalResampler *resampler;

};
