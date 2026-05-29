// ── Dim State Controller (Shelly Button) ─────────────────────────────────
//  Event-driven input from Shelly Dimmer Gen4 via webhook.
//
//  Behaviour:
//    single_push  (relay OFF) → relay ON, resume last saved state
//    single_push  (relay ON)  → cycle: Day → Dusk → Night → Day …
//                               saves new state to NVS immediately
//    long_push                → relay OFF, motor freezes; state kept for next ON
//
//  Shelly webhook setup (via Shelly web UI Actions or RPC):
//    single_push event → http://XIAO_IP/webhook/button?push=1
//    long_push  event  → http://XIAO_IP/webhook/button?sweep=1
//
//  The webhook handler is registered by web_interface.ino on the shared
//  WebServer instance to avoid port conflicts.

#include <WebServer.h>

static DimState      _dimState         = DIM_DAY;
static unsigned long _lastPushMs       = 0;
const  unsigned long PUSH_DEBOUNCE_MS  = 600;  // guard against Shelly double-fire

// ── Webhook handler (called from web_interface.ino) ──────────────────────
void handleButtonWebhook(WebServer* server) {
  if (server->hasArg("sweep")) {
    // Long press: relay OFF; state is kept so next ON resumes here.
    setRelay(false);
    holdMotorPosition();
    Serial.printf("Long press — relay OFF (state=%s).\n", getDimStateName(_dimState));
    server->send(200, "application/json", "{\"status\":\"ok\"}");
    return;
  }

  if (server->hasArg("push")) {
    // Debounce: ignore rapid re-fires (some Shelly firmware double-fires single_push)
    unsigned long now = millis();
    if (now - _lastPushMs < PUSH_DEBOUNCE_MS) {
      server->send(200, "application/json", "{\"status\":\"debounced\"}");
      return;
    }
    _lastPushMs = now;

    clearManualOverride();

    if (!getRelayState()) {
      // Relay was OFF — first press: turn on and resume last saved state.
      setRelay(true);
      Serial.printf("Relay ON — state: %s\n", getDimStateName(_dimState));
    } else {
      // Relay already ON — advance to next state and persist.
      _dimState = (DimState)(((int)_dimState + 1) % 3);
      saveLastDimState(_dimState);
      Serial.printf("State: %s\n", getDimStateName(_dimState));
    }
    server->send(200, "application/json", "{\"status\":\"ok\"}");
    return;
  }

  server->send(400, "application/json", "{\"error\":\"missing parameter\"}");
}

// ── Public API ────────────────────────────────────────────────────────────
void initWebhook() {
  _dimState = loadLastDimState();  // resume last active state from NVS
}

DimState getCurrentDimState() { return _dimState; }

const char* getDimStateName(DimState s) {
  switch (s) {
    case DIM_DAY:   return "Day";
    case DIM_DUSK:  return "Dusk";
    case DIM_NIGHT: return "Night";
    default:        return "---";
  }
}
