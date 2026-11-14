// Receives a 433 MHz remote and prints code, bit length, protocol, pulse length and RSSI.
// Same wiring as gate_opener.

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <RCSwitch.h>
#include <SmartRC_CC1101.h>

const uint8_t GDO0_PIN = 5;  // in asynchronous RX, GDO0 carries the demodulated data
const float RX_MHZ = 433.92;

RCSwitch rf;
SmartRC_CC1101 radio;

void setup() {
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  Serial.begin(115200);
  Serial.printf("\nsniffer %s\n", FIRMWARE_VERSION);

  radio.Init();
  Serial.println(radio.getCC1101() ? "CC1101 OK" : "CC1101 not found");
  radio.setModulation(2);  // ASK/OOK
  radio.setMHZ(RX_MHZ);

  rf.enableReceive(digitalPinToInterrupt(GDO0_PIN));
  radio.SetRx();
  Serial.printf("Listening on %.2f MHz\n", RX_MHZ);
}

void loop() {
  if (!rf.available()) return;

  unsigned long value = rf.getReceivedValue();
  unsigned int bits = rf.getReceivedBitlength();

  Serial.print("code=");
  for (int i = bits - 1; i >= 0; i--) Serial.print((value >> i) & 1);
  Serial.printf("  bits=%u  protocol=%u  pulse=%u us  RSSI=%d dBm\n", bits,
                rf.getReceivedProtocol(), rf.getReceivedDelay(), radio.getRssi());

  rf.resetAvailable();
}
