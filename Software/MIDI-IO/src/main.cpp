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

#define BUFFER_SIZE 32
MidiMessage msg[BUFFER_SIZE];
int Index  = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("ESP32 MIDI IN/OUT gestartet...");
  MIDIserial.begin(31250, SERIAL_8N1, 17, 16);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  Serial.println("Bereit für MIDI-Signale!");
}

void loop() {
  if (MIDI.read()) {
    msg[Index].type = MIDI.getType();
    msg[Index].channel = MIDI.getChannel();
    msg[Index].data1 = MIDI.getData1();
    msg[Index].data2 = MIDI.getData2();

    Serial.print("Empfangen -> Typ: ");
    Serial.print(msg[Index].type);
    Serial.print(" | Kanal: ");
    Serial.print(msg[Index].channel);
    Serial.print(" | Data1: ");
    Serial.print(msg[Index].data1);
    Serial.print(" | Data2: ");
    Serial.println(msg[Index].data2);

    switch (msg[Index].type) {
      case midi::NoteOn:
        MIDI.sendNoteOn(msg[Index].data1, msg[Index].data2, msg[Index].channel);
        break;
      case midi::NoteOff:
        MIDI.sendNoteOff(msg[Index].data1, msg[Index].data2, msg[Index].channel);
        break;
      default:
        break;
    }
    Serial.println("Nachricht weitergeleitet");
    Index = (Index == BUFFER_SIZE - 1) ? 0 : Index++;
  }

}