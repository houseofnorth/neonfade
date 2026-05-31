// ── OTA update ────────────────────────────────────────────────────────────
//  Two mechanisms:
//    1. ArduinoOTA — unit appears as a network port in Arduino IDE.
//       Select "spaceknot-N" from Tools → Port and upload normally.
//    2. Web OTA    — visit http://<unit-ip>/update, upload a .bin file.
//       Export the binary from Arduino IDE: Sketch → Export Compiled Binary.

#include <ArduinoOTA.h>

void initOta() {
  ArduinoOTA.setHostname(getDeviceName().c_str());

  ArduinoOTA.onStart([]() {
    LOG1("[OTA] Starting — do not power off\n");
  });
  ArduinoOTA.onEnd([]() {
    LOG1("[OTA] Done — rebooting\n");
  });
  ArduinoOTA.onProgress([](unsigned int done, unsigned int total) {
    LOG2("[OTA] %u%%\n", done / (total / 100));
  });
  ArduinoOTA.onError([](ota_error_t e) {
    LOG1("[OTA] Error %u\n", e);
  });

  ArduinoOTA.setPassword("spaceknot");
  ArduinoOTA.begin();
  LOG1("[OTA] Ready as '%s'\n", getDeviceName().c_str());
}

void updateOta() {
  ArduinoOTA.handle();
}
