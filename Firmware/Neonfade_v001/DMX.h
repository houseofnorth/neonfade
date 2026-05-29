#pragma once
#include <Wire.h>

class Motor;
class Calibration;

// ── DMXReceiver ───────────────────────────────────────────────────────────
// IS3710 I2C DMX512 receiver driver.
//
// Hardware chain:
//   XLR connector → TVS → RS485 transceiver (e.g. THVD1400DR)
//                → IS3710 RX pin → IS3710 I2C → XIAO ESP32C3
//
// I2C address : 0x10 (fixed)
// I2C pins    : SDA = D4, SCL = D5  (XIAO ESP32C3 defaults)
// I2CSPD      : tie to GND for 100 kHz
//
// NeonFade 5-channel footprint (base address = DMX_ADDRESS in config.h):
//   CH+0  Mode     : 0-127 = position,  128-255 = oscillate
//   CH+1  Position : 0-255 → mapped to calMin-calMax
//   CH+2  Speed    : 0-255 → PWM duty  (direct 0-255)
//   CH+3  Osc In   : 0-255 → mapped to calMin-calMax
//   CH+4  Osc Out  : 0-255 → mapped to calMin-calMax
//
// Priority: DMX and web both write to the same Motor setters.
// When DMX signal is present it updates every DMX_POLL_MS ms,
// effectively overriding web commands at that rate.
// ──────────────────────────────────────────────────────────────────────────
class DMXReceiver {
public:
  void init(Motor* motor, Calibration* cal);
  void poll();

  bool detected() const { return _detected; }

private:
  // Identical to the official IS3710 example sketch
  uint8_t readIS3710Register(uint16_t holdingRegisterAddress);

  Motor*       _motor    = nullptr;
  Calibration* _cal      = nullptr;
  bool         _detected = false;
  bool         _wasOsc   = false;   // tracks mode transitions to avoid direction reset
};
