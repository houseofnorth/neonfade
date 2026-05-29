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
#include <Update.h>

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

  // Execute locally first, then broadcast to all other units via MQTT.
  // MQTT callback uses the same command strings so all units run identical code.
  if      (act == "start")  { enterCalibration();         mqttPublishCal("start");    }
  else if (act == "min")    { captureMin();                mqttPublishCal("min");      }
  else if (act == "max")    { captureMax();                mqttPublishCal("max");      }
  else if (act == "auto")   { toggleAutoSweep();           mqttPublishCal("auto");     }
  else if (act == "save")   { saveCalibration();           mqttPublishCal("save");     }
  else if (act == "cancel") { cancelCalibration();         mqttPublishCal("cancel");   }
  else if (act == "day")    { captureStatePos(DIM_DAY);   mqttPublishCal("capDay");   }
  else if (act == "dusk")   { captureStatePos(DIM_DUSK);  mqttPublishCal("capDusk");  }
  else if (act == "night")  { captureStatePos(DIM_NIGHT); mqttPublishCal("capNight"); }
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

static void _onUpdatePage() {
  _server.send(200, "text/html",
    "<!doctype html><html><head><meta charset='utf-8'>"
    "<title>Firmware Update</title>"
    "<style>body{font-family:sans-serif;max-width:480px;margin:3rem auto;color:#eee;background:#1a1a1a}"
    "h2{letter-spacing:.1em}input[type=file]{margin:1rem 0;width:100%}"
    "input[type=submit]{background:#ff4d88;color:#fff;border:none;padding:.6rem 1.4rem;"
    "font-size:1rem;cursor:pointer;border-radius:4px}"
    ".note{color:#888;font-size:.85rem;margin-top:1.5rem}</style></head>"
    "<body><h2>FIRMWARE UPDATE</h2>"
    "<form method='POST' action='/update' enctype='multipart/form-data'>"
    "<input type='file' name='firmware' accept='.bin'>"
    "<input type='submit' value='Upload &amp; Flash'>"
    "</form>"
    "<p class='note'>Sketch → Export Compiled Binary in Arduino IDE, then upload the .bin here.</p>"
    "</body></html>"
  );
}

static void _onUpdatePost() {
  bool ok = !Update.hasError();
  _server.send(200, "text/html",
    ok ? "<!doctype html><html><body style='font-family:sans-serif;background:#1a1a1a;color:#eee;"
         "text-align:center;padding:3rem'><h2>&#10003; Update OK</h2><p>Rebooting…</p></body></html>"
       : "<!doctype html><html><body style='font-family:sans-serif;background:#1a1a1a;color:#ff4d88;"
         "text-align:center;padding:3rem'><h2>&#10007; Update failed</h2><a href='/update'>Try again</a></body></html>"
  );
  delay(500);
  ESP.restart();
}

static void _onUpdateUpload() {
  HTTPUpload& upload = _server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    LOG1("[OTA-Web] %s  (%u bytes)\n", upload.filename.c_str(), upload.totalSize);
    Update.begin(UPDATE_SIZE_UNKNOWN);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Update.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    Update.end(true);
    if (Update.hasError())
      LOG1("[OTA-Web] Failed\n");
    else
      LOG1("[OTA-Web] OK — %u bytes\n", upload.totalSize);
  }
}

void initWebInterface() {
  _server.on("/",        HTTP_GET,  _onPage);
  _server.on("/status",  HTTP_GET,  _onStatus);
  _server.on("/relay",   HTTP_GET,  _onRelay);
  _server.on("/cal",     HTTP_GET,  _onCal);
  _server.on("/settings",HTTP_GET,  _onSettingsGet);
  _server.on("/settings",HTTP_POST, _onSettingsPost);
  _server.on("/webhook/button", HTTP_GET,  [](){ handleButtonWebhook(&_server); });
  _server.on("/webhook/button", HTTP_POST, [](){ handleButtonWebhook(&_server); });
  _server.on("/update", HTTP_GET,  _onUpdatePage);
  _server.on("/update", HTTP_POST, _onUpdatePost, _onUpdateUpload);
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
