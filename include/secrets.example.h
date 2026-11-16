// Copy to secrets.h and fill in the values read from your remote with the sniffer firmware.
#pragma once

constexpr const char* GATE_CODE = "000000000000";  // binary code
constexpr int GATE_PROTOCOL = 11;                  // rc-switch protocol
constexpr int GATE_PULSE_US = 320;                 // base pulse length
constexpr float GATE_MHZ = 433.92;                 // carrier frequency

// HomeKit. Leave WIFI_SSID empty for a button-only opener with WiFi off.
constexpr const char* WIFI_SSID = "";                     // 2.4 GHz network
constexpr const char* WIFI_PASSWORD = "";
constexpr const char* HOMEKIT_SETUP_CODE = "000-00-000";  // pick your own, format XXX-XX-XXX
