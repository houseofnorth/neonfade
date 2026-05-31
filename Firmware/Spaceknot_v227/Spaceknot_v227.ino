// ── Spaceknot v2 ───────────────────────────────────────────────────────────
//  Target : Seeed XIAO ESP32-C3
//  Board  : Neon Fader V2 v376 (in fab — also flashes onto v1 proto with
//           OLED bodge-wired to D4/D5).
//
//  Building block for the production firmware. Starts from the bring-up
//  behaviour of Spaceknot_Test and layers on:
//    - SH1106 1.3" I2C OLED on the XIAO's default hardware I2C pair
//      (D4=SDA, D5=SCL — matches J2 on the v2 schematic).
//    - Status dashboard: WiFi line, pot/motor/relay state, brightness bar
//      driven by the calibrated pot position.
//
//  Step 1 of two:
//    1. OLED integration (this commit).
//    2. WiFi + Shelly Dimmer Gen4 control (ported from xiao_shelly).
//
//  Tabs:
//    motor · pot · relay · leds · report · oled · serial
//    wifi_manager · shelly · web_interface
// ──────────────────────────────────────────────────────────────────────────

#include "config.h"

void setup() {
  Serial.begin(BAUD_RATE);
  // Brief grace period for the USB-CDC link to enumerate before first prints.
  unsigned long t0 = millis();
  while (!Serial && (millis() - t0) < 1500) { /* wait */ }

  Serial.println();
  // Header printed after full init (see reportPrintHeader at end of setup)

  initSettings();   // load name/wifi/IP from NVS before anything else needs them
  promptUnitIdIfNeeded(); // blocks + reboots if unit number not yet assigned
  initDisplay();    // OLED first so splash is visible while everything else inits
  initLeds();       // status LEDs live immediately
  initPot();        // loads calibration from NVS (if any) — motor disabled if none
  initMotor();      // PWM attached, outputs at 0 until cal is loaded
  initRelay();      // 1 Hz toggle begins on first updateRelay() call
  initWebhook();    // start webhook server for Shelly button input
  initWifi();           // kicks off the first connection attempt (non-blocking)
  initShelly();         // spawns the HTTP task; idles until WiFi + Shelly come up
  initWebInterface();   // registers routes; server starts once WiFi is up
  initOta();            // ArduinoOTA + web OTA endpoints
  initMqtt();           // connect to broker (retries in updateMqtt if not up yet)
  initSerialCmd();      // prints the command hint

  if (!calibDone()) {
    LOG1("Motor disabled — type C to calibrate\n");
  }
}

void loop() {
  static bool _headerPrinted = false;
  if (!_headerPrinted) { _headerPrinted = true; reportPrintHeader(); }
  updateSerial();   // process single-char commands (C/L/R/A/S)
  updatePot();      // widens captured min/max while auto-sweep is on
  updateRelay();    // 1 Hz toggle, independent of calibration
  updateMotor();    // move to discrete position based on counter
  updateLeds();     // blink fast (cal) / slow (needs cal) / mirror (running)
  updateDisplay();  // routes to cal / needs-cal / dashboard screen
  updateWifi();         // non-blocking WiFi state machine + OLED status
  updateOta();          // ArduinoOTA handler
  updateMqtt();         // maintain broker connection, dispatch incoming messages
  updateShelly();       // just updates the shared brightness target (cheap)
  updateWebInterface(); // handles web requests including /webhook/button
  reportSerial();       // status columns every 250 ms
}
