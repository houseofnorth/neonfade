// ── Pot calibration ───────────────────────────────────────────────────────
//  No auto-sweep on boot. On power-up the firmware loads min/max from NVS
//  (Preferences). If nothing is stored, the motor stays disabled and the
//  OLED shows NEEDS CAL. The user drives calibration from the Serial Monitor:
//    C  enter cal mode
//    L  capture current pot reading as MIN
//    R  capture current pot reading as MAX
//    A  toggle live auto-sweep tracking (updates min/max as you move it)
//    S  validate (max-min ≥ CALIB_MIN_RANGE) and save to flash; exits cal
//
//  Saved values survive power cycles. To re-calibrate, just type C again.

#include <Preferences.h>

static int  _potMin      = 0;
static int  _potMax      = 0;
static bool _calibDone   = false;  // a valid calibration is loaded
static bool _calibrating = false;  // currently in cal mode
static bool _autoSweep   = false;  // live min/max tracking inside cal mode

static Preferences _prefs;

// ──────────────────────────────────────────────────────────────────────────
//  initPot()
//  Loads any previously-saved calibration from NVS. If none, leaves the
//  firmware in an un-calibrated state (motor stays stopped).
// ──────────────────────────────────────────────────────────────────────────

// ── Pot ADC moving average ────────────────────────────────────────────────
//  8-sample average suppresses motor-induced noise on the ADC rail.
#define POT_AVG_N 8
static int _potSamples[POT_AVG_N];
static int _potAvgIdx  = 0;
static bool _potAvgReady = false;

int readPot() {
  _potSamples[_potAvgIdx] = analogRead(POT_PIN);
  _potAvgIdx = (_potAvgIdx + 1) % POT_AVG_N;
  if (_potAvgIdx == 0) _potAvgReady = true;
  int n = _potAvgReady ? POT_AVG_N : _potAvgIdx;
  if (n == 0) return analogRead(POT_PIN);
  long sum = 0;
  for (int i = 0; i < n; i++) sum += _potSamples[i];
  return (int)(sum / n);
}

void initPot() {
  pinMode(POT_PIN, INPUT);

  if (!_prefs.begin(PREFS_NAMESPACE, /*readOnly=*/false)) {
    LOG1("WARN: NVS open failed\n");
    return;
  }

  if (_prefs.getBool(PREFS_KEY_VALID, false)) {
    _potMin    = _prefs.getInt(PREFS_KEY_MIN, 0);
    _potMax    = _prefs.getInt(PREFS_KEY_MAX, 0);
    _calibDone = (_potMax - _potMin) >= CALIB_MIN_RANGE;
    if (_calibDone) {
      LOG2("Cal loaded: min=%d max=%d range=%d\n",
                    _potMin, _potMax);
    } else {
      LOG1("WARN: saved cal invalid — ignoring\n");
    }
  } else {
    // No saved calibration — use full-travel defaults so motor can run.
    _potMin    = POT_DEFAULT_MIN;
    _potMax    = POT_DEFAULT_MAX;
    _calibDone = true;
    LOG1("No cal saved — using defaults (min=%d max=%d)\n",
                  POT_DEFAULT_MIN, POT_DEFAULT_MAX);
  }

  _loadStatePositions();
}

// ──────────────────────────────────────────────────────────────────────────
//  updatePot()
//  Only does work in cal mode with auto-sweep enabled, where it widens the
//  captured min/max from live pot readings.
// ──────────────────────────────────────────────────────────────────────────
void updatePot() {
  if (!(_calibrating && _autoSweep)) return;
  const int v = readPot();
  if (v < _potMin) _potMin = v;
  if (v > _potMax) _potMax = v;
}

// ── Serial command handlers (called from serial.ino) ─────────────────────

void enterCalibration() {
  _calibrating = true;
  _autoSweep   = false;
  // Reset capture window to extremes so any L/R/A widens it correctly.
  _potMin = 4095;
  _potMax = 0;
  // Relay on so the user can see the light level while sweeping.
  if (!getRelayState()) setRelay(true);
  Serial.println();
  Serial.println("=== CAL MODE ===");
  Serial.println("  Range calibration:");
  Serial.println("    L = capture MIN at current pot position");
  Serial.println("    R = capture MAX at current pot position");
  Serial.println("    A = toggle auto-sweep (move pot end-to-end)");
  Serial.println("    S = save range to flash and exit");
  Serial.println("  Step positions (or use web UI):");
  Serial.println("    D = capture DAY position");
  Serial.println("    U = capture DUSK position");
  Serial.println("    G = capture NIGHT position");
  Serial.println("  Other:");
  Serial.println("    N = rename this unit (prompts 1-999, reboots)");
  Serial.println("    X = cancel without saving");
}

void captureMin() {
  if (!_calibrating) { LOG1("Not in cal mode (type C first).\n"); return; }
  _potMin = readPot();
  LOG1("MIN captured: %d\n", _potMin);
}

void captureMax() {
  if (!_calibrating) { LOG1("Not in cal mode (type C first).\n"); return; }
  _potMax = readPot();
  LOG1("MAX captured: %d\n", _potMax);
}

