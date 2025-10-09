#include <Arduino.h>
#include <MIDI.h>
#include <HardwareSerial.h>

// UART2 für MIDI-Ein/Aus
HardwareSerial MIDIserial(2);

// MIDI-Instanz (auf UART2)
MIDI_CREATE_INSTANCE(HardwareSerial, MIDIserial, MIDI);

// Einfache Struktur zum Puffern empfangener Nachrichten
struct MidiMessage {
  byte type;
  byte channel;
  byte data1;
  byte data2;
};

#define BUFFER_SIZE 32
MidiMessage messageBuffer[BUFFER_SIZE];
int writeIndex = 0;
int readIndex  = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("ESP32 MIDI IN/OUT gestartet...");

  // UART2: RX = 17 (MIDI-IN), TX = 16 (MIDI-OUT)
  MIDIserial.begin(31250, SERIAL_8N1, 17, 16);

  // MIDI auf allen Kanälen empfangen
  MIDI.begin(MIDI_CHANNEL_OMNI);

  Serial.println("Bereit für MIDI-Signale!");
}

void loop() {
  // --- EMPFANG ---
  if (MIDI.read()) {
    MidiMessage msg;
    msg.type    = MIDI.getType();
    msg.channel = MIDI.getChannel();
    msg.data1   = MIDI.getData1();
    msg.data2   = MIDI.getData2();

    // In Puffer schreiben (FIFO)
    messageBuffer[writeIndex] = msg;
    writeIndex = (writeIndex + 1) % BUFFER_SIZE;

    Serial.print("Empfangen -> Typ: ");
    Serial.print(msg.type);
    Serial.print(" | Kanal: ");
    Serial.print(msg.channel);
    Serial.print(" | Data1: ");
    Serial.print(msg.data1);
    Serial.print(" | Data2: ");
    Serial.println(msg.data2);
  }

  // --- SENDEN (THRU) ---
  if (readIndex != writeIndex) {
    MidiMessage msg = messageBuffer[readIndex];
    readIndex = (readIndex + 1) % BUFFER_SIZE;

    switch (msg.type) {
      case midi::NoteOn:
        MIDI.sendNoteOn(msg.data1, msg.data2, msg.channel);
        break;
      case midi::NoteOff:
        MIDI.sendNoteOff(msg.data1, msg.data2, msg.channel);
        break;
      case midi::ControlChange:
        MIDI.sendControlChange(msg.data1, msg.data2, msg.channel);
        break;
      default:
        break; // andere Typen ignorieren
    }

    Serial.println("→ Nachricht weitergeleitet");
  }
}