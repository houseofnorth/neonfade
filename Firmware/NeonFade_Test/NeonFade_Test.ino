// ── NeonFade_Test ─────────────────────────────────────────────────────────
//  Target : Seeed XIAO ESP32-C3
//  Board  : Neon Fader BRD mkIII v367
//
//  Hardware bring-up firmware. Behaviour:
//    - Relay (D6) toggles every 1 s from boot — no calibration required.
//    - LED_A (D7) fast-blinks during pot-sweep calibration window.
//    - On boot, manually sweep the motorpot end-to-end within 15 s. If no
//      sweep is detected the firmware falls back to CALIB_DEFAULT_MIN/MAX.
//    - Once calibrated, motor oscillates between min/max via L293DD.
//    - LED_A mirrors relay (1 Hz); LED_B mirrors motor direction.
//    - Serial @ 115200 prints a column report every 250 ms.
//
//  Implementation lives in the sibling .ino tabs:
//    motor.ino · pot.ino · relay.ino · leds.ino · report.ino
//
//  See TEST_INSTRUCTIONS.txt for the test-point checklist.
// ──────────────────────────────────────────────────────────────────────────

#include "config.h"

void setup() {
  Serial.begin(BAUD_RATE);
  // Brief grace period for the USB-CDC link to enumerate before first prints.
  unsigned long t0 = millis();
  while (!Serial && (millis() - t0) < 1500) { /* wait */ }

  Serial.println();
  Serial.println("=== NeonFade Test Firmware ===");
  Serial.println("Sweep pot full range to calibrate (15 s timeout).");

  initLeds();    // LEDs first so calibration blinker is live immediately
  initPot();     // starts the 15 s calibration window
  initMotor();   // PWM attached, outputs at 0 until calib completes
  initRelay();   // 1 Hz toggle begins on first updateRelay() call
}

void loop() {
  updatePot();     // tracks min/max, latches calibration
  updateRelay();   // 1 Hz toggle, independent of calibration
  updateMotor();   // oscillates between calibrated endpoints once ready
  updateLeds();    // calibration blink / relay mirror / direction indicator
  reportSerial();  // status columns every 250 ms
}
