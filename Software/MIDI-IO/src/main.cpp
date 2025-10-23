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

#define MAX_NOTE_SIZE 8
#define MAX_STEP_SIZE 32
MidiMessage msg[MAX_NOTE_SIZE][MAX_STEP_SIZE];
int Note = 0;
int Index  = 0;
unsigned long start = 0, stop = 0;
bool read = false;

void setup() {
  Serial.begin(9600);
  Serial.println("ESP32 MIDI IN/OUT gestartet...");
  MIDIserial.begin(31250, SERIAL_8N1, 17, 16);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  Serial.println("Bereit für MIDI-Signale!");
}


void loop() {

  Serial.print("STEP ");
  Serial.print(Index+1);
  Serial.println(":");

  while (!MIDI.read()) {
    //Warten bis eine Note gespielt wurde
  }

  start = millis();
  stop = start;
  read = true; // Um die erste Note noch zu lesen
  while(stop - start <= 3000){
    if(MIDI.read() || read && Note != MAX_NOTE_SIZE - 1){
      read = false;
      msg[Note][Index].type = MIDI.getType();
      msg[Note][Index].channel = MIDI.getChannel();
      msg[Note][Index].data1 = MIDI.getData1();
      msg[Note][Index].data2 = MIDI.getData2();

      Serial.print("Empfangen -> Typ: ");
      Serial.print(msg[Note][Index].type);
      Serial.print(" | Kanal: ");
      Serial.print(msg[Note][Index].channel);
      Serial.print(" | Data1: ");
      Serial.print(msg[Note][Index].data1);
      Serial.print(" | Data2: ");
      Serial.println(msg[Note][Index].data2);

      switch (msg[Note][Index].type) {
        case midi::NoteOn:
          MIDI.sendNoteOn(msg[Note][Index].data1, msg[Note][Index].data2, msg[Note][Index].channel);
          break;
        case midi::NoteOff:
          MIDI.sendNoteOff(msg[Note][Index].data1, msg[Note][Index].data2, msg[Note][Index].channel);
          break;
        default:
          break;
      }
      Serial.println("Nachricht weitergeleitet");
      Note = (Note == MAX_NOTE_SIZE - 1) ? 0 : Note + 1;
      stop = millis();
    } else if (MIDI.read() || read && Note != MAX_NOTE_SIZE - 1){
      Serial.println("Die maximale Notenanzahl fuer einen Step wurde erreicht!");
    }
  }
  Note = 0;
  Index = (Index == MAX_STEP_SIZE - 1) ? 0 : Index + 1;
}