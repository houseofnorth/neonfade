#pragma once
#include <Arduino.h>

// ── Firmware Version ───────────────────────────────────────────────────────
//  Increment this after each code update so you can tell if new code has been
//  uploaded. Format: v<major>.<minor>
#define FIRMWARE_VERSION "v1.7-debug404"

// ── Pins ───────────────────────────────────────────────────────────────────
const int LED_A     = D7;
const int LED_B     = D8;

// ── OLED ───────────────────────────────────────────────────────────────────
//  AZ-Delivery 1.3" SH1106, 128×64, I2C address 0x3C (U8g2 default).
const unsigned long OLED_REFRESH_MS = 200;
const unsigned long OLED_SPLASH_MS  = 1200;

// ── WiFi ───────────────────────────────────────────────────────────────────
enum NF_WifiMode { NF_STA = 0, NF_AP = 1, NF_DUAL = 2 };
const NF_WifiMode WIFI_MODE_CHOICE = NF_STA;  // STA only, connect to House of North

#define WIFI_SSID     "House of North"
#define WIFI_PASSWORD "p3rman3ntr3cord"

#define WIFI_AP_SSID     "ShellyButtonTest"
#define WIFI_AP_PASSWORD "test"

const unsigned long WIFI_CONNECT_TIMEOUT_MS = 15000;
const unsigned long WIFI_RETRY_INTERVAL_MS  = 15000;

// ── Shelly Dimmer Gen4 ────────────────────────────────────────────────────
//  Webhook-based event-driven input (no polling).
//  Configure Shelly settings to POST to: http://<xiao-ip>/webhook/button
//  when button state changes. Payload: {"output": true/false}
#define SHELLY_IP "192.168.1.44"
const uint16_t      WEBHOOK_PORT             = 80;

// ── Position Counter ───────────────────────────────────────────────────────
//  How many divisions to split the motor pot range into. Increment on button
//  press, wrap back to 1 after reaching max.
const int MAX_POSITIONS      = 10;    // number of positions in the range
const int INITIAL_POSITION   = 1;     // start at position 3

// ── Serial ─────────────────────────────────────────────────────────────────
const int BAUD_RATE = 115200;
