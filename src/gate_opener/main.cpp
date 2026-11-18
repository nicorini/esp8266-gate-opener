// ESP8266 gate opener: replays the gate remote's code while the button is held, for at least
// MIN_TX_MS and at most MAX_HOLD_MS.
//
// With the OLED connected, a hold of HOLD_MS opens (a bar fills meanwhile) and a short click
// shows the next diagnostics page instead. Without a display, or with DISPLAY_ENABLED false,
// every press opens at once, as in firmware 2.0. The display is probed at boot and at every
// press, so it can be unplugged and plugged back.
//
// With WIFI_SSID set in secrets.h the gate is also a HomeKit switch: turning it on replays the
// remote for HOMEKIT_TX_MS, then it turns itself off. The button works with or without WiFi.
// Everything runs in loop(): while a frame goes out, HomeKit and the display wait. With
// WIFI_SSID empty, WiFi stays off and the firmware is button-only.

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <arduino_homekit_server.h>

#include "Button.h"
#include "Display.h"
#include "GateRadio.h"
#include "PressClassifier.h"
#include "TxWindow.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "include/secrets.h is missing: copy it from include/secrets.example.h"
#endif

const uint8_t GDO0_PIN = 5;    // D1
const uint8_t BUTTON_PIN = 4;  // D2, to GND
const uint8_t SDA_PIN = 0;     // D3, OLED SDA
const uint8_t SCL_PIN = 2;     // D4, OLED SCK

// false: ignore the display and open on every press, for a panel that answers but shows nothing
const bool DISPLAY_ENABLED = true;
const uint8_t CONTRAST = 1;  // 0-255; the minimum reads fine indoors

// ~10 dBm conducted (PATABLE 0xC0, the chip's maximum): within 10 mW e.r.p. with an antenna of
// 0 dBd (2.15 dBi) or less.
const int TX_POWER_DBM = 10;
const int FRAMES_PER_SEND = 2;  // short, so the loop notices a release quickly
const unsigned long DEBOUNCE_MS = 30;
const unsigned long HOLD_MS = 1000;
const unsigned long BAR_DELAY_MS = 200;  // a click is shorter: no bar flashing before a page
const unsigned long MIN_TX_MS = 500;
const unsigned long MAX_HOLD_MS = 10000;
const unsigned long HOMEKIT_TX_MS = 1000;  // also how long the switch stays on

const unsigned long SHOW_MS = 5000;          // messages and pages, then the screen goes off
const unsigned long AFTER_BUTTON_MS = 2000;  // "Apertura" after the button is released
const unsigned long AFTER_HOME_MS = 5000;    // "Apertura" after a HomeKit command
const unsigned long WIFI_WAIT_MS = 20000;    // at boot, then "WiFi no"
const unsigned long CORNER_MOVE_MS = 60000;  // persistent error text moves against burn-in
const uint8_t PAGES = 4;
const uint8_t BAR_FROM = BAR_DELAY_MS * 100 / HOLD_MS;  // progress % when the bar appears

extern "C" homekit_server_config_t config;  // accessory.c
extern "C" homekit_characteristic_t cha_switch_on;

GateRadio radio;
Button button(BUTTON_PIN, DEBOUNCE_MS);
PressClassifier presses(HOLD_MS);
TxWindow window(MIN_TX_MS, MAX_HOLD_MS);
Display display;

bool radioFound = false;
bool transmitting = false;
bool homekit = false;
bool displayOk = false;
unsigned long switchOnMs = 0;

enum Source { FromButton, FromHome };
Source txSource = FromButton;
Source nextSource = FromButton;

// What the screen shows. Each message lasts messageForMs from its time stamp.
const char* message = nullptr;  // big text: "Max 10s", "No radio", "WiFi ok"...
const char* messageSmall = "";
unsigned long messageMs = 0;
unsigned long messageForMs = SHOW_MS;
int page = -1;  // diagnostics page shown, -1 none
unsigned long pageMs = 0;

bool wifiUp = false;
bool wifiEverUp = false;
bool wifiGaveUp = false;

unsigned long opens = 0;
unsigned long lastOpenMs = 0;
Source lastSource = FromButton;

const char* sourceText(Source s) { return s == FromButton ? "pulsante" : "HomeKit"; }

