// ESP8266 gate opener: replays the gate remote's code while the button is held, for at least
// MIN_TX_MS, so a short click still sends several frames, and at most MAX_HOLD_MS. WiFi stays off.

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "Button.h"
#include "GateRadio.h"
#include "TxWindow.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "include/secrets.h is missing: copy it from include/secrets.example.h"
#endif

const uint8_t GDO0_PIN = 5;    // D1
const uint8_t BUTTON_PIN = 4;  // D2, to GND

// ~10 dBm conducted (PATABLE 0xC0, the chip's maximum): within 10 mW e.r.p. with an antenna of
// 0 dBd (2.15 dBi) or less.
const int TX_POWER_DBM = 10;
const int FRAMES_PER_SEND = 2;  // short, so the loop notices a release quickly
const unsigned long DEBOUNCE_MS = 30;
const unsigned long MIN_TX_MS = 500;
const unsigned long MAX_HOLD_MS = 10000;

GateRadio radio;
Button button(BUTTON_PIN, DEBOUNCE_MS);
TxWindow window(MIN_TX_MS, MAX_HOLD_MS);
bool radioFound = false;
bool transmitting = false;

void setup() {
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();

  Serial.begin(115200);
  Serial.printf("\ngate_opener %s\n", FIRMWARE_VERSION);
  button.begin();

  radioFound = radio.begin(
      {GDO0_PIN, GATE_MHZ, GATE_PROTOCOL, GATE_PULSE_US, TX_POWER_DBM, FRAMES_PER_SEND});
  if (!radioFound) {
    Serial.println("CC1101 not found: transmission disabled");
  }
  Serial.println("Ready");
}

void loop() {
  unsigned long now = millis();

  switch (button.update(now)) {
    case Button::Pressed:
      if (radioFound) {
        window.press(now);
        Serial.println("Transmitting");
      }
      break;
    case Button::Released:
      window.release();
      Serial.println("Released");
      break;
    case Button::None:
      break;
  }

  bool tx = window.update(now);
  if (window.takeTimeout()) Serial.println("Timeout");
  if (tx != transmitting) {
    tx ? radio.startTx() : radio.stopTx();
    transmitting = tx;
  }
  if (tx) radio.send(GATE_CODE);

  yield();
}
