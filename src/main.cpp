#include <Arduino.h>

#include <Types/SpscQueue.hpp>
#include <Task/Debugging.hpp>
#include <Task/BlinkOnboardLED.hpp>

task::TaskDebugging taskDebugging;
task::TaskBlinkOnboardLED taskBlinkOnboardLED;


void setup() {

  pinMode(LED_BUILTIN, OUTPUT);

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


/*
#include <WiFi.h>
#include <Preferences.h>

#include <FastLED.h>


#include <color/color.hpp>

#include <TaskDebugging.hpp>



TaskDebugging taskDebugging;



TaskHandle_t ledTaskHandle;
TaskHandle_t stateTaskHandle;
TaskHandle_t mqttTaskHandle;



#define LED_PIN  5
#define NUM_LEDS 30

CRGB leds[NUM_LEDS];

void ledTask(void* pvParameters) {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

  uint8_t hue = 0;

  while (true) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(hue, 255, 255);
    }

    FastLED.show();  // uses RMT → hardware timing

    hue++;
    vTaskDelay(pdMS_TO_TICKS(20));  // timing not critical anymore
  }
}

void ledTask(void* pvParameters) {
  const TickType_t period = pdMS_TO_TICKS(1000);
  TickType_t lastWakeTime = xTaskGetTickCount();

  uint8_t ledState = LOW;

  while (true) {
    digitalWrite(LED_BUILTIN, ledState);
    ledState = (ledState == LOW) ? HIGH : LOW;

    // precise periodic delay
    vTaskDelayUntil(&lastWakeTime, period);
  }
}

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);

  if (taskDebugging.start()) {
    Serial.println("Debugger task started successfully.");
  } else {
    Serial.begin(9600);
    Serial.println("Failed to start debugger task.");
  }

  xTaskCreatePinnedToCore(sensorTask, "Sensor Task", 2048, NULL, 3, &sensorTaskHandle, 1);
  xTaskCreatePinnedToCore(ledTask, "LED Task", 2048, NULL, 3, &ledTaskHandle, 1);
  xTaskCreatePinnedToCore(wifiTask, "WiFi Task", 4096, NULL, 1, &wifiTaskHandle, 0);
  xTaskCreatePinnedToCore(mqttTask, "MQTT Task", 4096, NULL, 2, &mqttTaskHandle, 0);
}

void loop() {
  color::RGB<int, 3U> color{255, 55, 0};
  digitalWrite(LED_BUILTIN, HIGH);
  delay(color.r() * 4);
  Serial.println("off");
  digitalWrite(LED_BUILTIN, LOW);
  delay(color.g() * 4);
  Serial.println("on");
}

void loop() {
  // dont do anything in the loop, all tasks are handled in separate threads
  vTaskDelay(portMAX_DELAY);
}
  */