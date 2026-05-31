// ── Persistent device settings ────────────────────────────────────────────
// Device name, WiFi credentials, and optional static IP.
// Stored in NVS under "skset" — separate from calibration ("spaceknot").
// Must be called before initWifi() so the right credentials are in place.
// Web interface can update all fields; changes take effect after reboot.

#include <Preferences.h>

static Preferences _cfgPrefs;

static String _devName  = "spaceknot";
static String _wSsid    = WIFI_SSID;
static String _wPass    = WIFI_PASSWORD;
static String _staticIp = "192.168.8.201";  // default static IP (unit 1 — change per unit via web UI or serial)
static String _staticGw = "192.168.8.1";   // default gateway

void initSettings() {
  if (!_cfgPrefs.begin("skset", false)) {
    LOG1("WARN: settings NVS failed — using defaults\n");
    return;
  }
  _devName  = _cfgPrefs.getString("name", "spaceknot");
  _wSsid    = _cfgPrefs.getString("ssid", WIFI_SSID);
  _wPass    = _cfgPrefs.getString("pass", WIFI_PASSWORD);
  _staticIp = _cfgPrefs.getString("sip",  "192.168.8.201");
  _staticGw = _cfgPrefs.getString("sgw",  "192.168.8.1");
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

// Write unit name to NVS and auto-derive static IP from trailing number.
// e.g. "spaceknot-3" → IP 192.168.2.203, gateway 192.168.2.1
void saveUnitId(const String& name) {
  if (!name.length()) return;
  _devName = name;
  _cfgPrefs.putString("name", _devName);
  LOG2("Unit ID saved: %s\n", _devName.c_str());

  int dash = name.lastIndexOf('-');
  if (dash >= 0) {
    int n = name.substring(dash + 1).toInt();
    if (n >= 1 && n <= 55) {
      int lastOctet = 200 + n;
      if (lastOctet <= 254) {
        _staticIp = String(SK_SUBNET) + "." + String(lastOctet);
        _staticGw = String(SK_SUBNET) + ".1";
        _cfgPrefs.putString("sip", _staticIp);
        _cfgPrefs.putString("sgw", _staticGw);
        LOG2("Static IP: %s  gw: %s\n", _staticIp.c_str(), _staticGw.c_str());
      } else {
        LOG1("Unit %d: IP out of range — set manually via web UI\n", n);
      }
    }
  }
}

// Returns true if this unit has been assigned a number (spaceknot-N pattern).
bool isUnitNamed() {
  int dash = _devName.lastIndexOf('-');
  if (dash < 0) return false;
  int n = _devName.substring(dash + 1).toInt();
  return (n >= 1 && n <= 55);
}

// Read digits from Serial. Waits for first digit, then collects for 500 ms.
// Works regardless of Serial Monitor line-ending setting.
static int _readUnitNumber() {
  // wait for first digit
  while (true) {
    if (!Serial.available()) continue;
    char c = (char)Serial.read();
    if (c >= '0' && c <= '9') {
      String buf = "";
      buf += c;
      unsigned long t = millis();
      while (millis() - t < 500) {
        if (Serial.available()) {
          char d = (char)Serial.read();
          if (d >= '0' && d <= '9') { buf += d; t = millis(); }
        }
      }
      return buf.toInt();
    }
  }
}

// Call once after Serial is ready. Blocks until the user enters a valid number.
// Saves name + IP to NVS, then reboots so WiFi starts on the correct address.
void promptUnitIdIfNeeded() {
  if (isUnitNamed()) return;

  Serial.println();
  Serial.println("=== UNIT NOT CONFIGURED ===");
  Serial.println("Enter unit number (1-55) and press Send:");

  while (true) {
    int n = _readUnitNumber();
    if (n >= 1 && n <= 55) {
      saveUnitId("spaceknot-" + String(n));
      LOG1("\nConfigured as spaceknot-%d  %s — rebooting...\n",
                    n, _staticIp.c_str());
      delay(500);
      ESP.restart();
    } else {
      Serial.println("Invalid — enter a number between 1 and 55:");
    }
  }
}

// Re-run the unit number prompt regardless of current name — for renaming.
void forceRenameUnit() {
  Serial.println();
  LOG1("Current: %s  (%s)\n", _devName.c_str(), _staticIp.c_str());
  Serial.println("Enter new unit number (1-55) and press Send:");

  while (true) {
    int n = _readUnitNumber();
    if (n >= 1 && n <= 55) {
      saveUnitId("spaceknot-" + String(n));
      LOG1("\nRenamed → spaceknot-%d  %s — rebooting...\n",
                    n, _staticIp.c_str());
      delay(500);
      ESP.restart();
    } else {
      Serial.println("Invalid — enter a number between 1 and 55:");
    }
  }
}
