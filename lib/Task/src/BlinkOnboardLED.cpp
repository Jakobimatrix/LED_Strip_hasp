#include <Task/BlinkOnboardLED.hpp>

#include <Arduino.h>

#include <Globals.hpp>
#include <Task/TaskPriority.hpp>

namespace task {


void ledBlinkTask(void* pvParameters) {
  auto* self = static_cast<TaskBlinkOnboardLED*>(pvParameters);

  self->startCycleTime();
  self->taskWiFi.setWifiOn(true);

  while (!self->isStopRequested()) {
    digitalWrite(self->ledPin, self->ledState);
    self->ledState = (self->ledState == LOW) ? HIGH : LOW;

    self->handleWiFiTask();

    self->logStackHighWaterMark(dbg::TOPIC::LED);

    /*
    if (digitalRead(RESET_PIN) == LOW) {
      delay(50);  // debounce
      if (digitalRead(RESET_PIN) == LOW) {
        // self->taskWiFi.resetWiFi();
        glob::dbgLedLogger.log(dbg::LEVEL::INFO,
                               dbg::TOPIC::LED,
                               "Reset detected. Cleared credentials.");
      }
    }*/
    self->sleepFixedRate(self->getPeriodSleep());
  }

  self->shutdown();
}

void TaskBlinkOnboardLED::handleWiFiTask() {
  if (!taskWiFi.isRunning()) {
    const bool success{taskWiFi.start()};
    if (!success) {
      glob::dbgLedLogger.log(
        dbg::LEVEL::INFO, dbg::TOPIC::LED, "Failed to start WiFi Task!");
    }
    return;
  }
  const WIFI_STATUS current_wifi_status{taskWiFi.getWifiStatus()};
  switch (current_wifi_status) {
    case WIFI_STATUS::CONNECTED:
      current_led_pattern = LED_PATTERN::HEART_BEAT;
      return;
    case WIFI_STATUS::CONNECTING:
      current_led_pattern = LED_PATTERN::WIFI_CONNECTION;
      return;
    case WIFI_STATUS::NO_WIFI:
      current_led_pattern = LED_PATTERN::NO_WIFI_ERROR;
      return;
    case WIFI_STATUS::NEED_PROVISIONING:
      current_led_pattern = LED_PATTERN::NEED_PROVISIONING;
      return;
    case WIFI_STATUS::IDLE:
      current_led_pattern = LED_PATTERN::IDLE;
      return;
    case WIFI_STATUS::OFF:
      taskWiFi.setWifiOn(true);
      current_led_pattern = LED_PATTERN::WIFI_CONNECTION;
      return;
  }
}

TickType_t TaskBlinkOnboardLED::getPeriodSleep() {
  ++current_period_index;
  if (current_period_index >= NUM_PERIODS) {
    current_period_index = 0;
  }
  switch (current_led_pattern) {
    case LED_PATTERN::HEART_BEAT:
      return HEART_BEAT_PATTERN[current_period_index];
    case LED_PATTERN::NEED_PROVISIONING:
      return NEED_PROVISIONING_PATTERN[current_period_index];
    case LED_PATTERN::WIFI_CONNECTION:
      return WIFI_CONNECTION_PATTERN[current_period_index];
    case LED_PATTERN::NO_WIFI_ERROR:
      return NO_WIFI_ERROR_PATTERN[current_period_index];
    case LED_PATTERN::IDLE:
      return IDLE_PATTERN[current_period_index];
    default:
      return HEART_BEAT_PATTERN[current_period_index];
  }
}

bool TaskBlinkOnboardLED::setup() {
  name = "BlinkOnboardLED";
  pinMode(ledPin, OUTPUT);
  pinMode(RESET_PIN, INPUT_PULLUP);
  return pdPASS == createTask(ledBlinkTask,
                              "LED Blink Task",
                              TaskBlinkOnboardLED::STACK_DEPTH,
                              this,
                              task::LED_TASK_PRIORITY,
                              Task::RealTimeCore);
}

void TaskBlinkOnboardLED::shutdown() {
  pinMode(ledPin, INPUT);
  deleteHandle();
}

}  // namespace task