void showMessage(const char* big, const char* small, unsigned long now,
                 unsigned long forMs = SHOW_MS) {
  message = big;
  messageSmall = small;
  messageMs = now;
  messageForMs = forMs;
  page = -1;
}

bool paired() {
  homekit_server_t* server = arduino_homekit_get_running_server();
  return server && server->paired;
}

// "3 g 4 h", "4 h 12 min", "12 min", "40 s"
void formatSpan(char* out, size_t size, unsigned long ms) {
  unsigned long s = ms / 1000, m = s / 60, h = m / 60, d = h / 24;
  if (d) snprintf(out, size, "%lu g %lu h", d, h % 24);
  else if (h) snprintf(out, size, "%lu h %lu min", h, m % 60);
  else if (m) snprintf(out, size, "%lu min", m);
  else snprintf(out, size, "%lu s", s);
}

void fillPage(Frame& f, unsigned long now) {
  f.kind = Frame::Page;
  snprintf(f.small, sizeof(f.small), "%d/%d", page + 1, PAGES);
  char span[16];
  switch (page) {
    case 0:
      strcpy(f.text, "Stato");
      snprintf(f.lines[0], 22, "Radio   %s", radioFound ? "ok" : "no");
      snprintf(f.lines[1], 22, "WiFi    %s", !homekit ? "spento" : wifiUp ? "ok" : "no");
      snprintf(f.lines[2], 22, "HomeKit %s",
               !homekit ? "spento" : !wifiUp ? "no" : paired() ? "ok" : "da abbinare");
      break;
    case 1:
      strcpy(f.text, "WiFi");
      if (!homekit) {
        strcpy(f.lines[0], "Spento");
      } else if (!wifiUp) {
        strcpy(f.lines[0], "Non connesso");
      } else {
        snprintf(f.lines[0], 22, "Segnale %d dBm", WiFi.RSSI());
        snprintf(f.lines[1], 22, "IP %s", WiFi.localIP().toString().c_str());
      }
      break;
    case 2:
      strcpy(f.text, "Aperture");
      snprintf(f.lines[0], 22, "Dall'avvio: %lu", opens);
      if (opens) {
        formatSpan(span, sizeof(span), now - lastOpenMs);
        snprintf(f.lines[1], 22, "Ultima %.11s fa", span);
        snprintf(f.lines[2], 22, "Da: %s", sourceText(lastSource));
      }
      break;
    case 3:
      strcpy(f.text, "Sistema");
      formatSpan(span, sizeof(span), now);
      snprintf(f.lines[0], 22, "Acceso da %.11s", span);
      snprintf(f.lines[1], 22, "Firmware %s", FIRMWARE_VERSION);
      snprintf(f.lines[2], 22, "Memoria %u KB", (unsigned)(ESP.getFreeHeap() / 1024));
      break;
  }
}

// Decides what the screen shows now, highest priority first.
Frame frameFor(unsigned long now) {
  Frame f;
  uint8_t bar = presses.progress(now);

  if (transmitting) {
    f.kind = Frame::Big;
    strcpy(f.text, "Apertura");
    if (txSource == FromHome) strcpy(f.small, "da HomeKit");  // the button needs no caption
  } else if (bar >= BAR_FROM && bar < 100) {
    f.kind = Frame::Bar;
    f.bar = (bar - BAR_FROM) * 100 / (100 - BAR_FROM) / 5 * 5;  // from empty; fewer redraws
    strcpy(f.small, "Apertura...");
  } else if (message && now - messageMs < messageForMs) {
    f.kind = Frame::Big;
    strcpy(f.text, message);
    strcpy(f.small, messageSmall);
  } else if (page >= 0 && now - pageMs < SHOW_MS) {
    fillPage(f, now);
  } else if (homekit && !wifiEverUp && !wifiGaveUp) {
    f.kind = Frame::Big;
    strcpy(f.text, "WiFi...");
  } else if (homekit && wifiUp && !paired()) {
    f.kind = Frame::Big;
    strcpy(f.text, "Da abbinare");
    strcpy(f.small, HOMEKIT_SETUP_CODE);
  } else if (!radioFound || (homekit && !wifiUp)) {
    f.kind = Frame::Corner;
    strcpy(f.text, !radioFound && homekit && !wifiUp ? "No radio, WiFi no"
                   : !radioFound                     ? "No radio"
                                                     : "WiFi no");
    unsigned long step = now / CORNER_MOVE_MS;
    f.x = step * 37 % (128 - 6 * strlen(f.text));
    f.y = 10 + step * 23 % 54;
  }
  return f;
}

