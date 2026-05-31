// ── Serial command processor ──────────────────────────────────────────────
//  Single-character commands (case-insensitive).
//
//    C  enter calibration mode
//    L  capture pot MIN at current position
//    R  capture pot MAX at current position
//    A  toggle live auto-sweep tracking
//    S  save calibration to flash and exit cal mode
//    D  capture DAY step position
//    U  capture DUSK step position
//    G  capture niGht step position
//    X  cancel calibration without saving
//    N  rename unit — prompts for number 1-999, saves name+IP, reboots

void initSerialCmd() {
  LOG1("Cmds: C=cal  L=min  R=max  A=sweep  S=save  D/U/G=steps  X=cancel  N=rename\n");
}

void updateSerial() {
  while (Serial.available()) {
    int c = Serial.read();
    if (c < 0) continue;

    if (c == '\r' || c == '\n' || c == ' ' || c == '\t') continue;
    c = toupper(c);
    switch (c) {
      case 'C': enterCalibration(); break;
      case 'L': captureMin();       break;
      case 'R': captureMax();       break;
      case 'A': toggleAutoSweep();  break;
      case 'S': saveCalibration();  break;
      case 'D': captureStatePos(DIM_DAY);   break;
      case 'U': captureStatePos(DIM_DUSK);  break;
      case 'G': captureStatePos(DIM_NIGHT); break;
      case 'X': cancelCalibration();        break;
      case 'N':
        forceRenameUnit();  // prompts for 1-8, saves name+IP, reboots
        break;
      default:
        LOG1("? unknown: '%c'\n", (char)c);
        break;
    }
  }
}
