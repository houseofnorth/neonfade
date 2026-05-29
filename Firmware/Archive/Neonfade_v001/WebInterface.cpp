#include "WebInterface.h"
#include "Motor.h"
#include "Calibration.h"
#include "config.h"

// ── HTML page (PROGMEM) ───────────────────────────────────────────────────

static const char WEB_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>NEONFADE</title>
  <style>
    *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

    body {
      background: #000;
      color: #fff;
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 2rem;
    }

    .wrap { width: 100%; max-width: 420px; }

    h1 {
      font-size: 2.2rem;
      font-weight: 900;
      text-align: center;
      letter-spacing: .25em;
      margin-bottom: 2rem;
    }

    /* ── Mode tabs ── */
    .tabs { display: flex; gap: .5rem; margin-bottom: 2.5rem; }
    .tab {
      flex: 1;
      padding: .85rem;
      border: 1px solid #2a2a2a;
      background: none;
      color: #555;
      font-size: .85rem;
      font-weight: 700;
      letter-spacing: .12em;
      text-transform: uppercase;
      cursor: pointer;
      transition: all .15s;
    }
    .tab.active { border-color: #ff4d88; color: #fff; background: rgba(255,77,136,.1); }

    /* ── Panels ── */
    .panel { display: none; }
    .panel.active { display: block; }

    /* ── Big position number ── */
    .big-number {
      text-align: center;
      font-size: 5.5rem;
      font-weight: 800;
      color: #ff4d88;
      line-height: 1;
      margin-bottom: .25rem;
    }
    .big-number small { font-size: 1.8rem; color: #444; font-weight: 400; }

    /* ── Section label ── */
    .label {
      text-align: center;
      font-size: .8rem;
      color: #555;
      text-transform: uppercase;
      letter-spacing: .15em;
      margin-bottom: 2rem;
    }

    /* ── Sliders ── */
    input[type=range] {
      -webkit-appearance: none;
      appearance: none;
      width: 100%;
      height: 3px;
      background: linear-gradient(to right, #ff4d88 var(--fill, 50%), #1e1e1e var(--fill, 50%));
      outline: none;
      cursor: pointer;
      margin-bottom: .6rem;
    }
    input[type=range]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 26px; height: 26px;
      border-radius: 50%;
      background: #fff;
      border: 2.5px solid #ff4d88;
      cursor: pointer;
      transition: transform .1s;
    }
    input[type=range]:active::-webkit-slider-thumb { transform: scale(1.2); }
    input[type=range]::-moz-range-thumb {
      width: 26px; height: 26px;
      border-radius: 50%;
      background: #fff;
      border: 2.5px solid #ff4d88;
      cursor: pointer;
    }

    /* ── Range end-labels ── */
    .range-labels {
      display: flex;
      justify-content: space-between;
      font-size: .75rem;
      color: #333;
      text-transform: uppercase;
      letter-spacing: .08em;
      margin-bottom: 2.5rem;
    }

    /* ── Oscillate in/out rows ── */
    .osc-row { margin-bottom: .25rem; }
    .osc-header { display: flex; justify-content: space-between; align-items: baseline; margin-bottom: .6rem; }
    .osc-name { font-size: .8rem; color: #555; text-transform: uppercase; letter-spacing: .12em; }
    .osc-val  { font-size: 1.6rem; font-weight: 700; color: #ff4d88; }
    .osc-val small { font-size: 1rem; color: #444; font-weight: 400; }

    /* ── Oscillate start/stop button ── */
    .osc-btn {
      width: 100%;
      padding: 1.1rem;
      margin-top: .5rem;
      margin-bottom: 2.5rem;
      background: none;
      border: 1px solid #ff4d88;
      color: #ff4d88;
      font-size: .95rem;
      font-weight: 700;
      letter-spacing: .15em;
      text-transform: uppercase;
      cursor: pointer;
      transition: all .15s;
    }
    .osc-btn.running { background: #ff4d88; color: #000; }
    .osc-btn:hover { opacity: .8; }

    /* ── Speed section ── */
    hr { border: none; border-top: 1px solid #111; margin-bottom: 2rem; }
    .speed-header { display: flex; justify-content: space-between; align-items: baseline; margin-bottom: .6rem; }
    .speed-name { font-size: .8rem; color: #555; text-transform: uppercase; letter-spacing: .12em; }
    .speed-val  { font-size: 1.6rem; font-weight: 700; color: #fff; }
    .speed-val small { font-size: 1rem; color: #444; font-weight: 400; }

    /* ── Status / calibration footer ── */
    .status {
      text-align: center;
      font-size: .8rem;
      color: #444;
      min-height: 1.4em;
      text-transform: uppercase;
      letter-spacing: .1em;
      margin-bottom: .75rem;
    }
    .status.ok    { color: #bbb; }
    .status.error { color: #ff4d88; }

    .cal-row {
      display: flex;
      justify-content: space-between;
      font-size: .75rem;
      color: #2e2e2e;
      text-transform: uppercase;
      letter-spacing: .08em;
    }
  </style>
</head>
<body>
<div class="wrap">

  <h1>NEONFADE</h1>

  <!-- Mode tabs -->
  <div class="tabs">
    <button class="tab active" id="tabPos" onclick="setMode('pos')">Position</button>
    <button class="tab"        id="tabOsc" onclick="setMode('osc')">Oscillate</button>
  </div>

  <!-- POSITION panel -->
  <div class="panel active" id="panelPos">
    <div class="big-number" id="posDisplay">50<small>%</small></div>
    <div class="label">target position</div>
    <input type="range" id="posSlider" min="0" max="100" value="50">
    <div class="range-labels"><span>Min</span><span>Max</span></div>
  </div>

  <!-- OSCILLATE panel -->
  <div class="panel" id="panelOsc">

    <div class="osc-row">
      <div class="osc-header">
        <span class="osc-name">In point</span>
        <span class="osc-val" id="oscInVal">20<small>%</small></span>
      </div>
      <input type="range" id="oscIn" min="0" max="100" value="20">
      <div class="range-labels"><span>Min</span><span>Max</span></div>
    </div>

    <div class="osc-row">
      <div class="osc-header">
        <span class="osc-name">Out point</span>
        <span class="osc-val" id="oscOutVal">80<small>%</small></span>
      </div>
      <input type="range" id="oscOut" min="0" max="100" value="80">
      <div class="range-labels"><span>Min</span><span>Max</span></div>
    </div>

    <button class="osc-btn" id="oscBtn" onclick="toggleOscillate()">&#9654; Start</button>
  </div>

  <hr>

  <!-- Speed — always visible -->
  <div class="speed-header">
    <span class="speed-name">Speed</span>
    <span class="speed-val" id="speedVal">100<small>%</small></span>
  </div>
  <input type="range" id="speedSlider" min="0" max="100" value="100">
  <div class="range-labels" style="margin-bottom:2rem"><span>Slow</span><span>Fast</span></div>

  <hr>

  <div class="status" id="status">Ready</div>
  <div class="cal-row">
    <span>Left: <b id="calMin">—</b></span>
    <span>Right: <b id="calMax">—</b></span>
  </div>

</div>
<script>

  let oscRunning  = false;
  let currentMode = 'pos';

  function fill(slider, pct) {
    slider.style.setProperty('--fill', pct + '%');
  }

  function setStatus(msg, cls) {
    const el = document.getElementById('status');
    el.textContent = msg;
    el.className   = 'status ' + (cls || '');
  }

  function setMode(mode) {
    currentMode = mode;
    document.getElementById('tabPos').classList.toggle('active', mode === 'pos');
    document.getElementById('tabOsc').classList.toggle('active', mode === 'osc');
    document.getElementById('panelPos').classList.toggle('active', mode === 'pos');
    document.getElementById('panelOsc').classList.toggle('active', mode === 'osc');
    if (mode === 'pos' && oscRunning) stopOscillate();
  }

  // ── Position mode ────────────────────────────────────────────────────────
  const posSlider  = document.getElementById('posSlider');
  const posDisplay = document.getElementById('posDisplay');

  posSlider.addEventListener('input', () => {
    posDisplay.innerHTML = posSlider.value + '<small>%</small>';
    fill(posSlider, posSlider.value);
    clearTimeout(posSlider._t);
    posSlider._t = setTimeout(() => sendPosition(posSlider.value), 120);
  });
  fill(posSlider, 50);

  function sendPosition(pct) {
    setStatus('Moving to ' + pct + '%…');
    fetch('/set?pos=' + pct)
      .then(r => { if (!r.ok) throw new Error(r.status); })
      .then(() => setStatus('Position  ' + pct + '%', 'ok'))
      .catch(e  => setStatus('Error: ' + e, 'error'));
  }

  // ── Oscillate mode ───────────────────────────────────────────────────────
  const oscInSlider  = document.getElementById('oscIn');
  const oscOutSlider = document.getElementById('oscOut');
  const oscInVal     = document.getElementById('oscInVal');
  const oscOutVal    = document.getElementById('oscOutVal');
  const oscBtn       = document.getElementById('oscBtn');

  oscInSlider.addEventListener('input', () => {
    oscInVal.innerHTML = oscInSlider.value + '<small>%</small>';
    fill(oscInSlider, oscInSlider.value);
    if (oscRunning) sendOscillate();
  });
  oscOutSlider.addEventListener('input', () => {
    oscOutVal.innerHTML = oscOutSlider.value + '<small>%</small>';
    fill(oscOutSlider, oscOutSlider.value);
    if (oscRunning) sendOscillate();
  });
  fill(oscInSlider,  20);
  fill(oscOutSlider, 80);

  function toggleOscillate() {
    if (oscRunning) stopOscillate();
    else            sendOscillate(true);
  }

  function sendOscillate(starting) {
    const i = oscInSlider.value;
    const o = oscOutSlider.value;
    fetch('/osc?in=' + i + '&out=' + o)
      .then(r => { if (!r.ok) throw new Error(r.status); return r.text(); })
      .then(() => {
        if (starting) {
          oscRunning = true;
          oscBtn.innerHTML = '&#9646; Stop';
          oscBtn.classList.add('running');
        }
        setStatus('Oscillating  ' + i + '% \u2194 ' + o + '%', 'ok');
      })
      .catch(e => setStatus('Error: ' + e, 'error'));
  }

  function stopOscillate() {
    fetch('/osc?stop=1')
      .then(() => {
        oscRunning = false;
        oscBtn.innerHTML = '&#9654; Start';
        oscBtn.classList.remove('running');
        setStatus('Stopped', 'ok');
      })
      .catch(e => setStatus('Error: ' + e, 'error'));
  }

  // ── Speed ────────────────────────────────────────────────────────────────
  const speedSlider = document.getElementById('speedSlider');
  const speedVal    = document.getElementById('speedVal');

  speedSlider.addEventListener('input', () => {
    speedVal.innerHTML = speedSlider.value + '<small>%</small>';
    fill(speedSlider, speedSlider.value);
    clearTimeout(speedSlider._t);
    speedSlider._t = setTimeout(() => sendSpeed(speedSlider.value), 120);
  });
  fill(speedSlider, 100);

  function sendSpeed(pct) {
    fetch('/speed?val=' + pct)
      .catch(e => setStatus('Speed error: ' + e, 'error'));
  }

  // ── Polling ──────────────────────────────────────────────────────────────
  setInterval(() => {
    fetch('/pos')
      .then(r => r.text())
      .then(t => setStatus('Pot  ' + t + '%'))
      .catch(() => {});
  }, 1000);

  function refreshCal() {
    fetch('/cal')
      .then(r => r.json())
      .then(d => {
        document.getElementById('calMin').textContent = d.min;
        document.getElementById('calMax').textContent = d.max;
      })
      .catch(() => {});
  }
  refreshCal();
  setInterval(refreshCal, 5000);

</script>
</body>
</html>
)rawliteral";

// ── Init ─────────────────────────────────────────────────────────────────

void WebInterface::init(Motor* motor, Calibration* cal) {
  _motor = motor;
  _cal   = cal;

  WiFi.softAP(WIFI_SSID, WIFI_PASS);
  Serial.printf("[wifi] SSID: %s  pass: %s\n", WIFI_SSID, WIFI_PASS);
  Serial.printf("[wifi] open http://%s\n", WiFi.softAPIP().toString().c_str());

  // Lambda bindings let member functions serve as route handlers
  _server.on("/",      [this]() { onPage(); });
  _server.on("/set",   [this]() { onSetPosition(); });
  _server.on("/pos",   [this]() { onGetPosition(); });
  _server.on("/cal",   [this]() { onGetCalibration(); });
  _server.on("/speed", [this]() { onSetSpeed(); });
  _server.on("/osc",   [this]() { onSetOscillate(); });
  _server.begin();
  Serial.println("[web] server started");
}

void WebInterface::handleClient() {
  _server.handleClient();
}

// ── Route handlers ────────────────────────────────────────────────────────

void WebInterface::onPage() {
  _server.send_P(200, "text/html", WEB_PAGE);
}

void WebInterface::onSetPosition() {
  if (!_server.hasArg("pos")) { _server.send(400, "text/plain", "missing pos"); return; }
  int pct = constrain(_server.arg("pos").toInt(), 0, 100);
  int adc = (int)map(pct, 0, 100, _cal->minVal(), _cal->maxVal());
  _motor->setTarget(adc);
  Serial.printf("[web] /set  pct=%d  target=%d  (pot=%d)\n", pct, adc, analogRead(POT_PIN));
  _server.send(200, "text/plain", String(pct));
}

void WebInterface::onGetPosition() {
  int pct = constrain((int)map(analogRead(POT_PIN), _cal->minVal(), _cal->maxVal(), 0, 100), 0, 100);
  _server.send(200, "text/plain", String(pct));
}

void WebInterface::onGetCalibration() {
  _server.send(200, "application/json",
    "{\"min\":" + String(_cal->minVal()) + ",\"max\":" + String(_cal->maxVal()) + "}");
}

void WebInterface::onSetSpeed() {
  if (!_server.hasArg("val")) { _server.send(400, "text/plain", "missing val"); return; }
  int pct  = constrain(_server.arg("val").toInt(), 0, 100);
  int duty = (int)map(pct, 0, 100, 0, 255);
  _motor->setSpeed(duty);
  Serial.printf("[web] /speed  pct=%d  duty=%d\n", pct, duty);
  _server.send(200, "text/plain", String(pct));
}

void WebInterface::onSetOscillate() {
  if (_server.hasArg("stop")) {
    _motor->stopAll();
    Serial.println("[web] /osc  stopped");
    _server.send(200, "text/plain", "stopped");
    return;
  }
  if (!_server.hasArg("in") || !_server.hasArg("out")) {
    _server.send(400, "text/plain", "missing in/out");
    return;
  }
  int inPct  = constrain(_server.arg("in").toInt(),  0, 100);
  int outPct = constrain(_server.arg("out").toInt(), 0, 100);
  int inAdc  = (int)map(inPct,  0, 100, _cal->minVal(), _cal->maxVal());
  int outAdc = (int)map(outPct, 0, 100, _cal->minVal(), _cal->maxVal());
  _motor->startOscillate(inAdc, outAdc);
  Serial.printf("[web] /osc  in=%d%%(%d ADC)  out=%d%%(%d ADC)\n", inPct, inAdc, outPct, outAdc);
  _server.send(200, "text/plain", "running");
}
