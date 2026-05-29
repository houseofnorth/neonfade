#include "Button.h"
#include "Motor.h"
#include "Calibration.h"
#include "config.h"

void Button::init(Motor* motor, Calibration* cal) {
  _motor = motor;
  _cal   = cal;
  pinMode(BUTTON, INPUT_PULLUP);
}

void Button::update() {
  bool          buttonDown = (digitalRead(BUTTON) == LOW);
  unsigned long now        = millis();

  // Log every raw state transition immediately
  if (buttonDown != _lastRaw) {
    _lastRaw = buttonDown;
    Serial.printf("[btn] GPIO9 → %s  (millis=%lu)\n",
      buttonDown ? "LOW (pressed)" : "HIGH (released)", now);
  }

  // Falling edge — button just pressed
  if (buttonDown && !_pressing) {
    _pressing   = true;
    _pressStart = now;
    _held       = true;
    Serial.printf("[btn] press start  (click %d in window)\n", _clickCount + 1);
  }

  // Rising edge — button just released
  if (!buttonDown && _pressing) {
    _pressing = false;
    _held     = false;
    if (now - _pressStart >= 20) {  // 20 ms debounce
      _clickCount++;
      _lastRelease = now;
      Serial.printf("[btn] released — %d click(s) so far\n", _clickCount);
    }
  }

  // Double-click window expired — act on accumulated count
  if (_clickCount > 0 && (now - _lastRelease) > DOUBLE_CLICK_MS) {
    int count = _clickCount;
    _clickCount = 0;
    if (_motor) _motor->stopAll();

    if (count == 1) {
      Serial.println("[btn] single click → saving LEFT limit");
      if (_cal) _cal->saveLeft();
    } else {
      Serial.println("[btn] double click → saving RIGHT limit");
      if (_cal) _cal->saveRight();
    }
  }
}
