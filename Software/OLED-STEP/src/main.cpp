#include <U8g2lib.h>
//SCL = Pin 19
//SDA = Pin 18
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, 19, 18, U8X8_PIN_NONE);

#define CLK 22
#define DT 23

int presetPageMax = 12;
int presetPageMin = 1;

int presetNumber = 0;

int currentStateCLK;
int lastStateCLK;

void setup(void) {
  
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  u8g2.begin();
  
  u8g2.setFont(u8g2_font_6x13B_t_cyrillic);     
   lastStateCLK = digitalRead(CLK);
}

void loop() {
  // current state clk
  currentStateCLK = digitalRead(CLK);

  char Ausgabe[20];

  if (currentStateCLK != lastStateCLK && currentStateCLK == HIGH) {
    
    if (digitalRead(DT) != currentStateCLK){
     if(presetNumber<=presetPageMax) presetNumber++; //rechts
      

      sprintf(Ausgabe, "Preset Page %d", presetNumber);

      u8g2.clearBuffer();
      u8g2.drawStr(12, 25, Ausgabe);
      u8g2.sendBuffer();  

    } else {
      if(presetNumber>= presetPageMin) presetNumber--; //links
      sprintf(Ausgabe, "Preset Page %d", presetNumber);

      u8g2.clearBuffer();
      u8g2.drawStr(12, 25, Ausgabe);
      u8g2.sendBuffer();
    }


  }

  // Letzten CLK Zustand speichern
  lastStateCLK = currentStateCLK;

  delay(1);
}