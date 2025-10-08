#include <U8g2lib.h>
#include <Arduino.h>

// OLED Display starten mit I2C
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void setup() {
  u8g2.begin();               // Display starten
  u8g2.clearBuffer();         // Puffer leeren
  u8g2.setFont(u8g2_font_ncenB14_tr); // Schriftart setzen
  Serial.begin(9600);         // optional zum Debuggen
}

void loop() {
  int state = 0; // Hier später deinen Wert einfügen (z.B. Schalterzustand)

  // Beispiel: Text auf Display
  u8g2.clearBuffer();          // Puffer leeren
  u8g2.setCursor(0, 30);       // Textposition
  u8g2.print("State: ");
  u8g2.print(state);
  u8g2.sendBuffer();           // Puffer auf Display übertragen

  delay(100);                  // kleines Delay
}
