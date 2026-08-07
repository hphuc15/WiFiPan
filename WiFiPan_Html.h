#ifndef WIFIPAN_HTML_H_
#define WIFIPAN_HTML_H_

/* ============================================================
 * HOME_HTML
 * ============================================================ */
static const char HOME_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0">
<title>Home · Sharp Edge</title>
<style>
@import url('https://fonts.googleapis.com/css2?family=IBM+Plex+Mono:wght@400;500&family=DM+Sans:ital,wght@0,300;0,400;0,500;1,400&display=swap');

*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

:root {
  --bg:        #0a0e14;
  --surface:   #111820;
  --surface2:  #0d1520;
  --border:    #1e2d3d;
  --border2:   #2a3d52;
  --accent:    #0ea5e9;
  --accent-h:  #38bdf8;
  --text:      #e2eaf2;
  --text-2:    #7bafc8;
  --text-3:    #3d5470;
  --text-4:    #2a3d52;
  --success:   #22c55e;
  --warning:   #f97316;
  --danger:    #ef4444;
  --font-sans: 'DM Sans', system-ui, sans-serif;
  --font-mono: 'IBM Plex Mono', monospace;
  --radius:    3px;
  --radius-lg: 6px;
}

html, body {
  min-height: 100%;
  background: var(--bg);
  color: var(--text);
  font-family: var(--font-sans);
  font-size: 14px;
  line-height: 1.5;
  -webkit-font-smoothing: antialiased;
}

body {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: flex-start;
  padding: 1.5rem 1rem 3rem;
  min-height: 100vh;
}

.card {
  width: 100%;
  max-width: 420px;
  background: var(--surface);
  border: 0.5px solid var(--border);
  border-radius: var(--radius-lg);
  overflow: hidden;
}

.hd {
  background: var(--surface2);
  border-bottom: 0.5px solid var(--border);
  padding: 1.25rem 1.5rem 1rem;
  display: flex;
  align-items: center;
  gap: 12px;
}
.hd-icon {
  display: flex;
  align-items: flex-end;
  justify-content: center;
  gap: 3px;
  padding-bottom: 3px;
  height: 32px;
}
.hd-icon span {
  width: 5px;
  background: var(--accent);
  border-radius: 1px;
}
.hd-info { flex: 1; }
.hd-title { font-size: 15px; font-weight: 500; color: var(--text); letter-spacing: 0.01em; }
.hd-sub {
  font-family: var(--font-mono);
  font-size: 10px;
  color: var(--text-3);
  margin-top: 2px;
}
.hd-dot {
  width: 8px; height: 8px;
  border-radius: 2px;
  background: var(--warning);
  flex-shrink: 0;
  transition: background 0.4s, box-shadow 0.4s;
}
.hd-dot.connected  { background: var(--success); box-shadow: 0 0 6px var(--success); }
.hd-dot.connecting { background: var(--accent);  box-shadow: 0 0 6px var(--accent); animation: pulse 1s infinite; }

@keyframes pulse { 0%,100%{opacity:1} 50%{opacity:0.4} }

.body { padding: 1.25rem 1.5rem; }

.sec {
  font-family: var(--font-mono);
  font-size: 10px;
  letter-spacing: 0.12em;
  text-transform: uppercase;
  color: var(--text-3);
  margin-bottom: 0.65rem;
}

.div { height: 0.5px; background: var(--border); margin: 1.1rem 0; }

.status-bar {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 10px 12px;
  background: var(--surface2);
  border: 0.5px solid var(--border);
  border-radius: var(--radius);
  margin-bottom: 1.25rem;
}
.status-bar.online  { border-color: rgba(34,197,94,0.3);  background: rgba(34,197,94,0.04); }
.status-bar.offline { border-color: rgba(239,68,68,0.3);  background: rgba(239,68,68,0.04); }
.status-led {
  width: 7px; height: 7px;
  border-radius: 2px;
  background: var(--text-4);
  flex-shrink: 0;
}
.status-bar.online  .status-led { background: var(--success); box-shadow: 0 0 5px var(--success); }
.status-bar.offline .status-led { background: var(--danger);  box-shadow: 0 0 5px var(--danger); }
.status-info { flex: 1; }
.status-label {
  font-size: 12px;
  font-weight: 500;
  color: var(--text);
}
.status-detail {
  font-family: var(--font-mono);
  font-size: 10px;
  color: var(--text-3);
  margin-top: 1px;
}

.nav-grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 8px;
  margin-bottom: 1.25rem;
}

.nav-card {
  display: flex;
  flex-direction: column;
  gap: 10px;
  padding: 14px 13px;
  background: var(--surface2);
  border: 0.5px solid var(--border);
  border-radius: var(--radius);
  cursor: pointer;
  text-decoration: none;
  transition: border-color 0.15s, background 0.15s, transform 0.1s;
  position: relative;
  overflow: hidden;
}
.nav-card::before {
  content: '';
  position: absolute;
  top: 0; left: 0; right: 0;
  height: 1.5px;
  background: transparent;
  transition: background 0.15s;
}
.nav-card:hover { border-color: var(--accent); background: rgba(14,165,233,0.05); }
.nav-card:hover::before { background: var(--accent); }
.nav-card:active { transform: scale(0.98); }

.nav-card.wide {
  grid-column: 1 / -1;
  flex-direction: row;
  align-items: center;
}

.nav-icon { font-size: 20px; line-height: 1; }
.nav-label { font-size: 13px; font-weight: 500; color: var(--text); letter-spacing: 0.01em; }
.nav-desc {
  font-family: var(--font-mono);
  font-size: 10px;
  color: var(--text-3);
  margin-top: 2px;
}
.nav-arrow {
  margin-left: auto;
  font-size: 12px;
  color: var(--text-4);
  transition: color 0.15s, transform 0.15s;
}
.nav-card:hover .nav-arrow { color: var(--accent); transform: translateX(2px); }

.nav-badge {
  font-family: var(--font-mono);
  font-size: 9px;
  letter-spacing: 0.06em;
  padding: 2px 5px;
  border-radius: 2px;
  background: rgba(14,165,233,0.15);
  color: var(--accent);
  margin-top: auto;
  align-self: flex-start;
}
.nav-badge.warn { background: rgba(249,115,22,0.15); color: var(--warning); }

.info-table { display: flex; flex-direction: column; gap: 6px; }
.info-row {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 7px 10px;
  background: var(--surface2);
  border: 0.5px solid var(--border);
  border-radius: var(--radius);
}
.info-key {
  font-family: var(--font-mono);
  font-size: 10px;
  color: var(--text-3);
  letter-spacing: 0.06em;
  text-transform: uppercase;
}
.info-val {
  font-family: var(--font-mono);
  font-size: 11px;
  color: var(--text-2);
}
.info-val.ok  { color: var(--success); }
.info-val.err { color: var(--danger); }

.ft {
  padding: 0.6rem 1.5rem 1.1rem;
  display: flex;
  align-items: center;
  justify-content: space-between;
}
.ft-chip {
  font-family: var(--font-mono);
  font-size: 10px;
  color: var(--text-4);
  letter-spacing: 0.04em;
}
.ft-uptime {
  font-family: var(--font-mono);
  font-size: 10px;
  color: var(--text-3);
}
</style>
</head>
<body>

