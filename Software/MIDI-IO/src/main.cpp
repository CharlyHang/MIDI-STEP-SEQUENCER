/*
  Test-Sketch: 4 Presets, 1 Page, 16 Steps, LittleFS storage
  ESP32-S3 (HardwareSerial MIDI)
  Playback is driven by external clock pulses on CLOCK_PIN (one HIGH pulse per beat).
*/

#include <Arduino.h>
#include <MIDI.h>
#include <HardwareSerial.h>
#include <LittleFS.h>

HardwareSerial MIDIserial(2);
MIDI_CREATE_INSTANCE(HardwareSerial, MIDIserial, MIDI);

// ---------------- Config ----------------
struct MidiMessage {
  uint8_t type;
  uint8_t channel;
  uint8_t data1;
  uint8_t data2;
};

const int MAX_NOTE_SIZE = 32;
const int MAX_STEP_SIZE = 16;     // Test: 16 Steps
const int PRESETS_PER_PAGE = 4;   // 4 Preset-Buttons
const int PAGES = 1;              // nur 1 Page in diesem Test
int currentPage = 0;              // 0 .. (PAGES-1)

MidiMessage msg[MAX_NOTE_SIZE][MAX_STEP_SIZE];

int Note = 0;
int Step = 0;

const unsigned long INACTIVITY_MS = 300; // Aufnahmefenster pro Step
unsigned long last_Received = 0;
bool first = false;

const uint8_t presetPins[PRESETS_PER_PAGE] = {19, 18, 22, 23}; // passe an deine HW an
const uint8_t LED_PIN = 2; // Status LED (optional)

const unsigned long DEBOUNCE_MS = 50;
uint16_t playbackStepDelayMs = 250; // kept but playback controlled by external clock

// ---------------- FS / Header ----------------
const size_t SEQ_BYTES = sizeof(msg); // mit 32x16x4 = 2048
const uint32_t PRESET_MAGIC = 0xDEADBEEF;
const uint16_t PRESET_VERSION = 1;

struct PresetHeader {
  uint32_t magic;
  uint16_t version;
  uint16_t reserved;
  uint32_t crc32;
};

// ---------------- CRC32 ----------------
static uint32_t crc_table[256];
static bool crc_table_inited = false;
void init_crc_table() {
  if (crc_table_inited) return;
  const uint32_t POLY = 0xEDB88320UL;
  for (uint32_t i = 0; i < 256; ++i) {
    uint32_t c = i;
    for (uint32_t j = 0; j < 8; ++j) {
      if (c & 1) c = POLY ^ (c >> 1);
      else c = c >> 1;
    }
    crc_table[i] = c;
  }
  crc_table_inited = true;
}
uint32_t calcCRC32(const uint8_t* buf, size_t len) {
  init_crc_table();
  uint32_t c = 0xFFFFFFFFUL;
  for (size_t i = 0; i < len; ++i) {
    uint8_t idx = (uint8_t)((c ^ buf[i]) & 0xFF);
    c = crc_table[idx] ^ (c >> 8);
  }
  return c ^ 0xFFFFFFFFUL;
}

// ---------------- Helpers FS ----------------
String presetFilename(int page, int slot) {
  return String("/page") + page + "_preset" + slot + ".bin";
}

bool initFS() {
  if (!LittleFS.begin(true)) { // format if fails
    Serial.println("LittleFS.begin() fehlgeschlagen!");
    return false;
  }
  Serial.println("LittleFS bereit.");
  return true;
}

