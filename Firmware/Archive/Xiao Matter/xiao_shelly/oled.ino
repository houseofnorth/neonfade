// =============================================================================
// oled.ino — OLED Display Functions
// Hardware: AZ-Delivery 1.3" I2C OLED (SH1106, 128x64)
// Library : U8g2 (install via Arduino Library Manager)
// =============================================================================
// Functions:
//   initDisplay()     — start the display and show a splash screen
//   showDashboard()   — main screen: WiFi, light state, brightness, status
//   showMessage()     — two-line helper for simple messages (title + body)
//   showWifiStatus()  — defined in wifi_manager.ino, calls showMessage()
// =============================================================================
// Display coordinates: x=0 is left, y=0 is top. Font baseline is at y.
// With font u8g2_font_6x10_tf each character is ~6px wide, 10px tall.
// Row positions used below leave ~2px padding between lines.
// =============================================================================

// Row Y positions for a 64px tall display (font height ~10px)
const int ROW1_Y = 12;   // Title / header
const int ROW2_Y = 26;   // Line 2
const int ROW3_Y = 40;   // Line 3
const int ROW4_Y = 54;   // Line 4 / footer

// -----------------------------------------------------------------------------
// initDisplay()
// Start the U8g2 driver and show a short splash screen.
// Call this first in setup() so you have display output immediately.
// -----------------------------------------------------------------------------
void initDisplay() {
  display.begin();
  display.setFont(u8g2_font_6x10_tf);   // clear, readable at this size
  display.setFontRefHeightExtendedText();
  display.setDrawColor(1);
  display.setFontPosTop();              // y coordinate = top of character

  // --- Splash screen ---
  display.clearBuffer();
  display.setFont(u8g2_font_7x14B_tf); // slightly larger for title
  display.drawStr(10, 10, "Shelly Ctrl");
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(20, 30, "XIAO ESP32");
  display.drawStr(18, 44, "Starting up...");
  display.sendBuffer();

  delay(2000);  // let the user read the splash
}

// -----------------------------------------------------------------------------
// showDashboard()
// The main screen shown during normal operation. Reads the global variables
// defined in xiao_shelly.ino (lightOn, brightness, lastStatus, etc.) and
// redraws the full display.
//
// Layout:
//   Line 1 (header): "Shelly Dimmer Ctrl"
//   Line 2 (wifi)  : "WiFi: OK 192.168.x.x"  or  "WiFi: No WiFi"
//   Line 3 (light) : "Light: ON   Dim: 75%"  or  "Light: OFF"
//   Line 4 (status): "Status: OK"
// -----------------------------------------------------------------------------
void showDashboard() {
  // Rate-limit redraws — software I2C can't keep up if called every loop()
  static unsigned long lastDrawAt = 0;
  const unsigned long DISPLAY_REFRESH_MS = 500;
  if (millis() - lastDrawAt < DISPLAY_REFRESH_MS) return;
  lastDrawAt = millis();

  String wifiLine   = "WiFi: " + getWifiStatusText();           // wifi_manager.ino
  String lightState = lightOn ? "ON" : "OFF";
  String lightLine  = "Light: " + lightState;
  if (lightOn) {
    lightLine += "  Dim: " + String(brightness) + "%";
  }
  String statusLine = "Status: " + lastStatus;

  display.clearBuffer();

  // Header — slightly bolder font
  display.setFont(u8g2_font_7x14B_tf);
  display.drawStr(0, 0, "Shelly Ctrl");

  // Horizontal separator line under header
  display.drawHLine(0, 14, 128);

  // Body — regular font
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(0, ROW2_Y, wifiLine.c_str());
  display.drawStr(0, ROW3_Y, lightLine.c_str());
  display.drawStr(0, ROW4_Y, statusLine.c_str());

  display.sendBuffer();
}

// -----------------------------------------------------------------------------
// showMessage(title, body)
// Simple two-line display for status messages during startup or errors.
// Title is shown in a larger font at the top; body below it.
//
// Example:  showMessage("WiFi OK", "192.168.1.42");
//           showMessage("Error", "Shelly unreachable");
// -----------------------------------------------------------------------------
void showMessage(const String& title, const String& body) {
  display.clearBuffer();

  display.setFont(u8g2_font_7x14B_tf);
  display.drawStr(0, 4, title.c_str());

  display.drawHLine(0, 20, 128);

  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(0, 26, body.c_str());

  display.sendBuffer();
}

// -----------------------------------------------------------------------------
// showBrightnessBar()
// Draws a progress bar representing the current brightness level.
// Useful when you are actively adjusting brightness during testing.
//
// Example:  showBrightnessBar(75);  // shows a 75% filled bar
// -----------------------------------------------------------------------------
void showBrightnessBar(int level) {
  // Clamp value to 0–100
  level = constrain(level, 0, 100);

  display.clearBuffer();

  display.setFont(u8g2_font_7x14B_tf);
  display.drawStr(0, 0, "Brightness");
  display.drawHLine(0, 16, 128);

  // Percentage text centred on line 3
  display.setFont(u8g2_font_10x20_tf);  // large digits
  String pct = String(level) + "%";
  int textX = (128 - (pct.length() * 10)) / 2;  // rough centre
  display.drawStr(textX, 20, pct.c_str());

  // Progress bar — full width is 120px, leaving 4px padding on each side
  int barWidth = map(level, 0, 100, 0, 120);
  display.drawFrame(4, 46, 120, 14);     // outer border
  display.drawBox(4, 46, barWidth, 14);  // filled portion

  display.sendBuffer();
}
