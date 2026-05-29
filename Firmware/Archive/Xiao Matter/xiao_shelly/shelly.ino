// =============================================================================
// shelly.ino — Shelly Dimmer Gen4 Communication
// Protocol: HTTP RPC (Shelly Gen4 built-in REST-like API over Wi-Fi)
// =============================================================================
// The Shelly Gen4 exposes an HTTP RPC interface at:
//   http://<shelly-ip>/rpc/<Method>?param=value
//
// Key methods used here:
//   Light.GetStatus?id=0          — read current state
//   Light.Set?id=0&on=true        — turn on
//   Light.Set?id=0&on=false       — turn off
//   Light.Set?id=0&brightness=75  — set brightness (0–100)
//
// All functions update the global variables (lightOn, brightness, lastStatus)
// defined in xiao_shelly.ino, so oled.ino can display them.
// =============================================================================
// Functions:
//   initShelly()         — fetch initial state at startup
//   updateShellyState()  — poll current state (call in loop)
//   shellyTurnOn()       — send turn-on command
//   shellyTurnOff()      — send turn-off command
//   shellySetBrightness() — set brightness level (0–100)
//   shellyRequest()      — internal helper: sends HTTP GET, returns response body
// =============================================================================

#include <ArduinoJson.h>  // Install "ArduinoJson" by Benoit Blanchon via Library Manager

// How long to wait for a response from Shelly before giving up (milliseconds)
const int SHELLY_TIMEOUT_MS = 5000;

// -----------------------------------------------------------------------------
// initShelly()
// Called once in setup(). Fetches the current state from the Shelly so we
// start with accurate values rather than defaults.
// -----------------------------------------------------------------------------
void initShelly() {
  Serial.println("[Shelly] Fetching initial state...");
  showMessage("Shelly", "Connecting...");  // oled.ino

  bool success = fetchShellyState();

  if (success) {
    shellyOnline = true;
    lastStatus   = "Ready";
    Serial.println("[Shelly] Online. Light=" + String(lightOn ? "ON" : "OFF")
                   + " Brightness=" + String(brightness) + "%");
  } else {
    shellyOnline = false;
    lastStatus   = "Unreachable";
    Serial.println("[Shelly] Could not reach device at " + String(SHELLY_IP));
  }

  showMessage("Shelly", lastStatus);  // oled.ino
  delay(1500);
}

// -----------------------------------------------------------------------------
// updateShellyState()
// Poll the Shelly for its current state. Call this every loop() iteration.
// Updates globals so the dashboard stays current.
// -----------------------------------------------------------------------------
void updateShellyState() {
  // Skip polling if Wi-Fi is down — avoids spurious errors
  if (WiFi.status() != WL_CONNECTED) return;

  bool success = fetchShellyState();
  lastStatus = success ? "OK" : "Poll fail";

  if (!success) {
    Serial.println("[Shelly] State poll failed.");
  }
}

// -----------------------------------------------------------------------------
// shellyTurnOn()
// Send a turn-on command to the Shelly Dimmer.
// -----------------------------------------------------------------------------
void shellyTurnOn() {
  Serial.println("[Shelly] Sending: Turn ON");
  String response = shellyRequest("/rpc/Light.Set?id=0&on=true");

  if (response.length() > 0) {
    lightOn    = true;
    lastStatus = "Turned ON";
    Serial.println("[Shelly] Light turned ON.");
  } else {
    lastStatus = "ON failed";
    Serial.println("[Shelly] Turn ON command failed.");
  }
}

// -----------------------------------------------------------------------------
// shellyTurnOff()
// Send a turn-off command to the Shelly Dimmer.
// -----------------------------------------------------------------------------
void shellyTurnOff() {
  Serial.println("[Shelly] Sending: Turn OFF");
  String response = shellyRequest("/rpc/Light.Set?id=0&on=false");

  if (response.length() > 0) {
    lightOn    = false;
    lastStatus = "Turned OFF";
    Serial.println("[Shelly] Light turned OFF.");
  } else {
    lastStatus = "OFF failed";
    Serial.println("[Shelly] Turn OFF command failed.");
  }
}

