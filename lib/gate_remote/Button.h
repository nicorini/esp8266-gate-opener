// Debounced push button to GND, with the internal pull-up.
#pragma once

#include <Arduino.h>

class Button {
 public:
  enum Event { None, Pressed, Released };

  Button(uint8_t pin, unsigned long debounceMs) : pin_(pin), debounceMs_(debounceMs) {}

  void begin() { pinMode(pin_, INPUT_PULLUP); }

  Event update(unsigned long now) {
    bool reading = digitalRead(pin_);
    if (reading != lastReading_) {
      lastReading_ = reading;
      lastChangeMs_ = now;
      return None;
    }
    if (now - lastChangeMs_ < debounceMs_) return None;

    bool down = (reading == LOW);
    if (down == down_) return None;
    down_ = down;
    return down ? Pressed : Released;
  }

 private:
  uint8_t pin_;
  unsigned long debounceMs_;
  bool lastReading_ = HIGH;
  bool down_ = false;
  unsigned long lastChangeMs_ = 0;
};
