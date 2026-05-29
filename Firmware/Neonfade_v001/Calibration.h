#pragma once
#include <Arduino.h>

class Leds;  // forward declare — used only for blink feedback

// ── Calibration ───────────────────────────────────────────────────────────
// Reads and writes calMin / calMax to flash via Preferences.
// saveLeft() / saveRight() take a stable pot reading, persist to flash,
// and blink the corresponding LED via the injected Leds pointer.
// ──────────────────────────────────────────────────────────────────────────
class Calibration {
public:
  void init(Leds* leds);
  void load();
  void saveLeft();
  void saveRight();

  int  minVal()  const { return _min; }
  int  maxVal()  const { return _max; }
  bool isValid() const { return _valid; }

private:
  void save();
  int  stableReading();

  Leds* _leds  = nullptr;
  int   _min   = 80;
  int   _max   = 3900;
  bool  _valid = false;
};
