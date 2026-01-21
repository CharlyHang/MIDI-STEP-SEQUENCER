#include "Anzeige.h"

class Step_Anzeige: public OLED_Anzeige{
    public:
        Step_Anzeige(int SCL, int SDA);
        void set_Step(int Step);
        void update();

    private:
        using OLED_Anzeige::set_Text;
        int my_Step = 0;
        void display();
};