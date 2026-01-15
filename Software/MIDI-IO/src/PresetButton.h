#include "Button.h"

class PresetButton: public Button {
    public:
        PresetButton(const int* pins, int maxPresets);
        ~PresetButton();
        int whichPressed();

    private:
        const int* my_pins;
        int my_maxPresets = 8;
        int* lastPresetState;
        unsigned long* lastPresetChange;

};