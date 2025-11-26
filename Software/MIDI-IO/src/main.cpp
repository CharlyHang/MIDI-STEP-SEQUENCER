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
#define MAX_STEP_SIZE 16
#define Presets 8
#define Presetpage 10
MidiMessage msg[MAX_NOTE_SIZE][MAX_STEP_SIZE];
int Note = 0;
int Step  = 0;
const unsigned long INACTIVITY_MS = 300;
unsigned long last_Received = 0;
unsigned long stop = 0;
bool first = false;

const uint8_t NUM_PRESETS = 4;
const uint8_t presetPins[NUM_PRESETS] = {19, 18, 22, 23}; // Beispiel-Pins, anpassen

const unsigned long DEBOUNCE_MS = 50;
int lastButtonState = LOW;
unsigned long lastButtonChange = 0;

const size_t SEQ_BYTES = sizeof(msg);                 // Bytes per sequence
const size_t TOTAL_EEPROM_BYTES = SEQ_BYTES * NUM_PRESETS; // total


// returns preset index (0..NUM_PRESETS-1) if pressed (debounced), else -1
int readWhichPresetPressed() {
  for (int i = 0; i < NUM_PRESETS; ++i) {
    if(digitalRead(presetPins[i])){
      return i;
    }
  }
  return -1;
}
// ---------- EEPROM: save/load ----------
// Ensure EEPROM.begin(TOTAL_EEPROM_BYTES) is called before using these functions.
// Writes entire msg[][] to presetIndex slot.
void saveSequenceToEEPROM(uint8_t presetIndex) {
  if (presetIndex >= NUM_PRESETS) return;

  if (!EEPROM.begin((int)TOTAL_EEPROM_BYTES)) {
    Serial.println("EEPROM.begin fehlgeschlagen!");
    return;
  }

  size_t base = (size_t)presetIndex * SEQ_BYTES;
  const uint8_t* p = (const uint8_t*)msg;
  Serial.print("Speichere Preset ");
  Serial.print(presetIndex);
  Serial.print(" an Adresse ");
  Serial.println(base);

  for (size_t i = 0; i < SEQ_BYTES; ++i) {
    if (EEPROM.read((int)(base + i)) != p[i]) {
      EEPROM.write((int)(base + i), p[i]);
    }
  }

  if (EEPROM.commit()) {
    Serial.print("Preset ");
    Serial.print(presetIndex);
    Serial.println(" erfolgreich gespeichert.");
  } else {
    Serial.println("EEPROM.commit fehlgeschlagen!");
  }
}

// Loads preset into RAM msg[][]
// If EEPROM unreadable, msg bleibt unverändert.
bool loadSequenceFromEEPROM(uint8_t presetIndex) {
  if (presetIndex >= NUM_PRESETS) return false;

  if (!EEPROM.begin((int)TOTAL_EEPROM_BYTES)) {
    Serial.println("EEPROM.begin fehlgeschlagen!");
    return false;
  }

  size_t base = (size_t)presetIndex * SEQ_BYTES;
  uint8_t* p = (uint8_t*)msg;

  Serial.print("Lade Preset ");
  Serial.print(presetIndex);
  Serial.print(" von Adresse ");
  Serial.println(base);

  for (size_t i = 0; i < SEQ_BYTES; ++i) {
    p[i] = EEPROM.read((int)(base + i));
  }
  Serial.println("Laden abgeschlossen.");
  return true;
}


// ---------- Playback ----------
void playSequenceFromRAM(uint16_t stepDelayMs = 250) {
  Serial.println("Starte Playback aus RAM...");
  for (int s = 0; s < MAX_STEP_SIZE; ++s) {
    for (int n = 0; n < MAX_NOTE_SIZE; ++n) {
      MidiMessage &m = msg[n][s];
      if (m.type == midi::NoteOn) {
        MIDI.sendNoteOn(m.data1, m.data2, m.channel);
      } else if (m.type == midi::NoteOff) {
        MIDI.sendNoteOff(m.data1, m.data2, m.channel);
      }
      // andere Message-Typen ignorieren / erweitern falls nötig
    }
    Serial.print("Step ");
    Serial.print(s + 1);
    Serial.println(" abgespielt.");
    delay(stepDelayMs);
  }
  Serial.println("Playback fertig.");
}

// Combines load + play
void playSequenceFromPreset(uint8_t presetIndex) {
  if (loadSequenceFromEEPROM(presetIndex)) {
    playSequenceFromRAM();
  } else {
    Serial.println("Fehler beim Laden, Playback abgebrochen.");
  }
}



void clearSequence() {
  for (int s = 0; s < MAX_STEP_SIZE; s++) {
    for (int n = 0; n < MAX_NOTE_SIZE; n++) {
      msg[n][s].type = 0;
      msg[n][s].channel = 0;
      msg[n][s].data1 = 0;
      msg[n][s].data2 = 0;
    }
  }
  Serial.println("Sequenz im RAM wurde gelöscht (alle Werte = 0).");
}

void setup() {
  Serial.begin(9600);
  Serial.println("ESP32 MIDI IN/OUT gestartet...");
  MIDIserial.begin(31250, SERIAL_8N1, 17, 16);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff(); // Für keine doppelte Signale
  Serial.println("Bereit für MIDI-Signale!");
  for(int i = 0; i < NUM_PRESETS; i++) pinMode(presetPins[i], INPUT);
}


void loop() {

  Serial.print("STEP ");
  Serial.print(Step+1);
  Serial.println(":");



  while (!MIDI.read() || MIDI.getType() == 254 || MIDI.getType() == 255) {
    //Warten bis eine Note gespielt wurde
    // Wenn Button gedrückt wird, bevor ein Step fertig ist -> gespeicherte Sequenz abspielen
    int presetpressed = readWhichPresetPressed();
      if(presetpressed >= 0){
      Serial.print("Preset-Button ");
      Serial.print(presetpressed);
      Serial.println(" gedrückt -> gespeicherte Sequenz abspielen...");
      playSequenceFromPreset((uint8_t)presetpressed);
      delay(300);
      return;
      }
  }

  last_Received = millis();
  first = true; // Um die erste Note noch zu lesen
  while(millis() - last_Received <= INACTIVITY_MS){
     bool hasMessage = MIDI.read();  // <-- nur einmal pro Durchlauf
  
      if ((hasMessage || first) && Note != MAX_NOTE_SIZE - 1 && MIDI.getType() != 254 && MIDI.getType() != 255) {
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
      MIDI.read(); // MIDI weiterverarbeiten
      int which = readWhichPresetPressed();
      if (which >= 0) {
        Serial.print("Preset ");
        Serial.print(which);
        Serial.println(" ausgewählt -> speichern...");
        saveSequenceToEEPROM((uint8_t)which);
        clearSequence();
        delay(300);
        break;
      }
    }

    Step = 0; // Start wieder bei Step 1
  } else {
    Step++;
  }
}