// ── Pot calibration ───────────────────────────────────────────────────────
// Tracks ADC range. Fires when range >= CALIB_THRESHOLD or CALIB_TIMEOUT_MS.
// Falls back to CALIB_DEFAULT_MIN/MAX on timeout without a sweep.

static int           _potMin    = 4095;
static int           _potMax    = 0;
static bool          _calibDone = false;
static unsigned long _calibStart;

void initPot() {
  pinMode(POT_PIN, INPUT);
  _calibStart = millis();
}

void updatePot() {
  int v = analogRead(POT_PIN);
  if (v < _potMin) _potMin = v;
  if (v > _potMax) _potMax = v;

  if (_calibDone) return;

  bool swept   = (_potMax - _potMin) >= CALIB_THRESHOLD;
  bool timeout = (millis() - _calibStart) >= CALIB_TIMEOUT_MS;

  if (swept || timeout) {
    if (!swept) {
      _potMin = CALIB_DEFAULT_MIN;
      _potMax = CALIB_DEFAULT_MAX;
      Serial.println("Calib timeout — using defaults.");
    }
    _calibDone = true;
    Serial.printf("Calibrated: min=%d  max=%d  range=%d\n",
                  _potMin, _potMax, _potMax - _potMin);
  }
}

bool calibDone() { return _calibDone; }
int  getPotMin() { return _potMin; }
int  getPotMax() { return _potMax; }
