// ── MQTT sync ─────────────────────────────────────────────────────────────
//  All Spaceknot units subscribe to MQTT_TOPIC_STATE and publish to it when
//  a Shelly webhook changes the dim state.  This keeps all units in step
//  even if a unit missed the original webhook (reboot, network blip).
//
//  Broker:  Mosquitto on the dedicated GL.iNet router.
//  Library: PubSubClient by Nick O'Leary  (install via Arduino Library Manager)
//
//  Message format (plain text payload):
//    "Day"   — relay on, move to Day position
//    "Dusk"  — relay on, move to Dusk position
//    "Night" — relay on, move to Night position
//    "OFF"   — relay off, hold motor
//
//  The unit uses its device name (settings NVS) as the MQTT client ID so
//  each unit is individually identifiable in the broker log.

#define MQTT_MAX_PACKET_SIZE 512
#include <PubSubClient.h>
#include <WiFi.h>

static WiFiClient   _wifiClient;
static PubSubClient _mqtt(_wifiClient);

static unsigned long _lastReconnectMs = 0;
static bool          _mqttConnected   = false;

// ── Forward declarations ──────────────────────────────────────────────────
void setDimStateFromMqtt(DimState s);  // defined in webhook.ino

// ── Internal callback ─────────────────────────────────────────────────────
static void _mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  LOG2("MQTT rx [%s]: %s\n", topic, msg.c_str());

  // ── State topic ──────────────────────────────────────────────────────
  if (String(topic) == MQTT_TOPIC_STATE) {
    if (msg == "OFF") {
      setRelay(false);
      holdMotorPosition();
      return;
    }
    DimState s = DIM_NONE;
    if (msg == "Day")   s = DIM_DAY;
    if (msg == "Dusk")  s = DIM_DUSK;
    if (msg == "Night") s = DIM_NIGHT;
    if (s != DIM_NONE) setDimStateFromMqtt(s);
    return;
  }

  // ── Calibration topic ─────────────────────────────────────────────────
  //  Commands are broadcast by whichever unit's web UI the user is on.
  //  Each unit executes the command against its OWN pot reading.
  if (String(topic) == MQTT_TOPIC_CAL) {
    if      (msg == "start")    enterCalibration();
    else if (msg == "min")      captureMin();
    else if (msg == "max")      captureMax();
    else if (msg == "auto")     toggleAutoSweep();
    else if (msg == "save")     saveCalibration();
    else if (msg == "cancel")   cancelCalibration();
    else if (msg == "capDay")   captureStatePos(DIM_DAY);
    else if (msg == "capDusk")  captureStatePos(DIM_DUSK);
    else if (msg == "capNight") captureStatePos(DIM_NIGHT);
    return;
  }

  // ── Shelly button topics ──────────────────────────────────────────────
  //  Both Shellys publish to their own events/rpc topic.
  //  Payload is JSON — we just strstr for the event string, no parser needed.
  // Match any topic ending in /events/rpc (wildcard sub catches all Shellys)
  if (strstr(topic, "/events/rpc") != nullptr) {
    // Convert byte* payload to a null-terminated char* for strstr
    char buf[length + 1];
    memcpy(buf, payload, length);
    buf[length] = '\0';

    if (strstr(buf, "single_push")) {
      handleButtonPush();
    } else if (strstr(buf, "long_push")) {
      handleButtonSweep();
    }
    return;
  }
}

// ── Connect / reconnect ───────────────────────────────────────────────────
static bool _mqttConnect() {
  const String clientId = getDeviceName();
  LOG2("MQTT connecting as '%s'…\n", clientId.c_str());
  if (_mqtt.connect(clientId.c_str())) {
    _mqtt.subscribe(MQTT_TOPIC_STATE);
    _mqtt.subscribe(MQTT_TOPIC_CAL);
    
    _mqtt.subscribe(MQTT_TOPIC_SHELLY_EVENTS);
    _mqttConnected = true;
    LOG1("MQTT connected (%s)\n", clientId.c_str());
    return true;
  }
  _mqttConnected = false;
  LOG1("MQTT connect failed (rc=%d)\n", _mqtt.state());
  return false;
}

// ── Public API ────────────────────────────────────────────────────────────
void initMqtt() {
  _mqtt.setServer(MQTT_BROKER_IP, MQTT_PORT);
  _mqtt.setBufferSize(512);  // Shelly payloads exceed 256-byte default
  _mqtt.setCallback(_mqttCallback);
  // First connection attempt (non-blocking — WiFi may not be up yet)
  if (WiFi.status() == WL_CONNECTED) _mqttConnect();
}

void updateMqtt() {
  if (WiFi.status() != WL_CONNECTED) {
    _mqttConnected = false;
    return;
  }
  if (!_mqtt.connected()) {
    _mqttConnected = false;
    unsigned long now = millis();
    if (now - _lastReconnectMs >= MQTT_RECONNECT_MS) {
      _lastReconnectMs = now;
      _mqttConnect();
    }
  } else {
    _mqtt.loop();
  }
}

void mqttPublishState(DimState s) {
  if (!_mqtt.connected()) return;
  const char* name = getDimStateName(s);
  _mqtt.publish(MQTT_TOPIC_STATE, name);
  LOG2("MQTT tx state: %s\n", name);
}

void mqttPublishOff() {
  if (!_mqtt.connected()) return;
  _mqtt.publish(MQTT_TOPIC_STATE, "OFF");
  LOG2("MQTT tx state: OFF\n");
}

bool mqttIsConnected() { return _mqttConnected; }

void mqttPublishCal(const char* cmd) {
  if (!_mqtt.connected()) return;
  _mqtt.publish(MQTT_TOPIC_CAL, cmd);
  LOG2("MQTT tx cal: %s\n", cmd);
}
