// ── Motor ─────────────────────────────────────────────────────────────────
// LEDC PWM on IN1/IN2 driving the L293DD H-bridge.
// Oscillates between calibrated min/max once calibration is done.
// Direction matches Motor.cpp in production: FORWARD = IN1 high, IN2 low.

// MotorDir enum is in config.h
static MotorDir _motorDir = STOPPED;

void initMotor() {
  ledcAttach(MOTOR_IN1, PWM_FREQ, PWM_BITS);
  ledcAttach(MOTOR_IN2, PWM_FREQ, PWM_BITS);
  ledcWrite(MOTOR_IN1, 0);
  ledcWrite(MOTOR_IN2, 0);
}

static void _driveForward() {
  ledcWrite(MOTOR_IN1, MOTOR_SPEED);
  ledcWrite(MOTOR_IN2, 0);
  _motorDir = FORWARD;
}

static void _driveReverse() {
  ledcWrite(MOTOR_IN1, 0);
  ledcWrite(MOTOR_IN2, MOTOR_SPEED);
  _motorDir = REVERSE;
}

static void _motorStop() {
  ledcWrite(MOTOR_IN1, 0);
  ledcWrite(MOTOR_IN2, 0);
  _motorDir = STOPPED;
}

// Oscillate between calibrated limits using pot feedback. Called every loop.
void updateMotor() {
  if (!calibDone()) { _motorStop(); return; }

  // Kick off oscillation after calibration completes.
  static bool started = false;
  if (!started) { _driveForward(); started = true; }

  int pos   = analogRead(POT_PIN);
  int error = (_motorDir == FORWARD)
              ? getPotMax() - pos
              : pos - getPotMin();

  if (error <= MOTOR_DEADBAND) {
    // Reached end — flip direction.
    (_motorDir == FORWARD) ? _driveReverse() : _driveForward();
  }
}

MotorDir getMotorDir() { return _motorDir; }
