# Shelly Button Test Firmware (Webhook Version)

Event-driven firmware for reading the Shelly Dimmer Gen4 button state via webhooks.
**No polling—instant response when button is pressed.**

## Hardware

- **MCU**: Seeed XIAO ESP32-C3
- **OLED**: AZ-Delivery 1.3" SH1106 on I2C (D4=SDA, D5=SCL)
- **Shelly**: Dimmer Gen4 on home WiFi at 192.168.1.44 (configurable in `config.h`)

## How It Works

1. XIAO connects to WiFi and starts a web server on port 80
2. You configure the Shelly to **POST** to the XIAO's webhook endpoint when the button changes
3. Shelly sends: `POST http://<xiao-ip>/webhook/button` with payload `{"output": true/false}`
4. XIAO receives the event and instantly increments the position counter
5. OLED displays the current position (1-10)

**Zero polling, zero latency—pure event-driven input.**

## OLED Display

Shows:
- Current position (e.g., `3 / 10`)
- WiFi status
- "Listening..." status

## Build & Flash

```bash
# In Arduino IDE:
# 1. Open ShellyButtonTest.ino
# 2. Select board: "XIAO_ESP32C3"
# 3. Serial: 115200 baud
# 4. Sketch → Upload
```

## Configuration

Edit `config.h` to customize:

| Setting | Default | Purpose |
|---------|---------|---------|
| `WIFI_SSID` / `WIFI_PASSWORD` | "House of North" / "p3rman3ntr3cord" | Home network |
| `MAX_POSITIONS` | 10 | How many position steps to cycle through |
| `INITIAL_POSITION` | 3 | Starting position |

## Setup: Configure Shelly Webhook

**Once the XIAO is running, it prints:**

```
[Webhook] Server started on port 80
[Webhook] Configure Shelly to POST to: http://<xiao-ip>/webhook/button
```

**To configure:**

1. Open Shelly web interface at `http://192.168.1.44/`
2. Go to **Settings → Automations**
3. Create a new automation:
   - **Trigger**: "Button short push" (or the button event)
   - **Action**: "Webhook"
   - **URL**: `http://<your-xiao-ip>/webhook/button`
   - **Method**: POST
   - **Payload**: Leave empty (Shelly will send `{"output": true/false}`)

**Find your XIAO's IP:**
- Check your router's DHCP clients
- Or watch the serial console for `[WiFi] STA connected. IP xxx.xxx.xxx.xxx`

## Serial Output

```
=== Shelly Button Test v1.1-webhook ===
Using webhooks (event-driven, not polling)
[Webhook] Server started on port 80
[Webhook] Configure Shelly to POST to: http://192.168.1.50/webhook/button
[WiFi] STA connected. IP 192.168.1.50

[Webhook] Button pressed → Position 4/10
[Webhook] Button pressed → Position 5/10
```

## Next Steps

Once button reading is confirmed, integrate this into `NeonFade_v2`:
- Add `shelly_button.ino` functions to `NeonFade_v2/shelly.ino`
- Use `shellyButtonIsPressed()` to drive motor movement
- Update motor control logic to respond to button state
