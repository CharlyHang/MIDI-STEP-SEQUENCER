#include <Arduino.h>

#define CLK 26
#define DT 25

// Variablen
int counter = 0;
int currentStateCLK;
int lastStateCLK;

void setup() {
  Serial.begin(9600);

  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);

  // Startzustand lesen
  lastStateCLK = digitalRead(CLK);

  Serial.println("Rotary Encoder gestartet !");
}

void loop() {
  // current state clk
  currentStateCLK = digitalRead(CLK);

  if (currentStateCLK != lastStateCLK && currentStateCLK == HIGH) {
    
    if (digitalRead(DT) != currentStateCLK){
      counter++; //rechts
    } else {
      counter--; //links
    }

    Serial.print("Counter: ");
    Serial.println(counter);
  }

  // Letzten CLK Zustand speichern
  lastStateCLK = currentStateCLK;

  delay(1);
}
