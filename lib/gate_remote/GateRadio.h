// Replays a fixed-code remote: rc-switch bit-bangs the OOK frames on GDO0 and the CC1101, in
// asynchronous serial mode, transmits them.
#pragma once

#include <RCSwitch.h>
#include <SmartRC_CC1101.h>

class GateRadio {
 public:
  struct Config {
    uint8_t gdo0Pin;
    float mhz;
    int protocol;
    int pulseUs;
    int powerDbm;
    int framesPerSend;
  };

  // Returns false if the CC1101 does not answer on SPI.
  bool begin(const Config& config);
  void startTx();
  void stopTx();
  // Blocks while framesPerSend frames go out.
  void send(const char* code);

 private:
  RCSwitch rf_;
  SmartRC_CC1101 cc1101_;
  uint8_t gdo0Pin_ = 0;
};