<div class="card">

  <div class="hd">
    <div class="hd-icon">
      <span style="height:6px"></span>
      <span style="height:10px"></span>
      <span style="height:16px"></span>
      <span style="height:22px"></span>
    </div>
    <div class="hd-info">
      <div class="hd-title">Sharp Edge</div>
      <div class="hd-sub" id="hd-sub">WIFIMANAGER &middot; v2.0.0</div>
    </div>
    <div class="hd-dot" id="status-dot"></div>
  </div>

  <div class="body">

    <div class="sec">Status</div>
    <div class="status-bar" id="status-bar">
      <div class="status-led"></div>
      <div class="status-info">
        <div class="status-label" id="status-label">Not connected</div>
        <div class="status-detail" id="status-detail">No WiFi configured</div>
      </div>
    </div>

    <div class="sec">Navigation</div>
    <div class="nav-grid">

      <a class="nav-card" href="/scan">
        <div class="nav-icon">&#128225;</div>
        <div>
          <div class="nav-label">WiFi Setup</div>
          <div class="nav-desc">Scan &amp; connect</div>
        </div>
        <div class="nav-badge" id="badge-networks">SCAN</div>
      </a>

      <a class="nav-card" href="/config">
        <div class="nav-icon">&#9881;</div>
        <div>
          <div class="nav-label">Config</div>
          <div class="nav-desc">Device settings</div>
        </div>
        <div class="nav-badge warn">PARAM</div>
      </a>

      <a class="nav-card" href="/info">
        <div class="nav-icon">&#8505;</div>
        <div>
          <div class="nav-label">System Info</div>
          <div class="nav-desc">Chip, memory, uptime</div>
        </div>
        <div class="nav-badge">SYS</div>
      </a>

      <a class="nav-card" href="/ota">
        <div class="nav-icon">&#11014;</div>
        <div>
          <div class="nav-label">OTA Update</div>
          <div class="nav-desc">Flash firmware</div>
        </div>
        <div class="nav-badge warn">OTA</div>
      </a>

      <a class="nav-card wide" href="/reset" id="btn-reset">
        <div class="nav-icon">&#8634;</div>
        <div>
          <div class="nav-label">Reset Settings</div>
          <div class="nav-desc">Erase saved WiFi &amp; config &mdash; device will reboot into AP mode</div>
        </div>
        <div class="nav-arrow">&#8250;</div>
      </a>

    </div>

    <div class="div"></div>

    <div class="sec">Device</div>
    <div class="info-table">
      <div class="info-row">
        <span class="info-key">Hostname</span>
        <span class="info-val" id="info-host">%HOSTNAME%</span>
      </div>
      <div class="info-row">
        <span class="info-key">IP Address</span>
        <span class="info-val" id="info-ip">%IP%</span>
      </div>
      <div class="info-row">
        <span class="info-key">SSID</span>
        <span class="info-val" id="info-ssid">%SSID%</span>
      </div>
      <div class="info-row">
        <span class="info-key">Mode</span>
        <span class="info-val" id="info-mode">%MODE%</span>
      </div>
    </div>

  </div>

  <div class="ft">
    <span class="ft-chip">WIFIMANAGER PORTAL</span>
    <span class="ft-uptime" id="ft-uptime">up 0s</span>
  </div>

</div>

<script>
(function(){
  var connected = (%CONNECTED%);
  var dot = document.getElementById('status-dot');
  var bar = document.getElementById('status-bar');
  var cnt = (%SCANCOUNT%);

  if(connected){
    dot.className = 'hd-dot connected';
    bar.className = 'status-bar online';
    document.getElementById('status-label').textContent  = document.getElementById('info-ssid').textContent || 'Connected';
    document.getElementById('status-detail').textContent = document.getElementById('info-ip').textContent   || 'Online';
    document.getElementById('info-ip').className = 'info-val ok';
    document.getElementById('hd-sub').textContent = 'WIFIMANAGER \u00B7 ' + document.getElementById('info-ip').textContent;
  } else {
    dot.className = 'hd-dot';
    bar.className = 'status-bar offline';
    document.getElementById('status-label').textContent  = 'Not connected';
    document.getElementById('status-detail').textContent = 'Running in AP mode';
    document.getElementById('info-ip').className = 'info-val err';
  }

  if(cnt > 0){
    document.getElementById('badge-networks').textContent = cnt + ' net' + (cnt !== 1 ? 's' : '');
  }

  var t0 = Date.now();
  function fmt(ms){
    var s = Math.floor(ms/1000);
    if(s < 60) return 'up ' + s + 's';
    var m = Math.floor(s/60); s = s%60;
    if(m < 60) return 'up ' + m + 'm ' + s + 's';
    var h = Math.floor(m/60); m = m%60;
    return 'up ' + h + 'h ' + m + 'm';
  }
  setInterval(function(){
    document.getElementById('ft-uptime').textContent = fmt(Date.now() - t0);
  }, 1000);

  document.getElementById('btn-reset').addEventListener('click', function(e){
    if(!confirm('Reset all settings?\nDevice will reboot into AP mode.')) e.preventDefault();
  });
})();
</script>
</body>
</html>
)rawliteral";

/* ============================================================
 * WP_PAGE_PART1 & WP_PAGE_PART2 (Scan Page)
 * ============================================================ */
static const char WP_PAGE_PART1[] = R"rawhtml(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0">
<title>WiFi Setup · Sharp Edge</title>
<style>
@import url('https://fonts.googleapis.com/css2?family=IBM+Plex+Mono:wght@400;500&family=DM+Sans:ital,wght@0,300;0,400;0,500;1,400&display=swap');

*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

:root {
  --bg:        #0a0e14;
  --surface:   #111820;
  --surface2:  #0d1520;
  --border:    #1e2d3d;
  --border2:   #2a3d52;
  --accent:    #0ea5e9;
  --accent-h:  #38bdf8;
  --text:      #e2eaf2;
  --text-2:    #7bafc8;
  --text-3:    #3d5470;
  --text-4:    #2a3d52;
  --success:   #22c55e;
  --warning:   #f97316;
  --danger:    #ef4444;
  --font-sans: 'DM Sans', system-ui, sans-serif;
  --font-mono: 'IBM Plex Mono', monospace;
  --radius:    3px;
  --radius-lg: 6px;
}

html, body {
  min-height: 100%;
  background: var(--bg);
  color: var(--text);
  font-family: var(--font-sans);
  font-size: 14px;
  line-height: 1.5;
  -webkit-font-smoothing: antialiased;
}

body {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: flex-start;
  padding: 1.5rem 1rem 3rem;
  min-height: 100vh;
}

.card {
  width: 100%;
  max-width: 420px;
  background: var(--surface);
  border: 0.5px solid var(--border);
  border-radius: var(--radius-lg);
  overflow: hidden;
}

.hd {
  background: var(--surface2);
  border-bottom: 0.5px solid var(--border);
  padding: 1.25rem 1.5rem 1rem;
  display: flex;
  align-items: center;
  gap: 12px;
}
.hd-icon {
  display: flex;
  align-items: flex-end;
  justify-content: center;
  gap: 3px;
  padding-bottom: 3px;
  height: 32px;
}
.hd-icon span {
  width: 5px;
  background: var(--accent);
  border-radius: 1px;
  transition: opacity 0.3s;
}
.hd-info { flex: 1; }
.hd-title { font-size: 15px; font-weight: 500; color: var(--text); letter-spacing: 0.01em; }
.hd-sub {
  font-family: var(--font-mono);
  font-size: 10px;
  color: var(--text-3);
  margin-top: 2px;
}
.hd-dot {
  width: 8px; height: 8px;
  border-radius: 2px;
  background: var(--warning);
  flex-shrink: 0;
  transition: background 0.4s, box-shadow 0.4s;
}
.hd-dot.connected   { background: var(--success); box-shadow: 0 0 6px var(--success); }
.hd-dot.connecting  { background: var(--accent);  box-shadow: 0 0 6px var(--accent); animation: pulse 1s infinite; }

@keyframes pulse { 0%,100%{opacity:1} 50%{opacity:0.4} }

.body { padding: 1.25rem 1.5rem; }

.sec {
  font-family: var(--font-mono);
  font-size: 10px;
  letter-spacing: 0.12em;
  text-transform: uppercase;
  color: var(--text-3);
  margin-bottom: 0.65rem;
}

#net-list { display: flex; flex-direction: column; gap: 5px; margin-bottom: 1.25rem; }

.net {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 9px 11px;
  background: var(--surface2);
  border: 0.5px solid var(--border);
  border-radius: var(--radius);
  cursor: pointer;
  transition: border-color 0.15s, background 0.15s;
}
.net:hover  { border-color: var(--accent); }
.net.sel    { border-color: var(--accent); background: rgba(14,165,233,0.06); }
.net-name   { flex: 1; font-size: 13px; font-weight: 500; color: var(--text); }
.net-meta   { display: flex; align-items: center; gap: 7px; }
.lock       { font-size: 11px; color: var(--text-3); }
.bars       { display: flex; align-items: flex-end; gap: 2px; height: 13px; }
.bars span  { width: 3px; background: var(--accent); border-radius: 1px; }

.net-scan-placeholder {
  text-align: center;
  padding: 1.25rem;
  font-family: var(--font-mono);
  font-size: 11px;
  color: var(--text-3);
}

.div { height: 0.5px; background: var(--border); margin: 1.1rem 0; }

.fields { display: flex; flex-direction: column; gap: 10px; margin-bottom: 1.1rem; }
.field  { display: flex; flex-direction: column; gap: 4px; }

label {
  font-size: 11px;
  font-weight: 500;
  color: var(--text-2);
  letter-spacing: 0.03em;
}

input[type=text], input[type=password] {
  background: var(--surface2);
  border: 0.5px solid var(--border);
  border-radius: 4px;
  padding: 9px 12px;
  font-family: var(--font-sans);
  font-size: 13px;
  color: var(--text);
  outline: none;
  width: 100%;
  transition: border-color 0.15s;
}
input[type=text]:focus, input[type=password]:focus { border-color: var(--accent); }
input[type=text]::placeholder, input[type=password]::placeholder { color: var(--text-4); }

