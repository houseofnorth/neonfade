// =====================================================================
//  NeonFade v004  —  XIAO ESP32C3
//
//  Multi-subsystem refactor — each concern in its own class / file:
//    config.h          pin definitions, tuning constants
//    Motor.h/.cpp      LEDC PWM, position + oscillation logic
//    Leds.h/.cpp       LED state, blink feedback
//    Calibration.h/.cpp Preferences flash storage
//    Button.h/.cpp     single / double click detection
//    WebInterface.h/.cpp WiFi AP + HTTP control page
//    DMX.h/.cpp        IS3710 I2C DMX512 receiver (5-ch footprint)
//    Matter.h/.cpp     Phase 2 stub — Thread / Matter
//
//  CALIBRATION
//    single click B  →  save left  (MIN) limit  →  LED_A blinks 3×
//    double click B  →  save right (MAX) limit  →  LED_B blinks 3×
//
//  WEB CONTROL  —  http://192.168.4.1  (join WiFi "NeonFade" / "neonfade")
//    Position tab   : slider sets motor target
//    Oscillate tab  : IN / OUT sliders + START/STOP
//    Speed slider   : PWM duty (both modes)
//
//  DMX CONTROL  —  5-channel footprint, base = DMX_ADDRESS (config.h)
//    CH+0  Mode     : 0-127 = position,  128-255 = oscillate
//    CH+1  Position : 0-255
//    CH+2  Speed    : 0-255
//    CH+3  Osc In   : 0-255
//    CH+4  Osc Out  : 0-255
//
//  MATTER  —  Phase 2, not yet implemented
//
//  Board:  Seeed Studio XIAO ESP32C3
//  BOOT button is GPIO9 on ESP32C3 (not GPIO0 as on ESP32S3).
//  Requires ESP32 Arduino core 3.x for ledcAttach() API.
// =====================================================================

#include "config.h"
#include "Motor.h"
#include "Leds.h"
#include "Calibration.h"
#include "Button.h"
#include "WebInterface.h"
#include "DMX.h"
#include "Matter.h"


// ── Objects ───────────────────────────────────────────────────────────────

Motor           motor;
Leds            leds;
Calibration     cal;
Button          btn;
WebInterface    web;
DMXReceiver     dmx;
MatterInterface matter;


// ── Status print — serial monitor, every 500 ms ───────────────────────────

void printStatus() {
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint < 500) return;
  lastPrint = millis();

  int pot = analogRead(POT_PIN);
  int pct = constrain((int)map(pot, cal.minVal(), cal.maxVal(), 0, 100), 0, 100);
  const char* calTag = cal.isValid() ? "[CAL  ]" : "[UNCAL]";

  if (motor.isOscillating()) {
    int target = motor.oscGoingOut() ? motor.oscOut() : motor.oscIn();
    Serial.printf("%s  pot %4d (%3d%%)  osc→%s  target %4d  speed %3d%%",
      calTag, pot, pct,
      motor.oscGoingOut() ? "OUT" : "IN ",
      target,
      (int)map(motor.speed(), 0, 255, 0, 100));
  } else {
    int tgt   = motor.targetPos();
    int error = (tgt >= 0) ? (tgt - pot) : 0;
    const char* motorTag = "IDLE";
    if      (tgt >= 0 && error >  MOTOR_DEADBAND) motorTag = "FORWARD";
    else if (tgt >= 0 && error < -MOTOR_DEADBAND) motorTag = "REVERSE";
    else if (tgt >= 0)                            motorTag = "AT TARGET";
    Serial.printf("%s  pot %4d (%3d%%)  target %4d  error %+5d  %-9s  speed %3d%%",
      calTag, pot, pct, tgt, error, motorTag,
      (int)map(motor.speed(), 0, 255, 0, 100));
  }

  if (dmx.detected())    Serial.print("  [DMX]");
  if (matter.active())   Serial.print("  [MATTER]");
  Serial.println();
}


// ── Arduino entry points ──────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  delay(500);

  motor.init();
  leds.init();
  cal.init(&leds);
  cal.load();
  btn.init(&motor, &cal);
  web.init(&motor, &cal);
  dmx.init(&motor, &cal);
  matter.init(&motor, &cal);

  leds.blinkBoth(3);  // startup confirmation

  Serial.println("\nReady.");
  Serial.println("  single click B  →  set left  (MIN) limit");
  Serial.println("  double click B  →  set right (MAX) limit");
  Serial.println("  Join WiFi 'NeonFade' / 'neonfade'");
  Serial.println("  Open http://192.168.4.1");
}

void loop() {
  btn.update();
  motor.update();
  leds.update(motor.dir(), btn.held());
  web.handleClient();
  dmx.poll();
  matter.poll();
  printStatus();
}
