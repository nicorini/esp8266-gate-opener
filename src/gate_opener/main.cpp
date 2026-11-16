// ESP8266 gate opener: replays the gate remote's code while the button is held, for at least
// MIN_TX_MS, so a short click still sends several frames, and at most MAX_HOLD_MS.
//
// With WIFI_SSID set in secrets.h the gate is also a HomeKit switch: turning it on replays the
// remote for HOMEKIT_TX_MS, then it turns itself off. The button works with or without WiFi.
// Everything runs in loop(): while a frame goes out, HomeKit waits. With WIFI_SSID empty, WiFi
// stays off and the firmware is button-only.

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <arduino_homekit_server.h>

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
const unsigned long HOMEKIT_TX_MS = 1000;  // also how long the switch stays on

extern "C" homekit_server_config_t config;  // accessory.c
extern "C" homekit_characteristic_t cha_switch_on;

GateRadio radio;
Button button(BUTTON_PIN, DEBOUNCE_MS);
TxWindow window(MIN_TX_MS, MAX_HOLD_MS);
bool radioFound = false;
bool transmitting = false;
bool homekit = false;
unsigned long switchOnMs = 0;

void setSwitch(bool on) {
  cha_switch_on.value = HOMEKIT_BOOL(on);
  homekit_characteristic_notify(&cha_switch_on, cha_switch_on.value);
}

// Called from arduino_homekit_loop() when the Home app or Siri changes the switch.
void onSwitch(const homekit_value_t value) {
  cha_switch_on.value = value;
  if (!value.bool_value) return;  // turned off before it went off by itself: nothing to do
  if (!radioFound) {
    Serial.println("HomeKit: ignored, no CC1101");
    setSwitch(false);
    return;
  }
  switchOnMs = millis();
  window.trigger(switchOnMs, HOMEKIT_TX_MS);
  Serial.println("HomeKit: transmitting");
}

void setupHomeKit() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  // non-blocking: the library starts mDNS on got-IP

  config.password = const_cast<char*>(HOMEKIT_SETUP_CODE);
  cha_switch_on.setter = onSwitch;  // before arduino_homekit_setup
  arduino_homekit_setup(&config);
}

void setup() {
  Serial.begin(115200);
  Serial.printf("\ngate_opener %s\n", FIRMWARE_VERSION);
  button.begin();

  radioFound = radio.begin(
      {GDO0_PIN, GATE_MHZ, GATE_PROTOCOL, GATE_PULSE_US, TX_POWER_DBM, FRAMES_PER_SEND});
  if (!radioFound) {
    Serial.println("CC1101 not found: transmission disabled");
  }

  homekit = WIFI_SSID[0] != '\0';
  if (homekit) {
    setupHomeKit();
  } else {
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
  }
  Serial.printf("Ready, HomeKit %s\n", homekit ? "on" : "off");
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

  if (homekit) {
    if (cha_switch_on.value.bool_value && now - switchOnMs >= HOMEKIT_TX_MS) setSwitch(false);
    arduino_homekit_loop();
  } else {
    yield();
  }
}
