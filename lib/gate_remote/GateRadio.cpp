#include "GateRadio.h"

bool GateRadio::begin(const Config& config) {
  gdo0Pin_ = config.gdo0Pin;

  cc1101_.Init();  // before getCC1101(), which does not start SPI
  bool found = cc1101_.getCC1101();

  cc1101_.setModulation(2);  // ASK/OOK
  cc1101_.setMHZ(config.mhz);
  cc1101_.setPA(config.powerDbm);  // after setMHZ: the power table depends on the band

  rf_.enableTransmit(config.gdo0Pin);
  rf_.setProtocol(config.protocol);
  rf_.setPulseLength(config.pulseUs);  // after setProtocol, which resets it
  rf_.setRepeatTransmit(config.framesPerSend);

  stopTx();
  return found;
}

void GateRadio::startTx() { cc1101_.SetTx(); }

// Idle rather than sleep: in SLEEP the CC1101 loses its PATABLE, i.e. the output power.
void GateRadio::stopTx() {
  digitalWrite(gdo0Pin_, LOW);
  cc1101_.setSidle();
}

void GateRadio::send(const char* code) { rf_.send(code); }
