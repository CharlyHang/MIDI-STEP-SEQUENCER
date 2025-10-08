#include <U8g2lib.h>
#include <Arduino.h>

// OLED-Display über I2C initialisieren (z. B. SSD1306 128x64)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

int presetNummer = 2;

void setup() {
  u8g2.begin();
}

void loop() {

  char text[20];
  sprintf(text, "%d", presetNummer);

  // Anzeige löschen & Schriftart setzen
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);

  // Text zentrieren (optional)
  int x = (128 - u8g2.getStrWidth(text)) / 2;
  int y = (64 + u8g2.getAscent() - u8g2.getDescent()) / 2;

  u8g2.drawStr(x, y, text);
  u8g2.sendBuffer();
  delay(1000);
}