#include <Arduino.h>

#include <Task/Debugging.hpp>
#include <Task/BlinkOnboardLED.hpp>

task::TaskDebugging taskDebugging;
task::TaskBlinkOnboardLED taskBlinkOnboardLED;


void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("BOOT: serial ready");

  if (!taskDebugging.start()) {
    Serial.println("BOOT: Failed to start debugger task.");
  }

  if (!taskBlinkOnboardLED.start()) {
    Serial.println("BOOT: Failed to start LED blink task.");
  }

  Serial.println("BOOT: setup done.");
}

void loop() {
  // dont do anything in the loop, all tasks are handled in separate threads
  vTaskDelay(portMAX_DELAY);
}
