#include <U8g2lib.h>
//Taster Pin 22
//SCL = Pin 19
//SDA = Pin 18
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, 19, 18, U8X8_PIN_NONE);

const int BUTTON_PIN = 22;

const int BUTTON2_PIN = 23;

int presetNumber = 0;

void setup(void) {
  
  pinMode(BUTTON_PIN, INPUT);
  u8g2.begin();
  
  u8g2.setFont(u8g2_font_6x13B_t_cyrillic);     

}


void loop(void) {
  if (digitalRead(BUTTON_PIN)) {
    presetNumber++;

    char Ausgabe[20];

    sprintf(Ausgabe, "Preset Page %d", presetNumber);

    u8g2.clearBuffer();
    u8g2.drawStr(12, 25, Ausgabe);
    u8g2.sendBuffer();
    while(digitalRead(BUTTON_PIN));
  }
  if(digitalRead(BUTTON2_PIN)&& presetNumber > 0){
    presetNumber--;

    char Ausgabe[20];

    sprintf(Ausgabe, "Preset Page %d", presetNumber);

    u8g2.clearBuffer();
    u8g2.drawStr(12, 25, Ausgabe);
    u8g2.sendBuffer();
    while(digitalRead(BUTTON2_PIN));

  }
}	
