#include "Calibration.h"
#include "Leds.h"
#include "config.h"
#include <Preferences.h>

void Calibration::init(Leds* leds) {
  _leds = leds;
}

void Calibration::load() {
  Preferences prefs;
  prefs.begin("neonfade", true);
  _valid = prefs.getBool("calValid", false);
  _min   = prefs.getInt("calMin",   80);
  _max   = prefs.getInt("calMax",   3900);
  prefs.end();

  if (_valid)
    Serial.printf("[cal] loaded  min=%d  max=%d\n", _min, _max);
  else
    Serial.println("[cal] NOT CALIBRATED — defaults in use");
}

void Calibration::save() {
  Preferences prefs;
  prefs.begin("neonfade", false);
  prefs.putBool("calValid", true);
  prefs.putInt("calMin",    _min);
  prefs.putInt("calMax",    _max);
  prefs.end();
  _valid = true;
  Serial.printf("[cal] saved   min=%d  max=%d\n", _min, _max);
}

int Calibration::stableReading() {
  long sum = 0;
  for (int i = 0; i < 8; i++) { sum += analogRead(POT_PIN); delay(5); }
  return (int)(sum / 8);
}

void Calibration::saveLeft() {
  _min = stableReading();
  save();
  Serial.println("[cal] ──────────────────────────────");
  Serial.printf( "[cal] ✓  LEFT  limit saved  →  %d\n", _min);
  if (_min >= _max)
    Serial.println("[cal] ⚠  WARNING: left limit is not below right — redo calibration");
  Serial.println("[cal] ──────────────────────────────");
  if (_leds) _leds->blinkOne(LED_A, 3);
}

void Calibration::saveRight() {
  _max = stableReading();
  save();
  Serial.println("[cal] ──────────────────────────────");
  Serial.printf( "[cal] ✓  RIGHT limit saved  →  %d\n", _max);
  if (_max <= _min)
    Serial.println("[cal] ⚠  WARNING: right limit is not above left — redo calibration");
  Serial.println("[cal] ──────────────────────────────");
  if (_leds) _leds->blinkOne(LED_B, 3);
}
