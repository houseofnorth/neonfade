// ── OLED ──────────────────────────────────────────────────────────────────
//  AZ-Delivery 1.3" SH1106, 128×64, I2C 0x3C, default Wire pins (D4/D5).
//  Library : U8g2 by oliver.
//  Fonts: 6x13B for titles, 6x10 for body (clean 2-font layout).
//
//  Four screens, picked by state:
//
//    !_initialized        →  SPLASH (version, boot message)
//    isCalibrating()      →  CAL screen (live pot values, sweep toggle)
//    !calibDone()         →  NEEDS-CAL screen (large "NEEDS CALIBRATION")
//    calibDone()          →  DASHBOARD (position, motor, relay, bar)
//
//  All screens use consistent y-positions: header (0–15), content (17–48), footer (54–64).

#include <U8g2lib.h>

static U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

// Header status — written by wifi_manager / web_interface tabs.
//  _wifiStatus capped at 4 chars; the bold title is 48 px so the right side
//  has ~75 px for "W:xxxx" at 5 px/char.
static String _wifiStatus     = "--";
static bool   _serverRunning  = false;
static bool   _initialized    = false;

void setOledWifiStatus(const String& s) {
  _wifiStatus = (s.length() > 4) ? s.substring(0, 4) : s;
}

void setOledServerRunning(bool running) {
  _serverRunning = running;
}

// ── Splash (boot screen with version) ──────────────────────────────────────
void initDisplay() {
  display.begin();
  display.setFontPosTop();
  display.setDrawColor(1);

  display.clearBuffer();

  // Version in connectivity row
  display.setFont(u8g2_font_6x10_tf);
  char vbuf[20];
  snprintf(vbuf, sizeof(vbuf), "SPACEKNOT v%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
  display.drawStr(0, 2, vbuf);
  display.drawHLine(0, 13, 128);

  // Boot message
  display.drawStr(0, 24, "Ready to calibrate");
  display.drawStr(0, 38, "Type C in serial to start");

  display.sendBuffer();
  _initialized = true;

  delay(OLED_SPLASH_MS);
}

// ── Header (connectivity status bar) ────────────────────────────────────────
//  Full top row: full IP left-aligned, SRV:OK right-aligned when running.
//  Not connected: "WIFI:<state>"
static void _drawHeader() {
  display.setFont(u8g2_font_6x10_tf);
  String ip = wifiIpString();
  if (ip.length() > 0) {
    display.drawStr(0, 2, ip.c_str());
  } else {
    char s[16];
    snprintf(s, sizeof(s), "WIFI:%s", _wifiStatus.c_str());
    display.drawStr(0, 2, s);
  }
  display.drawHLine(0, 13, 128);
}

// ── Cell helper ──────────────────────────────────────────────────────────────
//  Draws a 64×15 cell at (x, y). No border. Text vertically centered.
//  If 'on': white fill inset 2px each side + 1px top/bottom, black text.
static void _drawCell(int x, int y, const char* label, bool on) {
  const int cw = 64, ch = 15;
  if (on) {
    display.setDrawColor(1);
    display.drawBox(x + 2, y + 1, cw - 4, ch - 2);
    display.setDrawColor(0);
  }
  int tw = (int)strlen(label) * 6;
  display.drawStr(x + (cw - tw) / 2, y + 3, label);  // y+3 centers 10px font in 15px cell
  if (on) display.setDrawColor(1);
}

// ── Screen 1: live calibration view ────────────────────────────────────────
static void _drawCalScreen() {
  display.clearBuffer();

  // Title
  display.setFont(u8g2_font_6x13B_tf);
  display.drawStr(0, 0, "CALIBRATING");
  display.drawHLine(0, 15, 128);

  // Content grid: pot value, min/max range, sweep toggle
  display.setFont(u8g2_font_6x10_tf);
  char buf[28];

  snprintf(buf, sizeof(buf), "Pot:  %4d", analogRead(POT_PIN));
  display.drawStr(0, 20, buf);

  snprintf(buf, sizeof(buf), "Min: %4d  Max: %4d", getPotMin(), getPotMax());
  display.drawStr(0, 32, buf);

  snprintf(buf, sizeof(buf), "Sweep: %s", isAutoSweep() ? "ON " : "OFF");
  display.drawStr(0, 44, buf);

  // Footer with command hints
  display.drawHLine(0, 50, 128);
  display.drawStr(0, 54, "L=min R=max A=auto S=save");

  display.sendBuffer();
}

// ── Screen 2: no calibration saved ───────────────────────────────────────
static void _drawNeedsCalScreen() {
  display.clearBuffer();
  _drawHeader();

  // Large text centered (use 6x13B for impact)
  display.setFont(u8g2_font_6x13B_tf);
  display.drawStr(0, 24, "NEEDS CALIBRATION");

  // Instruction (6x10)
  display.setFont(u8g2_font_6x10_tf);
  display.drawStr(0, 42, "Type C in serial");

  display.sendBuffer();
}

// ── Screen 3: dashboard ───────────────────────────────────────────────────
//  Header: y=0..13  (14px, HLine at 13)
//  Row 1:  y=14..28 (15px)  [ Day/Dusk/Night ] [ XX% ]
//  Row 2:  y=29..43 (15px)  [   Motor   ] [   Relay   ]
//  Row 3:  y=44..58 (15px)  [   Shelly  ] [  v1.0.0   ]
//  Bar:    y=59..63  (5px)  full width
static void _drawDashboard() {
  const int pot   = analogRead(POT_PIN);
  const int pMin  = getPotMin();
  const int pMax  = getPotMax();
  const int range = pMax - pMin;
  int pct = (range > 0)
          ? constrain((int) map(pot, pMin, pMax, 0, 100), 0, 100)
          : 0;

  display.clearBuffer();
  _drawHeader();
  display.setFont(u8g2_font_6x10_tf);

  // Row 1: dim state name (highlighted when relay on) | live pot %
  const char* stateName = getRelayState() ? getDimStateName(getCurrentDimState()) : "---";
  char pctBuf[6];
  snprintf(pctBuf, sizeof(pctBuf), "%d%%", pct);
  _drawCell(0,  14, stateName, getRelayState());
  _drawCell(64, 14, pctBuf,    false);

  // Row 2: Motor | Relay
  _drawCell(0,  29, "Motor", getMotorDir() != STOPPED);
  _drawCell(64, 29, "Relay", getRelayState());

  // Row 3: Shelly online status | firmware version
  char vbuf[8];
  snprintf(vbuf, sizeof(vbuf), "v%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
  _drawCell(0,  44, "Shelly", shellyIsOnline());
  _drawCell(64, 44, vbuf,     false);

  // Bar: inset 2px each side, 5px tall
  display.drawFrame(2, 59, 124, 5);
  const int fillW = (int) map(pct, 0, 100, 0, 122);
  if (fillW > 0) display.drawBox(3, 60, fillW, 3);

  display.sendBuffer();
}

// ── Public refresh entry point ───────────────────────────────────────────
void updateDisplay() {
  static unsigned long _lastDraw = 0;
  if (millis() - _lastDraw < OLED_REFRESH_MS) return;
  _lastDraw = millis();

  if (isCalibrating())   _drawCalScreen();
  else if (!calibDone()) _drawNeedsCalScreen();
  else                   _drawDashboard();
}
