// RF link test, transmitter: sends a known code (protocol 1, 24 bits, low byte is a counter)
// once a second, at low power. Run rf_rx_test on a second board. The on-board LED blinks on every
// send. It keeps transmitting: run it briefly, the 433 MHz band has duty-cycle limits.

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <RCSwitch.h>
#include <SmartRC_CC1101.h>

const uint8_t GDO0_PIN = 5;
const uint8_t LED_PIN = 2;  // on-board LED, active low
const float TX_MHZ = 433.92;
const int TX_POWER_DBM = -10;  // plenty at 1-2 m
const unsigned long TEST_CODE = 0xA5A500;
const unsigned long PERIOD_MS = 1000;

RCSwitch rf;
SmartRC_CC1101 radio;
uint8_t counter = 0;
unsigned long lastSendMs = 0;

// CC1101 state machine: 0x01 IDLE, 0x0D RX, 0x13 TX.
uint8_t marcState() { return radio.SpiReadStatus(CC1101_MARCSTATE) & 0x1F; }

void setup() {
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  Serial.begin(115200);
  delay(200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  Serial.printf("\nrf_tx_test %s\n", FIRMWARE_VERSION);

  radio.Init();
  Serial.printf("PARTNUM=0x%02X VERSION=0x%02X (expected 0x00 and 0x14 or 0x04)\n",
                radio.SpiReadStatus(CC1101_PARTNUM), radio.SpiReadStatus(CC1101_VERSION));

  radio.setModulation(2);  // ASK/OOK
  radio.setMHZ(TX_MHZ);
  radio.setPA(TX_POWER_DBM);

  rf.enableTransmit(GDO0_PIN);
  rf.setProtocol(1);
  rf.setRepeatTransmit(3);

  radio.SetTx();
  Serial.printf("Transmitting on %.2f MHz, %d dBm\n", TX_MHZ, TX_POWER_DBM);
}

void loop() {
  unsigned long now = millis();
  if (now - lastSendMs < PERIOD_MS) return;
  lastSendMs = now;

  unsigned long code = TEST_CODE | counter++;
  digitalWrite(LED_PIN, LOW);
  rf.send(code, 24);
  digitalWrite(LED_PIN, HIGH);
  Serial.printf("sent 0x%06lX  MARCSTATE=0x%02X\n", code, marcState());
}
