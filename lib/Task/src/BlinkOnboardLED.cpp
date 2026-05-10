#include <Task/BlinkOnboardLED.hpp>

#include <Arduino.h>

#include <Globals.hpp>

namespace task {


void ledBlinkTask(void* pvParameters) {


  TickType_t lastWakeTime = xTaskGetTickCount();

  while (true) {
    digitalWrite(LED_BUILTIN, ledState);
    dbgLedLogger.log(
      dbg::LEVEL::INFO, dbg::TOPIC::LED, "LED state: ", (ledState == LOW) ? "On" : "Off");
    ledState = (ledState == LOW) ? HIGH : LOW;

    // precise periodic delay
    vTaskDelayUntil(&lastWakeTime, period);
  }
}

bool TaskBlinkOnboardLED::setup() {
  pinMode(ledPin, OUTPUT);
  return pdFREERTOS_ERRNO_NONE == createTask(ledBlinkTask,
                                             "LED Blink Task",
                                             TaskBlinkOnboardLED::STACK_DEPTH,
                                             NULL,
                                             glob::LED_TASK_PRIORITY,
                                             Task::RealTimeCore);
}

void TaskBlinkOnboardLED::shutdown() { pinMode(ledPin, INPUT); }

}  // namespace task
