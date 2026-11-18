// Bench test for the 0.96" 128x64 I2C OLED: I2C scan, then four test pages.
// The button (D2) moves to the next page; without it, pages change every 5 s.
// Wiring: GND -> G, VDD -> 3V, SCK -> D4, SDA -> D3. The CC1101 can stay connected.

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <U8g2lib.h>
#include <Wire.h>

const uint8_t SDA_PIN = 0;     // D3
const uint8_t SCL_PIN = 2;     // D4
const uint8_t BUTTON_PIN = 4;  // D2, other leg to G
const unsigned long AUTO_MS = 5000;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE, SCL_PIN, SDA_PIN);

uint8_t page = 0;
const uint8_t PAGES = 4;
unsigned long shownAt = 0;
bool buttonWasDown = false;
bool buttonSeen = false;

uint8_t scanI2c() {
  uint8_t found = 0;
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) {
      Serial.printf("I2C device at 0x%02X\n", a);
      if (!found) found = a;
    }
  }
  if (!found) Serial.println("No I2C device: check wiring (SCK -> D4, SDA -> D3, VDD -> 3V)");
  return found;
}

void centred(const char *s, int y) { oled.drawStr((128 - oled.getStrWidth(s)) / 2, y, s); }

void show() {
  oled.setContrast(page == 3 ? 1 : 255);
  oled.clearBuffer();
  switch (page) {
    case 0:  // edges: the frame must be whole on all four sides
      oled.drawFrame(0, 0, 128, 64);
      oled.setFont(u8g2_font_6x10_tr);
      centred("ALTO", 12);
      centred("128 x 64", 36);
      centred("BASSO", 60);
      Serial.println("1/4 frame: border whole on 4 sides? ALTO next to the pins?");
      break;
    case 1:  // all on: measure the lit area against the PCB edges
      oled.drawBox(0, 0, 128, 64);
      Serial.println("2/4 all on: measure the lit area from the PCB edges");
      break;
    case 2:  // the kind of text the firmware will show
      oled.setFont(u8g2_font_logisoso22_tr);
      centred("APRO", 30);
      oled.setFont(u8g2_font_6x10_tr);
      centred("pulsante", 50);
      Serial.println("3/4 text at full contrast");
      break;
    case 3:
      oled.setFont(u8g2_font_logisoso22_tr);
      centred("APRO", 30);
      oled.setFont(u8g2_font_6x10_tr);
      centred("contrasto minimo", 50);
      Serial.println("4/4 same text at minimum contrast");
      break;
  }
  oled.sendBuffer();
  shownAt = millis();
}

void setup() {
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.printf("\noled_test %s\n", FIRMWARE_VERSION);

  Wire.begin(SDA_PIN, SCL_PIN);
  uint8_t addr = scanI2c();
  if (addr) oled.setI2CAddress(addr * 2);
  oled.begin();
  show();
}

void loop() {
  bool down = digitalRead(BUTTON_PIN) == LOW;
  if (down && !buttonWasDown && millis() - shownAt > 200) {
    buttonSeen = true;
    page = (page + 1) % PAGES;
    show();
  }
  buttonWasDown = down;
  if (!buttonSeen && millis() - shownAt > AUTO_MS) {
    page = (page + 1) % PAGES;
    show();
  }
}
