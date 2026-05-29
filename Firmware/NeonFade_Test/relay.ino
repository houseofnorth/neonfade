// ── Relay ─────────────────────────────────────────────────────────────────
// Toggles K1 every RELAY_INTERVAL_MS via MMBT3904 on RELAY_PIN (D6).
// TP 'R': ~5V relay OFF  ↔  ~0.1V relay ON.

static bool          _relayState = false;
static unsigned long _lastToggle = 0;

void initRelay() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
}

void updateRelay() {
  if (millis() - _lastToggle < RELAY_INTERVAL_MS) return;
  _relayState = !_relayState;
  digitalWrite(RELAY_PIN, _relayState);
  _lastToggle = millis();
}

bool getRelayState() { return _relayState; }
