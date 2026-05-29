// ── Shelly Dimmer Gen4 ────────────────────────────────────────────────────
//  Sends the calibrated pot position (as 0-100 brightness) to a Shelly
//  Dimmer Gen4 via HTTP RPC. The catch: HTTPClient.GET() blocks until it
//  gets a response or times out, and if we run that on the main loop the
//  motor controller will overshoot its endpoints while we wait. So all HTTP
//  traffic lives on a dedicated FreeRTOS task; the main loop only writes
//  the current target brightness into a shared variable.
//
//  Protocol (local, no cloud):
//    GET http://<ip>/rpc/Light.GetStatus?id=0           ← probe
//    GET http://<ip>/rpc/Light.Set?id=0&on=true&brightness=NN
//
//  When the Shelly isn't reachable, the task probes every
//  SHELLY_PROBE_INTERVAL_MS (60 s) and stays silent in between, so a
//  missing dimmer doesn't churn the network or the serial log.

#include <WiFi.h>
#include <HTTPClient.h>

// ── Shared state between main loop (writer) and task (reader) ─────────────
//  volatile because they're touched from two tasks. They're 32-bit aligned
//  on RISC-V so individual reads/writes are atomic — no mutex needed for a
//  simple int.
static volatile int  _shellyTargetPct = -1;   // -1 = no target yet
static volatile bool _shellyOnline    = false;

// ── Brightness derivation ────────────────────────────────────────────────
static int _potToBrightness() {
  if (!calibDone()) return -1;
  const int pot   = analogRead(POT_PIN);
  const int pMin  = getPotMin();
  const int pMax  = getPotMax();
  if (pMax - pMin < 1) return -1;
  return constrain((int) map(pot, pMin, pMax, 0, 100), 0, 100);
}

// ── HTTP helpers (run only from the Shelly task) ─────────────────────────
static bool _httpGet(const String& url) {
  HTTPClient http;
  http.setConnectTimeout(SHELLY_HTTP_TIMEOUT_MS);
  http.setTimeout(SHELLY_HTTP_TIMEOUT_MS);
  if (!http.begin(url)) return false;
  int code = http.GET();
  http.end();
  return code > 0 && code < 400;
}

static bool _probeShelly() {
  return _httpGet(String("http://") + SHELLY_IP + "/rpc/Light.GetStatus?id=0");
}

static bool _sendBrightness(int pct) {
  String url = String("http://") + SHELLY_IP
             + "/rpc/Light.Set?id=0&on=true&brightness=" + String(pct);
  return _httpGet(url);
}

// ── The task itself ──────────────────────────────────────────────────────
//  Runs at the same priority as the Arduino loopTask. When this task blocks
//  on the socket recv() inside HTTPClient, FreeRTOS lets the loopTask run —
//  so the motor controller stays responsive.
static void _shellyTask(void* /*arg*/) {
  int           lastSent  = -1;
  unsigned long lastProbe = 0;

  for (;;) {
    if (!wifiIsConnected()) {
      _shellyOnline = false;
      vTaskDelay(pdMS_TO_TICKS(500));
      continue;
    }

    // Periodic probe — first time runs immediately (lastProbe==0).
    if (millis() - lastProbe >= SHELLY_PROBE_INTERVAL_MS || lastProbe == 0) {
      const bool was = _shellyOnline;
      _shellyOnline = _probeShelly();
      lastProbe = millis();
      if (_shellyOnline && !was) {
        Serial.printf("[Shelly] online at %s\n", SHELLY_IP);
      } else if (!_shellyOnline && was) {
        Serial.println("[Shelly] gone offline");
      } else if (!_shellyOnline && !was) {
        Serial.printf("[Shelly] not reachable at %s\n", SHELLY_IP);
      }
    }

    // Forward brightness changes to the dimmer.
    if (_shellyOnline) {
      const int target = _shellyTargetPct;
      if (target >= 0 && (lastSent < 0 || abs(target - lastSent) >= SHELLY_MIN_DELTA)) {
        if (_sendBrightness(target)) {
          lastSent = target;
        } else {
          Serial.println("[Shelly] send failed, marking offline");
          _shellyOnline = false;
          lastSent = -1;
        }
      }
    }

    vTaskDelay(pdMS_TO_TICKS(SHELLY_SEND_MIN_INTERVAL_MS));
  }
}

// ── Public API ───────────────────────────────────────────────────────────
void initShelly() {
  _shellyTargetPct = -1;
  _shellyOnline    = false;
  xTaskCreate(_shellyTask, "shelly",
              SHELLY_TASK_STACK, NULL, SHELLY_TASK_PRIO, NULL);
}

// Called from main loop. Just updates the shared target; the task does the
// rest. Cheap (single int write).
void updateShelly() {
  if (!calibDone() || isCalibrating()) {
    _shellyTargetPct = -1;
    return;
  }
  _shellyTargetPct = _potToBrightness();
}

bool shellyIsOnline() { return _shellyOnline; }
