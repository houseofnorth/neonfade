// ── WiFi ──────────────────────────────────────────────────────────────────
//  Three operating modes picked at compile time via WIFI_MODE_CHOICE:
//    NF_STA   — join the home network only. Retries forever on failure.
//    NF_AP    — board hosts its own hotspot.
//    NF_DUAL  — try STA once; on timeout, fall back to AP.

enum WifiState {
  WF_AP,           // AP mode running
  WF_STA_TRYING,   // WiFi.begin() called, polling for WL_CONNECTED
  WF_STA_UP,       // STA connected with IP
  WF_STA_BACKOFF   // STA failed; waiting WIFI_RETRY_INTERVAL_MS to retry
};

// Forward declarations
static void _enter(WifiState s);
static void _startSta();
static void _startAp();

static WifiState     _state          = WF_STA_TRYING;
static unsigned long _stateEnteredAt = 0;
static bool          _apActive       = false;
static String        _ipString       = "";

#include <WiFi.h>

static void _setWifiStatus(const String& s) {
  setShellyButtonWifiStatus(s);
}

static void _enter(WifiState s) {
  _state = s;
  _stateEnteredAt = millis();

  switch (_state) {
    case WF_AP:         _setWifiStatus("AP"); break;
    case WF_STA_TRYING: _setWifiStatus(".."); break;
    case WF_STA_UP:     _setWifiStatus("OK"); break;
    case WF_STA_BACKOFF:_setWifiStatus("ERR"); break;
  }
}

static void _startSta() {
  Serial.printf("[WiFi] STA connecting to '%s'...\n", WIFI_SSID);
  Serial.printf("[WiFi] Password: %s\n", WIFI_PASSWORD);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  _ipString = "";
  _enter(WF_STA_TRYING);
}

static void _startAp() {
  Serial.printf("[WiFi] starting AP '%s'\n", WIFI_AP_SSID);
  WiFi.mode(WIFI_AP);
  if (WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD)) {
    _apActive = true;
    _ipString = WiFi.softAPIP().toString();
    Serial.printf("[WiFi] AP up at %s (pass: %s)\n",
                  _ipString.c_str(), WIFI_AP_PASSWORD);
  } else {
    Serial.println("[WiFi] AP start failed");
    _apActive = false;
  }
  _enter(WF_AP);
}

void initWifi() {
  WiFi.mode(WIFI_OFF);
  _apActive = false;
  _ipString = "";

  if (WIFI_MODE_CHOICE == NF_AP) _startAp();
  else                           _startSta();
}

void updateWifi() {
  switch (_state) {
    case WF_STA_TRYING:
      if (WiFi.status() == WL_CONNECTED) {
        _ipString = WiFi.localIP().toString();
        Serial.printf("[WiFi] STA connected. IP %s\n", _ipString.c_str());
        _enter(WF_STA_UP);
      } else if (millis() - _stateEnteredAt > WIFI_CONNECT_TIMEOUT_MS) {
        int status = WiFi.status();
        Serial.printf("[WiFi] STA connect timeout (status=%d)\n", status);
        if (WIFI_MODE_CHOICE == NF_DUAL) {
          _startAp();
        } else {
          _enter(WF_STA_BACKOFF);
        }
      }
      break;

    case WF_STA_UP:
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WiFi] STA link dropped");
        _ipString = "";
        _enter(WF_STA_BACKOFF);
      }
      break;

    case WF_STA_BACKOFF:
      if (millis() - _stateEnteredAt > WIFI_RETRY_INTERVAL_MS) {
        _startSta();
      }
      break;

    case WF_AP:
      break;
  }
}

bool   wifiIsConnected() { return _state == WF_STA_UP && WiFi.status() == WL_CONNECTED; }
bool   wifiIsApActive()  { return _apActive; }
String wifiIpString()    { return _ipString; }
