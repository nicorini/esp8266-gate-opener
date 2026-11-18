# Changelog

## [2.1] - unreleased

- `gate_opener`: optional 0.96" I2C OLED on D3/D4. With the display, the button opens after a
  1 s hold, with a filling bar, and a short click shows status pages (state, WiFi, openings,
  system). Boot, WiFi and HomeKit pairing messages; lasting faults stay on as small moving text.
  Without a display, or with it unplugged, every press opens at once as in 2.0.
- `oled_test`: I2C scan and display test pages.
- 3D-printed enclosure in `hardware/enclosure.scad`.

## [2.0] - 2026-11-16

- `gate_opener`: optional HomeKit switch. Turning it on in the Home app or with Siri, also from a
  HomePod, transmits for 1 s. WiFi and the pairing code go in `secrets.h`; with no WiFi
  configured it stays button-only, WiFi off, as in 1.0.
- Radio, button and transmit timing moved into `lib/gate_remote/`, with native unit tests.

## [1.0] - 2026-11-14

First release, tested on the barrier.

- `gate_opener`: replays the remote's code while the button is held, from 0.5 s up to 10 s.
- `sniffer`: decodes a remote and prints code, protocol, pulse length and RSSI.
- `rf_tx_test` and `rf_rx_test`: radio link test between two boards.
