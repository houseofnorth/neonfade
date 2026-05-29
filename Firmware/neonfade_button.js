// neonfade_button.js — Shelly Dimmer Gen4 script
// ─────────────────────────────────────────────────────────────────────────
// Paste this into the Shelly web UI under Scripts → Create script.
// Name it whatever you like, then hit Save and Enable.
//
// What it does:
//   Single press  → GET /webhook/button?push=1
//                   Steps the position counter (ping-pong 1..MAX_POSITIONS).
//                   If oscillation is running it stops it first.
//
//   Long press    → GET /webhook/button?sweep=start  (on press detection)
//                   GET /webhook/button?sweep=stop   (on button release)
//                   True hold-to-oscillate: motor sweeps while held,
//                   freezes in place the moment the button comes up.
//
// The script watches the raw button state so the stop fires immediately on
// release — Shelly's built-in long_push webhook only fires once, on detect,
// which is why a script is needed for the release event.
//
// ── Configuration ─────────────────────────────────────────────────────────

var XIAO_IP          = "192.168.1.XX";   // ← set to your XIAO's IP
var INPUT_ID         = 0;                // Shelly input channel (0 = channel 1)
var LONG_PRESS_MS    = 800;              // ms held before it's treated as long press
var POLL_INTERVAL_MS = 80;              // how often to check for button release

// ── State ─────────────────────────────────────────────────────────────────

var pressTimer     = null;   // fires after LONG_PRESS_MS to classify as long press
var releaseTimer   = null;   // polling timer — runs while we wait for release
var isLongPress    = false;  // true once we've classified this press as long
var lastState      = false;  // last known button state (true = pressed)

// ── Helpers ───────────────────────────────────────────────────────────────

function send(path) {
  var url = "http://" + XIAO_IP + "/webhook/button?" + path;
  Shelly.call("HTTP.GET", { url: url, timeout: 3 }, function(res, err) {
    if (err !== 0 || !res || res.code !== 200) {
      print("NeonFade: HTTP error sending ?" + path + " (err=" + err + ")");
    }
  });
}

function stopPolling() {
  if (releaseTimer !== null) {
    Timer.clear(releaseTimer);
    releaseTimer = null;
  }
}

function stopClassifyTimer() {
  if (pressTimer !== null) {
    Timer.clear(pressTimer);
    pressTimer = null;
  }
}

// Poll Input.GetStatus until the button is released, then fire sweep=stop.
function pollForRelease() {
  Shelly.call("Input.GetStatus", { id: INPUT_ID }, function(res) {
    if (!res || res.state === false) {
      // Button is up — send stop and clean up
      stopPolling();
      isLongPress = false;
      send("sweep=stop");
      print("NeonFade: long press released → sweep=stop");
    }
    // else: still held, let the timer fire again
  });
}

// ── Button event handler ───────────────────────────────────────────────────
// btn_down / btn_up give us raw press + release on Gen2+ devices.
// single_push arrives ~300 ms after btn_up so we don't act on it — the
// btn_down handler already started the classification timer.

Shelly.addEventHandler(function(ev) {
  // Only care about our input channel
  if (ev.component !== ("input:" + INPUT_ID)) return;

  var event = ev.info.event;

  // ── Press (btn_down) ───────────────────────────────────────────────────
  if (event === "btn_down") {
    lastState   = true;
    isLongPress = false;

    // Start a timer: if the button is still down after LONG_PRESS_MS,
    // classify as long press and kick off oscillation + release polling.
    pressTimer = Timer.set(LONG_PRESS_MS, false, function() {
      pressTimer  = null;
      isLongPress = true;
      send("sweep=start");
      print("NeonFade: long press detected → sweep=start");
      // Start polling so we catch the release
      releaseTimer = Timer.set(POLL_INTERVAL_MS, true, pollForRelease);
    });
  }

  // ── Release (btn_up) ──────────────────────────────────────────────────
  else if (event === "btn_up") {
    lastState = false;

    if (!isLongPress) {
      // Short press: cancel the classify timer, nothing further needed —
      // single_push will fire shortly and we handle it there.
      stopClassifyTimer();
    }
    // Long press release is handled by pollForRelease() via the timer,
    // which fires sweep=stop. btn_up arriving here is just informational.
  }

  // ── Single push (tap) ─────────────────────────────────────────────────
  else if (event === "single_push") {
    // Only act if we didn't already classify this as a long press.
    if (!isLongPress) {
      stopClassifyTimer();
      send("push=1");
      print("NeonFade: single push → push=1");
    }
  }
});

print("NeonFade button script started. XIAO target: " + XIAO_IP);
