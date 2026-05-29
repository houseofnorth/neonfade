// =============================================================================
// wifi_manager.ino — Wi-Fi Connection Management
// =============================================================================
// Functions:
//   initWifi()           — initial connection at startup
//   checkWifiConnection() — call in loop() to reconnect if dropped
//   getWifiStatusText()  — returns a short status string for the display
// =============================================================================

// How long to wait for a connection before giving up (milliseconds)
const int WIFI_TIMEOUT_MS = 10000;

// -----------------------------------------------------------------------------
// initWifi()
// Called once in setup(). Blocks until connected or timeout is reached.
// Shows progress on Serial so you can follow along in the IDE monitor.
// -----------------------------------------------------------------------------
void initWifi() {
  Serial.println("[WiFi] Connecting to: " + String(WIFI_SSID));
  showMessage("WiFi", "Connecting...");   // oled.ino

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startTime = millis();

  // Wait until connected or timeout expires
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > WIFI_TIMEOUT_MS) {
      Serial.println("[WiFi] Timeout — could not connect.");
      showMessage("WiFi", "TIMEOUT!");    // oled.ino
      return;
    }
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("[WiFi] Connected. IP: " + WiFi.localIP().toString());
}

// -----------------------------------------------------------------------------
// checkWifiConnection()
// Call this every loop() iteration. If the connection dropped, it will
// attempt to reconnect quietly in the background.
// -----------------------------------------------------------------------------
void checkWifiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WiFi] Connection lost — reconnecting...");
    showMessage("WiFi", "Reconnecting");  // oled.ino
    WiFi.reconnect();

    // Give it a moment to re-establish
    delay(3000);

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("[WiFi] Reconnected. IP: " + WiFi.localIP().toString());
    } else {
      Serial.println("[WiFi] Still disconnected.");
    }
  }
}

// -----------------------------------------------------------------------------
// getWifiStatusText()
// Returns a short string describing the current Wi-Fi state.
// Used by oled.ino to display one line of Wi-Fi info.
// -----------------------------------------------------------------------------
String getWifiStatusText() {
  if (WiFi.status() == WL_CONNECTED) {
    return "OK " + WiFi.localIP().toString();
  }
  return "No WiFi";
}

// -----------------------------------------------------------------------------
// showWifiStatus()
// Shows the full Wi-Fi connection result on the OLED after initWifi().
// -----------------------------------------------------------------------------
void showWifiStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    showMessage("WiFi OK", WiFi.localIP().toString());  // oled.ino
  } else {
    showMessage("WiFi", "Not connected");               // oled.ino
  }
  delay(1500);  // pause so the user can read it
}
