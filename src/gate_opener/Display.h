// 0.96" 128x64 SSD1306 OLED on I2C. main.cpp describes what to show as a Frame; show() sends
// it to the panel only when it differs from the last one, so drawing costs nothing at rest.
#pragma once

#include <Arduino.h>

struct Frame {
  enum Kind : uint8_t { Off, Big, Bar, Page, Corner };

  Frame() { memset(this, 0, sizeof(*this)); }

  Kind kind;
  char text[22];      // Big: big line; Page: title; Corner: the text
  char small[22];     // Big, Bar: small line; Page: "1/4"
  char lines[4][22];  // Page only
  uint8_t bar;        // Bar: 0-100
  uint8_t x, y;       // Corner: position, moved now and then against burn-in
};

class Display {
 public:
  void begin(uint8_t sdaPin, uint8_t sclPin, uint8_t contrast);
  // True if the panel answers on I2C (~1 ms).
  bool present();
  // Re-initialises the panel, after it was unplugged and plugged back.
  void reset();
  void show(const Frame& frame);

 private:
  Frame last_;
  uint8_t contrast_ = 0;
};
