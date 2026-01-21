#include "Stepanzeige.h"


Step_Anzeige::Step_Anzeige(int SCL, int SDA) :OLED_Anzeige(SCL, SDA) {}

void Step_Anzeige::set_Step(int Step){
    my_Step = Step;
}

void Step_Anzeige::display(){
    if(my_Step <= 16){
        my_Text = "Step: " + String(my_Step);
    } else {
        my_Text = "Speichern:";
    }
    OLED_Anzeige::display();
}

void Step_Anzeige::update(){
    Step_Anzeige::display();
}