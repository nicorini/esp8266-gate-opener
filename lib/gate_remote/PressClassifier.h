// Tells a click from a hold. A click (released before holdMs) shows the next diagnostics page;
// a hold opens the gate from holdMs until release. In basic mode (no display, so nothing to
// show) every press is a hold from the start, like firmware v2.0. Call update() on every loop,
// before handling the button's events, so a hold is never reported late as a click.
#pragma once

class PressClassifier {
 public:
  enum Event { None, Click, HoldStart, HoldEnd };

  explicit PressClassifier(unsigned long holdMs) : holdMs_(holdMs) {}

  Event press(unsigned long now, bool basic) {
    down_ = true;
    holding_ = basic;
    pressMs_ = now;
    return basic ? HoldStart : None;
  }

  Event release(unsigned long now) {
    if (!down_) return None;
    down_ = false;
    return holding_ ? HoldEnd : Click;
  }

  Event update(unsigned long now) {
    if (!down_ || holding_ || now - pressMs_ < holdMs_) return None;
    holding_ = true;
    return HoldStart;
  }

  unsigned char progress(unsigned long now) const {
    if (!down_) return 0;
    unsigned long held = now - pressMs_;
    if (holding_ || held >= holdMs_) return 100;
    return held * 100 / holdMs_;
  }

 private:
  unsigned long holdMs_;
  unsigned long pressMs_ = 0;
  bool down_ = false;
  bool holding_ = false;
};
