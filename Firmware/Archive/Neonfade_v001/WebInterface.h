#pragma once
#include <WiFi.h>
#include <WebServer.h>

class Motor;
class Calibration;

// ── WebInterface ──────────────────────────────────────────────────────────
// Starts a WiFi access point and serves the control UI on port 80.
// Route handlers write directly to Motor and Calibration via pointers.
//
// Routes:
//   GET  /         — serve HTML page
//   GET  /set?pos= — set position target (0-100%)
//   GET  /pos      — return current pot position (0-100%)
//   GET  /cal      — return calibration limits as JSON
//   GET  /speed?val=— set motor speed (0-100%)
//   GET  /osc?in=&out= — start oscillation
//   GET  /osc?stop=1   — stop oscillation
// ──────────────────────────────────────────────────────────────────────────
class WebInterface {
public:
  void init(Motor* motor, Calibration* cal);
  void handleClient();

private:
  void onPage();
  void onSetPosition();
  void onGetPosition();
  void onGetCalibration();
  void onSetSpeed();
  void onSetOscillate();

  WebServer    _server{80};
  Motor*       _motor = nullptr;
  Calibration* _cal   = nullptr;
};
