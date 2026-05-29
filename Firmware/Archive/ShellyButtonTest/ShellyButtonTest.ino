// ── ShellyButtonTest ───────────────────────────────────────────────────────
//  Minimal test firmware: read Shelly Dimmer Gen4 button state and display
//  on OLED.
//
//  MCU: Seeed XIAO ESP32-C3
//  OLED: AZ-Delivery 1.3" SH1106 on I2C (D4=SDA, D5=SCL)
//  Shelly: Dimmer Gen4 on home WiFi
//
//  Tabs:
//    wifi_manager · oled · shelly_button
// ──────────────────────────────────────────────────────────────────────────

#include "config.h"

void setup() {
  Serial.begin(BAUD_RATE);
  // Brief grace period for USB-CDC enumeration
  unsigned long t0 = millis();
  while (!Serial && (millis() - t0) < 1500) { /* wait */ }

  Serial.println();
  Serial.println("=== Shelly Button Test " FIRMWARE_VERSION " ===");
  Serial.println("Using webhooks (event-driven, not polling)");

  initDisplay();           // OLED splash
  initWifi();              // WiFi connection attempt
  initWebhook();           // Start webhook server
}

void loop() {
  updateWifi();            // WiFi state machine
  updateWebhook();         // Handle incoming webhook requests
  updateDisplay();         // Refresh OLED with position
  delay(10);
}
