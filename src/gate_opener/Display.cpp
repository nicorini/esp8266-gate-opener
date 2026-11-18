#include "Display.h"

#include <U8g2lib.h>
#include <Wire.h>

namespace {

const uint8_t ADDRESS = 0x3C;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C* panel = nullptr;

void centred(const char* s, int y) { panel->drawStr((128 - panel->getStrWidth(s)) / 2, y, s); }

}  // namespace

void Display::begin(uint8_t sdaPin, uint8_t sclPin, uint8_t contrast) {
  contrast_ = contrast;
  panel = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE, sclPin, sdaPin);
  panel->setBusClock(400000);
  reset();
}

bool Display::present() {
  Wire.beginTransmission(ADDRESS);
  return Wire.endTransmission() == 0;
}

void Display::reset() {
  panel->begin();
  panel->setContrast(contrast_);
  panel->setPowerSave(1);
  last_ = Frame();  // whatever comes next gets drawn
}

void Display::show(const Frame& f) {
  if (memcmp(&f, &last_, sizeof(f)) == 0) return;
  bool wasOff = last_.kind == Frame::Off;
  last_ = f;

  if (f.kind == Frame::Off) {
    panel->setPowerSave(1);
    return;
  }
  panel->clearBuffer();
  switch (f.kind) {
    case Frame::Big:
      panel->setFont(u8g2_font_logisoso22_tr);
      if (panel->getStrWidth(f.text) > 124) panel->setFont(u8g2_font_logisoso16_tr);
      centred(f.text, f.small[0] ? 30 : 43);
      panel->setFont(u8g2_font_6x10_tr);
      centred(f.small, 54);
      break;
    case Frame::Bar:
      panel->drawFrame(4, 20, 120, 14);
      panel->drawBox(6, 22, 116 * f.bar / 100, 10);
      panel->setFont(u8g2_font_6x10_tr);
      centred(f.small, 54);
      break;
    case Frame::Page:
      panel->setFont(u8g2_font_6x10_tr);
      panel->drawStr(0, 9, f.text);
      panel->drawStr(128 - panel->getStrWidth(f.small), 9, f.small);
      panel->drawHLine(0, 12, 128);
      for (int i = 0; i < 4; i++) panel->drawStr(0, 25 + 12 * i, f.lines[i]);
      break;
    case Frame::Corner:
      panel->setFont(u8g2_font_6x10_tr);
      panel->drawStr(f.x, f.y, f.text);
      break;
    case Frame::Off:
      break;
  }
  panel->sendBuffer();
  if (wasOff) panel->setPowerSave(0);
}