.check-row {
  display: flex; align-items: center; gap: 8px;
  font-size: 12px; color: var(--text-3);
  cursor: pointer;
  user-select: none;
}
.check-row input[type=checkbox] {
  width: 14px; height: 14px;
  accent-color: var(--accent);
  cursor: pointer;
}

.btn {
  width: 100%;
  padding: 11px;
  background: var(--accent);
  border: none;
  border-radius: 4px;
  font-family: var(--font-sans);
  font-size: 13px;
  font-weight: 500;
  color: #fff;
  cursor: pointer;
  letter-spacing: 0.02em;
  transition: background 0.15s, opacity 0.15s;
  margin-top: 0.25rem;
}
.btn:hover   { background: var(--accent-h); }
.btn:active  { opacity: 0.85; }
.btn:disabled { background: var(--border2); color: var(--text-3); cursor: default; }

#screen-status {
  display: none;
  flex-direction: column;
  align-items: center;
  text-align: center;
  padding: 2rem 1.5rem;
  gap: 12px;
}
.status-icon {
  width: 52px; height: 52px;
  border-radius: 12px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 24px;
  font-weight: 500;
}
.status-icon.ok   { background: rgba(34,197,94,0.12);  color: var(--success); }
.status-icon.err  { background: rgba(239,68,68,0.12);  color: var(--danger); }
.status-icon.spin { background: rgba(14,165,233,0.12); color: var(--accent); animation: spin 1s linear infinite; }

@keyframes spin { to { transform: rotate(360deg); } }

.status-title { font-size: 15px; font-weight: 500; }
.status-msg   { font-size: 12px; color: var(--text-3); font-family: var(--font-mono); }

.btn-ghost {
  background: transparent;
  border: 0.5px solid var(--border);
  border-radius: 4px;
  padding: 8px 20px;
  font-family: var(--font-sans);
  font-size: 12px;
  color: var(--text-3);
  cursor: pointer;
  margin-top: 4px;
  transition: color 0.15s, border-color 0.15s;
}
.btn-ghost:hover { color: var(--accent); border-color: var(--accent); }

.ft {
  padding: 0.6rem 1.5rem 1.1rem;
  display: flex;
  align-items: center;
  justify-content: space-between;
}
.ft-chip {
  font-family: var(--font-mono);
  font-size: 10px;
  color: var(--text-4);
  letter-spacing: 0.04em;
}
.btn-rescan {
  font-family: var(--font-mono);
  font-size: 10px;
  color: var(--text-3);
  background: transparent;
  border: 0.5px solid var(--border);
  border-radius: 3px;
  padding: 3px 8px;
  cursor: pointer;
  letter-spacing: 0.04em;
  transition: color 0.15s, border-color 0.15s;
}
.btn-rescan:hover { color: var(--accent); border-color: var(--accent); }
</style>
</head>
<body>
<div class="card">
  <div class="hd">
    <div class="hd-icon" id="sig-bars">
      <span style="height:6px"  id="b1"></span>
      <span style="height:10px" id="b2"></span>
      <span style="height:16px" id="b3"></span>
      <span style="height:22px" id="b4"></span>
    </div>
    <div class="hd-info">
      <div class="hd-title">WiFi Setup</div>
      <div class="hd-sub" id="hd-ip">WIFIPANEL &middot; PORTAL</div>
    </div>
    <div class="hd-dot" id="status-dot"></div>
  </div>
  <div id="screen-form">
    <div class="body">
      <div class="sec">Available networks</div>
      <div id="net-list">
        <div class="net-scan-placeholder" id="scan-placeholder">↻ Scanning...</div>
      </div>
      <div class="div"></div>
      <div class="sec">Connection details</div>
      <div class="fields">
        <div class="field">
          <label for="s">SSID</label>
          <input type="text" id="s" name="s" value="" placeholder="WiFi name" autocomplete="off" />
        </div>
        <div class="field">
          <label for="p">Password</label>
          <input type="password" id="p" name="p" value="" placeholder="••••••••" autocomplete="off" />
          <label class="check-row" style="margin-top:4px">
            <input type="checkbox" id="show-pw" onchange="togglePw(this)"> Show password
          </label>
        </div>
      </div>
      <button class="btn" id="btn-connect" onclick="doConnect()">Connect</button>
    </div>
    <div class="ft">
      <span class="ft-chip">WIFIPANEL PORTAL</span>
      <button class="btn-rescan" onclick="doRescan()">&#x21BB; Rescan</button>
    </div>
  </div>
  <div id="screen-status">
    <div class="status-icon spin" id="st-icon">&#9675;</div>
    <div class="status-title"    id="st-title">Connecting...</div>
    <div class="status-msg"      id="st-msg">Please wait</div>
    <button class="btn-ghost"    id="st-back" onclick="showForm()" style="display:none">&#8592; Back</button>
  </div>
</div>
<script>
)rawhtml";

static const char WP_PAGE_PART2[] = R"rawhtml(
var NETWORKS=(function(){
  if(window.wifiscandata&&Array.isArray(window.wifiscandata))return window.wifiscandata;
  return [];
})();
function rssiToBars(r){if(r>-55)return 4;if(r>-65)return 3;if(r>-75)return 2;return 1;}
function renderBars(n){var h=[5,9,13,17],s='';for(var i=0;i<4;i++)s+='<span style="height:'+h[i]+'px;opacity:'+(i<n?'1':'0.18')+'"></span>';return s;}
function escSSID(s){if(!s)return'';return s.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;').replace(/"/g,'&quot;').replace(/'/g,'&#39;');}
function buildNetList(){
  var list=document.getElementById('net-list');
  if(!NETWORKS||NETWORKS.length===0){list.innerHTML='<div class="net-scan-placeholder">No networks found</div>';return;}
  var sorted=NETWORKS.slice().sort(function(a,b){return b.rssi-a.rssi;});
  var html='';
  sorted.forEach(function(n,idx){
    var bars=rssiToBars(n.rssi);
    var lock=n.enc?'<span class="lock" title="Secured">&#128274;</span>':'';
    var safe=escSSID(n.ssid);
    html+='<div class="net'+(idx===0?' sel':'')+'" onclick="selectNet(this,\''+safe.replace(/'/g,"\\'")+'\')">'
      +'<span class="net-name">'+safe+'</span>'
      +'<div class="net-meta">'+lock+'<div class="bars">'+renderBars(bars)+'</div></div>'
      +'</div>';
  });
  list.innerHTML=html;
  if(sorted.length>0)document.getElementById('s').value=sorted[0].ssid;
}
function selectNet(el,ssid){document.querySelectorAll('.net').forEach(function(n){n.classList.remove('sel');});el.classList.add('sel');document.getElementById('s').value=ssid;document.getElementById('p').value='';document.getElementById('p').focus();}
function togglePw(cb){document.getElementById('p').type=cb.checked?'text':'password';}
function doRescan(){var b=document.querySelector('.btn-rescan');b.textContent='Scanning...';setTimeout(function(){window.location.href='/';},600);}
function showForm(){document.getElementById('screen-form').style.display='';document.getElementById('screen-status').style.display='none';setDot('idle');}
function setDot(s){var d=document.getElementById('status-dot');d.className='hd-dot'+(s==='ok'?' connected':s==='connecting'?' connecting':'');}
function doConnect(){
  var ssid=document.getElementById('s').value.trim();
  var pass=document.getElementById('p').value;
  if(!ssid){document.getElementById('s').focus();document.getElementById('s').style.borderColor='var(--danger)';return;}
  document.getElementById('screen-form').style.display='none';
  var ss=document.getElementById('screen-status');ss.style.display='flex';
  document.getElementById('st-icon').textContent='◌';document.getElementById('st-icon').className='status-icon spin';
  document.getElementById('st-title').textContent='Connecting...';
  document.getElementById('st-msg').textContent=ssid;
  document.getElementById('st-back').style.display='none';
  setDot('connecting');
  var fd=new FormData();fd.append('ssid',ssid);fd.append('password',pass);
  fetch('/',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},
    body:'ssid='+encodeURIComponent(ssid)+'&password='+encodeURIComponent(pass)})
    .then(function(){
      document.getElementById('st-icon').textContent='✓';document.getElementById('st-icon').className='status-icon ok';
      document.getElementById('st-title').textContent='Submitted!';
      document.getElementById('st-msg').textContent='Device is connecting to '+ssid;
      setDot('ok');
    }).catch(function(){
      document.getElementById('st-icon').textContent='✕';document.getElementById('st-icon').className='status-icon err';
      document.getElementById('st-title').textContent='Error';
      document.getElementById('st-msg').textContent='Could not reach device';
      document.getElementById('st-back').style.display='inline-block';
      setDot('idle');
    });
}
buildNetList();setDot('idle');
document.getElementById('s').addEventListener('input',function(){this.style.borderColor='';});
</script>
</body>
</html>
)rawhtml";

