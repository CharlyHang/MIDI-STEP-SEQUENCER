#include <Arduino.h>
#include <U8g2lib.h>

//Taster Pin 22
//SCL = Pin 19
//SDA = Pin 18
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, 19, 18, U8X8_PIN_NONE);

#define ClockButtonSwitch 33

#define LED1 32
#define LED2 27
#define LED3 26

int counter = 0;


const char* ClockStates[3] = {"MIDI", "Intern", "Analog"}; //analog, intern, midi clocks
const char* Ausgabe;

void setup() {
  pinMode(ClockButtonSwitch, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  Serial.begin(9600);

  u8g2.begin();
  u8g2.setFont(u8g2_font_6x13B_t_cyrillic);  
  Ausgabe = "Idle";
  u8g2.clearBuffer();
  u8g2.drawStr(12, 25, Ausgabe);
  u8g2.sendBuffer();
}

void ClockCount(int counter) {
  if(counter % 3 == 0) {
    Serial.println("Selected Clock: MIDI");
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);

    Ausgabe = ClockStates[0];
    u8g2.clearBuffer();
    u8g2.drawStr(12, 25, Ausgabe);
    u8g2.sendBuffer();
  }
  else if(counter % 3 == 1) {
    Serial.println("Selected Clock: Intern");
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, LOW);

    Ausgabe = ClockStates[1];
    u8g2.clearBuffer();
    u8g2.drawStr(12, 25, Ausgabe);
    u8g2.sendBuffer();
  }
  else if(counter % 3 == 2) {
    Serial.println("Selected Clock: Analog");
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, HIGH);

    Ausgabe = ClockStates[2];
    u8g2.clearBuffer();
    u8g2.drawStr(12, 25, Ausgabe);
    u8g2.sendBuffer();
  }
  else {
    Serial.println("ClockCount Error");
  }
}

void loop() {
  if(digitalRead(ClockButtonSwitch) == HIGH){
    ClockCount(counter);
    counter++;
    delay(300);
    while(digitalRead(ClockButtonSwitch) == HIGH);
  }
}