void toggleAutoSweep() {
  if (!_calibrating) { LOG1("Not in cal mode (type C first).\n"); return; }
  _autoSweep = !_autoSweep;
  if (_autoSweep) {
    // Restart the tracking window so previous L/R values don't lock the range.
    _potMin = 4095;
    _potMax = 0;
    LOG1("Auto-sweep ON — move pot end-to-end, then A again to stop.\n");
  } else {
    LOG1("Auto-sweep OFF  min=%d max=%d range=%d\n",
                  _potMin, _potMax);
  }
}

// Exit cal mode WITHOUT writing to flash. Used by the web UI's Cancel
// button; not exposed over the serial command set (kept minimal as
// requested). If we never had a valid calibration, calibDone() stays false.
void cancelCalibration() {
  if (!_calibrating) return;
  _calibrating = false;
  _autoSweep   = false;
  // If we already had a saved calibration before entering cal mode, the
  // current _potMin/_potMax may have been clobbered by L/R/A. Reload from
  // NVS so the running state matches what's on disk.
  if (_prefs.getBool(PREFS_KEY_VALID, false)) {
    _potMin    = _prefs.getInt(PREFS_KEY_MIN, 0);
    _potMax    = _prefs.getInt(PREFS_KEY_MAX, 0);
    _calibDone = (_potMax - _potMin) >= CALIB_MIN_RANGE;
  } else {
    _calibDone = false;
  }
  LOG1("Cal cancelled\n");
}

void saveCalibration() {
  if (!_calibrating) { LOG1("Not in cal mode (type C first).\n"); return; }
  const int range = _potMax - _potMin;
  if (range < CALIB_MIN_RANGE) {
    LOG1("ERR: range too small (%d). Capture endpoints first.\n",
                  _potMin, _potMax, range, CALIB_MIN_RANGE);
    return;
  }
  _prefs.putInt(PREFS_KEY_MIN, _potMin);
  _prefs.putInt(PREFS_KEY_MAX, _potMax);
  _prefs.putBool(PREFS_KEY_VALID, true);
  _calibDone   = true;
  _calibrating = false;
  _autoSweep   = false;
  LOG1("Cal saved: min=%d max=%d. Motor enabled.\n",
                _potMin, _potMax, range);
}

// ── Range calibration getters ─────────────────────────────────────────────
bool calibDone()     { return _calibDone; }
bool isCalibrating() { return _calibrating; }
bool isAutoSweep()   { return _autoSweep; }
int  getPotMin()     { return _potMin; }
int  getPotMax()     { return _potMax; }

// ── Dim state positions ───────────────────────────────────────────────────
//  Three named pot targets (Day / Dusk / Night).
//  Captured via web UI; written to NVS immediately on capture.
//  A value of -1 means that state has not been calibrated yet.

static int _statePos[3] = { -1, -1, -1 };  // indexed by DimState

static void _loadStatePositions() {
  _statePos[DIM_DAY]   = _prefs.getInt(PREFS_KEY_DAY,   POT_DEFAULT_DAY);
  _statePos[DIM_DUSK]  = _prefs.getInt(PREFS_KEY_DUSK,  POT_DEFAULT_DUSK);
  _statePos[DIM_NIGHT] = _prefs.getInt(PREFS_KEY_NIGHT, POT_DEFAULT_NIGHT);
  LOG2("State pos loaded: Day=%d Dusk=%d Night=%d\n",
                _statePos[DIM_DAY], _statePos[DIM_DUSK], _statePos[DIM_NIGHT]);
}

void captureStatePos(DimState s) {
  if (s < DIM_DAY || s > DIM_NIGHT) return;
  static const char* names[] = { "Day", "Dusk", "Night" };
  static const char* keys[]  = { PREFS_KEY_DAY, PREFS_KEY_DUSK, PREFS_KEY_NIGHT };
  // Ensure relay is on so the user can see the brightness level being captured.
  if (!getRelayState()) setRelay(true);
  int v = readPot();
  _statePos[s] = v;
  _prefs.putInt(keys[s], v);
  LOG1("Pos [%s] = %d\n", names[s], v);
}

int  getStatePos(DimState s)  { return (s >= DIM_DAY && s <= DIM_NIGHT) ? _statePos[s] : -1; }
bool stateCalDone()            { return _statePos[DIM_DAY] >= 0 && _statePos[DIM_DUSK] >= 0 && _statePos[DIM_NIGHT] >= 0; }
bool statePosSet(DimState s)   { return (s >= DIM_DAY && s <= DIM_NIGHT) && _statePos[s] >= 0; }

// Last active dim state — persisted so relay-ON resumes the correct step.
void saveLastDimState(DimState s) {
  _prefs.putInt(PREFS_KEY_LAST_STATE, (int)s);
}

DimState loadLastDimState() {
  int s = _prefs.getInt(PREFS_KEY_LAST_STATE, (int)DIM_DAY);
  return (s >= 0 && s <= 2) ? (DimState)s : DIM_DAY;
}