/* ============================================================
 * CONFIG_HTML
 * ============================================================ */
static const char CONFIG_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0">
<title>Config · Sharp Edge</title>
<style>
@import url('https://fonts.googleapis.com/css2?family=IBM+Plex+Mono:wght@400;500&family=DM+Sans:ital,wght@0,300;0,400;0,500;1,400&display=swap');

*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

:root {
  --bg:        #0a0e14;
  --surface:   #111820;
  --surface2:  #0d1520;
  --border:    #1e2d3d;
  --border2:   #2a3d52;
  --accent:    #0ea5e9;
  --accent-h:  #38bdf8;
  --text:      #e2eaf2;
  --text-2:    #7bafc8;
  --text-3:    #3d5470;
  --text-4:    #2a3d52;
  --success:   #22c55e;
  --warning:   #f97316;
  --danger:    #ef4444;
  --font-sans: 'DM Sans', system-ui, sans-serif;
  --font-mono: 'IBM Plex Mono', monospace;
  --radius:    3px;
  --radius-lg: 6px;
}

html, body {
  min-height: 100%;
  background: var(--bg);
  color: var(--text);
  font-family: var(--font-sans);
  font-size: 14px;
  line-height: 1.5;
  -webkit-font-smoothing: antialiased;
}

body {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: flex-start;
  padding: 1.5rem 1rem 3rem;
  min-height: 100vh;
}

.card {
  width: 100%;
  max-width: 420px;
  background: var(--surface);
  border: 0.5px solid var(--border);
  border-radius: var(--radius-lg);
  overflow: hidden;
}

.hd {
  background: var(--surface2);
  border-bottom: 0.5px solid var(--border);
  padding: 1.25rem 1.5rem 1rem;
  display: flex;
  align-items: center;
  gap: 12px;
}
.hd-icon {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 32px;
  height: 32px;
}
.hd-icon svg { opacity: 0.85; }
.hd-info { flex: 1; }
.hd-title { font-size: 15px; font-weight: 500; color: var(--text); letter-spacing: 0.01em; }
.hd-sub {
  font-family: var(--font-mono);
  font-size: 10px;
  color: var(--text-3);
  margin-top: 2px;
}
.hd-dot {
  width: 8px; height: 8px;
  border-radius: 2px;
  background: var(--warning);
  flex-shrink: 0;
  transition: background 0.4s, box-shadow 0.4s;
}
.hd-dot.saved { background: var(--success); box-shadow: 0 0 6px var(--success); }
@keyframes pulse { 0%,100%{opacity:1} 50%{opacity:0.4} }

.body { padding: 1.25rem 1.5rem; }

.sec {
  font-family: var(--font-mono);
  font-size: 10px;
  letter-spacing: 0.12em;
  text-transform: uppercase;
  color: var(--text-3);
  margin-bottom: 0.65rem;
}

.div { height: 0.5px; background: var(--border); margin: 1.1rem 0; }

#field-list {
  display: flex;
  flex-direction: column;
  gap: 6px;
  margin-bottom: 0.75rem;
}

.field-row {
  background: var(--surface2);
  border: 0.5px solid var(--border);
  border-radius: var(--radius);
  overflow: hidden;
  transition: border-color 0.15s;
}
.field-row:focus-within { border-color: var(--border2); }

.field-header {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 8px 10px;
  cursor: pointer;
  user-select: none;
}
.field-key-preview {
  flex: 1;
  font-family: var(--font-mono);
  font-size: 11px;
  color: var(--text-2);
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}
.field-key-preview .key  { color: var(--accent); }
.field-key-preview .sep  { color: var(--text-4); margin: 0 4px; }
.field-key-preview .type-badge {
  font-size: 9px;
  letter-spacing: 0.06em;
  padding: 1px 5px;
  border-radius: 2px;
  background: rgba(14,165,233,0.12);
  color: var(--accent);
  text-transform: uppercase;
}
.field-key-preview .type-badge.str  { background: rgba(34,197,94,0.12);  color: var(--success); }
.field-key-preview .type-badge.int  { background: rgba(14,165,233,0.12); color: var(--accent); }
.field-key-preview .type-badge.flt  { background: rgba(249,115,22,0.12); color: var(--warning); }
.field-key-preview .type-badge.bool { background: rgba(168,85,247,0.12); color: #a855f7; }
.field-key-preview .type-badge.pass { background: rgba(239,68,68,0.12);  color: var(--danger); }
.field-key-preview .type-badge.sel  { background: rgba(251,191,36,0.12); color: #fbbf24; }
.field-key-preview .val-hint { color: var(--text-3); font-size: 10px; }

.field-actions { display: flex; align-items: center; gap: 4px; }
.icon-btn {
  background: transparent;
  border: none;
  color: var(--text-3);
  cursor: pointer;
  font-size: 13px;
  padding: 2px 4px;
  border-radius: 2px;
  line-height: 1;
  transition: color 0.12s, background 0.12s;
}
.icon-btn:hover { color: var(--text-2); background: rgba(255,255,255,0.04); }
.icon-btn.del:hover { color: var(--danger); }
.chevron {
  font-size: 11px;
  color: var(--text-4);
  transition: transform 0.15s;
  display: inline-block;
}
.field-row.open .chevron { transform: rotate(90deg); }

.field-body {
  display: none;
  padding: 8px 10px 10px;
  border-top: 0.5px solid var(--border);
  background: rgba(0,0,0,0.12);
}
.field-row.open .field-body { display: block; }

.mini-grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 7px;
  margin-bottom: 7px;
}
.mini-grid.full { grid-template-columns: 1fr; }

.mini-field { display: flex; flex-direction: column; gap: 3px; }
.mini-label {
  font-family: var(--font-mono);
  font-size: 9px;
  letter-spacing: 0.08em;
  text-transform: uppercase;
  color: var(--text-3);
}
.mini-input {
  background: var(--surface);
  border: 0.5px solid var(--border);
  border-radius: 3px;
  padding: 6px 8px;
  font-family: var(--font-mono);
  font-size: 11px;
  color: var(--text);
  outline: none;
  width: 100%;
  transition: border-color 0.15s;
}
.mini-input:focus { border-color: var(--accent); }
.mini-input::placeholder { color: var(--text-4); }

.mini-select {
  background: var(--surface);
  border: 0.5px solid var(--border);
  border-radius: 3px;
  padding: 6px 8px;
  font-family: var(--font-mono);
  font-size: 11px;
  color: var(--text-2);
  outline: none;
  width: 100%;
  cursor: pointer;
  appearance: none;
  transition: border-color 0.15s;
}
.mini-select:focus { border-color: var(--accent); }

.btn-add {
  width: 100%;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 7px;
  padding: 9px;
  background: transparent;
  border: 0.5px dashed var(--border2);
  border-radius: var(--radius);
  font-family: var(--font-mono);
  font-size: 11px;
  color: var(--text-3);
  letter-spacing: 0.06em;
  cursor: pointer;
  transition: border-color 0.15s, color 0.15s, background 0.15s;
}
.btn-add:hover {
  border-color: var(--accent);
  color: var(--accent);
  background: rgba(14,165,233,0.04);
}

.fields { display: flex; flex-direction: column; gap: 8px; margin-bottom: 1.1rem; }

.dyn-field { display: flex; flex-direction: column; gap: 4px; }
.dyn-label {
  font-size: 11px;
  font-weight: 500;
  color: var(--text-2);
  display: flex;
  align-items: center;
  gap: 6px;
}
.dyn-label .type-tag {
  font-family: var(--font-mono);
  font-size: 9px;
  letter-spacing: 0.06em;
  padding: 1px 5px;
  border-radius: 2px;
  text-transform: uppercase;
}
.dyn-label .hint { font-size: 10px; color: var(--text-4); font-weight: 400; margin-left: auto; font-family: var(--font-mono); }

input.cfg-input, textarea.cfg-input {
  background: var(--surface2);
  border: 0.5px solid var(--border);
  border-radius: 4px;
  padding: 9px 12px;
  font-family: var(--font-sans);
  font-size: 13px;
  color: var(--text);
  outline: none;
  width: 100%;
  transition: border-color 0.15s;
}
input.cfg-input:focus, textarea.cfg-input:focus { border-color: var(--accent); }
input.cfg-input::placeholder { color: var(--text-4); }
textarea.cfg-input { resize: vertical; min-height: 64px; font-family: var(--font-mono); font-size: 12px; }

.toggle-row {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 9px 12px;
  background: var(--surface2);
  border: 0.5px solid var(--border);
  border-radius: 4px;
}
.toggle-label { flex: 1; font-size: 13px; color: var(--text-3); }
.toggle-switch {
  position: relative;
  width: 34px;
  height: 18px;
  cursor: pointer;
  flex-shrink: 0;
}
.toggle-switch input { opacity: 0; width: 0; height: 0; }
.toggle-track {
  position: absolute;
  inset: 0;
  background: var(--border2);
  border-radius: 9px;
  transition: background 0.2s;
}
.toggle-switch input:checked + .toggle-track { background: var(--accent); }
.toggle-thumb {
  position: absolute;
  top: 2px; left: 2px;
  width: 14px; height: 14px;
  background: white;
  border-radius: 50%;
  transition: transform 0.2s;
}
.toggle-switch input:checked ~ .toggle-thumb { transform: translateX(16px); }

.btn {
  width: 100%;
  padding: 11px;
  background: var(--accent);
  border: none;
  border-radius: 4px;
  font-family: var(--font-sans);
  font-size: 13px;
  font-weight: 500;
  color: #fff;
  cursor: pointer;
  letter-spacing: 0.02em;
  transition: background 0.15s, opacity 0.15s;
  margin-top: 0.25rem;
}
.btn:hover  { background: var(--accent-h); }
.btn:active { opacity: 0.85; }

.btn-row { display: flex; gap: 8px; }
.btn.secondary {
  background: transparent;
  border: 0.5px solid var(--border);
  color: var(--text-3);
  flex: 0 0 auto;
  width: auto;
  padding: 11px 16px;
}
.btn.secondary:hover { border-color: var(--accent); color: var(--accent); background: rgba(14,165,233,0.05); }

.toast {
  position: fixed;
  bottom: 1.5rem;
  left: 50%;
  transform: translateX(-50%) translateY(8px);
  background: var(--surface);
  border: 0.5px solid var(--border2);
  border-radius: var(--radius);
  padding: 8px 14px;
  font-family: var(--font-mono);
  font-size: 11px;
  color: var(--text-2);
  opacity: 0;
  pointer-events: none;
  transition: opacity 0.2s, transform 0.2s;
  z-index: 100;
  white-space: nowrap;
  box-shadow: 0 4px 20px rgba(0,0,0,0.4);
}
.toast.show { opacity: 1; transform: translateX(-50%) translateY(0); }
.toast.ok  { border-color: rgba(34,197,94,0.4); color: var(--success); }
.toast.err { border-color: rgba(239,68,68,0.4); color: var(--danger); }

.empty-state {
  text-align: center;
  padding: 1.5rem 1rem;
  font-family: var(--font-mono);
  font-size: 11px;
  color: var(--text-4);
  line-height: 1.8;
  display: none;
}

.tab-strip {
  display: flex;
  gap: 0;
  border: 0.5px solid var(--border);
  border-radius: var(--radius);
  overflow: hidden;
  margin-bottom: 1.25rem;
}
.tab {
  flex: 1;
  padding: 7px 10px;
  background: var(--surface2);
  border: none;
  font-family: var(--font-mono);
  font-size: 10px;
  letter-spacing: 0.08em;
  text-transform: uppercase;
  color: var(--text-3);
  cursor: pointer;
  transition: background 0.15s, color 0.15s;
}
.tab + .tab { border-left: 0.5px solid var(--border); }
.tab.active { background: rgba(14,165,233,0.1); color: var(--accent); }

#screen-status {
  display: none;
  flex-direction: column;
  align-items: center;
  text-align: center;
  padding: 2rem 1.5rem;
  gap: 12px;
}
.status-icon {
  width: 52px; height: 52px;
  border-radius: 12px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 24px;
  font-weight: 500;
}
.status-icon.ok  { background: rgba(34,197,94,0.12);  color: var(--success); }
.status-icon.err { background: rgba(239,68,68,0.12);  color: var(--danger); }
.status-title { font-size: 15px; font-weight: 500; }
.status-msg   { font-size: 12px; color: var(--text-3); font-family: var(--font-mono); }
.btn-ghost {
  background: transparent;
  border: 0.5px solid var(--border);
  border-radius: 4px;
  padding: 8px 20px;
  font-family: var(--font-sans);
  font-size: 12px;
  color: var(--text-3);
  cursor: pointer;
  margin-top: 4px;
  transition: color 0.15s, border-color 0.15s;
}
.btn-ghost:hover { color: var(--accent); border-color: var(--accent); }

