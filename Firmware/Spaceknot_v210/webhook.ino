// ── Dim State Controller ──────────────────────────────────────────────────
//  Button logic shared by HTTP webhook (fallback/test) and Shelly MQTT.
//
//  Behaviour:
//    single_push  (relay OFF) → relay ON, resume last saved state
//    single_push  (relay ON)  → cycle: Day → Dusk → Night → Day …
//                               saves new state to NVS immediately
//    long_push                → relay OFF, motor freezes; state kept for next ON
//
//  Primary input: Shelly Gen4 native MQTT (see mqtt.ino).
//  Fallback/test: HTTP GET /webhook/button?push=1 or ?sweep=1

#include <WebServer.h>

static DimState      _dimState         = DIM_DAY;
static unsigned long _lastPushMs       = 0;
const  unsigned long PUSH_DEBOUNCE_MS  = 600;

// ── Shared button logic ───────────────────────────────────────────────────
//  Called from both the HTTP handler and the MQTT Shelly callback.

void handleButtonPush() {
  unsigned long now = millis();
  if (now - _lastPushMs < PUSH_DEBOUNCE_MS) {
    LOG2("Button push debounced.\n");
    return;
  }
  _lastPushMs = now;
  clearManualOverride();

  if (!getRelayState()) {
    setRelay(true);
    mqttPublishState(_dimState);
    LOG2("→ ON  [%s]\n", getDimStateName(_dimState));
  } else {
    _dimState = (DimState)(((int)_dimState + 1) % 3);
    saveLastDimState(_dimState);
    mqttPublishState(_dimState);
    LOG2("→ %s\n", getDimStateName(_dimState));
  }
}

void handleButtonSweep() {
  setRelay(false);
  holdMotorPosition();
  mqttPublishOff();
  LOG2("→ OFF  [long press]\n");
}

// ── HTTP webhook handler (fallback/test — registered by web_interface.ino) 
void handleButtonWebhook(WebServer* server) {
  if (server->hasArg("sweep")) {
    handleButtonSweep();
    server->send(200, "application/json", "{\"status\":\"ok\"}");
    return;
  }
  if (server->hasArg("push")) {
    handleButtonPush();
    server->send(200, "application/json", "{\"status\":\"ok\"}");
    return;
  }
  server->send(400, "application/json", "{\"error\":\"missing parameter\"}");
}

// ── MQTT state injection (called from mqtt.ino callback) ──────────────────
void setDimStateFromMqtt(DimState s) {
  _dimState = s;
  saveLastDimState(s);
  clearManualOverride();
  if (!getRelayState()) setRelay(true);
  LOG2("MQTT state applied: %s\n", getDimStateName(s));
}

// ── Public API ────────────────────────────────────────────────────────────
void initWebhook() {
  _dimState = loadLastDimState();
}

DimState    getCurrentDimState()    { return _dimState; }

const char* getDimStateName(DimState s) {
  switch (s) {
    case DIM_DAY:   return "Day";
    case DIM_DUSK:  return "Dusk";
    case DIM_NIGHT: return "Night";
    default:        return "---";
  }
}
