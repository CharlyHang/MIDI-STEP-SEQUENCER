#include <Arduino.h>
#include <MIDI.h>
#include <HardwareSerial.h>
#include <EEPROM.h>

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
#define Presets 8
#define Presetpage 10
MidiMessage msg[MAX_NOTE_SIZE][MAX_STEP_SIZE];
int Note = 0;
int Step  = 0;
const unsigned long INACTIVITY_MS = 1000;
unsigned long last_Received = 0;
unsigned long stop = 0;
bool first = false;

int Preset_Button1 = 19;

const unsigned long DEBOUNCE_MS = 50;
int lastButtonState = LOW;
unsigned long lastButtonChange = 0;

// ----- EEPROM Speicherung -----
void saveSequenceToEEPROM() {
  size_t needed = sizeof(msg);
  Serial.print("Benötigte Bytes zum Speichern: ");
  Serial.println(needed);

  if (!EEPROM.begin((int)needed)) {
    Serial.println("EEPROM.begin fehlgeschlagen!");
    return;
  }

  Serial.println("Speichere Sequenz in EEPROM...");

  const uint8_t* p = (const uint8_t*)msg;
  for (size_t i = 0; i < needed; ++i) {
    if (EEPROM.read((int)i) != p[i]) {
      EEPROM.write((int)i, p[i]);
    }
  }

  if (EEPROM.commit()) {
    Serial.println("Speichern erfolgreich!");
  } else {
    Serial.println("Fehler beim EEPROM.commit!");
  }
}

// ----- EEPROM Laden & Abspielen -----
void playSequenceFromEEPROM() {
  size_t needed = sizeof(msg);

  if (!EEPROM.begin((int)needed)) {
    Serial.println("EEPROM.begin fehlgeschlagen!");
    return;
  }

  // Sequenz aus EEPROM laden
  Serial.println("Lade Sequenz aus EEPROM...");
  uint8_t* p = (uint8_t*)msg;
  for (size_t i = 0; i < needed; ++i) {
    p[i] = EEPROM.read((int)i);
  }

  Serial.println("Spiele gespeicherte Sequenz ab...");
  
  // Alle Steps abspielen
  for (int s = 0; s < MAX_STEP_SIZE; s++) {
    for (int n = 0; n < MAX_NOTE_SIZE; n++) {
      MidiMessage &m = msg[n][s];
      if (m.type == midi::NoteOn) {
        MIDI.sendNoteOn(m.data1, m.data2, m.channel);
      } else if (m.type == midi::NoteOff) {
        MIDI.sendNoteOff(m.data1, m.data2, m.channel);
      }
    }
    Serial.print("Step ");
    Serial.print(s + 1);
    Serial.println(" abgespielt.");
    delay(250); // kurze Pause zwischen Steps
  }

  Serial.println("Sequenz komplett abgespielt.");
}

void setup() {
  Serial.begin(9600);
  Serial.println("ESP32 MIDI IN/OUT gestartet...");
  MIDIserial.begin(31250, SERIAL_8N1, 17, 16);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff(); // Für keine doppelte Signale
  Serial.println("Bereit für MIDI-Signale!");
  pinMode(Preset_Button1, INPUT);
}


void loop() {

  Serial.print("STEP ");
  Serial.print(Step+1);
  Serial.println(":");

  // Wenn Button gedrückt wird, bevor ein Step fertig ist -> gespeicherte Sequenz abspielen
  if (digitalRead(Preset_Button1)) {
    Serial.println("Button gedrückt -> gespeicherte Sequenz abspielen...");
    playSequenceFromEEPROM();
    delay(300);
    return; // zurück zum Anfang der Loop
  }

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
  // Wenn letzter Step -> auf Button warten
  if (Step == MAX_STEP_SIZE - 1) {
    Serial.println("LETZTER STEP erreicht. Drücke Button (HIGH) um zu speichern...");

    while (true) {
      MIDI.read(); // MIDI weiter lesen, um Buffer leer zu halten
      if (digitalRead(Preset_Button1)) {
        Serial.println("Button gedrueckt -> Speichern läuft...");
        saveSequenceToEEPROM();
        Serial.println("Sequenz gespeichert. Weiter mit Step 1.");
        delay(300); // Entprellen
        break;
      }
    }

    Step = 0; // Start wieder bei Step 1
  } else {
    Step++;
  }
}