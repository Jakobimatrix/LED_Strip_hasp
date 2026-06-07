#pragma once

#include <Arduino.h>

#include <Task/Task.hpp>

namespace task {

void ledBlinkTask(void* pvParameters);

class TaskBlinkOnboardLED : public Task {
  friend void ledBlinkTask(void* pvParameters);

  constexpr static uint32_t STACK_DEPTH{2048};
  [[nodiscard]] bool setup() override;
  void shutdown() override;

  uint8_t ledState{1};
  int ledPin{5};
  static constexpr TickType_t period = pdMS_TO_TICKS(1000);
};

}  // namespace task
