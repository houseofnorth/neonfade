// ── WiFi ──────────────────────────────────────────────────────────────────
//  Three operating modes picked at compile time via WIFI_MODE_CHOICE:
//    STA   — join the home network only. Retries forever on failure.
//    AP    — board hosts its own hotspot. Web UI at 192.168.4.1. No
//            Shelly comms because we're not on the home LAN.
//    DUAL  — try STA once; on timeout, fall back to AP. AP stays up for
//            the rest of the session so any connected browser keeps working.
//
//  Exposes (read by oled.ino, web_interface.ino, shelly.ino):
//    initWifi(), updateWifi()
//    wifiIsConnected()  — true iff STA has an IP right now
//    wifiIsApActive()   — true if AP is up
//    wifiIpString()     — STA IP, AP IP, or "" — whatever is reachable

#include <WiFi.h>

// WifiState enum is defined in config.h

static WifiState     _state          = WF_STA_TRYING; // overridden in initWifi
static unsigned long _stateEnteredAt = 0;
static bool          _apActive       = false;
static String        _ipString       = "";

static void _setOledFromState() {
  switch (_state) {
    case WF_AP:         setOledWifiStatus("AP"); break;
    case WF_STA_TRYING: setOledWifiStatus(".."); break;
    case WF_STA_UP:     setOledWifiStatus("OK"); break;
    case WF_STA_BACKOFF:setOledWifiStatus("ERR"); break;
  }
}

static void _enter(WifiState s) {
  _state = s;
  _stateEnteredAt = millis();
  _setOledFromState();
}

static void _startSta() {
  if (getStaticIp().length() > 0) {
    IPAddress ip, gw, sn(255, 255, 255, 0);
    if (ip.fromString(getStaticIp())) {
      gw.fromString(getStaticGw().length() ? getStaticGw() : "192.168.1.1");
      WiFi.config(ip, gw, sn);
    }
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(getWifiSsid().c_str(), getWifiPass().c_str());
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

  if (WIFI_MODE_CHOICE == NF_WIFI_AP) _startAp();
  else                                _startSta();
}

void updateWifi() {
  switch (_state) {
    case WF_STA_TRYING:
      if (WiFi.status() == WL_CONNECTED) {
        _ipString = WiFi.localIP().toString();
        Serial.printf("[WiFi] STA connected. IP %s\n", _ipString.c_str());
        _enter(WF_STA_UP);
      } else if (millis() - _stateEnteredAt > WIFI_CONNECT_TIMEOUT_MS) {
        Serial.println("[WiFi] STA connect timeout");
        if (WIFI_MODE_CHOICE == NF_WIFI_DUAL) {
          // One-shot fall back to AP; we stay there for the session.
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
      // AP is autonomous; nothing to poll. Intentionally no STA retry from
      // here — it would tear down active browser sessions on the AP.
      break;
  }
}

bool   wifiIsConnected() { return _state == WF_STA_UP && WiFi.status() == WL_CONNECTED; }
bool   wifiIsApActive()  { return _apActive; }
String wifiIpString()    { return _ipString; }
