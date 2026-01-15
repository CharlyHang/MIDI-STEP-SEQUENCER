#pragma once
#include <iostream>
#include <string.h>
#include <U8g2lib.h>

class OLED_Anzeige {
    public:
        OLED_Anzeige(int SCL, int SDA);
        ~OLED_Anzeige();
        void display();
        void set_Text(const String& Text);

    protected:
        U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C my_u8g2;
        String my_Text;
};