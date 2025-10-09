#include <Arduino.h>
#include <MIDI.h>
#include <HardwareSerial.h>

// Wir benutzen UART2 (Nummer 2) auf RX=17
HardwareSerial MIDIserial(2);

// Erstelle MIDI-Objekt
MIDI_CREATE_INSTANCE(HardwareSerial, MIDIserial, MIDI);

void setup()
{  
    Serial.begin(9600);

    // MIDI Baudrate = 31250, Format SERIAL_8N1, RX=17, TX nicht benutzt (-1)
    MIDIserial.begin(31250, SERIAL_8N1, 17, -1);

    // MIDI starten
    MIDI.begin(MIDI_CHANNEL_OMNI);  // empfängt auf allen Kanälen
    Serial.println("MIDI Listener gestartet...");
}
 

void loop()
{
    // Prüfen, ob eine neue MIDI-Nachricht angekommen ist
  if (MIDI.read()) {
    byte type = MIDI.getType();        // Nachrichtentyp (NoteOn, NoteOff, CC, etc.)
    byte channel = MIDI.getChannel();  // Kanal (1–16)
    byte data1 = MIDI.getData1();      // Note oder Controller-Nummer
    byte data2 = MIDI.getData2();      // Velocity oder Wert

    Serial.print("Typ: ");
    Serial.print(type);
    Serial.print(" | Kanal: ");
    Serial.print(channel);
    Serial.print(" | Data1: ");
    Serial.print(data1);
    Serial.print(" | Data2: ");
    Serial.println(data2);
  }
}

