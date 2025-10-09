#include <Arduino.h>
#include <MIDI.h>
#include <HardwareSerial.h>

// UART2 für MIDI verwenden
HardwareSerial MIDIserial(2);

// MIDI-Instanz auf UART2
MIDI_CREATE_INSTANCE(HardwareSerial, MIDIserial, MIDI);

void setup() {
  Serial.begin(9600);
  Serial.println("MIDI Sender gestartet...");

  // TX-Pin = 16, RX nicht benutzt (-1)
  // Baudrate für MIDI: 31250
  MIDIserial.begin(31250, SERIAL_8N1, -1, 16);

  // MIDI starten auf Kanal 4
  MIDI.begin(4);

  delay(2000); // kurz warten, bis Interface bereit ist

  Serial.println("Sende Testnote...");
  // Sende C4 (Note 60) auf Kanal 4
  MIDI.sendNoteOn(60, 100, 4);
  delay(500);
  MIDI.sendNoteOff(60, 0, 4);
  Serial.println("Note gesendet!");
}

void loop() {
  // Beispiel: alle 2 Sekunden NoteOn/NoteOff
  static unsigned long last = 0;
  if (millis() - last > 2000) {
    last = millis();
    Serial.println("NoteOn + NoteOff");
    MIDI.sendNoteOn(64, 100, 4);  // E4
    delay(200);
    MIDI.sendNoteOff(64, 0, 4);
  }
}