.ft {
  padding: 0.6rem 1.5rem 1.1rem;
  display: flex;
  align-items: center;
  justify-content: space-between;
}
.ft-chip {
  font-family: var(--font-mono);
  font-size: 10px;
  color: var(--text-4);
  letter-spacing: 0.04em;
}
.ft-count {
  font-family: var(--font-mono);
  font-size: 10px;
  color: var(--text-3);
}
</style>
</head>
<body>

<div class="card">

  <div class="hd">
    <div class="hd-icon">
      <svg width="22" height="22" viewBox="0 0 22 22" fill="none">
        <rect x="3" y="3" width="6" height="6" rx="1" stroke="#0ea5e9" stroke-width="1.2"/>
        <rect x="13" y="3" width="6" height="6" rx="1" stroke="#0ea5e9" stroke-width="1.2" opacity=".5"/>
        <rect x="3" y="13" width="6" height="6" rx="1" stroke="#0ea5e9" stroke-width="1.2" opacity=".5"/>
        <rect x="13" y="13" width="6" height="6" rx="1" stroke="#0ea5e9" stroke-width="1.2" opacity=".35"/>
      </svg>
    </div>
    <div class="hd-info">
      <div class="hd-title">Config</div>
      <div class="hd-sub" id="hd-sub">WIFIPANEL · PARAM</div>
    </div>
    <div class="hd-dot" id="status-dot"></div>
  </div>

  <div id="screen-form">
    <div class="body">

      <div class="tab-strip">
        <button class="tab active" id="tab-define" onclick="switchTab('define')">Define Fields</button>
        <button class="tab"        id="tab-fill"   onclick="switchTab('fill')">Fill Values</button>
      </div>

      <div id="pane-define">
        <div class="sec">Parameter schema</div>

        <div id="field-list"></div>
        <div class="empty-state" id="empty-state">
          No fields defined yet.<br>Press + to add a parameter.
        </div>

        <button class="btn-add" onclick="addField()">
          <span style="font-size:15px;line-height:1">+</span>
          ADD PARAMETER
        </button>
      </div>

      <div id="pane-fill" style="display:none">
        <div class="sec">Parameter values</div>
        <div class="fields" id="value-form"></div>
        <div class="empty-state" id="fill-empty" style="display:block">
          No fields defined.<br>Switch to <em>Define Fields</em> to add parameters.
        </div>

        <div class="btn-row">
          <button class="btn secondary" onclick="doReset()">Reset</button>
          <button class="btn" onclick="doSave()">Save &amp; Apply</button>
        </div>
      </div>

    </div>

    <div class="ft">
      <span class="ft-chip">WIFIPANEL PORTAL</span>
      <span class="ft-count" id="ft-count">0 params</span>
    </div>
  </div>

  <div id="screen-status">
    <div class="status-icon ok" id="st-icon">&#10003;</div>
    <div class="status-title"   id="st-title">Saved!</div>
    <div class="status-msg"     id="st-msg">Configuration applied</div>
    <button class="btn-ghost" onclick="showForm()">&#8592; Back</button>
  </div>

</div>

<div class="toast" id="toast"></div>

<script>
var TYPE_META = {
  str:  { label:'String',   badge:'STR',  badgeClass:'str',  placeholder:'text value' },
  int:  { label:'Integer',  badge:'INT',  badgeClass:'int',  placeholder:'0' },
  flt:  { label:'Float',    badge:'FLT',  badgeClass:'flt',  placeholder:'0.0' },
  bool: { label:'Boolean',  badge:'BOOL', badgeClass:'bool', placeholder:'' },
  pass: { label:'Password', badge:'PASS', badgeClass:'pass', placeholder:'••••••••' },
  sel:  { label:'Select',   badge:'SEL',  badgeClass:'sel',  placeholder:'opt1,opt2' },
};

var schema = [];
var values = {};
var uid = 0;

(function(){
  if(window.wifiparam_schema && Array.isArray(window.wifiparam_schema)){
    schema = window.wifiparam_schema.map(function(f){
      return Object.assign({ _id: uid++ }, f);
    });
  }
  if(window.wifiparam_values && typeof window.wifiparam_values === 'object'){
    values = Object.assign({}, window.wifiparam_values);
  } else {
    schema.forEach(function(f){ if(f.default !== undefined) values[f.key] = f.default; });
  }
  renderDefine();
})();

