#include <Arduino.h>
#include <U8g2lib.h>

#define ClockButtonSwitch 12

#define LED1 32
#define LED2 27
#define LED3 26



int counter = 0;
String ClockStates[3] = {"MIDI", "Intern", "Analog"}; //analog, intern, midi clocks


void setup() {
  pinMode(ClockButtonSwitch, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
      
  Serial.begin(9600);
}

void ClockCount(int counter) {
  if(counter % 3 == 0) {
    Serial.println("Selected Clock: " + ClockStates[0]);
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
  }
  else if(counter % 3 == 1) {
    Serial.println("Selected Clock: " + ClockStates[1]);
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, LOW);
  }
  else if(counter % 3 == 2) {
    Serial.println("Selected Clock: " + ClockStates[2]);
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, HIGH);
  }
  else {
    Serial.println("ClockCount Error");
  }
}

void loop() {
  if(digitalRead(ClockButtonSwitch) == HIGH){
    ClockCount(counter);
    counter++;
    while(digitalRead(ClockButtonSwitch) == HIGH);
  }
  
}

