// ── Serial report ─────────────────────────────────────────────────────────
// Level 1: prints a compact header + status line on every state change.
// Level 2: same, plus action logs from other modules.

static int      _lastPot   = -1;
static bool     _lastRelay = false;
static MotorDir _lastMotor = STOPPED;
static DimState _lastState = DIM_NONE;

static void _printHeader() {
  String ip   = wifiIpString();
  String name = getDeviceName();
  Serial.println();
  Serial.printf("[ %s  |  %s  |  WiFi:%s  MQTT:%s  |  v%d.%d.%d ]\n",
    name.c_str(),
    ip.length() ? ip.c_str() : "---.---.---.---",
    ip.length() ? "OK" : "--",
    mqttIsConnected() ? "OK" : "--",
    VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH
  );
  Serial.printf("  Steps: Day=%-5d  Dusk=%-5d  Night=%-5d\n",
    getStatePos(DIM_DAY), getStatePos(DIM_DUSK), getStatePos(DIM_NIGHT));
  Serial.println("  POT    MIN    MAX    MOTOR    RELAY  STATE");
}

// Call once after setup is complete to print the initial header.
void reportPrintHeader() {
  _printHeader();
}

void reportSerial() {
  if (isCalibrating()) return;

  int      pot   = readPot();
  bool     relay = getRelayState();
  MotorDir motor = getMotorDir();
  DimState state = getCurrentDimState();

  // Only reprint on meaningful state changes — pot noise ignored
  if (relay == _lastRelay && motor == _lastMotor && state == _lastState) return;

  _lastPot   = pot;
  _lastRelay = relay;
  _lastMotor = motor;
  _lastState = state;

  const char* mot = (motor == FORWARD) ? "FWD    " :
                    (motor == REVERSE)  ? "REV    " : "STOP   ";

  const char* stateStr;
  if      (!calibDone())     stateStr = "NEED  ";
  else if (!stateCalDone())  stateStr = "NO-POS";
  else if (!relay)           stateStr = "---   ";
  else                       stateStr = getDimStateName(state);

  _printHeader();
  Serial.printf("  %-6d %-6d %-6d %-8s %-6s %s\n",
    pot, getPotMin(), getPotMax(), mot,
    relay ? "ON" : "OFF", stateStr
  );
}
