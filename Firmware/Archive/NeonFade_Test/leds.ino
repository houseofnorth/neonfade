// ── LEDs ──────────────────────────────────────────────────────────────────
// LED_A (D7) — blinks fast during calibration, then mirrors relay.
// LED_B (D8) — solid ON when motor FORWARD, OFF when REVERSE/STOPPED.

static unsigned long _lastBlink  = 0;
static bool          _blinkState = false;

void initLeds() {
  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);
  digitalWrite(LED_A, LOW);
  digitalWrite(LED_B, LOW);
}

void updateLeds() {
  if (!calibDone()) {
    // Fast blink on A — "waiting for sweep".
    if (millis() - _lastBlink >= 150) {
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
