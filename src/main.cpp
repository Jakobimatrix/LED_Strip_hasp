#include <Arduino.h>

#include <color/color.hpp>

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
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