// ── Relay ─────────────────────────────────────────────────────────────────
// Event-driven control of K1 via MMBT3904 on RELAY_PIN (D6).
// Driven by the Shelly button webhook — not a timer.
//   1st single press (relay OFF) → setRelay(true)
//   long press                   → setRelay(false)
// TP 'R': ~5V relay OFF  ↔  ~0.1V relay ON.

static bool _relayState = false;

void initRelay() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  _relayState = false;
}

void setRelay(bool on) {
  _relayState = on;
  digitalWrite(RELAY_PIN, on ? HIGH : LOW);
}

void updateRelay() {
  // No-op: relay is event-driven via setRelay().
  // Kept in loop() for API compatibility.
}

bool getRelayState() { return _relayState; }
