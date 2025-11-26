#include "GraphView.hpp"


GraphView::GraphView(
  std::shared_ptr<RingBuffer<float, cfg::AUDIO_BUFFER_SIZE>> buffer,
  uint16_t sampleCount,
  uint32_t updateMs)
  : audioBuffer(buffer),
    m_sampleCount(sampleCount),
    m_updateMs(updateMs) {
  //resampler = new FractionalResampler(44100, 100);
}

void GraphView::initChart(lv_obj_t* parent) {

  // Create chart object
  m_chart = lv_chart_create(parent);
  lv_obj_set_size(m_chart, LV_PCT(100), LV_PCT(100));

  lv_chart_set_type(m_chart, LV_CHART_TYPE_LINE);
  //lv_chart_set_update_mode(m_chart, LV_CHART_UPDATE_MODE_CIRCULAR);
  lv_obj_set_style_size(m_chart, 0, 0, LV_PART_INDICATOR);
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

void GraphView::update() {
  constexpr int CHUNK = 448;  // expected approximate chunk
  float samples[CHUNK];


  int popped = audioBuffer->pop(samples, CHUNK);
  if (popped <= 0) return;
  //float raw;
  float sample;
  //while (audioBuffer->pop(raw) && !resampler->process(raw, sample)) {}


  for (int i = 0; i < popped; i++) {
    sample = samples[i];

    // clamp
    if (sample < -1.0f) sample = -1.0f;
    if (sample > 1.0f) sample = 1.0f;

    uint16_t y = static_cast<uint16_t>((sample + 1.0f) * 50.0f);
    lv_chart_set_next_value(m_chart, m_series, y);
  }

  /*
  uint32_t p = lv_chart_get_point_count(m_chart);
  uint32_t s = lv_chart_get_x_start_point(m_chart, m_series);
  int32_t* a = lv_chart_get_series_y_array(m_chart, m_series);

  a[(s + 1) % p] = LV_CHART_POINT_NONE;
  a[(s + 2) % p] = LV_CHART_POINT_NONE;
  //lv_chart_refresh(m_chart);
  */
}
