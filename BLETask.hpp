#pragma once
#include <memory>
#include "Oscillator.hpp"
#include "ITask.hpp"
#include "AudioBLE.hpp"


class BLETask : public ITask {
private:
  //std::shared_ptr<Oscillator> m_osc;
  std::shared_ptr<AudioBLE> bleServer;

public:
  explicit BLETask(std::shared_ptr<Oscillator> osc)
    : ITask("BLE Task", 0), bleServer(std::make_shared<AudioBLE>(osc)) {}

  void setup() override {
    bleServer->begin();
    bleServer->start();
  }

  void loop() override {
    bleServer->update();
    vTaskDelay(pdMS_TO_TICKS(2));
  }
};