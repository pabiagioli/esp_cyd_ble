#include "LVGLOscilloscope.hpp"

LVGLOscilloscope::LVGLOscilloscope(
  std::shared_ptr<Oscillator> osc,
  uint16_t sampleCount,
  uint32_t updateMs)
  : m_osc(osc),
    m_sampleCount(sampleCount),
    m_updateMs(updateMs) {}

void LVGLOscilloscope::initChart(lv_obj_t* parent) {
  // Create chart object
  m_chart = lv_chart_create(parent);
  lv_obj_set_size(m_chart, LV_PCT(100), LV_PCT(100));

  lv_chart_set_type(m_chart, LV_CHART_TYPE_LINE);
  lv_chart_set_point_count(m_chart, m_sampleCount);
  lv_chart_set_range(m_chart,
                     LV_CHART_AXIS_PRIMARY_Y,
                     0, 100);

  // Series = actual waveform
  m_series = lv_chart_add_series(m_chart,
                                 lv_palette_main(LV_PALETTE_GREEN),
                                 LV_CHART_AXIS_PRIMARY_Y);

  // Initialize with baseline
  for (int i = 0; i < m_sampleCount; i++)
    m_series->y_points[i] = 50;  // midline
}

void LVGLOscilloscope::update() {
  float s = m_osc->nextSample();

  // clamp -1..1
  if (s < -1.0f) s = -1.0f;
  if (s > 1.0f) s = 1.0f;

  // convert to 0..100
  uint16_t y = (uint16_t)((s + 1.0f) * 50.0f);

  lv_chart_set_next_value(m_chart, m_series, y);
}
