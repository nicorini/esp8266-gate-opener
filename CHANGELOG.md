# Changelog

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
