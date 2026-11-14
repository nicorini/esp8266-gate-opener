// RF link test, receiver: prints every decoded code and, every 500 ms, the peak RSSI.
// A peak that rises while the transmitter sends proves the RF path works even if decoding fails.

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <RCSwitch.h>
#include <SmartRC_CC1101.h>

const uint8_t GDO0_PIN = 5;
const float RX_MHZ = 433.92;
const unsigned long RSSI_PERIOD_MS = 500;

RCSwitch rf;
SmartRC_CC1101 radio;
unsigned long lastRssiMs = 0;
int peakRssi = -200;

uint8_t marcState() { return radio.SpiReadStatus(CC1101_MARCSTATE) & 0x1F; }

void setup() {
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  Serial.begin(115200);
  delay(200);
  Serial.printf("\nrf_rx_test %s\n", FIRMWARE_VERSION);

  radio.Init();
  Serial.printf("PARTNUM=0x%02X VERSION=0x%02X (expected 0x00 and 0x14 or 0x04)\n",
                radio.SpiReadStatus(CC1101_PARTNUM), radio.SpiReadStatus(CC1101_VERSION));

  radio.setModulation(2);  // ASK/OOK
  radio.setMHZ(RX_MHZ);

  rf.enableReceive(digitalPinToInterrupt(GDO0_PIN));
  radio.SetRx();
  Serial.printf("Listening on %.2f MHz\n", RX_MHZ);
}

void loop() {
  int rssi = radio.getRssi();
  if (rssi > peakRssi) peakRssi = rssi;

  if (rf.available()) {
    Serial.printf(">>> received 0x%06lX  bits=%u  protocol=%u  pulse=%u us\n",
                  rf.getReceivedValue(), rf.getReceivedBitlength(), rf.getReceivedProtocol(),
                  rf.getReceivedDelay());
    rf.resetAvailable();
  }

  unsigned long now = millis();
  if (now - lastRssiMs >= RSSI_PERIOD_MS) {
    lastRssiMs = now;
    Serial.printf("RSSI peak=%d dBm  MARCSTATE=0x%02X\n", peakRssi, marcState());
    peakRssi = -200;
  }
  yield();
}
