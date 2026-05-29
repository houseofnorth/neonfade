// ── Web interface ─────────────────────────────────────────────────────────
//  Routes:
//    GET /              — dashboard page
//    GET /status        — JSON: motor, relay, dimState, cal, step positions, ip, version
//    GET /relay?on=1|0  — force relay ON or OFF (calibration helper)
//    GET /cal?act=...   — calibration commands (start/min/max/auto/save/cancel/day/dusk/night)
//    GET /settings      — JSON: current name, ssid, static IP
//    POST /settings     — save settings + reboot
//    GET /webhook/button— Shelly button webhook

#include <WebServer.h>

static WebServer _server(WEB_PORT);
void handleButtonWebhook(WebServer* server);

// ── HTML page (PROGMEM) — defined in web_page.h to avoid Arduino preprocessor
//   misreading JavaScript 'function' keywords as C++ identifiers.
#include "web_page.h"

// ── Helpers ──────────────────────────────────────────────────────────────
static int _adcToPct(int adc) {
  const int pMin = getPotMin(), pMax = getPotMax();
  if (pMax - pMin < 1) return -1;
  return constrain((int) map(adc, pMin, pMax, 0, 100), 0, 100);
}

// ── Route handlers ───────────────────────────────────────────────────────
static void _onPage() {
  String page = FPSTR(WEB_PAGE);
  page.replace("%POLL_MS%", String(WEB_STATUS_POLL_MS));
  _server.send(200, "text/html", page);
}

static void _onStatus() {
  const int pot   = analogRead(POT_PIN);
  const int pMin  = getPotMin();
  const int pMax  = getPotMax();
  const int range = pMax - pMin;
  int pct = (range > 0) ? constrain((int) map(pot, pMin, pMax, 0, 100), 0, 100) : 0;

  int dayAdc   = getStatePos(DIM_DAY);
  int duskAdc  = getStatePos(DIM_DUSK);
  int nightAdc = getStatePos(DIM_NIGHT);

  String ver = "v" + String(VERSION_MAJOR) + "." + String(VERSION_MINOR) + "." + String(VERSION_PATCH);

  String json = "{";
  json += "\"pot\":"            + String(pot);
  json += ",\"potMin\":"        + String(pMin);
  json += ",\"potMax\":"        + String(pMax);
  json += ",\"brightnessPct\":" + String(pct);
  json += ",\"motorOn\":"       + String(getMotorDir() != STOPPED ? "true" : "false");
  json += ",\"relayOn\":"       + String(getRelayState() ? "true" : "false");
  json += ",\"dimState\":\""    + String(getDimStateName(getCurrentDimState())) + "\"";
  json += ",\"calOk\":"         + String(calibDone() ? "true" : "false");
  json += ",\"calibrating\":"   + String(isCalibrating() ? "true" : "false");
  json += ",\"dayOk\":"         + String(statePosSet(DIM_DAY)   ? "true" : "false");
  json += ",\"duskOk\":"        + String(statePosSet(DIM_DUSK)  ? "true" : "false");
  json += ",\"nightOk\":"       + String(statePosSet(DIM_NIGHT) ? "true" : "false");
  json += ",\"dayPct\":"        + String(dayAdc   >= 0 ? _adcToPct(dayAdc)   : -1);
  json += ",\"duskPct\":"       + String(duskAdc  >= 0 ? _adcToPct(duskAdc)  : -1);
  json += ",\"nightPct\":"      + String(nightAdc >= 0 ? _adcToPct(nightAdc) : -1);
  json += ",\"shellyOnline\":"  + String(shellyIsOnline() ? "true" : "false");
  json += ",\"ip\":\""          + wifiIpString() + "\"";
  json += ",\"version\":\""     + ver + "\"";
  json += "}";
  _server.send(200, "application/json", json);
}

static void _onRelay() {
  if (_server.hasArg("on")) setRelay(_server.arg("on") == "1");
  _server.send(200, "text/plain", "ok");
}

static void _onCal() {
  if (!_server.hasArg("act")) { _server.send(400, "text/plain", "missing act"); return; }
  const String act = _server.arg("act");
  if      (act == "start")  enterCalibration();
  else if (act == "min")    captureMin();
  else if (act == "max")    captureMax();
  else if (act == "auto")   toggleAutoSweep();
  else if (act == "save")   saveCalibration();
  else if (act == "cancel") cancelCalibration();
  else if (act == "day")    captureStatePos(DIM_DAY);
  else if (act == "dusk")   captureStatePos(DIM_DUSK);
  else if (act == "night")  captureStatePos(DIM_NIGHT);
  else { _server.send(400, "text/plain", "bad act"); return; }
  _server.send(200, "text/plain", "ok");
}

static void _onSettingsGet() {
  String json = "{";
  json += "\"name\":\"" + getDeviceName() + "\"";
  json += ",\"ssid\":\"" + getWifiSsid() + "\"";
  json += ",\"sip\":\""  + getStaticIp() + "\"";
  json += ",\"sgw\":\""  + getStaticGw() + "\"";
  json += "}";
  _server.send(200, "application/json", json);
}

static void _onSettingsPost() {
  saveSettings(
    _server.arg("name"),
    _server.arg("ssid"),
    _server.arg("pass"),
    _server.arg("sip"),
    _server.arg("sgw")
  );
  _server.send(200, "text/plain", "ok");
  delay(500);
  ESP.restart();
}

// ── Public API ───────────────────────────────────────────────────────────
void initWebInterface() {
  _server.on("/",        HTTP_GET,  _onPage);
  _server.on("/status",  HTTP_GET,  _onStatus);
  _server.on("/relay",   HTTP_GET,  _onRelay);
  _server.on("/cal",     HTTP_GET,  _onCal);
  _server.on("/settings",HTTP_GET,  _onSettingsGet);
  _server.on("/settings",HTTP_POST, _onSettingsPost);
  _server.on("/webhook/button", HTTP_GET,  [](){ handleButtonWebhook(&_server); });
  _server.on("/webhook/button", HTTP_POST, [](){ handleButtonWebhook(&_server); });
  _server.onNotFound([]() { _server.send(404, "text/plain", "not found"); });
}

void updateWebInterface() {
  const bool wifiUp = wifiIsConnected() || wifiIsApActive();

  if (!_serverRunning && wifiUp) {
    _server.begin();
    _serverRunning = true;
    setOledServerRunning(true);
  } else if (_serverRunning && !wifiUp) {
    _server.stop();
    _serverRunning = false;
    setOledServerRunning(false);
  }

  if (_serverRunning) _server.handleClient();
}

bool webServerRunning() { return _serverRunning; }
