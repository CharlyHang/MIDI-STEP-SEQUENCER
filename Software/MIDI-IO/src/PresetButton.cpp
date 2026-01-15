#include "PresetButton.h"


PresetButton::PresetButton(const int* pins, int maxPresets)
    : Button(-1), // Base class constructor
      my_pins(pins),
      my_maxPresets(maxPresets)
{
    for (int i = 0; i < my_maxPresets; ++i) {
        pinMode(my_pins[i], INPUT);
        lastPresetState[i] = LOW;
        lastPresetChange[i] = 0;
    }
}

int PresetButton::whichPressed() {
    for (int i = 0; i < my_maxPresets; ++i) {
        int state = digitalRead(my_pins[i]);
        if (state != lastPresetState[i]) {
            lastPresetChange[i] = millis();
            lastPresetState[i] = state;
        }
        if ((millis() - lastPresetChange[i]) > DEBOUNCE_MS) {
            if (state == HIGH) return i;
        }
    }
    return -1; // No button pressed
}