#pragma once
#include <memory>
#include "Oscillator.hpp"
#include "ITask.hpp"
#include "AudioBLE.hpp"


class BLETask : public ITask {
private:
  std::shared_ptr<Oscillator> m_osc;
  std::shared_ptr<AudioBLE> bleServer;

public:
  explicit BLETask(const char* name, uint32_t core, std::shared_ptr<Oscillator> osc)
    : ITask(name, core, 1024 * 8), m_osc(osc) {}

  void setup() override {
    bleServer = std::make_shared<AudioBLE>(m_osc);
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