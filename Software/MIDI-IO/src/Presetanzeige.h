#include "Anzeige.h"
#include "Arduino.h"

class Preset_Anzeige: public OLED_Anzeige{
    public:
        Preset_Anzeige(int SCL, int SDA, int CLK, int DT);
        ~Preset_Anzeige();
        void initialize();
        void update();
        void set_maxPresetPage(int max_Page);

    private:
        const int my_CLK;
        const int my_DT;
        int maxPresetPage = 1;
        const int minPresetPage = 1;
        int currentPresetPage = 1;
        int currentStateCLK;
        int lastStateCLK;

    void handleEncoder();
};