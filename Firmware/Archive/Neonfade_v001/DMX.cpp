#include "DMX.h"
#include "Motor.h"
#include "Calibration.h"
#include "config.h"

// ── IS3710 register read ──────────────────────────────────────────────────
// Identical to the official IS3710 example sketch — do not modify this
// function without also updating the example for consistency.
// -----------------------------------------------------------------------------
// The IS3710 uses a 16-bit register address over I2C (MSB first).
// DMX channels are at addresses 1-512.  CHIP_ID is at address 513.
// -----------------------------------------------------------------------------
uint8_t DMXReceiver::readIS3710Register(uint16_t holdingRegisterAddress) {
  uint8_t result;

  Wire.beginTransmission(I2C_DEVICE_ADDRESS);
  Wire.write((holdingRegisterAddress >> 8) & 0xFF);  // address MSB
  Wire.write( holdingRegisterAddress       & 0xFF);  // address LSB
  Wire.endTransmission(false);                        // repeated START

  Wire.requestFrom((uint8_t)I2C_DEVICE_ADDRESS, (uint8_t)1);
  result = Wire.read();

  return result;
}

// ── Init ─────────────────────────────────────────────────────────────────

void DMXReceiver::init(Motor* motor, Calibration* cal) {
  _motor = motor;
  _cal   = cal;

  Wire.begin();  // SDA = D4 (GPIO6), SCL = D5 (GPIO7) on XIAO ESP32C3

  uint8_t id = readIS3710Register(CHIP_ID_REG);

  if (id == CHIP_ID_VALUE) {
    _detected = true;
    Serial.printf("[dmx] IS3710 detected  CHIP_ID=0x%02X  base ch=%d\n", id, DMX_ADDRESS);
  } else {
    _detected = false;
    Serial.printf("[dmx] IS3710 NOT detected  (got 0x%02X)  DMX disabled\n", id);
  }
}

// ── Poll — called every loop() ───────────────────────────────────────────

void DMXReceiver::poll() {
  if (!_detected) return;

  static uint32_t lastPoll = 0;
  if (millis() - lastPoll < DMX_POLL_MS) return;
  lastPoll = millis();

  // Read 5 consecutive channels starting at DMX_ADDRESS
  uint8_t chMode   = readIS3710Register(DMX_ADDRESS + 0);
  uint8_t chPos    = readIS3710Register(DMX_ADDRESS + 1);
  uint8_t chSpeed  = readIS3710Register(DMX_ADDRESS + 2);
  uint8_t chOscIn  = readIS3710Register(DMX_ADDRESS + 3);
  uint8_t chOscOut = readIS3710Register(DMX_ADDRESS + 4);

  // Speed applies in both modes — 0-255 direct to PWM duty
  _motor->setSpeed(chSpeed);

  bool wantOsc = (chMode >= 128);

  if (!wantOsc) {
    // ── Position mode ────────────────────────────────────────────────
    if (_wasOsc) _wasOsc = false;
    int adc = (int)map(chPos, 0, 255, _cal->minVal(), _cal->maxVal());
    _motor->setTarget(adc);
  } else {
    // ── Oscillate mode ───────────────────────────────────────────────
    int inAdc  = (int)map(chOscIn,  0, 255, _cal->minVal(), _cal->maxVal());
    int outAdc = (int)map(chOscOut, 0, 255, _cal->minVal(), _cal->maxVal());

    if (!_wasOsc) {
      // Mode just changed to oscillate — full reset including direction
      _motor->startOscillate(inAdc, outAdc);
      _wasOsc = true;
    } else {
      // Already oscillating — update endpoints without resetting direction
      _motor->setOscPoints(inAdc, outAdc);
    }
  }
}