function showForm(){
  document.getElementById('screen-form').style.display   = '';
  document.getElementById('screen-status').style.display = 'none';
  document.getElementById('status-dot').className = 'hd-dot';
}

function showStatus(ok, title, msg){
  document.getElementById('screen-form').style.display   = 'none';
  document.getElementById('screen-status').style.display = 'flex';
  document.getElementById('st-icon').className  = 'status-icon ' + (ok ? 'ok' : 'err');
  document.getElementById('st-icon').innerHTML  = ok ? '&#10003;' : '&#10005;';
  document.getElementById('st-title').textContent = title;
  document.getElementById('st-msg').textContent   = msg;
  document.getElementById('status-dot').className = ok ? 'hd-dot saved' : 'hd-dot';
}

function switchTab(t){
  document.getElementById('tab-define').classList.toggle('active', t==='define');
  document.getElementById('tab-fill').classList.toggle('active',   t==='fill');
  document.getElementById('pane-define').style.display = t==='define' ? '' : 'none';
  document.getElementById('pane-fill').style.display   = t==='fill'   ? '' : 'none';
  if(t==='fill') renderFill();
}

function addField(tpl){
  var f = tpl || { _id: uid++, key:'', type:'str', label:'', default:'', desc:'' };
  schema.push(f);
  renderDefine();
  var rows = document.querySelectorAll('.field-row');
  var last = rows[rows.length-1];
  if(last){ last.classList.add('open'); last.querySelector('.mini-input').focus(); }
  updateCount();
}

function removeField(id){
  schema = schema.filter(function(f){ return f._id !== id; });
  renderDefine();
  updateCount();
}

function renderDefine(){
  var list  = document.getElementById('field-list');
  var empty = document.getElementById('empty-state');
  if(schema.length === 0){
    list.innerHTML = '';
    empty.style.display = 'block';
    updateCount();
    return;
  }
  empty.style.display = 'none';
  list.innerHTML = schema.map(function(f){ return buildFieldRow(f); }).join('');

  schema.forEach(function(f){
    var row = document.getElementById('fr-' + f._id);
    if(!row) return;

    row.querySelector('.field-header').addEventListener('click', function(e){
      if(e.target.closest('.icon-btn')) return;
      row.classList.toggle('open');
    });

    ['key','type','default'].forEach(function(attr){
      var inp = row.querySelector('[data-attr='+attr+']');
      if(inp) inp.addEventListener('input', function(){
        f[attr] = inp.value;
        updatePreview(row, f);
      });
    });
    ['label','desc'].forEach(function(attr){
      var inp = row.querySelector('[data-attr='+attr+']');
      if(inp) inp.addEventListener('input', function(){ f[attr] = inp.value; });
    });

    row.querySelector('.del').addEventListener('click', function(e){
      e.stopPropagation();
      removeField(f._id);
    });
  });
  updateCount();
}

function buildFieldRow(f){
  var tm    = TYPE_META[f.type] || TYPE_META.str;
  var badge = '<span class="type-badge '+tm.badgeClass+'">'+tm.badge+'</span>';
  var valHint = f.default ? ' <span class="val-hint">= '+esc(f.default)+'</span>' : '';

  return '<div class="field-row" id="fr-'+f._id+'">'
    + '<div class="field-header">'
    +   '<span class="chevron">&#8250;</span>'
    +   '<span class="key-preview field-key-preview">'
    +     '<span class="key">'+esc(f.key||'...')+'</span>'
    +     '<span class="sep">&#183;</span>'
    +     badge + valHint
    +   '</span>'
    +   '<div class="field-actions">'
    +     '<button class="icon-btn del" title="Remove">&#10005;</button>'
    +   '</div>'
    + '</div>'
    + '<div class="field-body">'
    +   '<div class="mini-grid">'
    +     '<div class="mini-field"><span class="mini-label">Key</span>'
    +       '<input class="mini-input" data-attr="key" value="'+esc(f.key)+'" placeholder="param_key" spellcheck="false" autocomplete="off"/>'
    +     '</div>'
    +     '<div class="mini-field"><span class="mini-label">Type</span>'
    +       '<select class="mini-select" data-attr="type">'
    +         Object.keys(TYPE_META).map(function(t){
                return '<option value="'+t+'"'+(f.type===t?' selected':'')+'>'+TYPE_META[t].label+'</option>';
              }).join('')
    +       '</select>'
    +     '</div>'
    +     '<div class="mini-field"><span class="mini-label">Label</span>'
    +       '<input class="mini-input" data-attr="label" value="'+esc(f.label)+'" placeholder="Human label" autocomplete="off"/>'
    +     '</div>'
    +     '<div class="mini-field"><span class="mini-label">Default</span>'
    +       '<input class="mini-input" data-attr="default" value="'+esc(f.default)+'" placeholder="'+esc(tm.placeholder)+'" autocomplete="off"/>'
    +     '</div>'
    +   '</div>'
    +   '<div class="mini-grid full">'
    +     '<div class="mini-field"><span class="mini-label">Description</span>'
    +       '<input class="mini-input" data-attr="desc" value="'+esc(f.desc||'')+'" placeholder="Short description (optional)" autocomplete="off"/>'
    +     '</div>'
    +   '</div>'
    + '</div>'
  + '</div>';
}

function updatePreview(row, f){
  var tm = TYPE_META[f.type] || TYPE_META.str;
  var preview = row.querySelector('.key-preview');
  var valHint = f.default ? ' <span class="val-hint">= '+esc(f.default)+'</span>' : '';
  preview.innerHTML = '<span class="key">'+esc(f.key||'...')+'</span>'
    + '<span class="sep">&#183;</span>'
    + '<span class="type-badge '+tm.badgeClass+'">'+tm.badge+'</span>'
    + valHint;
}

function renderFill(){
  var form  = document.getElementById('value-form');
  var empty = document.getElementById('fill-empty');
  if(schema.length === 0){
    form.innerHTML = '';
    empty.style.display = 'block';
    return;
  }
  empty.style.display = 'none';

  form.innerHTML = schema.map(function(f){
    var tm     = TYPE_META[f.type] || TYPE_META.str;
    var curVal = (values[f.key] !== undefined) ? values[f.key] : (f.default || '');
    var labelStr = f.label || f.key;
    var typeTag  = '<span class="type-tag '+tm.badgeClass+'" style="background:'+typeBg(f.type)+';color:'+typeColor(f.type)+'">'+tm.badge+'</span>';
    var hintStr  = f.desc ? '<span class="hint">'+esc(f.desc)+'</span>' : '';

    var input = '';
    if(f.type === 'bool'){
      var checked = (curVal === '1' || curVal === 'true') ? 'checked' : '';
      input = '<div class="toggle-row">'
        + '<span class="toggle-label">'+(f.desc||'Enable / Disable')+'</span>'
        + '<label class="toggle-switch">'
        +   '<input type="checkbox" id="v-'+esc(f.key)+'" '+checked+'>'
        +   '<div class="toggle-track"></div>'
        +   '<div class="toggle-thumb"></div>'
        + '</label>'
        + '</div>';
    } else if(f.type === 'pass'){
      input = '<input class="cfg-input" type="password" id="v-'+esc(f.key)+'" value="'+esc(curVal)+'" placeholder="'+esc(tm.placeholder)+'" autocomplete="new-password">';
    } else if(f.type === 'sel'){
      var opts = (f.desc || '').split(',');
      input = '<select class="cfg-input" id="v-'+esc(f.key)+'">'
        + opts.map(function(o){
            o = o.trim();
            return '<option value="'+esc(o)+'"'+(curVal===o?' selected':'')+'>'+esc(o)+'</option>';
          }).join('')
        + '</select>';
    } else {
      input = '<input class="cfg-input" type="text" id="v-'+esc(f.key)+'" value="'+esc(curVal)+'" placeholder="'+esc(f.default||tm.placeholder)+'" autocomplete="off" inputmode="'+(f.type==='int'||f.type==='flt'?'decimal':'text')+'">';
    }

    return '<div class="dyn-field">'
      + '<div class="dyn-label">'+esc(labelStr)+' '+typeTag+hintStr+'</div>'
      + input
      + '</div>';
  }).join('');
}

function typeBg(t){
  return {str:'rgba(34,197,94,0.12)',int:'rgba(14,165,233,0.12)',flt:'rgba(249,115,22,0.12)',bool:'rgba(168,85,247,0.12)',pass:'rgba(239,68,68,0.12)',sel:'rgba(251,191,36,0.12)'}[t]||'rgba(14,165,233,0.12)';
}
function typeColor(t){
  return {str:'#22c55e',int:'#0ea5e9',flt:'#f97316',bool:'#a855f7',pass:'#ef4444',sel:'#fbbf24'}[t]||'#0ea5e9';
}

