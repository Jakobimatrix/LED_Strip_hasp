#include <Arduino.h>

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  Serial.println("off");
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  Serial.println("on");
}