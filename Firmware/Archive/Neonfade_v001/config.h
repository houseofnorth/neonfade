#pragma once
#include <Arduino.h>

// ── Pins ──────────────────────────────────────────────────────────────────
const int MOTOR_IN1 = D2;
const int MOTOR_IN2 = D3;
const int POT_PIN   = A0;
const int LED_A     = D7;
const int LED_B     = D8;
const int BUTTON    = 9;    // BOOT button — GPIO9 on XIAO ESP32C3, active LOW

// ── Motor tuning ──────────────────────────────────────────────────────────
const int MOTOR_DEADBAND = 30;    // ADC counts — stop this close to target
const int PWM_FREQ       = 5000;  // Hz
const int PWM_BITS       = 8;     // resolution — duty 0-255

// ── Button timing ─────────────────────────────────────────────────────────
const unsigned long DOUBLE_CLICK_MS = 350;  // ms gap between clicks

// ── WiFi ──────────────────────────────────────────────────────────────────
#define WIFI_SSID "NeonFade"
#define WIFI_PASS "neonfade"

// ── IS3710 / DMX ──────────────────────────────────────────────────────────
// Constants match the official IS3710 example sketches
#define I2C_DEVICE_ADDRESS  16    // IS3710 fixed I2C address (0x10)
#define CHIP_ID_REG         513   // Identification register address
#define CHIP_ID_VALUE       16    // Expected value (0x10)

// NeonFade 5-channel DMX footprint — change DMX_ADDRESS to set start channel
//   CH+0  Mode     : 0-127 = position,  128-255 = oscillate
//   CH+1  Position : 0-255 → mapped to calMin-calMax
//   CH+2  Speed    : 0-255 → PWM duty (direct)
//   CH+3  Osc In   : 0-255 → mapped to calMin-calMax
//   CH+4  Osc Out  : 0-255 → mapped to calMin-calMax
const uint16_t DMX_ADDRESS = 1;    // First DMX channel of this fixture
const uint32_t DMX_POLL_MS = 100;  // IS3710 read interval (ms)
