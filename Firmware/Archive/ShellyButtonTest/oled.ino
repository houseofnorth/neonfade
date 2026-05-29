// ── OLED ───────────────────────────────────────────────────────────────────
//  AZ-Delivery 1.3" SH1106, 128×64, I2C 0x3C, default Wire pins (D4/D5).
//  Simple display: shows WiFi status, Shelly status, and button state.

#include <U8g2lib.h>

static U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

// Status strings — written by shelly_button.ino
static String _wifiStatus   = "--";
static String _shellyStatus = "offline";
static String _buttonState  = "?";

void setShellyButtonWifiStatus(const String& s) {
  _wifiStatus = (s.length() > 4) ? s.substring(0, 4) : s;
}

void setShellyButtonStatus(const String& status) {
  _shellyStatus = status;
}

void setShellyButtonState(const String& state) {
  _buttonState = state;
}

// ── Splash ─────────────────────────────────────────────────────────────────
void initDisplay() {
  display.begin();
  display.setFontPosTop();
  display.setDrawColor(1);

  display.clearBuffer();
  display.setFont(u8g2_font_6x13B_tf);
  display.drawStr(0, 0, "SHELLY BUTTON");
  display.setFont(u8g2_font_5x7_tf);
  display.drawStr(0, 15, FIRMWARE_VERSION);
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(0, 26, "Event-driven webhooks");
  display.drawStr(0, 36, "Waiting for WiFi...");
  display.drawStr(0, 50, "No polling!");
  display.sendBuffer();

  delay(OLED_SPLASH_MS);
}

// ── Main dashboard ─────────────────────────────────────────────────────────
static void _drawDashboard() {
  display.clearBuffer();

  display.setFont(u8g2_font_6x13B_tf);
  display.drawStr(0, 0, "POSITION");

  // Header status line
  display.setFont(u8g2_font_5x7_tf);
  char buf[48];
  snprintf(buf, sizeof(buf), "W:%s Webhook",
           _wifiStatus.c_str());
  display.drawStr(0, 10, buf);

  display.drawHLine(0, 18, 128);

  // Main position display — large number
  display.setFont(u8g2_font_fub20_tr);
  char posBuf[16];
  snprintf(posBuf, sizeof(posBuf), "%d / %d",
           getCurrentPosition(),
           getMaxPositions());

  // Center the position number
  display.drawStr(25, 30, posBuf);

  // Bottom: version + status
  display.setFont(u8g2_font_5x7_tf);
  display.drawStr(0, 58, FIRMWARE_VERSION);
  display.drawStr(55, 58, "Listening...");

  display.sendBuffer();
}

// ── Public refresh entry point ────────────────────────────────────────────
void updateDisplay() {
  static unsigned long _lastDraw = 0;
  if (millis() - _lastDraw < OLED_REFRESH_MS) return;
  _lastDraw = millis();

  _drawDashboard();
}