function doSave(){
  schema.forEach(function(f){
    var el = document.getElementById('v-'+f.key);
    if(!el) return;
    if(f.type === 'bool') values[f.key] = el.checked ? '1' : '0';
    else values[f.key] = el.value;
  });

  var errors = [];
  schema.forEach(function(f){
    var v = values[f.key] || '';
    if(f.type === 'int' && v !== '' && !/^-?\d+$/.test(v.trim()))
      errors.push('"'+(f.label||f.key)+'" must be an integer');
    if(f.type === 'flt' && v !== '' && isNaN(Number(v.trim())))
      errors.push('"'+(f.label||f.key)+'" must be a number');
  });

  if(errors.length > 0){
    showToast(errors[0], 'err');
    return;
  }

  var body = Object.keys(values).map(function(k){
    return encodeURIComponent(k) + '=' + encodeURIComponent(values[k]);
  }).join('&');

  var btn = document.querySelector('#pane-fill .btn:not(.secondary)');
  if(btn){ btn.disabled = true; btn.textContent = 'Saving...'; }

  fetch('/configsave', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: body
  }).then(function(r){
    if(btn){ btn.disabled = false; btn.textContent = 'Save & Apply'; }
    if(r.ok){
      showStatus(true, 'Saved!', 'Configuration updated successfully');
    } else {
      showToast('Save failed (' + r.status + ')', 'err');
    }
  }).catch(function(e){
    if(btn){ btn.disabled = false; btn.textContent = 'Save & Apply'; }
    showToast('Network error: ' + e.message, 'err');
  });
}

function doReset(){
  if(!confirm('Reset values to defaults?')) return;
  values = {};
  schema.forEach(function(f){ if(f.default !== undefined) values[f.key] = f.default; });
  renderFill();
  showToast('Values reset to defaults');
}

function updateCount(){
  var n = schema.length;
  document.getElementById('ft-count').textContent = n + ' param' + (n!==1?'s':'');
}

function showToast(msg, type){
  var t = document.getElementById('toast');
  t.textContent = msg;
  t.className   = 'toast show ' + (type||'');
  setTimeout(function(){ t.className = 'toast'; }, 2600);
}

function esc(s){
  if(!s) return '';
  return String(s).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;').replace(/"/g,'&quot;').replace(/'/g,'&#39;');
}
</script>
</body>
</html>
)rawliteral";

/* ============================================================
 * OTA_HTML
 * ============================================================ */
static const char OTA_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0">
<title>OTA Update · Sharp Edge</title>
<style>
@import url('https://fonts.googleapis.com/css2?family=IBM+Plex+Mono:wght@400;500&family=DM+Sans:ital,wght@0,300;0,400;0,500;1,400&display=swap');

*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

:root {
  --bg:        #0a0e14;
  --surface:   #111820;
  --surface2:  #0d1520;
  --border:    #1e2d3d;
  --border2:   #2a3d52;
  --accent:    #0ea5e9;
  --accent-h:  #38bdf8;
  --text:      #e2eaf2;
  --text-2:    #7bafc8;
  --text-3:    #3d5470;
  --text-4:    #2a3d52;
  --success:   #22c55e;
  --warning:   #f97316;
  --danger:    #ef4444;
  --font-sans: 'DM Sans', system-ui, sans-serif;
  --font-mono: 'IBM Plex Mono', monospace;
  --radius:    3px;
  --radius-lg: 6px;
}

html, body {
  min-height: 100%;
  background: var(--bg);
  color: var(--text);
  font-family: var(--font-sans);
  font-size: 14px;
  line-height: 1.5;
  -webkit-font-smoothing: antialiased;
}

body {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: flex-start;
  padding: 1.5rem 1rem 3rem;
  min-height: 100vh;
}

.card {
  width: 100%;
  max-width: 420px;
  background: var(--surface);
  border: 0.5px solid var(--border);
  border-radius: var(--radius-lg);
  overflow: hidden;
}

.hd {
  background: var(--surface2);
  border-bottom: 0.5px solid var(--border);
  padding: 1.25rem 1.5rem 1rem;
  display: flex;
  align-items: center;
  gap: 12px;
}
.hd-icon {
  display: flex;
  align-items: flex-end;
  justify-content: center;
  gap: 3px;
  padding-bottom: 3px;
  height: 32px;
}
.hd-icon span { width: 5px; background: var(--accent); border-radius: 1px; }
.hd-info { flex: 1; }
.hd-title { font-size: 15px; font-weight: 500; color: var(--text); letter-spacing: 0.01em; }
.hd-sub { font-family: var(--font-mono); font-size: 10px; color: var(--text-3); margin-top: 2px; }
.hd-dot { width: 8px; height: 8px; border-radius: 2px; flex-shrink: 0; }
.hd-dot.warn { background: var(--warning); box-shadow: 0 0 6px var(--warning); }

.body { padding: 1.25rem 1.5rem; }

.sec {
  font-family: var(--font-mono);
  font-size: 10px;
  letter-spacing: 0.12em;
  text-transform: uppercase;
  color: var(--text-3);
  margin-bottom: 0.65rem;
}

.div { height: 0.5px; background: var(--border); margin: 1.1rem 0; }

.info-table { display: flex; flex-direction: column; gap: 6px; margin-bottom: 1.25rem; }
.info-row {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 7px 10px;
  background: var(--surface2);
  border: 0.5px solid var(--border);
  border-radius: var(--radius);
}
.info-key { font-family: var(--font-mono); font-size: 10px; color: var(--text-3); letter-spacing: 0.06em; text-transform: uppercase; }
.info-val { font-family: var(--font-mono); font-size: 11px; color: var(--text-2); }
.info-val.ok { color: var(--success); }

.dropzone {
  border: 1px dashed var(--border2);
  border-radius: var(--radius-lg);
  padding: 2rem 1rem;
  text-align: center;
  cursor: pointer;
  transition: border-color 0.15s, background 0.15s;
  background: var(--surface2);
  margin-bottom: 1rem;
  position: relative;
}
.dropzone:hover, .dropzone.drag { border-color: var(--accent); background: rgba(14,165,233,0.05); }
.dropzone input[type=file] { position: absolute; inset: 0; opacity: 0; cursor: pointer; width: 100%; height: 100%; }
.dz-icon { font-size: 28px; margin-bottom: 8px; color: var(--text-3); }
.dz-label { font-size: 13px; color: var(--text-2); font-weight: 500; }
.dz-hint { font-family: var(--font-mono); font-size: 10px; color: var(--text-3); margin-top: 4px; }

.file-info {
  display: none;
  align-items: center;
  gap: 10px;
  padding: 10px 12px;
  background: var(--surface2);
  border: 0.5px solid var(--border);
  border-radius: var(--radius);
  margin-bottom: 1rem;
}
.file-info.show { display: flex; }
.fi-icon { font-size: 18px; color: var(--accent); }
.fi-name { font-family: var(--font-mono); font-size: 11px; color: var(--text); }
.fi-size { font-family: var(--font-mono); font-size: 10px; color: var(--text-3); margin-top: 2px; }
.fi-remove { margin-left: auto; font-size: 16px; color: var(--text-3); cursor: pointer; line-height: 1; padding: 2px 4px; }
.fi-remove:hover { color: var(--danger); }

.btn-flash {
  width: 100%;
  padding: 11px;
  border-radius: var(--radius);
  background: var(--accent);
  border: none;
  color: #fff;
  font-family: var(--font-sans);
  font-size: 13px;
  font-weight: 500;
  cursor: pointer;
  letter-spacing: 0.03em;
  transition: background 0.15s, opacity 0.15s;
}
.btn-flash:hover:not(:disabled) { background: var(--accent-h); }
.btn-flash:disabled { opacity: 0.4; cursor: not-allowed; }

.progress-wrap { display: none; margin-top: 1rem; }
.progress-wrap.show { display: block; }
.progress-bar-bg { height: 4px; background: var(--border); border-radius: 2px; overflow: hidden; margin-bottom: 6px; }
.progress-bar-fill { height: 100%; width: 0%; background: var(--accent); border-radius: 2px; transition: width 0.2s ease; }
.progress-label { display: flex; justify-content: space-between; font-family: var(--font-mono); font-size: 10px; color: var(--text-3); }

.log-box {
  margin-top: 1rem;
  background: var(--surface2);
  border: 0.5px solid var(--border);
  border-radius: var(--radius);
  padding: 10px 12px;
  font-family: var(--font-mono);
  font-size: 10px;
  color: var(--text-3);
  line-height: 1.8;
  max-height: 120px;
  overflow-y: auto;
  display: none;
}
.log-box.show { display: block; }
.log-line { color: var(--text-3); }
.log-line.ok   { color: var(--success); }
.log-line.err  { color: var(--danger); }
.log-line.info { color: var(--accent); }

