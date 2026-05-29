#pragma once

class Motor;
class Calibration;

// ── MatterInterface ───────────────────────────────────────────────────────
// Phase 2 stub — Matter protocol integration (Thread / IP-based smart home).
//
// Planned channel mapping (to be defined in Phase 2):
//   - OnOff cluster    → motor stop / resume
//   - LevelControl     → motor position (0-254 → calMin-calMax)
//   - Scenes           → preset positions or oscillation patterns
//
// When implemented, Matter commands will write to the same Motor setters
// as web and DMX, following the same last-write-wins priority model.
// ──────────────────────────────────────────────────────────────────────────
class MatterInterface {
public:
  void init(Motor* motor, Calibration* cal);
  void poll();

  bool active() const { return false; }  // Phase 2 — always false for now
};
