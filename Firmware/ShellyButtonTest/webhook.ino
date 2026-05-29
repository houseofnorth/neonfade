// ── Webhook Server ────────────────────────────────────────────────────────
//  Event-driven button input: Shelly POSTs to the XIAO whenever the button
//  state changes. Zero polling, instant response.
//
//  Setup:
//    1. XIAO boots and exposes /webhook/button endpoint
//    2. Configure Shelly settings to POST to:
//       http://<xiao-ip>:80/webhook/button
//    3. When button changes, Shelly sends JSON: {"output": true/false}
//    4. XIAO increments position immediately

#include <WebServer.h>
#include <ArduinoJson.h>

static WebServer _server(80);
static volatile int _position = INITIAL_POSITION;

// ── Webhook handler ────────────────────────────────────────────────────────
static void _handleButtonWebhook() {
  Serial.printf("[Webhook] Received request\n");

  // Debug: print request method, path, args
  Serial.printf("[Webhook] Method: %s, URI: %s\n",
                _server.method() == HTTP_POST ? "POST" : "GET",
                _server.uri().c_str());
  Serial.printf("[Webhook] Arg count: %d\n", _server.args());

  // Button push event: just increment position on every push
  if (_server.hasArg("push")) {
    _position++;
    if (_position > MAX_POSITIONS) {
      _position = 1;
    }
    Serial.printf("[Webhook] Button pushed → Position %d/%d\n", _position, MAX_POSITIONS);
    setShellyButtonStatus("online");
  } else {
    Serial.println("[Webhook] No 'push' parameter found");
  }

  // Send 200 OK response
  _server.send(200, "application/json", "{\"status\":\"ok\"}");
}

// ── Test endpoint (for diagnostics) ────────────────────────────────────────
static void _handleTest() {
  Serial.println("[Webhook] Test endpoint accessed via GET");
  _server.send(200, "text/plain", "XIAO is alive!");
}

// ── 404 handler ────────────────────────────────────────────────────────────
static void _handleNotFound() {
  String method = _server.method() == HTTP_POST ? "POST" :
                  _server.method() == HTTP_GET ? "GET" : "OTHER";
  Serial.printf("[Webhook] 404 - %s %s\n", method.c_str(), _server.uri().c_str());
  Serial.printf("[Webhook] Raw URI: %s\n", _server.uri().c_str());

  // If it looks like our webhook, call the handler directly
  if (_server.uri() == "/webhook/button") {
    Serial.println("[Webhook] Matched /webhook/button, calling handler...");
    _handleButtonWebhook();
    return;
  }

  _server.send(404, "text/plain", "Not found");
}

// ── Public API ─────────────────────────────────────────────────────────────
void initWebhook() {
  _position = INITIAL_POSITION;

  Serial.println("[Webhook] Registering handlers...");
  _server.on("/test", HTTP_GET, _handleTest);
  _server.on("/webhook/button", HTTP_POST, _handleButtonWebhook);
  _server.on("/webhook/button", HTTP_GET, _handleButtonWebhook);  // Accept GET too for debugging
  _server.onNotFound(_handleNotFound);

  Serial.println("[Webhook] Starting server on port 80...");
  _server.begin();
  Serial.println("[Webhook] ✓ Server started successfully!");
  Serial.println("[Webhook] Test endpoint: http://192.168.1.20/test");
  Serial.println("[Webhook] Configure Shelly to POST to: http://192.168.1.20/webhook/button");
  Serial.println("[Webhook] Waiting for requests...");
}

int getCurrentPosition() {
  return _position;
}

int getMaxPositions() {
  return MAX_POSITIONS;
}

// Handle incoming webhook requests (call from loop)
void updateWebhook() {
  _server.handleClient();
}
