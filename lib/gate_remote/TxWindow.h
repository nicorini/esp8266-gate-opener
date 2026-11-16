// Decides when to transmit. A button press transmits while the button is held, for at least
// minMs and at most maxMs; trigger() transmits for a fixed time, on a HomeKit command.
#pragma once

class TxWindow {
 public:
  TxWindow(unsigned long minMs, unsigned long maxMs) : minMs_(minMs), maxMs_(maxMs) {}

  void press(unsigned long now) {
    start(now);
    held_ = true;
    untilMs_ = minMs_;
  }

  void release() { held_ = false; }

  void trigger(unsigned long now, unsigned long durationMs) {
    if (!active_) start(now);
    unsigned long until = now - startMs_ + durationMs;
    if (until > untilMs_) untilMs_ = until;
  }

  // Returns true while the radio should transmit.
  bool update(unsigned long now) {
    if (!active_) return false;
    unsigned long elapsed = now - startMs_;
    if (elapsed >= maxMs_) {
      active_ = false;
      timedOut_ = held_;
    } else if (!held_ && elapsed >= untilMs_) {
      active_ = false;
    }
    return active_;
  }

  // True once after a press was cut off at maxMs.
  bool takeTimeout() {
    bool t = timedOut_;
    timedOut_ = false;
    return t;
  }

 private:
  void start(unsigned long now) {
    startMs_ = now;
    active_ = true;
    untilMs_ = 0;
  }

  unsigned long minMs_;
  unsigned long maxMs_;
  unsigned long startMs_ = 0;
  unsigned long untilMs_ = 0;
  bool active_ = false;
  bool held_ = false;
  bool timedOut_ = false;
};
