#include "Presetanzeige.h"

Preset_Anzeige::Preset_Anzeige(int SCL, int SDA, int CLK, int DT)
    :OLED_Anzeige(SCL, SDA),
    my_CLK(CLK),
    my_DT(DT)
{}

void Preset_Anzeige::initialize(){
    OLED_Anzeige::initialize();
    pinMode(my_CLK, INPUT);
    pinMode(my_DT, INPUT);
    lastStateCLK = digitalRead(my_CLK);
}

void Preset_Anzeige::update() {
    handleEncoder();
}

void Preset_Anzeige::handleEncoder() {
    currentStateCLK = digitalRead(my_CLK);

    if (currentStateCLK != lastStateCLK && currentStateCLK == HIGH) {
    if (digitalRead(my_DT) != currentStateCLK){
        if(currentPresetPage<=maxPresetPage) currentPresetPage++; //rechts
        my_Text = "Seite: " + String(currentPresetPage);
    } else {
        if(currentPresetPage >= minPresetPage) currentPresetPage--; //links
        my_Text = "Seite: " + String(currentPresetPage);
    }

        OLED_Anzeige::display();   // Anzeige aktualisieren
    }

    lastStateCLK = currentStateCLK;
}

void Preset_Anzeige::set_maxPresetPage(int max_Page){
    maxPresetPage = max_Page;
}