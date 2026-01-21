#pragma once
#include <Arduino.h>

class Button {
    public:
        Button(int pin);
        bool pressed();

    protected:
        const int my_pin;
        int lastState = LOW;
        unsigned long lastChange = 0;
        const unsigned long DEBOUNCE_MS = 50;

};