// Replays the gate remote's code while the button is held: at least MIN_TX_MS, so a short click
// still sends several frames, and at most MAX_HOLD_MS.
// rc-switch bit-bangs the OOK signal on GDO0; the CC1101, in asynchronous serial mode, transmits it.

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <RCSwitch.h>
#include <SmartRC_CC1101.h>

#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "include/secrets.h is missing: copy it from include/secrets.example.h"
#endif

const uint8_t GDO0_PIN = 5;    // D1
const uint8_t BUTTON_PIN = 4;  // D2, to GND, internal pull-up

// ~10 dBm conducted (PATABLE 0xC0, the chip's maximum): within 10 mW e.r.p. with an antenna of
// 0 dBd (2.15 dBi) or less.
const int TX_POWER_DBM = 10;
const int TX_REPEAT = 2;  // frames per send(); short, so the loop notices a release quickly
const unsigned long DEBOUNCE_MS = 30;
const unsigned long MIN_TX_MS = 500;
const unsigned long MAX_HOLD_MS = 10000;

RCSwitch rf;
SmartRC_CC1101 radio;

bool radioFound = false;
bool lastReading = HIGH;
bool pressed = false;
bool transmitting = false;
unsigned long lastChangeMs = 0;
unsigned long pressStartMs = 0;

void startTx() {
  radio.SetTx();
  transmitting = true;
}

// Idle rather than sleep: in SLEEP the CC1101 loses its PATABLE, i.e. the output power.
void stopTx() {
  digitalWrite(GDO0_PIN, LOW);
  radio.setSidle();
  transmitting = false;
}

void setup() {
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();

  Serial.begin(115200);
  Serial.printf("\ngate_opener %s\n", FIRMWARE_VERSION);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  radio.Init();  // before getCC1101(), which does not start SPI
  radioFound = radio.getCC1101();
  if (!radioFound) {
    Serial.println("CC1101 not found: transmission disabled");
  }

  radio.setModulation(2);  // ASK/OOK
  radio.setMHZ(GATE_MHZ);
  radio.setPA(TX_POWER_DBM);  // after setMHZ: the power table depends on the band

  rf.enableTransmit(GDO0_PIN);
  rf.setProtocol(GATE_PROTOCOL);
  rf.setPulseLength(GATE_PULSE_US);  // after setProtocol, which resets it
  rf.setRepeatTransmit(TX_REPEAT);

  stopTx();
  Serial.println("Ready");
}

void loop() {
  unsigned long now = millis();
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastReading) {
    lastReading = reading;
    lastChangeMs = now;
  }

  if (now - lastChangeMs >= DEBOUNCE_MS) {
    bool isDown = (reading == LOW);
    if (isDown && !pressed) {
      pressed = true;
      pressStartMs = now;
      if (radioFound) {
        startTx();
        Serial.println("Transmitting");
      }
    } else if (!isDown && pressed) {
      pressed = false;
      Serial.println("Released");
    }
  }

  if (transmitting) {
    unsigned long elapsed = now - pressStartMs;
    if (elapsed >= MAX_HOLD_MS) {
      stopTx();
      Serial.println("Timeout");
    } else if (!pressed && elapsed >= MIN_TX_MS) {
      stopTx();
    }
  }

  if (transmitting) {
    rf.send(GATE_CODE);
  }

  yield();
}
