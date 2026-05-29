#pragma once
#include <Arduino.h>

// ── Pins ──────────────────────────────────────────────────────────────────
const int MOTOR_IN1 = D3;
const int MOTOR_IN2 = D2;
const int RELAY_PIN = D6;
const int LED_A     = D7;   // matches production config
const int LED_B     = D8;
const int POT_PIN   = A0;

// ── Motor PWM ─────────────────────────────────────────────────────────────
const int PWM_FREQ        = 5000;  // Hz — matches production
const int PWM_BITS        = 8;     // duty 0-255
const int MOTOR_SPEED     = 200;   // test speed, dial back if needed
const int MOTOR_DEADBAND  = 30;    // ADC counts — at target within this

// ── Pot calibration ───────────────────────────────────────────────────────
const int           CALIB_THRESHOLD  = 2000;  // min range for a valid sweep
const unsigned long CALIB_TIMEOUT_MS = 15000; // use defaults if not swept
const int           CALIB_DEFAULT_MIN = 150;
const int           CALIB_DEFAULT_MAX = 3900;

// ── Relay ─────────────────────────────────────────────────────────────────
const unsigned long RELAY_INTERVAL_MS = 5000; // toggle period

// ── Serial ────────────────────────────────────────────────────────────────
const unsigned long REPORT_INTERVAL_MS = 250;
const int           BAUD_RATE          = 115200;

// ── Motor direction — defined here so all tabs see it before motor.ino ────
enum MotorDir { STOPPED, FORWARD, REVERSE };
