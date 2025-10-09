#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#define ClockButtonSwitch 12

#define LED1 32
#define LED2 27
#define LED3 26


#define SDA_PIN 13
#define SCL_PIN 25


U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, SDA_PIN, SCL_PIN, U8X8_PIN_NONE);


int counter = 0;
String ClockStates[3] = {"MIDI", "Intern", "Analog"}; //analog, intern, midi clocks


void setup() {
  pinMode(ClockButtonSwitch, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  Serial.begin(9600);

  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);  

 

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

 
  u8g2.clearBuffer();
  u8g2.drawStr(0, 10, "Hallo OLED");
  u8g2.sendBuffer();
  
}

