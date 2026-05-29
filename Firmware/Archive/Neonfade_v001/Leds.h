#pragma once
#include <Arduino.h>
#include "Motor.h"  // for MotorDir enum

// ── Leds ──────────────────────────────────────────────────────────────────
// Sole owner of LED_A and LED_B state.
// update() is the only place that drives the pins — call once per loop().
// Priority: button held > motor moving > off.
// ──────────────────────────────────────────────────────────────────────────
class Leds {
public:
  void init();
  void update(MotorDir dir, bool buttonHeld);
  void blinkOne(int pin, int times);
  void blinkBoth(int times);

private:
  void off();
  void left();
  void right();
  void both();
};
