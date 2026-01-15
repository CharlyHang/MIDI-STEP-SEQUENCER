#include "Button.h"

Button::Button(int pin)
    : my_pin(pin)
{
    pinMode(my_pin, INPUT);
}

bool Button::pressed() {
    int state = digitalRead(my_pin);

    if (state != lastState) {
        lastChange = millis();
        lastState = state;
    }

    if ((millis() - lastChange) > DEBOUNCE_MS) {
        if (state == HIGH) return true;
    }
    return false;
}