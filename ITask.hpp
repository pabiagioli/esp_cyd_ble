#pragma once
#include <Arduino.h>
#include <memory>
#include <string>

/// Abstract class for making Dedicated Core Tasks
class ITask {

public:
  explicit ITask(const char* task_name,
                 const BaseType_t pinned_core,
                 const uint32_t task_stack_size = 1024 * 32,
                 UBaseType_t task_priority = 1)
    : m_task_name(task_name), m_pinned_core(pinned_core), m_task_stack_size(task_stack_size), m_task_priority(task_priority) {}

  void start() {
    BaseType_t result = xTaskCreatePinnedToCore(
      TaskWrapper,     /* Task function. */
      m_task_name,       /* name of task. */
      m_task_stack_size, /* Stack size of task */
      this,            /* parameter of the task */
      m_task_priority,   /* priority of the task */
      &task,           /* Task handle to keep track of created task */
      m_pinned_core);    /* pin task to core 0 */
    if (result == pdPASS) {
      Serial.printf("--- Successfully created %s on core %d with priority %d ---\n", m_task_name, m_pinned_core, m_task_priority);
    } else {
      Serial.printf("--- FreeRTOS Task Creation Error: %d ---\n", result);
    }
  }
  // Pure virtual functions
  virtual void setup() = 0;
  virtual void loop() = 0;

  virtual ~ITask() = default;

private:
  const char* m_task_name;
  const BaseType_t m_pinned_core;
  const uint32_t m_task_stack_size;
  UBaseType_t m_task_priority;
  TaskHandle_t task;
  // Task function (similar to std::thread::run )
  static void TaskWrapper(void* pvParameters) {
    ITask* instance = static_cast<ITask*>(pvParameters);
    if (!instance) {
      vTaskDelete(nullptr);
      return;
    }
    instance->setup();
    for (;;) {
      instance->loop();
      // if loop() is expected to be non-blocking, add a tiny delay to yield CPU:
      //vTaskDelay(pdMS_TO_TICKS(1));
    }
  }
};