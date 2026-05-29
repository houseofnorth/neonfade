#include "Matter.h"
#include <Arduino.h>

// Phase 2 stub — nothing implemented yet.
// init() and poll() are no-ops; the compiler will optimise them away.

void MatterInterface::init(Motor* /*motor*/, Calibration* /*cal*/) {
  // TODO Phase 2: initialise Matter stack, commission device,
  //              register OnOff + LevelControl clusters.
  Serial.println("[matter] stub — Phase 2 not yet implemented");
}

void MatterInterface::poll() {
  // TODO Phase 2: check for incoming Matter commands and
  //              forward them to Motor via setTarget() / setSpeed() etc.
}
