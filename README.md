# Spaceknot

Motorised dimmer controller for networked multi-room lighting. Custom PCBs each drive an ALPS RK168 motorised pot via an L293DD H-bridge, controlled by two Shelly Dimmer Gen4 buttons over MQTT. Built and installed at Waldmannstrasse, Berlin.

---

## Hardware

| Component | Part |
|---|---|
| MCU | Seeed XIAO ESP32-C3 |
| Motor driver | L293DD |
| Pot | ALPS RK168 100kΩ motorised dual-gang |
| Relay | G6RN-1 5V via MMBT3904 |
| OLED | 1.3" SH1106 128×64 I2C (Qwiic J2) |
| Buttons | Shelly Dimmer Gen4 ×2 |
| Router / MQTT broker | GL.iNet GL-MT3000 running Mosquitto |

**PCB:** Neon Fader BRD mkIII v367 · **Schematic:** `Neon_Fader_V2_v376.sch`

**Network (192.168.2.x)**

| Device | IP |
|---|---|
| Router / MQTT broker | 192.168.2.1 |
| Shelly 1 | 192.168.2.101 |
| Shelly 2 | 192.168.2.102 |
| spaceknot-1 … spaceknot-N | 192.168.2.201 … 192.168.2.20N |

Up to 55 units supported (IPs .201–.255).

---

## Firmware

```
Firmware/
├── Spaceknot_v227/   ← active firmware (open this one)
└── Archive/          ← all previous versions
```

Open `Firmware/Spaceknot_v227/Spaceknot_v227.ino` in Arduino IDE.

Compiled binaries are in `releases/` — use `Sketch → Export Compiled Binary` in Arduino IDE, then copy `Spaceknot_v227.ino.bin` there.

**Dependencies (Arduino Library Manager)**

- `PubSubClient` by Nick O'Leary
- `U8g2` by olikraus
- `ArduinoOTA` (bundled with ESP32 Arduino core)
- `WebServer` (bundled with ESP32 Arduino core)

**Board:** Seeed XIAO ESP32-C3 — install via Seeed boards package URL in Arduino IDE preferences.

---

## Flashing & First-Time Setup

Flash all units with identical firmware. On first boot each unit prompts for its number via Serial Monitor (115200 baud):

```
=== UNIT NOT CONFIGURED ===
Enter unit number (1-55):
```

Type the unit number and press Send. The unit sets its own name (`spaceknot-N`) and static IP (`192.168.2.20N`) automatically, then reboots. No further configuration needed for units 1–55.

---

## OTA Updates

After the initial USB flash, subsequent updates can be pushed wirelessly:

**Via Arduino IDE:** The unit appears as a network port under `Tools → Port` once on the network. Select it and upload normally.

**Via web interface:** Go to the unit's IP → Settings → Configure → pick a `.bin` file → Flash firmware. Export the binary from Arduino IDE with `Sketch → Export Compiled Binary` — use `Spaceknot_v227.ino.bin`. OTA password: `spaceknot`

---

## Serial Monitor Commands

Baud rate: **115200**

| Command | Action |
|---|---|
| `C` | Enter calibration mode |
| `L` | Capture current pot position as MIN (range cal) |
| `R` | Capture current pot position as MAX (range cal) |
| `A` | Toggle auto-sweep end-to-end |
| `S` | Save range calibration and exit |
| `X` | Cancel calibration without saving |
| `D` | Capture current pot position as **Day** step |
| `U` | Capture current pot position as **Dusk** step |
| `G` | Capture current pot position as ni**G**ht step |
| `N` | Rename unit — prompts for number, saves name + IP, reboots |

**Range calibration flow**

*Via web UI (recommended):*
1. Click **Recalibrate Range** — cal panel opens, live ADC readout appears on bar
2. Hold **Drive FWD** or **Drive REV** to motor the pot to each hard stop
3. Click **Capture MIN** at one end, **Capture MAX** at the other
4. Click **Save Range** — broadcasts to all units via MQTT

*Via Serial Monitor:*
1. `C` — enter cal mode
2. Move pot to one end → `L` (MIN)
3. Move pot to other end → `R` (MAX)
4. Alternatively: `A` to auto-sweep, then `A` again to stop
5. `S` to save

---

## MQTT Topics

Broker: `192.168.2.1:1883` — Mosquitto on GL.iNet router, no authentication.

| Topic | Direction | Payload |
|---|---|---|
| `spaceknot/state` | pub/sub | `Day` / `Dusk` / `Night` / `OFF` |
| `spaceknot/cal` | pub/sub | `start` / `min:NNN` / `max:NNN` / `auto` / `save` / `cancel` / `capDay:NNN` / `capDusk:NNN` / `capNight:NNN` / `setrange:NNN:MMM` |
| `+/events/rpc` | sub | Shelly Gen4 RPC event JSON (`single_push` / `long_push`) |

All units subscribe to `spaceknot/state` and `spaceknot/cal`. A calibration command from any unit's web UI broadcasts to all units. min/max/capXxx messages carry the master unit's captured ADC value so all units receive identical calibration numbers.

---

## Shelly Setup

1. Connect Shelly to the `spaceknot` WiFi network
2. **Settings → MQTT** → enable, broker `192.168.2.1:1883`
3. Set topic prefix to `spaceknot/shelly1` (or `shelly2`)
4. Enable **RPC status notifications**, set input mode to **Button**
5. Save and reboot

Button events arrive via MQTT — no webhook actions needed.

---

## Web Interface

Each unit hosts a page at its static IP (e.g. `http://192.168.2.201`):

- Live status grid (step, brightness, motor, relay, Shelly, firmware version)
- Range calibration (min / max)
- Step position capture (Day / Dusk / Night) — broadcasts to all units
- Settings panel: device name, WiFi credentials, static IP, firmware update

---

## Dim States

Three named positions stored per unit in NVS flash:

| State | Trigger |
|---|---|
| **Day** | First press after relay-on; or press to cycle |
| **Dusk** | Second press |
| **Night** | Third press — wraps back to Day on next press |

Long press → relay off, motor holds position. Next press resumes last active state.

---

*Made at the [House of North](https://north-berlin.com)*
