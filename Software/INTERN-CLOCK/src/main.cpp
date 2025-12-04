#include <Arduino.h>

#define CLOCK_PIN 19

unsigned int BPM = 60;
float BPS = BPM / 60.0;

void setup() {
  pinMode(CLOCK_PIN, OUTPUT);
}

void loop() {
  digitalWrite(CLOCK_PIN, HIGH);
  delay(50);
  digitalWrite(CLOCK_PIN, LOW);
  delay(200);
}
