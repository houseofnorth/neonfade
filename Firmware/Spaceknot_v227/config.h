#pragma once
#include <Arduino.h>

// ── Firmware Version ──────────────────────────────────────────────────────
const int VERSION_MAJOR = 2;
const int VERSION_MINOR = 0;
const int VERSION_PATCH = 27;

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
const int MOTOR_DEADBAND         = 80;    // ADC counts — stop within this of target
const int MANUAL_TURN_THRESHOLD  = 150;   // ADC counts — displacement while settled
                                           // that's treated as a manual pot turn

// ── Pot / range calibration ───────────────────────────────────────────────
//  Persisted in NVS (Preferences) under PREFS_NAMESPACE.
//  Serial commands: C=enter cal  L=min  R=max  A=auto-sweep  S=save
const char* const PREFS_NAMESPACE = "spaceknot"; // ≤15 chars (NVS limit)
const char* const PREFS_KEY_MIN   = "potMin";
const char* const PREFS_KEY_MAX   = "potMax";
const char* const PREFS_KEY_VALID = "calOK";
const int         CALIB_MIN_RANGE = 500;   // refuse to save tiny ranges

// ── Pot defaults (used when no calibration has been saved) ───────────────
//  Full ALPS RK168 travel is ~40–4075. Units ship with these so the motor
//  can run from first boot; calibrate on location to fine-tune.
const int POT_DEFAULT_MIN   = 40;
const int POT_DEFAULT_MAX   = 4075;
const int POT_DEFAULT_DAY   = 4075;
const int POT_DEFAULT_DUSK  = 2057;   // midpoint
const int POT_DEFAULT_NIGHT = 40;

// ── Dim state positions (NVS keys) ────────────────────────────────────────
//  Three named pot targets (Day / Dusk / Night), each stored as a raw ADC value.
//  Captured via the web UI; saved immediately on capture.
//  lastState remembers which state was active so relay-ON resumes it.
const char* const PREFS_KEY_DAY        = "dayPos";
const char* const PREFS_KEY_DUSK       = "duskPos";
const char* const PREFS_KEY_NIGHT      = "nightPos";
const char* const PREFS_KEY_LAST_STATE = "lastState";


// ── Debug level ───────────────────────────────────────────────────────────
//  1 = operational (boot, connect, state changes) — default
//  2 = verbose (MQTT payloads, cal values, WiFi detail)
//  Change here and reflash to switch modes.
#define DEBUG_LEVEL 1

#if DEBUG_LEVEL >= 1
  #define LOG1(...)    Serial.printf(__VA_ARGS__)
  #define LOGLN1(s)    Serial.println(s)
#else
  #define LOG1(...)    do {} while(0)
  #define LOGLN1(s)    do {} while(0)
#endif

#if DEBUG_LEVEL >= 2
  #define LOG2(...)    Serial.printf(__VA_ARGS__)
  #define LOGLN2(s)    Serial.println(s)
#else
  #define LOG2(...)    do {} while(0)
  #define LOGLN2(s)    do {} while(0)
#endif

// ── Serial ────────────────────────────────────────────────────────────────
const unsigned long REPORT_INTERVAL_MS = 1000;
const int           BAUD_RATE          = 115200;

// ── OLED ──────────────────────────────────────────────────────────────────
//  AZ-Delivery 1.3" SH1106, 128×64, I2C 0x3C.
//  Full-buffer SH1106 redraw takes ~20–30 ms; 200 ms keeps the loop responsive.
const unsigned long OLED_REFRESH_MS = 200;
const unsigned long OLED_SPLASH_MS  = 1200;

// ── WiFi ──────────────────────────────────────────────────────────────────
enum WifiModeChoice { SK_WIFI_STA = 0, SK_WIFI_AP = 1, SK_WIFI_DUAL = 2 };
const WifiModeChoice WIFI_MODE_CHOICE = SK_WIFI_DUAL;

// Home-network creds (STA + DUAL).
#define WIFI_SSID     "spaceknot"
#define WIFI_PASSWORD "spaceknot2026"

// AP-mode credentials (AP + DUAL-fallback).
#define WIFI_AP_SSID     "Spaceknot"
#define WIFI_AP_PASSWORD "spaceknot"

const unsigned long WIFI_CONNECT_TIMEOUT_MS = 15000;
const unsigned long WIFI_RETRY_INTERVAL_MS  = 15000;

// ── Web interface ─────────────────────────────────────────────────────────
const uint16_t      WEB_PORT           = 80;
const unsigned long WEB_STATUS_POLL_MS = 500;

// ── Shelly Dimmer Gen4 ────────────────────────────────────────────────────
//  HTTP RPC: GET /rpc/Light.Set?id=0&on=true&brightness=NN
//  Update SHELLY_IP to match the device's address on your network
//  (check the Shelly Smart Control app → Device Information).
// Dedicated Spaceknot subnet (GL.iNet router default — change if you reconfigure)
#define SK_SUBNET        "192.168.2"

#define SHELLY_IP_1      SK_SUBNET ".101"
#define SHELLY_IP_2      SK_SUBNET ".102"
const unsigned long SHELLY_HTTP_TIMEOUT_MS      = 1500;
const unsigned long SHELLY_PROBE_INTERVAL_MS    = 60000;
const unsigned long SHELLY_SEND_MIN_INTERVAL_MS = 200;
const int           SHELLY_MIN_DELTA            = 2;
const uint32_t      SHELLY_TASK_STACK           = 8192;
const UBaseType_t   SHELLY_TASK_PRIO            = 1;


// ── MQTT ──────────────────────────────────────────────────────────────────
//  Broker runs on the dedicated router (GL.iNet default: 192.168.8.1).
//  All units publish/subscribe to MQTT_TOPIC_STATE so they stay in sync.
//  Unit identity comes from the device name stored in settings NVS.
#define MQTT_BROKER_IP          SK_SUBNET ".1"
const uint16_t      MQTT_PORT             = 1883;
const char* const   MQTT_TOPIC_STATE      = "spaceknot/state";
const char* const   MQTT_TOPIC_CAL        = "spaceknot/cal";
// Shelly Gen4 publishes button events to <device-id>/events/rpc.
// The wildcard subscription catches any Shelly Dimmer on the network
// without needing to know MAC addresses.
#define MQTT_TOPIC_SHELLY_EVENTS  "+/events/rpc"





const unsigned long MQTT_RECONNECT_MS     = 5000;  // retry interval

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
