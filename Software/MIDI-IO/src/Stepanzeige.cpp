#include "Stepanzeige.h"


Step_Anzeige::Step_Anzeige(int SCL, int SDA) :OLED_Anzeige(SCL, SDA) {}

void Step_Anzeige::set_Step(int Step){
    my_Step = Step;
}

void Step_Anzeige::display(){
    my_Text = "Step: " + String(my_Step);
    OLED_Anzeige::display();
}