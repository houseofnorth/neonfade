// ── Motor ─────────────────────────────────────────────────────────────────
// LEDC PWM on IN1/IN2 driving the L293DD H-bridge.
// Moves to the pot target saved for the current DimState (Day/Dusk/Night).
// Relay must be ON and state positions must be calibrated to run.
// Direction: FORWARD = IN1 high / IN2 low (matches Motor.cpp in production).
//
// Manual override:
//   If the pot is turned by hand while the motor is settled, _manualOverride
//   is set and the motor freezes. Cleared by the next button press so the
//   motor drives to the current state target from wherever the pot is.

// MotorDir enum is in config.h
static MotorDir _motorDir       = STOPPED;
static bool     _settled        = false;   // hysteresis: wider threshold to restart
static bool     _manualOverride = false;   // pot moved by hand; motor frozen
static int      _targetPotPos   = -1;      // raw ADC target; -1 = not set
static DimState _lastDimState   = DIM_NONE; // sentinel — forces target calc on first run

void initMotor() {
  ledcAttach(MOTOR_IN1, PWM_FREQ, PWM_BITS);
  ledcAttach(MOTOR_IN2, PWM_FREQ, PWM_BITS);
  ledcWrite(MOTOR_IN1, 0);
  ledcWrite(MOTOR_IN2, 0);
}

static void _driveForward(int speed = MOTOR_SPEED_MAX) {
  ledcWrite(MOTOR_IN1, speed);
  ledcWrite(MOTOR_IN2, 0);
  _motorDir = FORWARD;
}

static void _driveReverse(int speed = MOTOR_SPEED_MAX) {
  ledcWrite(MOTOR_IN1, 0);
  ledcWrite(MOTOR_IN2, speed);
  _motorDir = REVERSE;
}

static void _motorStop() {
  ledcWrite(MOTOR_IN1, 0);
  ledcWrite(MOTOR_IN2, 0);
  _motorDir = STOPPED;
}

// Return the saved pot ADC target for the current dim state, or -1 if not set.
static int _calculateTargetPos() {
  return getStatePos(getCurrentDimState());
}

// Discrete position control: drive to the current state target and hold.
static void _updateDiscreteMode() {
  if (_manualOverride) return;

  // Recalculate target whenever dim state changes (including first call via DIM_NONE sentinel).
  DimState curState = getCurrentDimState();
  if (curState != _lastDimState) {
    _targetPotPos = _calculateTargetPos();
    _lastDimState = curState;
    if (_targetPotPos >= 0) _settled = false;  // new valid target — start driving
  }

  // State position not calibrated yet — stop and wait.
  if (_targetPotPos < 0) { _motorStop(); return; }

  int currentPotPos = analogRead(POT_PIN);
  int error = currentPotPos - _targetPotPos;
  int dist  = abs(error);

  const int restartThresh = MOTOR_DEADBAND + 12;

  if (dist <= MOTOR_DEADBAND) {
    if (_motorDir != STOPPED) { _motorStop(); _settled = true; }
    return;
  }

  if (_settled) {
    // Large displacement while settled = manual pot turn → freeze.
    if (dist > MANUAL_TURN_THRESHOLD) {
      _manualOverride = true;
      _targetPotPos   = currentPotPos;
      Serial.println("Manual pot turn detected — motor frozen. Press button to resume.");
      return;
    }
    if (dist < restartThresh) return;  // noise — stay stopped
  }

  _settled = false;
  if (error > 0) _driveReverse();
  else           _driveForward();
}

// Main motor update.
// Motor is stopped if: range not calibrated, in cal mode, relay OFF.
void updateMotor() {
  if (!calibDone() || isCalibrating()) {
    _motorStop();
    return;
  }
  if (!getRelayState()) {
    _motorStop();
    return;
  }
  _updateDiscreteMode();
}

MotorDir getMotorDir()      { return _motorDir; }
bool     isManualOverride() { return _manualOverride; }

// Called by webhook on each button press: clears any manual override so the
// motor drives to the (possibly new) state target.
void clearManualOverride() {
  if (!_manualOverride) return;
  _manualOverride = false;
  _settled        = false;
  Serial.println("Manual override cleared — motor resuming.");
}

// Called on long press (relay OFF): cuts PWM and freezes target at current
// pot position so discrete mode won't try to snap on next wake.
void holdMotorPosition() {
  ledcWrite(MOTOR_IN1, 0);
  ledcWrite(MOTOR_IN2, 0);
  _motorDir       = STOPPED;
  _targetPotPos   = analogRead(POT_PIN);
  _settled        = true;
  _manualOverride = false;
}