// -----------------------------------------------------------------------------
// shellySetBrightness(int level)
// Set brightness on the Shelly Dimmer. Level must be 0–100.
// Also turns the light on if it is currently off.
// -----------------------------------------------------------------------------
void shellySetBrightness(int level) {
  // Clamp to valid range
  level = constrain(level, 0, 100);

  Serial.println("[Shelly] Setting brightness to " + String(level) + "%");

  // Build the RPC URL with brightness and turn on at the same time
  String path = "/rpc/Light.Set?id=0&on=true&brightness=" + String(level);
  String response = shellyRequest(path);

  if (response.length() > 0) {
    brightness = level;
    lightOn    = true;
    lastStatus = "Dim " + String(level) + "%";
    Serial.println("[Shelly] Brightness set to " + String(level) + "%");
    showBrightnessBar(level);  // oled.ino — show the bar briefly
    delay(1000);
  } else {
    lastStatus = "Dim failed";
    Serial.println("[Shelly] Set brightness failed.");
  }
}

// -----------------------------------------------------------------------------
// oscillateBrightness()
// Drives brightness up and down in a smooth triangle wave over a 3-second
// period. Call this every loop() — it manages its own non-blocking timing
// and only sends a new HTTP command every OSCILLATE_STEP_MS milliseconds.
//
// Triangle wave shape (3000 ms period):
//   0 ms  →  1500 ms :  brightness rises  0 → 100
//   1500 ms → 3000 ms :  brightness falls 100 →  0
// -----------------------------------------------------------------------------
void oscillateBrightness() {
  static unsigned long lastSentAt = 0;
  const unsigned long OSCILLATE_STEP_MS = 300;   // send update every 300 ms (10 steps / cycle)
  const unsigned long PERIOD_MS         = 3000;  // full up-down cycle length

  // Rate-limit: only act every OSCILLATE_STEP_MS ms
  if (millis() - lastSentAt < OSCILLATE_STEP_MS) return;
  lastSentAt = millis();

  // Position within the 3-second period: 0.0 → 1.0
  float pos = (float)(millis() % PERIOD_MS) / (float)PERIOD_MS;

  // Triangle wave: up in first half, down in second half
  int level;
  if (pos < 0.5f) {
    level = (int)(pos * 2.0f * 100.0f);        //   0 → 100
  } else {
    level = (int)((1.0f - pos) * 2.0f * 100.0f); // 100 →   0
  }

  // Keep minimum at 1 so the light stays physically on throughout
  level = constrain(level, 1, 100);

  // Send directly via shellyRequest() — no blocking OLED delay
  String path = "/rpc/Light.Set?id=0&on=true&brightness=" + String(level);
  String response = shellyRequest(path);

  if (response.length() > 0) {
    brightness = level;
    lightOn    = true;
    lastStatus = "Wave " + String(level) + "%";
    Serial.println("[Shelly] Oscillate → " + String(level) + "%");
  } else {
    lastStatus = "Wave fail";
    Serial.println("[Shelly] Oscillate HTTP failed.");
  }
}

// =============================================================================
// INTERNAL HELPERS — not called from loop() directly
// =============================================================================

// -----------------------------------------------------------------------------
// fetchShellyState()
// Internal: queries Light.GetStatus, parses JSON, updates globals.
// Returns true on success, false on failure.
// -----------------------------------------------------------------------------
bool fetchShellyState() {
  String response = shellyRequest("/rpc/Light.GetStatus?id=0");
  if (response.length() == 0) return false;

  // Parse the JSON response
  // Expected shape: {"id":0,"source":"...","output":true,"brightness":75,...}
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    Serial.println("[Shelly] JSON parse error: " + String(error.c_str()));
    return false;
  }

  // Update global state variables
  lightOn    = doc["output"].as<bool>();
  brightness = doc["brightness"].as<int>();

  return true;
}

// -----------------------------------------------------------------------------
// shellyRequest(String path)
// Internal: sends an HTTP GET to the Shelly at the given path.
// Returns the response body as a String, or "" on failure.
//
// Example:  shellyRequest("/rpc/Light.Set?id=0&on=true")
// -----------------------------------------------------------------------------
String shellyRequest(const String& path) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[Shelly] No Wi-Fi — skipping request.");
    return "";
  }

  HTTPClient http;
  String url = "http://" + String(SHELLY_IP) + path;

  Serial.println("[Shelly] GET " + url);

  http.begin(url);
  http.setTimeout(SHELLY_TIMEOUT_MS);

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String body = http.getString();
    http.end();
    Serial.println("[Shelly] Response (" + String(httpCode) + "): " + body);
    return body;
  } else {
    Serial.println("[Shelly] HTTP error: " + String(httpCode));
    http.end();
    return "";
  }
}
