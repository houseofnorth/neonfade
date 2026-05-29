# Xiao ESP32 + Shelly Dimmer Gen4 — Project Notes

## Overview

Personal project to set up communication between a **Seeed Studio XIAO ESP32** and a **Shelly Dimmer Gen4**, programmed via the **Arduino IDE**. The goal is for the ESP32 to control light brightness through the Shelly Dimmer.

## Goals

- Establish reliable communication between the XIAO ESP32 and the Shelly Dimmer Gen4
- Send dimmer control commands from the ESP32 (e.g. on/off, brightness level)
- Develop and flash firmware using Arduino IDE
- Decide on and implement a communication protocol (see options below)

## Communication Protocol — Options to Explore

| Protocol | Notes |
|---|---|
| **MQTT** | Lightweight, great for IoT. Shelly has native MQTT support. Requires a broker (e.g. Mosquitto). |
| **HTTP / REST API** | Shelly has a built-in web API. Simple to test with GET/POST requests from ESP32. |
| **WebSocket** | Real-time, bidirectional. Shelly Gen4 supports RPC over WebSocket. |

> **Decision pending** — start with HTTP for simplicity, then evaluate MQTT for reliability.

## Hardware

- Seeed Studio XIAO ESP32 (variant: _fill in — S3, C3, etc._)
- Shelly Dimmer Gen4
- **AZ-Delivery 1.3" I2C OLED display** (SH1106 driver, 128×64px)
  - SDA → Pin 4
  - SCL → Pin 8
  - I2C address: `0x3C` (default, try `0x3D` if no display)
- Power supply / wiring (TBD)

## Software & Tools

- Arduino IDE
- Required libraries:
  - `WiFi` — built-in for ESP32
  - `HTTPClient` — for HTTP/REST to Shelly
  - `PubSubClient` — if using MQTT
  - `U8g2` — recommended for SH1106 OLED (supports 1.3" AZ-Delivery display)
  - _(Alternative: `Adafruit SH110X` + `Adafruit GFX`)_
- Shelly firmware version: _fill in_

---

## Tasks

### Setup
- [ ] Install Arduino IDE and configure for XIAO ESP32 board
- [ ] Install U8g2 library via Arduino Library Manager
- [ ] Wire OLED display (SDA→Pin 4, SCL→Pin 8) and confirm it powers on
- [ ] Run basic U8g2 "Hello World" sketch to verify OLED works
- [ ] Connect XIAO ESP32 to Wi-Fi, show IP on OLED as confirmation
- [ ] Find Shelly Dimmer Gen4 on local network, note its IP address
- [ ] Confirm Shelly Gen4 API is accessible (e.g. via browser/curl)

### Communication
- [ ] Choose communication protocol (HTTP, MQTT, or WebSocket)
- [ ] Write basic Arduino sketch to send a command to Shelly
- [ ] Display command status on OLED (e.g. "Sending...", "OK", "Error")
- [ ] Test on/off control from ESP32
- [ ] Display current on/off state on OLED
- [ ] Test brightness level control (0–100%)
- [ ] Display brightness level on OLED as progress bar or percentage

### Refinement
- [ ] Handle error/retry logic in the sketch
- [ ] Add status feedback (e.g. read current dimmer state)
- [ ] Clean up and comment the code
- [ ] Document final wiring and pin assignments

---

## Notes & Findings

_Use this section as a running log during development._

-

---

## OLED Display Layout (planned)

```
┌────────────────────────┐
│ Shelly Dimmer Ctrl     │  ← title
│ WiFi: Connected        │  ← network status
│ Brightness: 75%        │  ← current dimmer value
│ Status: OK             │  ← last command result
└────────────────────────┘
```

*Last updated: 2026-04-03*
