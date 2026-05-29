#pragma once
#include <Arduino.h>

enum MotorDir { STOPPED, FORWARD, REVERSE };

// ── Motor ─────────────────────────────────────────────────────────────────
// Owns LEDC PWM setup and all motion logic (position + oscillation).
// driveMotorToTarget() is called once per loop() via update().
//
// Commands from web routes and DMX both call the same setters —
// whoever wrote last wins.
// ──────────────────────────────────────────────────────────────────────────
class Motor {
public:
  void init();
  void update();  // call every loop — drives motor toward current target

  // ── Commands ────────────────────────────────────────────────────────────
  void setTarget(int adcValue);              // enter position mode
  void startOscillate(int inAdc, int outAdc); // enter oscillate mode (resets direction)
  void setOscPoints(int inAdc, int outAdc);   // update endpoints without resetting direction
  void stopAll();                            // stop motor and clear all targets
  void setSpeed(int duty);                   // 0-255 PWM duty

  // ── Getters ─────────────────────────────────────────────────────────────
  MotorDir dir()          const { return _dir; }
  int      speed()        const { return _speedDuty; }
  bool     isOscillating() const { return _oscillating; }
  int      targetPos()    const { return _targetPos; }
  int      oscIn()        const { return _oscIn; }
  int      oscOut()       const { return _oscOut; }
  bool     oscGoingOut()  const { return _oscGoingOut; }

private:
  void _forward();
  void _reverse();
  void _stop();

  MotorDir _dir         = STOPPED;
  int      _speedDuty   = 255;
  bool     _oscillating = false;
  int      _targetPos   = -1;
  int      _oscIn       = -1;
  int      _oscOut      = -1;
  bool     _oscGoingOut = true;
};
