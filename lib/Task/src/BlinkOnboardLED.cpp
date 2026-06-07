#include <Task/BlinkOnboardLED.hpp>

#include <Arduino.h>

#include <Globals.hpp>

namespace task {


void ledBlinkTask(void* pvParameters) {
  auto* self = static_cast<TaskBlinkOnboardLED*>(pvParameters);

  TickType_t lastWakeTime = xTaskGetTickCount();

  while (true) {
    digitalWrite(LED_BUILTIN, self->ledState);
    glob::dbgLedLogger.log(
      dbg::LEVEL::INFO, dbg::TOPIC::LED, "LED state: ", (self->ledState == LOW) ? "On" : "Off");

    glob::dbgLedLogger.log(dbg::LEVEL::INFO,
                           dbg::TOPIC::LED,
                           "Stack size: ",
                           self->getStackDepth(),
                           " bytes, High Water Mark: ",
                           self->getStackHighWaterMark(),
                           " bytes");
    self->ledState = (self->ledState == LOW) ? HIGH : LOW;

    // precise periodic delay
    vTaskDelayUntil(&lastWakeTime, self->period);
  }
}

bool TaskBlinkOnboardLED::setup() {
  pinMode(ledPin, OUTPUT);
  return pdFREERTOS_ERRNO_NONE == createTask(ledBlinkTask,
                                             "LED Blink Task",
                                             TaskBlinkOnboardLED::STACK_DEPTH,
                                             this,
                                             glob::LED_TASK_PRIORITY,
                                             Task::RealTimeCore);
}

void TaskBlinkOnboardLED::shutdown() { pinMode(ledPin, INPUT); }

}  // namespace task
