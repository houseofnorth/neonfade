# ShellyXiao — Project Briefing
*Paste this as the first message in the new project conversation*

---

## What this project is
Controlling a **Shelly Dimmer Gen4** with a **Seeed Studio XIAO ESP32-C3** via HTTP over WiFi, developed in Arduino IDE. An **AZ-Delivery 1.3" I2C OLED** (SH1106) is connected for on-device feedback during development.

---

## Hardware
- **XIAO ESP32-C3** (not S3, not C6 — specifically C3)
- **Shelly Dimmer Gen4** — wired with neutral, 40W incandescent test load
- **AZ-Delivery 1.3" OLED** (SH1106 driver, 128×64) — SDA → Pin 4, SCL → Pin 8
- Button wired to S1 on the Shelly (one leg S1, other leg to L/live)

## WiFi
- SSID: `House of North`
- Password: `p3rman3ntr3cord`
- Already hardcoded in the sketch (HTTP, no MQTT, no Matter for now)

---

## Code structure
Four-tab Arduino sketch located at `/Users/fubbi/Documents/Arduino/Xiao Matter/xiao_shelly/`:

| File | Contents |
|---|---|
| `xiao_shelly.ino` | Main file — globals, `setup()`, `loop()` only. Loop contains only function calls. |
| `wifi_manager.ino` | WiFi connect, reconnect, status string |
| `oled.ino` | All display functions — splash, dashboard, `showMessage()`, `showBrightnessBar()` |
| `shelly.ino` | All Shelly HTTP/RPC communication — turn on/off, set brightness, poll state |

### Key libraries needed (install via Arduino Library Manager)
- `U8g2` by oliver
- `ArduinoJson` by Benoit Blanchon

### Board setting in Arduino IDE
Tools → Board → ESP32 Arduino → **XIAO_ESP32C3**

---

## Communication approach
- **Protocol:** HTTP (Shelly local RPC API)
- **Example calls:**
  - `GET http://<shelly-ip>/rpc/Light.GetStatus?id=0`
  - `GET http://<shelly-ip>/rpc/Light.Set?id=0&on=true`
  - `GET http://<shelly-ip>/rpc/Light.Set?id=0&brightness=75`
- Shelly IP is hardcoded in `xiao_shelly.ino` as `SHELLY_IP` — **still needs to be filled in**

---

## What still needs doing
1. Find Shelly's local IP (Shelly Smart Control app → device → Settings → Device Information) and update `SHELLY_IP` in `xiao_shelly.ino`
2. Install the two libraries above
3. Flash and test — verify OLED shows splash, WiFi connects, Shelly state shows on dashboard
4. Test button → light toggle → OLED reflects state change

---

## Decisions parked for later
- **WiFi provisioning:** Will add WiFiManager captive portal for field installation (no hardcoded credentials)
- **Shelly discovery:** Will add mDNS so the ESP32 finds the Shelly by hostname, not hardcoded IP
- **Protocol upgrade:** May move to MQTT later for push events instead of polling

---

## Specs folder
All hardware docs are at `/Users/fubbi/Documents/Arduino/Xiao Matter/Controller/specs/`
Including XIAO ESP32-C3 pinout, schematic, and Shelly Dimmer Gen4 manual.
