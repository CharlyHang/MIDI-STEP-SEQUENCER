#include <MIDI.h>

// RX Pin für MIDI IN (ESP32): GPIO 17
const int MIDI_RX_PIN = 17;

// LED Pin (onboard LED)
const int LED_PIN = LED_BUILTIN;

// MIDI-Library Instance auf Serial1
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

int clockCount = 0;
unsigned long lastBeatTime = 0;
bool ledOn = false;

void handleClock() {
  clockCount++;
  // 24 Messages = 1 Viertelnote (1 Beat)
  if (clockCount >= 24) {
    clockCount = 0;
    digitalWrite(LED_PIN, HIGH);
    ledOn = true;
    lastBeatTime = millis();
  }
}

void handleStart() {
  // Wenn die DAW Start (0xFA) sendet, beginne sauber:
  clockCount = 0;
  digitalWrite(LED_PIN, LOW);
  ledOn = false;
  // optional: direkten Beat simulieren:
  // digitalWrite(LED_PIN, HIGH); ledOn = true; lastBeatTime = millis();
}

void setup() {
  // Debug über USB
  Serial.begin(9600);
  delay(10);
  Serial.println("MIDI Clock Test - Serial1 RX=" + String(MIDI_RX_PIN));
  Serial.println("LED_PIN = " + String(LED_PIN));

  // LED initialisieren
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Serial1: MIDI Baud 31250, RX=MIDI_RX_PIN, kein TX
  Serial1.begin(31250, SERIAL_8N1, MIDI_RX_PIN, -1);

  // MIDI-Library initialisieren
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleClock(handleClock);   // Clock Callback
  MIDI.setHandleStart(handleStart);   // optional: Start Callback

  Serial.println("Ready. Waiting for MIDI Clock...");
}

void loop() {
  // Muss regelmäßig aufgerufen werden
  MIDI.read();

  // LED kurz ausschalten (80 ms)
  if (ledOn && millis() - lastBeatTime > 80) {
    digitalWrite(LED_PIN, LOW);
    ledOn = false;
  }

  // Optional: Debug alle 5s
  static unsigned long lastDbg = 0;
  if (millis() - lastDbg > 100) {
    lastDbg = millis();
    Serial.print("clockCount=");
    Serial.println(clockCount);
  }
}
