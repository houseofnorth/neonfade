#include "Motor.h"
#include "config.h"

// ── Private helpers ───────────────────────────────────────────────────────

void Motor::_stop() {
  ledcWrite(MOTOR_IN1, 0);
  ledcWrite(MOTOR_IN2, 0);
}

void Motor::_forward() {
  ledcWrite(MOTOR_IN1, _speedDuty);
  ledcWrite(MOTOR_IN2, 0);
}

void Motor::_reverse() {
  ledcWrite(MOTOR_IN1, 0);
  ledcWrite(MOTOR_IN2, _speedDuty);
}

// ── Public interface ──────────────────────────────────────────────────────

void Motor::init() {
  // ESP32 Arduino core 3.x LEDC API: ledcAttach(pin, freq_hz, resolution_bits)
  // For core 2.x: ledcSetup(ch, freq, bits) + ledcAttachPin(pin, ch)
  ledcAttach(MOTOR_IN1, PWM_FREQ, PWM_BITS);
  ledcAttach(MOTOR_IN2, PWM_FREQ, PWM_BITS);
  _stop();
}

void Motor::setTarget(int adcValue) {
  _oscillating = false;
  _targetPos   = adcValue;
}

void Motor::startOscillate(int inAdc, int outAdc) {
  _targetPos   = -1;
  _oscIn       = inAdc;
  _oscOut      = outAdc;
  _oscGoingOut = true;   // always start heading outward
  _oscillating = true;
}

void Motor::setOscPoints(int inAdc, int outAdc) {
  // Update endpoints without resetting direction — used by DMX polling
  // to adjust live endpoints while already oscillating.
  _oscIn  = inAdc;
  _oscOut = outAdc;
  if (!_oscillating) {
    _targetPos   = -1;
    _oscGoingOut = true;
    _oscillating = true;
  }
}

void Motor::stopAll() {
  _oscillating = false;
  _targetPos   = -1;
  _stop();
  _dir = STOPPED;
}

void Motor::setSpeed(int duty) {
  _speedDuty = duty;
}

// ── Update — called every loop() ─────────────────────────────────────────

void Motor::update() {
  // Oscillation mode
  if (_oscillating) {
    if (_oscIn < 0 || _oscOut < 0) { _stop(); _dir = STOPPED; return; }

    int target = _oscGoingOut ? _oscOut : _oscIn;
    int error  = target - analogRead(POT_PIN);

    if (abs(error) <= MOTOR_DEADBAND) {
      _oscGoingOut = !_oscGoingOut;  // flip direction at each end-point
      _stop();
      _dir = STOPPED;
    }
    else if (error > 0) { _forward(); _dir = FORWARD; }
    else                { _reverse(); _dir = REVERSE; }
    return;
  }

  // Position mode
  if (_targetPos < 0) { _stop(); _dir = STOPPED; return; }

  int error = _targetPos - analogRead(POT_PIN);

  if (abs(error) <= MOTOR_DEADBAND) { _stop();    _dir = STOPPED; }
  else if (error > 0)               { _forward(); _dir = FORWARD; }
  else                              { _reverse(); _dir = REVERSE; }
}
