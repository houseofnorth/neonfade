#pragma once
#include <Arduino.h>

// ── Firmware Version ──────────────────────────────────────────────────────
const int VERSION_MAJOR = 1;
const int VERSION_MINOR = 2;
const int VERSION_PATCH = 4;

// ── Pins ──────────────────────────────────────────────────────────────────
const int MOTOR_IN1 = D3;
const int MOTOR_IN2 = D2;
const int RELAY_PIN = D6;
const int LED_A     = D7;
const int LED_B     = D8;
const int POT_PIN   = A0;

// ── Motor PWM ─────────────────────────────────────────────────────────────
const int PWM_FREQ               = 5000;  // Hz
const int PWM_BITS               = 8;     // duty 0–255
const int MOTOR_SPEED_MAX        = 255;
const int MOTOR_DEADBAND         = 40;    // ADC counts — stop within this of target
const int MANUAL_TURN_THRESHOLD  = 150;   // ADC counts — displacement while settled
                                           // that's treated as a manual pot turn

// ── Pot / range calibration ───────────────────────────────────────────────
//  Persisted in NVS (Preferences) under PREFS_NAMESPACE.
//  Serial commands: C=enter cal  L=min  R=max  A=auto-sweep  S=save
const char* const PREFS_NAMESPACE = "neonfade"; // ≤15 chars (NVS limit)
const char* const PREFS_KEY_MIN   = "potMin";
const char* const PREFS_KEY_MAX   = "potMax";
const char* const PREFS_KEY_VALID = "calOK";
const int         CALIB_MIN_RANGE = 500;   // refuse to save tiny ranges

// ── Dim state positions (NVS keys) ────────────────────────────────────────
//  Three named pot targets (Day / Dusk / Night), each stored as a raw ADC value.
//  Captured via the web UI; saved immediately on capture.
//  lastState remembers which state was active so relay-ON resumes it.
const char* const PREFS_KEY_DAY        = "dayPos";
const char* const PREFS_KEY_DUSK       = "duskPos";
const char* const PREFS_KEY_NIGHT      = "nightPos";
const char* const PREFS_KEY_LAST_STATE = "lastState";

// ── Serial ────────────────────────────────────────────────────────────────
const unsigned long REPORT_INTERVAL_MS = 1000;
const int           BAUD_RATE          = 115200;

// ── OLED ──────────────────────────────────────────────────────────────────
//  AZ-Delivery 1.3" SH1106, 128×64, I2C 0x3C.
//  Full-buffer SH1106 redraw takes ~20–30 ms; 200 ms keeps the loop responsive.
const unsigned long OLED_REFRESH_MS = 200;
const unsigned long OLED_SPLASH_MS  = 1200;

// ── WiFi ──────────────────────────────────────────────────────────────────
enum WifiModeChoice { NF_WIFI_STA = 0, NF_WIFI_AP = 1, NF_WIFI_DUAL = 2 };
const WifiModeChoice WIFI_MODE_CHOICE = NF_WIFI_DUAL;

// Home-network creds (STA + DUAL).
#define WIFI_SSID     "spaceknot"
#define WIFI_PASSWORD "spaceknot2026"

// AP-mode credentials (AP + DUAL-fallback).
#define WIFI_AP_SSID     "NeonFade"
#define WIFI_AP_PASSWORD "neonfade"

const unsigned long WIFI_CONNECT_TIMEOUT_MS = 15000;
const unsigned long WIFI_RETRY_INTERVAL_MS  = 15000;

// ── Web interface ─────────────────────────────────────────────────────────
const uint16_t      WEB_PORT           = 80;
const unsigned long WEB_STATUS_POLL_MS = 500;

// ── Shelly Dimmer Gen4 ────────────────────────────────────────────────────
//  HTTP RPC: GET /rpc/Light.Set?id=0&on=true&brightness=NN
//  Update SHELLY_IP to match the device's address on your network
//  (check the Shelly Smart Control app → Device Information).
#define SHELLY_IP "192.168.1.100"
const unsigned long SHELLY_HTTP_TIMEOUT_MS      = 1500;
const unsigned long SHELLY_PROBE_INTERVAL_MS    = 60000;
const unsigned long SHELLY_SEND_MIN_INTERVAL_MS = 200;
const int           SHELLY_MIN_DELTA            = 2;
const uint32_t      SHELLY_TASK_STACK           = 8192;
const UBaseType_t   SHELLY_TASK_PRIO            = 1;

// ── Motor direction ────────────────────────────────────────────────────────
enum MotorDir { STOPPED, FORWARD, REVERSE };

// ── WiFi state ────────────────────────────────────────────────────────────
enum WifiState {
  WF_AP,
  WF_STA_TRYING,
  WF_STA_UP,
  WF_STA_BACKOFF
};

// ── Dim states (Shelly button) ─────────────────────────────────────────────
//  Day / Dusk / Night — each maps to a saved pot ADC target set via the web UI.
//  DIM_NONE is an internal sentinel (never a valid user state).
enum DimState { DIM_DAY = 0, DIM_DUSK = 1, DIM_NIGHT = 2, DIM_NONE = 3 };
