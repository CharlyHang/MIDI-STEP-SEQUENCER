#include "Anzeige.h"

class Step_Anzeige: public OLED_Anzeige{
    public:
        Step_Anzeige(int SCL, int SDA);
        ~Step_Anzeige();
        void display();
        void set_Step(int Step);


    private:
        int my_Step = 0;
};