bool savePresetToFS(int page, int slot) {
  if (page < 0 || page >= PAGES) return false;
  if (slot < 0 || slot >= PRESETS_PER_PAGE) return false;

  String path = presetFilename(page, slot);
  String tmp = path + ".tmp";

  PresetHeader hdr;
  hdr.magic = PRESET_MAGIC;
  hdr.version = PRESET_VERSION;
  hdr.reserved = 0;
  hdr.crc32 = calcCRC32((const uint8_t*)msg, SEQ_BYTES);

  File f = LittleFS.open(tmp, FILE_WRITE);
  if (!f) {
    Serial.println("Fehler: tmp-Datei nicht öffnbar.");
    return false;
  }

  if (f.write((const uint8_t*)&hdr, sizeof(hdr)) != sizeof(hdr)) {
    f.close();
    LittleFS.remove(tmp);
    Serial.println("Fehler beim Schreiben des Headers.");
    return false;
  }

  size_t written = f.write((const uint8_t*)msg, SEQ_BYTES);
  f.close();

  if (written != SEQ_BYTES) {
    Serial.printf("Warnung: nur %u von %u Bytes geschrieben\n", (unsigned)written, (unsigned)SEQ_BYTES);
    LittleFS.remove(tmp);
    return false;
  }

  if (LittleFS.exists(path)) LittleFS.remove(path);
  if (!LittleFS.rename(tmp, path)) {
    Serial.println("Fehler beim Umbenennen (rename).");
    LittleFS.remove(tmp);
    return false;
  }

  Serial.printf("Preset gespeichert: %s (crc=%08X)\n", path.c_str(), hdr.crc32);
  return true;
}

bool loadPresetFromFS(int page, int slot) {
  if (page < 0 || page >= PAGES) return false;
  if (slot < 0 || slot >= PRESETS_PER_PAGE) return false;

  String path = presetFilename(page, slot);
  if (!LittleFS.exists(path)) {
    Serial.println("Preset existiert nicht: " + path);
    return false;
  }

  File f = LittleFS.open(path, FILE_READ);
  if (!f) {
    Serial.println("Fehler: Datei nicht lesbar.");
    return false;
  }

  PresetHeader hdr;
  ssize_t got = f.read((uint8_t*)&hdr, sizeof(hdr));
  if (got != sizeof(hdr)) { f.close(); Serial.println("Header-Lese-Fehler."); return false; }
  if (hdr.magic != PRESET_MAGIC) { f.close(); Serial.println("Ungültiger Preset-Magic."); return false; }

  size_t readBytes = f.read((uint8_t*)msg, SEQ_BYTES);
  f.close();
  if (readBytes != SEQ_BYTES) {
    Serial.printf("Fehler: gelesene Bytes %u != %u\n", (unsigned)readBytes, (unsigned)SEQ_BYTES);
    return false;
  }

  uint32_t crc = calcCRC32((const uint8_t*)msg, SEQ_BYTES);
  if (crc != hdr.crc32) {
    Serial.printf("CRC mismatch: got %08X expected %08X\n", crc, hdr.crc32);
    return false;
  }

  Serial.printf("Preset geladen: %s (crc=%08X)\n", path.c_str(), crc);
  return true;
}

// ---------------- Playback (per step) ----------------
// play all messages of a single step; prints debug raw format and sends MIDI
void playStep(uint16_t stepIndex) {
  if (stepIndex >= MAX_STEP_SIZE) return;
  Serial.printf("PLAY STEP %d\n", stepIndex + 1);
  digitalWrite(LED_PIN, HIGH);

  for (int n = 0; n < MAX_NOTE_SIZE; ++n) {
    MidiMessage &m = msg[n][stepIndex];
    if (m.type == 0) continue; // skip empty slots

    // Debug output in requested format
    Serial.printf("Gesendet -> Typ: %u | Kanal: %u | Data1: %u | Data2: %u\n",
                  (unsigned)m.type, (unsigned)m.channel, (unsigned)m.data1, (unsigned)m.data2);

    // Actually send the MIDI message
    switch (m.type) {
      case midi::NoteOn:
        MIDI.sendNoteOn(m.data1, m.data2, m.channel);
        break;
      case midi::NoteOff:
        MIDI.sendNoteOff(m.data1, m.data2, m.channel);
        break;
      default:
        // unsupported types ignored for now
        break;
    }
  }

  digitalWrite(LED_PIN, LOW);
}

