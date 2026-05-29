#pragma once
#include <Arduino.h>

class Motor;
class Calibration;

// ── Button ────────────────────────────────────────────────────────────────
// Single / double click detection on the BOOT button (GPIO9, active LOW).
//   1 click  → save left  (MIN) calibration limit
//   2 clicks → save right (MAX) calibration limit
//
// held() returns true while the button is physically down, so Leds::update()
// can light both LEDs as immediate "registered" feedback.
// ──────────────────────────────────────────────────────────────────────────
class Button {
public:
  void init(Motor* motor, Calibration* cal);
  void update();
  bool held() const { return _held; }

private:
  Motor*        _motor      = nullptr;
  Calibration*  _cal        = nullptr;

  bool          _pressing    = false;
  unsigned long _pressStart  = 0;
  int           _clickCount  = 0;
  unsigned long _lastRelease = 0;
  bool          _lastRaw     = true;   // HIGH = not pressed
  bool          _held        = false;
};