void refreshDisplay(unsigned long now) {
  if (displayOk) display.show(frameFor(now));
}

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
  if (!transmitting) nextSource = FromHome;
  window.trigger(switchOnMs, HOMEKIT_TX_MS);
  page = -1;
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

void checkWifi(unsigned long now) {
  bool up = WiFi.status() == WL_CONNECTED;
  if (up != wifiUp) {
    wifiUp = up;
    if (up) {
      showMessage("WiFi ok", wifiEverUp ? "di nuovo" : "", now);
      wifiEverUp = true;
    } else {
      showMessage("WiFi no", "connessione persa", now);
    }
    Serial.println(up ? "WiFi connected" : "WiFi lost");
  }
  if (!wifiEverUp && !wifiGaveUp && now >= WIFI_WAIT_MS) {
    wifiGaveUp = true;
    showMessage("WiFi no", "", now);
    Serial.println("WiFi: no connection");
  }
}

void onHold(unsigned long now) {
  if (radioFound) {
    window.press(now);
  } else {
    showMessage("No radio", "", now);
    Serial.println("No CC1101: not transmitting");
  }
}

void onPress(unsigned long now) {
  bool wasOk = displayOk;
  displayOk = DISPLAY_ENABLED && display.present();
  if (displayOk && !wasOk) {
    display.reset();
    Serial.println("Display back");
  } else if (!displayOk && wasOk) {
    Serial.println("Display gone: every press opens");
  }
  nextSource = FromButton;
  if (presses.press(now, !displayOk) == PressClassifier::HoldStart) onHold(now);
}

void onClick(unsigned long now) {
  page = (page >= 0 && now - pageMs < SHOW_MS) ? (page + 1) % PAGES : 0;
  pageMs = now;
  message = nullptr;
}

void setup() {
  Serial.begin(115200);
  Serial.printf("\ngate_opener %s\n", FIRMWARE_VERSION);
  button.begin();

  display.begin(SDA_PIN, SCL_PIN, CONTRAST);
  displayOk = DISPLAY_ENABLED && display.present();
  Serial.println(displayOk ? "Display OK" : "No display: every press opens");

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

  // Before the button's events, so a hold is never reported late as a click.
  if (presses.update(now) == PressClassifier::HoldStart) onHold(now);

  switch (button.update(now)) {
    case Button::Pressed:
      onPress(now);
      break;
    case Button::Released:
      switch (presses.release(now)) {
        case PressClassifier::Click:
          onClick(now);
          break;
        case PressClassifier::HoldEnd:
          window.release();
          Serial.println("Released");
          break;
        default:
          break;
      }
      break;
    case Button::None:
      break;
  }

  bool tx = window.update(now);
  if (tx && !transmitting) {
    txSource = nextSource;
    opens++;
    lastOpenMs = now;
    lastSource = txSource;
    transmitting = true;
    refreshDisplay(now);  // "Apertura" goes out before the radio starts: no drawing during TX
    radio.startTx();
    Serial.printf("Transmitting (%s)\n", sourceText(txSource));
  } else if (!tx && transmitting) {
    radio.stopTx();
    transmitting = false;
    bool timeout = window.takeTimeout();
    if (timeout) showMessage("Max 10s", "", now);
    else if (txSource == FromHome) showMessage("Apertura", "da HomeKit", now, AFTER_HOME_MS);
    else showMessage("Apertura", "", now, AFTER_BUTTON_MS);
    Serial.println(timeout ? "Timeout" : "Sent");
  }
  if (tx) radio.send(GATE_CODE);

  if (homekit) {
    checkWifi(now);
    if (cha_switch_on.value.bool_value && now - switchOnMs >= HOMEKIT_TX_MS) setSwitch(false);
    arduino_homekit_loop();
  } else {
    yield();
  }
  if (!transmitting) refreshDisplay(now);
}