// wrapper that loads a preset and enters playback mode (clock-driven)
bool startPresetPlayback(int page, int slot) {
  Serial.printf("Playing preset request: page=%d slot=%d\n", page, slot);
  if (!loadPresetFromFS(page, slot)) return false;
  Serial.println("Preset geladen. Starte Playback (warte auf externen Clock pulses)...");
  // Start at step 0, set playbackMode; external pulses will advance
  Step = 0;
  return true;
}

// ---------------- Clear RAM ----------------
void clearSequenceRAM() {
  for (int s = 0; s < MAX_STEP_SIZE; ++s) {
    for (int n = 0; n < MAX_NOTE_SIZE; ++n) {
      msg[n][s].type = 0;
      msg[n][s].channel = 0;
      msg[n][s].data1 = 0;
      msg[n][s].data2 = 0;
    }
  }
  Serial.println("RAM-Sequenz gelöscht.");
}

// ---------------- Button handling (debounced) ----------------
int lastPresetState[PRESETS_PER_PAGE];
unsigned long lastPresetChange[PRESETS_PER_PAGE];

void setupButtons() {
  for (int i = 0; i < PRESETS_PER_PAGE; ++i) {
    pinMode(presetPins[i], INPUT);
    lastPresetState[i] = digitalRead(presetPins[i]);
    lastPresetChange[i] = millis();
  }
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

int readWhichPresetPressed() {
  for (int i = 0; i < PRESETS_PER_PAGE; ++i) {
    int state = digitalRead(presetPins[i]);
    if (state != lastPresetState[i]) {
      lastPresetChange[i] = millis();
      lastPresetState[i] = state;
    }
    if ((millis() - lastPresetChange[i]) > DEBOUNCE_MS) {
      if (state == HIGH) return i;
    }
  }
  return -1;
}

// ---------------- External clock (ISR) ----------------
// Choose a pin available on your board for clock pulses:
const uint8_t CLOCK_PIN = 34; // adjust if needed

volatile bool clockPulse = false;
volatile unsigned long lastClockMicros = 0;
const unsigned long CLOCK_DEBOUNCE_US = 5000UL; // 5 ms debounce

void IRAM_ATTR onClockISR() {
  unsigned long now = micros();
  // simple debounce in ISR
  if (now - lastClockMicros > CLOCK_DEBOUNCE_US) {
    lastClockMicros = now;
    clockPulse = true;
  }
}

// ---------------- Playback mode flag ----------------
bool playbackMode = false; // when true, external clock advances playback steps

// ---------------- Setup & Loop ----------------
void setup() {
  Serial.begin(9600);
  delay(10);
  Serial.println("ESP32-S3 Sequencer (Test) starting...");

  // MIDI init (RX=17, TX=16 in this wiring)
  MIDIserial.begin(31250, SERIAL_8N1, 17, 16);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();

  // LittleFS
  if (!initFS()) {
    Serial.println("LittleFS Init failed - continue (but save/load won't work).");
  }

  setupButtons();
  clearSequenceRAM();

  // Clock pin setup and attach ISR (external short HIGH pulses)
  pinMode(CLOCK_PIN, INPUT_PULLDOWN); // change to INPUT/INPUT_PULLUP as your wiring requires
  attachInterrupt(digitalPinToInterrupt(CLOCK_PIN), onClockISR, RISING);

  Serial.printf("Config: %d Presets/Page, %d Pages, SEQ_BYTES=%u\n", PRESETS_PER_PAGE, PAGES, (unsigned)SEQ_BYTES);
  Serial.printf("Using currentPage = %d (manuell setzbar im Code)\n", currentPage);
}

void loop() {
  // If not in playbackMode, run the recording/step-capture logic (your existing behavior)
  if (!playbackMode) {
    Serial.print("STEP ");
    Serial.print(Step + 1);
    Serial.println(":");

    // Quick: check preset pressed before waiting for MIDI start
    int presetPressed = readWhichPresetPressed();
    if (presetPressed >= 0) {
      if (Step == MAX_STEP_SIZE - 1) {
        Serial.printf("Preset %d gedrückt am letzten Step -> speichern (page %d)...\n", presetPressed, currentPage);
        if (savePresetToFS(currentPage, presetPressed)) clearSequenceRAM();
        delay(300);
        Step = 0; Note = 0;
        return;
      } else {
        // Start playback mode for that preset (clock-driven)
        Serial.printf("Preset %d gedrückt -> Starte Playback (page %d)...\n", presetPressed, currentPage);
        if (startPresetPlayback(currentPage, presetPressed)) {
          playbackMode = true; // now wait for external clock pulses to advance steps
        }
        delay(300);
        return;
      }
    }

    // wait for first valid MIDI message (ignore Active Sensing 254/255)
    while (!MIDI.read() || MIDI.getType() == 254 || MIDI.getType() == 255) {
      // check preset during waiting too (allow starting playback even while idle)
      int p = readWhichPresetPressed();
      if (p >= 0) {
        if (Step == MAX_STEP_SIZE - 1) {
          Serial.printf("Preset %d gedrückt am letzten Step -> speichern (page %d)...\n", p, currentPage);
          if (savePresetToFS(currentPage, p)) clearSequenceRAM();
          delay(300);
          Step = 0; Note = 0;
          return;
        } else {
          Serial.printf("Preset %d gedrückt -> Starte Playback (page %d)...\n", p, currentPage);
          if (startPresetPlayback(currentPage, p)) playbackMode = true;
          delay(300);
          return;
        }
      }
    }

    last_Received = millis();
    first = true;

    while (millis() - last_Received <= INACTIVITY_MS) {
      bool hasMessage = MIDI.read();

      int pDuring = readWhichPresetPressed();
      if (pDuring >= 0) {
        Serial.printf("Preset %d gedrückt während Aufnahme -> Play (page %d)...\n", pDuring, currentPage);
        if (startPresetPlayback(currentPage, pDuring)) playbackMode = true;
        delay(300);
        return;
      }

      if ((hasMessage || first) && Note != MAX_NOTE_SIZE - 1) {
        if (hasMessage && (MIDI.getType() == 254 || MIDI.getType() == 255)) {
          // skip Active Sensing / invalid
        } else {
          last_Received = millis();
          first = false;
          msg[Note][Step].type = MIDI.getType();
          msg[Note][Step].channel = MIDI.getChannel();
          msg[Note][Step].data1 = MIDI.getData1();
          msg[Note][Step].data2 = MIDI.getData2();

          Serial.printf("Empfangen -> Typ: %u | Kanal: %u | Data1: %u | Data2: %u\n",
                        msg[Note][Step].type, msg[Note][Step].channel,
                        msg[Note][Step].data1, msg[Note][Step].data2);

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
        }
      } else if (hasMessage && Note == MAX_NOTE_SIZE - 1 && MIDI.getType() != 254) {
        Serial.println("Die maximale Notenanzahl fuer einen Step wurde erreicht!");
      }
      // keep looping until inactivity window expires
    }

    // end of capture window
    Note = 0;

    if (Step == MAX_STEP_SIZE - 1) {
      Serial.println("LETZTER STEP erreicht. Drücke Preset-Button (HIGH) um zu speichern...");

      while (true) {
        MIDI.read(); // keep MIDI flowing
        int which = readWhichPresetPressed();
        if (which >= 0) {
          Serial.printf("Preset %d ausgewählt -> speichern (page %d)...\n", which, currentPage);
          if (savePresetToFS(currentPage, which)) {
            clearSequenceRAM();
          }
          delay(300);
          break;
        }
        // optional: add timeout to abort
      }
      Step = 0;
    } else {
      Step++;
    }
  } // end if (!playbackMode)

  // ---------------- playbackMode handling: advance step when external clock pulse arrives ----------------
  bool doPulse = false;
  noInterrupts();
  if (clockPulse) {
    clockPulse = false;
    doPulse = true;
  }
  interrupts();

  if (doPulse && playbackMode) {
    // play current step
    playStep(Step);

    // advance step, wrap around
    Step = (Step == MAX_STEP_SIZE - 1) ? 0 : Step + 1;

    // optionally: if you want playback to stop automatically after one loop:
    // if (Step == 0) playbackMode = false;
  }

  // small yield
  delay(1);
}
