#include <Arduino.h>

#include <Task/Debugging.hpp>
#include <Task/BlinkOnboardLED.hpp>

task::TaskDebugging taskDebugging;
task::TaskBlinkOnboardLED taskBlinkOnboardLED;


void setup() {
  if (!taskDebugging.start()) {
    Serial.begin(9600);
    Serial.println("Failed to start debugger task.");
  }

  if (!taskBlinkOnboardLED.start()) {
    Serial.println("Failed to start LED blink task.");
  }
}

void loop() {
  // dont do anything in the loop, all tasks are handled in separate threads
  vTaskDelay(portMAX_DELAY);
}
