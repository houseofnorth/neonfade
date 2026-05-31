// ── LEDs ──────────────────────────────────────────────────────────────────
//  LED_A (D7) blinks to surface calibration state, then mirrors the relay
//  once the unit is calibrated and running:
//     fast blink (150 ms) → IN CALIBRATION MODE
//     slow blink (500 ms) → NEEDS CALIBRATION (no saved cal at boot)
//     steady mirror       → calibrated; LED_A follows the relay (~1 Hz)
//  LED_B (D8) is solid ON when motor is FORWARD, OFF otherwise.

static unsigned long _lastBlink  = 0;
static bool          _blinkState = false;

void initLeds() {
  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);
  digitalWrite(LED_A, LOW);
  digitalWrite(LED_B, LOW);
}

void updateLeds() {
  if (isCalibrating()) {
    // Fast blink — "actively calibrating, watching for L/R/A/S".
    if (millis() - _lastBlink >= 150) {
      _blinkState = !_blinkState;
      digitalWrite(LED_A, _blinkState);
      _lastBlink = millis();
    }
    digitalWrite(LED_B, LOW);
  } else if (!calibDone()) {
    // Slow blink — "needs cal, motor disabled".
    if (millis() - _lastBlink >= 500) {
      _blinkState = !_blinkState;
      digitalWrite(LED_A, _blinkState);
      _lastBlink = millis();
    }
    digitalWrite(LED_B, LOW);
  } else {
    digitalWrite(LED_A, getRelayState());            // mirrors relay 1 Hz
    digitalWrite(LED_B, getMotorDir() == FORWARD);   // motor direction
  }
}