.result-box { display: none; margin-top: 1rem; padding: 12px; border-radius: var(--radius); text-align: center; font-size: 13px; font-weight: 500; }
.result-box.ok  { display: block; background: rgba(34,197,94,0.08);  border: 0.5px solid rgba(34,197,94,0.3);  color: var(--success); }
.result-box.err { display: block; background: rgba(239,68,68,0.08);  border: 0.5px solid rgba(239,68,68,0.3);  color: var(--danger); }

.ft { padding: 0.6rem 1.5rem 1.1rem; display: flex; align-items: center; justify-content: space-between; }
.ft-chip { font-family: var(--font-mono); font-size: 10px; color: var(--text-4); letter-spacing: 0.04em; }
.back-link { font-family: var(--font-mono); font-size: 10px; color: var(--text-3); text-decoration: none; letter-spacing: 0.04em; }
.back-link:hover { color: var(--accent); }
</style>
</head>
<body>

<div class="card">

  <div class="hd">
    <div class="hd-icon">
      <span style="height:6px"></span>
      <span style="height:10px"></span>
      <span style="height:16px"></span>
      <span style="height:22px"></span>
    </div>
    <div class="hd-info">
      <div class="hd-title">OTA Update</div>
      <div class="hd-sub">SHARP EDGE &middot; FIRMWARE FLASH</div>
    </div>
    <div class="hd-dot warn"></div>
  </div>

  <div class="body">

    <div class="sec">Current firmware</div>
    <div class="info-table">
      <div class="info-row">
        <span class="info-key">Version</span>
        <span class="info-val ok" id="cur-ver">%FW_VERSION%</span>
      </div>
      <div class="info-row">
        <span class="info-key">Build</span>
        <span class="info-val">%FW_BUILD%</span>
      </div>
      <div class="info-row">
        <span class="info-key">Partition</span>
        <span class="info-val">%FW_PARTITION%</span>
      </div>
      <div class="info-row">
        <span class="info-key">Flash free</span>
        <span class="info-val">%FW_FLASHFREE%</span>
      </div>
    </div>

    <div class="div"></div>

    <div class="sec">Upload firmware (.bin)</div>

    <div class="dropzone" id="dropzone">
      <input type="file" id="file-input" accept=".bin">
      <div class="dz-icon">&#11014;</div>
      <div class="dz-label">Drop .bin file here</div>
      <div class="dz-hint">or click to browse</div>
    </div>

    <div class="file-info" id="file-info">
      <span class="fi-icon">&#128230;</span>
      <div>
        <div class="fi-name" id="fi-name">firmware.bin</div>
        <div class="fi-size" id="fi-size">0 KB</div>
      </div>
      <span class="fi-remove" id="fi-remove" title="Remove">&#10005;</span>
    </div>

    <button class="btn-flash" id="btn-flash" disabled>Flash firmware</button>

    <div class="progress-wrap" id="progress-wrap">
      <div class="progress-bar-bg">
        <div class="progress-bar-fill" id="progress-fill"></div>
      </div>
      <div class="progress-label">
        <span id="progress-status">Uploading...</span>
        <span id="progress-pct">0%</span>
      </div>
    </div>

    <div class="log-box" id="log-box"></div>
    <div class="result-box" id="result-box"></div>

  </div>

  <div class="ft">
    <span class="ft-chip">WIFIMANAGER PORTAL</span>
    <a href="/" class="back-link">&#8592; back</a>
  </div>

</div>

<script>
(function(){
  var fileInput      = document.getElementById('file-input');
  var dropzone       = document.getElementById('dropzone');
  var fileInfo       = document.getElementById('file-info');
  var fiName         = document.getElementById('fi-name');
  var fiSize         = document.getElementById('fi-size');
  var fiRemove       = document.getElementById('fi-remove');
  var btnFlash       = document.getElementById('btn-flash');
  var progressWrap   = document.getElementById('progress-wrap');
  var progressFill   = document.getElementById('progress-fill');
  var progressStatus = document.getElementById('progress-status');
  var progressPct    = document.getElementById('progress-pct');
  var logBox         = document.getElementById('log-box');
  var resultBox      = document.getElementById('result-box');
  var selectedFile   = null;

  function formatSize(bytes) {
    if (bytes < 1024)        return bytes + ' B';
    if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB';
    return (bytes / (1024 * 1024)).toFixed(2) + ' MB';
  }

  function log(msg, cls) {
    var line = document.createElement('div');
    line.className = 'log-line' + (cls ? ' ' + cls : '');
    line.textContent = '> ' + msg;
    logBox.appendChild(line);
    logBox.scrollTop = logBox.scrollHeight;
  }

  function setFile(file) {
    if (!file || !file.name.endsWith('.bin')) {
      alert('Please select a valid .bin firmware file.');
      return;
    }
    selectedFile = file;
    fiName.textContent = file.name;
    fiSize.textContent = formatSize(file.size);
    dropzone.style.display = 'none';
    fileInfo.classList.add('show');
    btnFlash.disabled = false;
    resultBox.className = 'result-box';
    resultBox.textContent = '';
    logBox.innerHTML = '';
    logBox.classList.remove('show');
    progressWrap.classList.remove('show');
    progressFill.style.width = '0%';
  }

  fileInput.addEventListener('change', function() {
    if (this.files[0]) setFile(this.files[0]);
  });

  dropzone.addEventListener('dragover',  function(e) { e.preventDefault(); this.classList.add('drag'); });
  dropzone.addEventListener('dragleave', function()  { this.classList.remove('drag'); });
  dropzone.addEventListener('drop', function(e) {
    e.preventDefault();
    this.classList.remove('drag');
    if (e.dataTransfer.files[0]) setFile(e.dataTransfer.files[0]);
  });

  fiRemove.addEventListener('click', function() {
    selectedFile = null;
    fileInput.value = '';
    fileInfo.classList.remove('show');
    dropzone.style.display = '';
    btnFlash.disabled = true;
  });

  btnFlash.addEventListener('click', function() {
    if (!selectedFile) return;
    if (!confirm('Flash "' + selectedFile.name + '"?\nDevice will reboot after flashing.')) return;

    btnFlash.disabled = true;
    fiRemove.style.pointerEvents = 'none';
    progressWrap.classList.add('show');
    logBox.innerHTML = '';
    logBox.classList.add('show');
    resultBox.className = 'result-box';

    log('Initiating OTA upload...', 'info');
    log('File: ' + selectedFile.name + ' (' + formatSize(selectedFile.size) + ')');

    var xhr = new XMLHttpRequest();
    xhr.open('POST', '/ota', true);

    xhr.upload.onprogress = function(e) {
      if (e.lengthComputable) {
        var pct = Math.round(e.loaded / e.total * 100);
        progressFill.style.width = pct + '%';
        progressPct.textContent  = pct + '%';
        progressStatus.textContent = pct < 100 ? 'Uploading...' : 'Writing flash...';
        if (pct === 100) log('Upload done -- writing to flash partition...', 'info');
      }
    };

    xhr.onload = function() {
      progressFill.style.width = '100%';
      if (xhr.status === 200) {
        log('Flash verified OK.', 'ok');
        log('Rebooting device...', 'ok');
        resultBox.className   = 'result-box ok';
        progressStatus.textContent = 'Done';
        var countdown = 10;
        resultBox.textContent = 'Flash successful -- rebooting (' + countdown + 's)';
        var iv = setInterval(function() {
          countdown--;
          resultBox.textContent = 'Flash successful -- rebooting (' + countdown + 's)';
          if (countdown <= 0) { clearInterval(iv); window.location.href = '/'; }
        }, 1000);
      } else {
        log('Server returned ' + xhr.status + ': ' + xhr.responseText, 'err');
        resultBox.className   = 'result-box err';
        resultBox.textContent = 'Flash failed -- ' + (xhr.responseText || 'unknown error');
        btnFlash.disabled = false;
        fiRemove.style.pointerEvents = '';
        progressStatus.textContent = 'Failed';
      }
    };

    xhr.onerror = function() {
      log('Connection lost -- device may have rebooted.', 'err');
      resultBox.className   = 'result-box err';
      resultBox.textContent = 'Connection lost -- try reconnecting to device';
      progressStatus.textContent = 'Error';
      btnFlash.disabled = false;
      fiRemove.style.pointerEvents = '';
    };

    var formData = new FormData();
    formData.append('firmware', selectedFile, selectedFile.name);
    xhr.send(formData);
  });
})();
</script>
</body>
</html>
)rawliteral";

#endif /* WIFIPAN_HTML_H_ */
