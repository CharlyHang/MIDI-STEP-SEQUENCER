#include "Anzeige.h"

OLED_Anzeige::OLED_Anzeige(int SCL, int SDA)
    :my_u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE)
{
}

void OLED_Anzeige::initialize(){
    my_u8g2.begin();
    my_u8g2.setFont(u8g2_font_6x13B_t_cyrillic);
}

void OLED_Anzeige::set_Text(const String& Text){
    my_Text = Text;
}

void OLED_Anzeige::display(){
    my_u8g2.clearBuffer();
    my_u8g2.drawStr(12, 25, my_Text.c_str());
    my_u8g2.sendBuffer(); 
}