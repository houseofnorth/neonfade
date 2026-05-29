// ── Serial report ─────────────────────────────────────────────────────────
// One status line every REPORT_INTERVAL_MS — columns map to test points.

static unsigned long _lastReport = 0;
static uint8_t       _lineCount  = 0;

void reportSerial() {
  if (millis() - _lastReport < REPORT_INTERVAL_MS) return;
  _lastReport = millis();

  if (_lineCount == 0)
    Serial.println("POT    MIN    MAX    MOTOR    RELAY  CALIB");
  if (++_lineCount >= 20) _lineCount = 0;

  const char* mot = (getMotorDir() == FORWARD)  ? "FORWARD" :
                    (getMotorDir() == REVERSE)  ? "REVERSE" : "STOP   ";

  Serial.printf("%-6d %-6d %-6d %-8s %-6s %s\n",
    analogRead(POT_PIN),
    getPotMin(), getPotMax(),
    mot,
    getRelayState() ? "ON" : "OFF",
    calibDone()     ? "OK" : "..."
  );
}
