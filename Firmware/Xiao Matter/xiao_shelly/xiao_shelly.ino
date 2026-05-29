// =============================================================================
// xiao_shelly.ino — Main Sketch
// Project : XIAO ESP32 + Shelly Dimmer Gen4 controller
// Hardware : Seeed Studio XIAO ESP32
//            AZ-Delivery 1.3" I2C OLED (SH1106) — SDA=Pin4, SCL=Pin8
//            Shelly Dimmer Gen4 (HTTP/RPC over Wi-Fi)
// IDE      : Arduino IDE
// =============================================================================
// TAB OVERVIEW
//   xiao_shelly.ino  — (this file) globals, setup(), loop()
//   wifi_manager.ino — Wi-Fi connection and status helpers
//   oled.ino         — OLED display init and all display functions
//   shelly.ino       — Shelly Dimmer HTTP/RPC communication
// =============================================================================

#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <U8g2lib.h>

// -----------------------------------------------------------------------------
// Wi-Fi credentials — fill in your network details
// -----------------------------------------------------------------------------
const char* WIFI_SSID     = "House of North";
const char* WIFI_PASSWORD = "p3rman3ntr3cord";

// -----------------------------------------------------------------------------
// Shelly Dimmer Gen4 — set the IP address assigned by your router
// -----------------------------------------------------------------------------
const char* SHELLY_IP = "192.168.1.200";   // <-- update this

// -----------------------------------------------------------------------------
// OLED — I2C pins for XIAO ESP32
// -----------------------------------------------------------------------------
const int OLED_SDA_PIN = 4;
const int OLED_SCL_PIN = 8;

// U8g2 display object using hardware I2C.
// HW_I2C uses the ESP32 I2C peripheral which is interrupt-safe — required
// because the WiFi stack fires interrupts that corrupt software I2C timing.
// SH1106 driver, 128x64, full frame buffer (F), hardware I2C (HW_I2C).
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(
  U8G2_R0,        // rotation: none
  U8X8_PIN_NONE,  // no reset pin
  OLED_SCL_PIN,   // clock (SCL) — overrides Wire default
  OLED_SDA_PIN    // data  (SDA) — overrides Wire default
);

// -----------------------------------------------------------------------------
// Shared state — updated by shelly.ino, read by oled.ino
// -----------------------------------------------------------------------------
bool   shellyOnline     = false;  // true once first successful contact made
bool   lightOn          = false;  // current on/off state of the Shelly light
int    brightness       = 0;      // current brightness 0–100
String lastStatus       = "Init"; // last action result ("OK", "Error", etc.)

// -----------------------------------------------------------------------------
// setup() — runs once at power-on
// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  initDisplay();           // oled.ino  — start display, show splash
  initWifi();              // wifi_manager.ino — connect to Wi-Fi
  showWifiStatus();        // oled.ino  — confirm connection on screen
  initShelly();            // shelly.ino — fetch initial Shelly state
  showDashboard();         // oled.ino  — show main dashboard
}

// -----------------------------------------------------------------------------
// loop() — runs repeatedly; only function calls here
// -----------------------------------------------------------------------------
void loop() {
  checkWifiConnection();    // wifi_manager.ino — reconnect if dropped
  oscillateBrightness();    // shelly.ino       — triangle-wave brightness 0→100→0 / 3 s
  showDashboard();          // oled.ino         — refresh display
  // No delay here — oscillateBrightness() manages its own non-blocking timing
}
