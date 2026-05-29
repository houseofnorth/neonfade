#pragma once
#include <pgmspace.h>

// ── HTML page (PROGMEM) ───────────────────────────────────────────────────
// Kept in a separate header so the Arduino IDE preprocessor does not scan
// the JavaScript inside the raw-string literal and misread 'function' as C++.
static const char WEB_PAGE[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="en"><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SPACEKNOT</title>
<style>
*,*::before,*::after{box-sizing:border-box;margin:0;padding:0}
body{background:#000;color:#fff;font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif;min-height:100vh;display:flex;align-items:center;justify-content:center;padding:1.5rem}
.wrap{width:100%;max-width:420px}
.title{text-align:center;margin-bottom:2rem}
.t1{font-size:2rem;font-weight:900;letter-spacing:.45em;text-transform:uppercase}
.t2{font-size:.75rem;color:#555;letter-spacing:.3em;margin-top:.35rem}
.oled{display:grid;grid-template-columns:1fr 1fr;gap:3px;margin-bottom:.6rem}
.c{border:1px solid #1c1c1c;padding:.65rem .4rem;text-align:center;min-height:54px;display:flex;flex-direction:column;justify-content:center;gap:.15rem;transition:all .15s}
.c.on{background:rgba(255,77,136,.12);border-color:#ff4d88}
.cl{font-size:.58rem;text-transform:uppercase;letter-spacing:.14em;color:#444}
.cv{font-size:.9rem;font-weight:700;color:#fff}
.c.on .cl{color:#ff4d88}
.info{display:flex;justify-content:space-between;font-size:.68rem;color:#383838;margin-bottom:1.5rem;letter-spacing:.05em}
.bar-row{display:flex;align-items:center;gap:.6rem;margin-bottom:2rem;font-variant-numeric:tabular-nums;font-size:.75rem;color:#555}
.bar{flex:1;height:14px;border:1px solid #1c1c1c;position:relative;overflow:hidden}
.fill{height:100%;background:#ff4d88;width:0%;transition:width .15s}
h2{font-size:.75rem;font-weight:700;letter-spacing:.2em;text-transform:uppercase;color:#555;margin-bottom:.8rem;margin-top:1.8rem}
.btn{display:block;width:100%;padding:.85rem;border:1px solid #1c1c1c;background:none;color:#fff;font-size:.85rem;font-weight:700;letter-spacing:.12em;text-transform:uppercase;cursor:pointer;margin-bottom:.4rem;transition:all .12s;font-family:inherit}
.btn:hover{border-color:#ff4d88}
.btn.primary{border-color:#ff4d88;background:rgba(255,77,136,.12)}
.btn.danger{border-color:#ff4d4d;color:#ff4d4d}
.btn.danger:hover{background:rgba(255,77,77,.12)}
.panel{display:none;border:1px solid #1c1c1c;padding:.9rem;margin-top:.4rem}
.panel.show{display:block}
.cal-panel{border-color:#ff4d88;background:rgba(255,77,136,.04)}
.cal-grid{display:grid;grid-template-columns:1fr 1fr;gap:.4rem;margin-bottom:.4rem}
.step-row{display:flex;align-items:center;gap:.6rem;margin-bottom:.4rem}
.sn{width:3.5rem;font-size:.85rem;font-weight:700;letter-spacing:.08em;text-transform:uppercase}
.sv{width:3rem;font-size:.85rem;font-variant-numeric:tabular-nums}
.step-row .btn{flex:1;margin-bottom:0;padding:.6rem}
.field{margin-bottom:.7rem}
.field label{display:block;font-size:.7rem;color:#555;text-transform:uppercase;letter-spacing:.08em;margin-bottom:.3rem}
input[type=text],input[type=password]{width:100%;padding:.6rem;background:#0a0a0a;border:1px solid #1c1c1c;color:#fff;font-size:.85rem;font-family:inherit}
input:focus{outline:none;border-color:#ff4d88}
.msg{font-size:.75rem;color:#555;margin-top:.5rem;letter-spacing:.08em}
small{color:#444;font-size:.7rem;letter-spacing:.08em}
footer{text-align:center;font-size:.68rem;color:#2a2a2a;margin-top:2.5rem;letter-spacing:.08em}
footer a{color:#444;text-decoration:none}
footer a:hover{color:#ff4d88}
</style>
</head><body><div class="wrap">

<div class="title">
  <div class="t1">SPACEKNOT</div>
  <div class="t2">waldmannstrasse</div>
</div>

<div class="oled">
  <div class="c" id="cStep"><span class="cl">Step</span><span class="cv" id="vStep">--</span></div>
  <div class="c" id="cBri"><span class="cl">Brightness</span><span class="cv" id="vBri">--</span></div>
  <div class="c" id="cMot"><span class="cl">Motor</span><span class="cv" id="vMot">--</span></div>
  <div class="c" id="cRel"><span class="cl">Relay</span><span class="cv" id="vRel">--</span></div>
  <div class="c" id="cShl"><span class="cl">Shelly</span><span class="cv" id="vShl">--</span></div>
  <div class="c"         ><span class="cl">Version</span><span class="cv" id="vVer">--</span></div>
</div>

<div class="info">
  <span id="ipInfo">--</span>
  <span id="calInfo">--</span>
</div>

<div class="bar-row">
<span id="bmin">0</span>
<div class="bar"><div class="fill" id="bfill"></div></div>
<span id="bmax">4095</span>
</div>

<button class="btn" onclick="push()">Next Step</button>

<h2>Range Calibration</h2>
<button class="btn primary" id="btnStart" onclick="cal('start')">Recalibrate Range</button>
<div class="panel cal-panel" id="calPanel">
<small>Move pot to each end of travel, then capture.</small><br><br>
<div class="cal-grid">
<button class="btn" onclick="cal('min')">Capture MIN</button>
<button class="btn" onclick="cal('max')">Capture MAX</button>
</div>
<button class="btn primary" onclick="cal('save')">Save Range</button>
<button class="btn danger" onclick="cal('cancel')">Cancel</button>
</div>

<h2>Step Positions</h2>
<small>Move the dimmer to the desired level, then capture each step.</small><br><br>
<button class="btn" id="btnRelay" onclick="toggleRelay()">Relay: --</button>
<div class="step-row">
<span class="sn">Day</span>
<span class="sv" id="dayPct">--</span>
<button class="btn" onclick="capState('day')">Capture</button>
</div>
<div class="step-row">
<span class="sn">Dusk</span>
<span class="sv" id="duskPct">--</span>
<button class="btn" onclick="capState('dusk')">Capture</button>
</div>
<div class="step-row">
<span class="sn">Night</span>
<span class="sv" id="nightPct">--</span>
<button class="btn" onclick="capState('night')">Capture</button>
</div>

<h2>Settings</h2>
<button class="btn" onclick="toggleCfg()">Configure</button>
<div class="panel" id="cfgPanel">
<div class="field"><label>Device name</label><input id="cfgName" type="text"></div>
<div class="field"><label>WiFi SSID</label><input id="cfgSsid" type="text"></div>
<div class="field"><label>WiFi password <small>(blank = keep current)</small></label><input id="cfgPass" type="password" placeholder="(unchanged)"></div>
<div class="field"><label>Static IP <small>(blank = DHCP)</small></label><input id="cfgIp" type="text" placeholder="e.g. 192.168.1.20"></div>
<div class="field"><label>Gateway <small>(blank = auto)</small></label><input id="cfgGw" type="text" placeholder="e.g. 192.168.1.1"></div>
<button class="btn danger" onclick="saveSettings()">Save &amp; Reboot</button>
<div class="msg" id="cfgMsg"></div>
</div>

<footer>made at the <a href="https://north-berlin.com" target="_blank">House of North</a></footer>

<script>
const $ = id => document.getElementById(id);

function push(){fetch('/webhook/button?push=1').catch(()=>{});}
function cal(act){fetch('/cal?act='+act).catch(()=>{});}
function capState(s){fetch('/cal?act='+s).catch(()=>{});}
function toggleRelay(){const on=$('btnRelay').dataset.on!=='1';fetch('/relay?on='+(on?1:0)).catch(()=>{});}

function toggleCfg(){
  const p=$('cfgPanel');
  const show=!p.classList.contains('show');
  p.classList.toggle('show',show);
  if(show) loadSettings();
}

async function loadSettings(){
  try{
    const r=await fetch('/settings');
    const d=await r.json();
    $('cfgName').value=d.name||'';
    $('cfgSsid').value=d.ssid||'';
    $('cfgIp').value=d.sip||'';
    $('cfgGw').value=d.sgw||'';
  }catch(e){}
}

async function saveSettings(){
  $('cfgMsg').textContent='Saving...';
  try{
    const body=new URLSearchParams({
      name:$('cfgName').value,
      ssid:$('cfgSsid').value,
      pass:$('cfgPass').value,
      sip:$('cfgIp').value,
      sgw:$('cfgGw').value
    });
    await fetch('/settings',{method:'POST',
      headers:{'Content-Type':'application/x-www-form-urlencoded'},
      body:body.toString()});
    $('cfgMsg').textContent='Rebooting… reconnect in a few seconds.';
  }catch(e){$('cfgMsg').textContent='Error: '+e.message;}
}

function pctSpan(el, ok, pct){
  el.textContent = ok ? pct+'%' : '--';
  el.style.color = ok ? '#3aff8c' : '#555';
}

async function poll(){
  try{
    const r=await fetch('/status');
    if(!r.ok)return;
    const d=await r.json();

    // OLED grid cells
    $('vStep').textContent = d.relayOn ? d.dimState : '---';
    $('cStep').classList.toggle('on', d.relayOn);
    $('vBri').textContent  = d.brightnessPct+'%';
    $('vMot').textContent  = d.motorOn ? 'ON' : 'OFF';
    $('cMot').classList.toggle('on', d.motorOn);
    $('vRel').textContent  = d.relayOn ? 'ON' : 'OFF';
    $('cRel').classList.toggle('on', d.relayOn);
    $('vShl').textContent  = d.shellyOnline ? 'ONLINE' : 'OFFLINE';
    $('cShl').classList.toggle('on', d.shellyOnline);
    $('vVer').textContent  = d.version;

    // Info row
    $('ipInfo').textContent  = d.ip ? 'IP '+d.ip : '--';
    $('calInfo').textContent = d.calibrating ? 'CAL IN PROGRESS' : (d.calOk ? 'Cal OK' : 'CAL NEEDED');

    // Bar
    $('bmin').textContent = d.potMin;
    $('bmax').textContent = d.potMax;
    $('bfill').style.width = d.brightnessPct+'%';

    // Cal panel
    $('calPanel').classList.toggle('show', d.calibrating);
    $('btnStart').textContent = d.calibrating ? 'Cal in progress…' : 'Recalibrate Range';

    // Relay toggle button
    const rb=$('btnRelay');
    rb.textContent='Relay: '+(d.relayOn?'ON':'OFF');
    rb.dataset.on=d.relayOn?'1':'0';
    rb.classList.toggle('primary',d.relayOn);

    // Step positions
    pctSpan($('dayPct'),  d.dayOk,  d.dayPct);
    pctSpan($('duskPct'), d.duskOk, d.duskPct);
    pctSpan($('nightPct'),d.nightOk,d.nightPct);
  }catch(e){}
}
setInterval(poll,%POLL_MS%);
poll();
</script>
</div></body></html>)HTML";
