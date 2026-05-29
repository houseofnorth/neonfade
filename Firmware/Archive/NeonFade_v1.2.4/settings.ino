// ── Persistent device settings ────────────────────────────────────────────
// Device name, WiFi credentials, and optional static IP.
// Stored in NVS under "nfset" — separate from calibration ("neonfade").
// Must be called before initWifi() so the right credentials are in place.
// Web interface can update all fields; changes take effect after reboot.

#include <Preferences.h>

static Preferences _cfgPrefs;

static String _devName  = "spaceknot";
static String _wSsid    = WIFI_SSID;
static String _wPass    = WIFI_PASSWORD;
static String _staticIp = "192.168.1.200";  // default static IP
static String _staticGw = "192.168.1.1";   // default gateway

void initSettings() {
  if (!_cfgPrefs.begin("nfset", false)) {
    Serial.println("WARN: settings NVS failed — using compile-time defaults.");
    return;
  }
  _devName  = _cfgPrefs.getString("name", "spaceknot");
  _wSsid    = _cfgPrefs.getString("ssid", WIFI_SSID);
  _wPass    = _cfgPrefs.getString("pass", WIFI_PASSWORD);
  _staticIp = _cfgPrefs.getString("sip",  "192.168.1.200");
  _staticGw = _cfgPrefs.getString("sgw",  "192.168.1.1");
}

// Blank pass = keep existing (user left the password field empty in the form).
void saveSettings(const String& name, const String& ssid, const String& pass,
                  const String& sip,  const String& sgw) {
  if (name.length()) _devName  = name;
  if (ssid.length()) _wSsid    = ssid;
  if (pass.length()) _wPass    = pass;
  _staticIp = sip;
  _staticGw = sgw;
  _cfgPrefs.putString("name", _devName);
  _cfgPrefs.putString("ssid", _wSsid);
  _cfgPrefs.putString("pass", _wPass);
  _cfgPrefs.putString("sip",  _staticIp);
  _cfgPrefs.putString("sgw",  _staticGw);
}

String getDeviceName() { return _devName; }
String getWifiSsid()   { return _wSsid; }
String getWifiPass()   { return _wPass; }
String getStaticIp()   { return _staticIp; }
String getStaticGw()   { return _staticGw; }
