#include <Arduino.h>
#include <MIDI.h>
#include <HardwareSerial.h>

HardwareSerial MIDIserial(2);

MIDI_CREATE_INSTANCE(HardwareSerial, MIDIserial, MIDI);

struct MidiMessage {
  byte type;
  byte channel;
  byte data1;
  byte data2;
};

#define MAX_NOTE_SIZE 32
#define MAX_STEP_SIZE 32
MidiMessage msg[MAX_NOTE_SIZE][MAX_STEP_SIZE];
int Note = 0;
int Step  = 0;
const unsigned long INACTIVITY_MS = 1000;
unsigned long last_Received = 0;
unsigned long stop = 0;
bool first = false;

void setup() {
  Serial.begin(9600);
  Serial.println("ESP32 MIDI IN/OUT gestartet...");
  MIDIserial.begin(31250, SERIAL_8N1, 17, 16);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff(); // Für keine doppelte Signale
  Serial.println("Bereit für MIDI-Signale!");
}


void loop() {

  Serial.print("STEP ");
  Serial.print(Step+1);
  Serial.println(":");

  while (!MIDI.read() || MIDI.getType() == 254) {
    //Warten bis eine Note gespielt wurde
  }

  last_Received = millis();
  first = true; // Um die erste Note noch zu lesen
  while(millis() - last_Received <= INACTIVITY_MS){
     bool hasMessage = MIDI.read();  // <-- nur einmal pro Durchlauf
  
      if ((hasMessage || first) && Note != MAX_NOTE_SIZE - 1 && MIDI.getType() != 254) {
        last_Received = millis();
        first = false;
        msg[Note][Step].type = MIDI.getType();
        msg[Note][Step].channel = MIDI.getChannel();
        msg[Note][Step].data1 = MIDI.getData1();
        msg[Note][Step].data2 = MIDI.getData2();

        Serial.print("Empfangen -> Typ: ");
        Serial.print(msg[Note][Step].type);
        Serial.print(" | Kanal: ");
        Serial.print(msg[Note][Step].channel);
        Serial.print(" | Data1: ");
        Serial.print(msg[Note][Step].data1);
        Serial.print(" | Data2: ");
        Serial.println(msg[Note][Step].data2);

        switch (msg[Note][Step].type) {
          case midi::NoteOn:
            MIDI.sendNoteOn(msg[Note][Step].data1, msg[Note][Step].data2, msg[Note][Step].channel);
            break;
          case midi::NoteOff:
            MIDI.sendNoteOff(msg[Note][Step].data1, msg[Note][Step].data2, msg[Note][Step].channel);
            break;
          default:
            break;
        }
        Serial.println("Nachricht weitergeleitet");
        Note = (Note == MAX_NOTE_SIZE - 1) ? 0 : Note + 1;
      } else if (hasMessage && Note == MAX_NOTE_SIZE - 1 && MIDI.getType() != 254){
        Serial.println("Die maximale Notenanzahl fuer einen Step wurde erreicht!");
      } else {
        //
      }
  }
  Note = 0;
  Step = (Step == MAX_STEP_SIZE - 1) ? 0 : Step + 1;
}