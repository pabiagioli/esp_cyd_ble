#pragma once
#include <memory>
#include "FPOscillator.hpp"
#include "ITask.hpp"
#include "AudioBLE.hpp"


class BLETask : public ITask {
private:
  std::shared_ptr<AudioBLE> bleServer;
  //std::shared_ptr<FPOscillator> m_osc;
  std::shared_ptr<RingBuffer<float, cfg::BLE_BUFFER_SIZE>> audioBuffer;

public:
  explicit BLETask(const char* name, uint32_t core,
                   //std::shared_ptr<FPOscillator> osc, 
                   std::shared_ptr<RingBuffer<float, cfg::BLE_BUFFER_SIZE>> buffer)
    : ITask(name, core, 1024 * 8, 5), /*m_osc(osc),*/ audioBuffer(buffer) {}

  void setup() override {
    bleServer = std::make_shared<AudioBLE>(audioBuffer);
    bleServer->begin();
    bleServer->start();
  }

  void loop() override {
    bleServer->update();
    //vTaskDelay(pdMS_TO_TICKS(2)); --> doesn't work, too tight
    //10ms = 100Hz
    //vTaskDelay(pdMS_TO_TICKS(10));
    //20ms = 50Hz - safer for BLE
    vTaskDelay(pdMS_TO_TICKS(20));
  }
};