// ── Serial command processor ──────────────────────────────────────────────
//  Single-character commands, case-insensitive, line endings ignored.
//
//    C  enter calibration mode
//    L  capture pot MIN at current position
//    R  capture pot MAX at current position
//    A  toggle live auto-sweep tracking
//    S  save calibration to flash and exit cal mode

void initSerialCmd() {
  Serial.println("Cmds: C=cal  L=min  R=max  A=sweep  S=save");
}

void updateSerial() {
  while (Serial.available()) {
    int c = Serial.read();
    if (c < 0) continue;
    // Skip line endings and whitespace so paste-with-newline works too.
    if (c == '\r' || c == '\n' || c == ' ' || c == '\t') continue;

    c = toupper(c);
    switch (c) {
      case 'C': enterCalibration(); break;
      case 'L': captureMin();       break;
      case 'R': captureMax();       break;
      case 'A': toggleAutoSweep();  break;
      case 'S': saveCalibration();  break;
      default:
        Serial.printf("? unknown command: '%c'\n", (char) c);
        break;
    }
  }
}
