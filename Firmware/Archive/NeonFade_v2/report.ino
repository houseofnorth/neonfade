// ── Serial report ─────────────────────────────────────────────────────────
// One status line every REPORT_INTERVAL_MS.

static unsigned long _lastReport = 0;
static uint8_t       _lineCount  = 0;

void reportSerial() {
  if (millis() - _lastReport < REPORT_INTERVAL_MS) return;
  _lastReport = millis();

  if (isCalibrating()) return;  // don't stomp L/R/A/S feedback

  if (_lineCount == 0)
    Serial.println("POT    MIN    MAX    MOTOR    RELAY  STATE");
  if (++_lineCount >= 20) _lineCount = 0;

  const char* mot = (getMotorDir() == FORWARD) ? "FORWARD" :
                    (getMotorDir() == REVERSE) ? "REVERSE" : "STOP   ";

  const char* stateStr;
  if      (!calibDone())     stateStr = "NEED  ";
  else if (!stateCalDone())  stateStr = "NO-POS";
  else if (!getRelayState()) stateStr = "---   ";
  else                       stateStr = getDimStateName(getCurrentDimState());

  Serial.printf("%-6d %-6d %-6d %-8s %-6s %s\n",
    analogRead(POT_PIN),
    getPotMin(), getPotMax(),
    mot,
    getRelayState() ? "ON" : "OFF",
    stateStr
  );
}
