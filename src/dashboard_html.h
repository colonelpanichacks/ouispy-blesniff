#pragma once

#include <Arduino.h>

// NOTE for maintainer: the banner below is a placeholder in the same slant
// character style as the ouispy-pcap dashboard. Regenerate the exact "OUI-SPY
// BLESNIFF" figlet at build time with pyfiglet if you want the identical look:
//     pyfiglet -f slant "OUI-SPY BLESNIFF"
// The current text uses the same block-style font as ouispy-pcap's PCAP banner.

const char INDEX_HTML[] PROGMEM = R"HTML(<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
<title>OUI-SPY BLESNIFF</title>
<style>
  :root {
    --bg:        #0a0f14;
    --panel:     #10161d;
    --panel-2:   #161d25;
    --border:    #232d38;
    --border-2:  #1a222b;
    --text:      #d5dee8;
    --muted:     #7d8896;
    --dim:       #56616f;
    --accent:    #58a6ff;
    --good:      #3fb950;
    --warn:      #d29922;
    --bad:       #f85149;
    --purple:    #d2a8ff;
    --teal:      #56d4dd;
    --pink:      #ff7b72;
    --f-adv-ind:      #58a6ff;
    --f-adv-direct:   #a5d6ff;
    --f-adv-nonconn:  #56d4dd;
    --f-adv-scan:     #d2a8ff;
    --f-scan-req:     #d29922;
    --f-scan-rsp:     #d2a8ff;
    --f-connect-req:  #7ee787;
    --f-extended:     #d29922;
    --f-addr-pub:     #58a6ff;
    --f-addr-rnd-s:   #56d4dd;
    --f-addr-rnd-nrp: #d29922;
    --f-addr-rnd-rpa: #d2a8ff;
    --v-ring:    #58a6ff;
    --v-axon:    #d2a8ff;
    --v-flock:   #ff7b72;
    --v-dji:     #3fb950;
    --v-parrot:  #7ee787;
    --v-skydio:  #56d4dd;
    --v-meta:    #d29922;
  }
  * { box-sizing: border-box; -webkit-tap-highlight-color: transparent; }
  html, body {
    margin: 0; padding: 0;
    background: var(--bg);
    color: var(--text);
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Inter, system-ui, sans-serif;
    font-size: 13px;
    line-height: 1.4;
    height: 100dvh;
    overflow: hidden;
  }
  a { color: var(--accent); text-decoration: none; }
  b { color: var(--text); font-weight: 600; }
  input, select, button { font-family: inherit; }
  input, select, button { color: var(--text); background: var(--panel-2); border: 1px solid var(--border); }
  ::-webkit-scrollbar { width: 10px; height: 10px; }
  ::-webkit-scrollbar-track { background: var(--bg); }
  ::-webkit-scrollbar-thumb { background: var(--border); }
  ::-webkit-scrollbar-thumb:hover { background: var(--accent); }

  .app {
    display: grid;
    grid-template-columns: 300px 1fr;
    grid-template-rows: auto 1fr auto;
    grid-template-areas:
      "topbar topbar"
      "rail   main"
      "footer footer";
    height: 100dvh;
  }

  .topbar {
    grid-area: topbar;
    display: grid;
    grid-template-columns: minmax(0, 1fr) auto;
    gap: 20px;
    align-items: center;
    padding: 10px 14px;
    background: var(--panel);
    border-bottom: 1px solid var(--border);
    overflow: hidden;
    min-width: 0;
  }
  .banner-wrap { overflow: hidden; min-width: 0; }
  .banner-stack { display: flex; flex-wrap: wrap; gap: 8px 20px; align-items: center; }
  .banner-stack .banner { flex: 0 0 auto; }
  .banner {
    font-family: ui-monospace, Menlo, Consolas, monospace;
    color: var(--accent);
    font-size: 6px;
    line-height: 1.2;
    white-space: pre;
    margin: 0;
    letter-spacing: -0.3px;
    display: block;
    max-width: 100%;
    overflow: hidden;
  }
  .banner-compact { display: none; color: var(--accent); font-family: ui-monospace, Menlo, monospace; letter-spacing: 3px; }
  .status {
    display: grid;
    grid-template-columns: auto auto;
    gap: 2px 12px;
    font-size: 11px;
    color: var(--muted);
    text-transform: uppercase;
    letter-spacing: 1px;
  }
  .status .v {
    font-family: ui-monospace, Menlo, monospace;
    color: var(--text);
    text-transform: none;
    letter-spacing: 0;
    text-align: right;
    font-variant-numeric: tabular-nums;
  }
  .status .v.good { color: var(--good); }
  .status .v.bad  { color: var(--bad); }

  .rail {
    grid-area: rail;
    background: var(--panel);
    border-right: 1px solid var(--border);
    overflow-y: auto;
  }
  .rail section {
    padding: 12px 14px;
    border-bottom: 1px solid var(--border-2);
  }
  .rail h3 {
    margin: 0 0 8px;
    font-size: 10px;
    letter-spacing: 2px;
    text-transform: uppercase;
    color: var(--muted);
    font-weight: 500;
  }
  .rail label { display: block; margin: 6px 0 3px; font-size: 11px; color: var(--muted); }
  .rail input[type=text], .rail input[type=password], .rail select, .rail input[type=number] {
    width: 100%; padding: 6px 8px;
    font-family: ui-monospace, Menlo, monospace; font-size: 12px; outline: none;
  }
  .rail input:focus, .rail select:focus { border-color: var(--accent); }
  .radio-row, .check-row { display: flex; gap: 12px; margin: 4px 0; align-items: center; flex-wrap: wrap; }
  .radio-row label, .check-row label { margin: 0; color: var(--text); font-size: 12px; cursor: pointer; }
  input[type=checkbox], input[type=radio] { accent-color: var(--accent); margin: 0; }
  .warn-banner {
    background: rgba(210,153,34,0.08); border: 1px solid var(--warn);
    color: var(--warn); padding: 6px 8px; margin: 6px 0; font-size: 11px;
  }
  .slider-row { display: flex; align-items: center; gap: 8px; }
  .slider-row input[type=range] { flex: 1; accent-color: var(--accent); }
  .slider-row .val {
    font-family: ui-monospace, Menlo, monospace; color: var(--text);
    font-size: 11px; min-width: 62px; text-align: right;
  }

  .main { grid-area: main; display: grid; grid-template-rows: auto 1fr auto; overflow: hidden; min-width: 0; }

  .toolbar {
    display: flex; align-items: center; gap: 6px;
    padding: 8px 14px; background: var(--panel);
    border-bottom: 1px solid var(--border);
    flex-wrap: wrap;
  }
  .toolbar input[type=text] {
    flex: 1 1 240px; padding: 6px 10px;
    font-family: ui-monospace, Menlo, monospace; font-size: 12px; outline: none;
  }
  .toolbar input[type=text]:focus { border-color: var(--accent); }
  .toolbar input[type=text]::placeholder { color: var(--dim); }
  .btn {
    padding: 6px 10px; font-size: 12px; cursor: pointer; white-space: nowrap;
    letter-spacing: 1px;
  }
  .btn:hover { border-color: var(--accent); color: var(--accent); }
  .btn.active { border-color: var(--accent); color: var(--accent); background: rgba(88,166,255,0.06); }
  .btn.paused { border-color: var(--warn); color: var(--warn); background: rgba(210,153,34,0.06); }
  .btn.danger { color: var(--bad); border-color: var(--bad); }
  .btn.settings { display: none; }

  .qf {
    background: var(--panel);
    border-top: 1px solid var(--border);
    padding: 4px 14px 4px;
    max-height: 150px;
    overflow-y: auto;
    scrollbar-gutter: stable;
  }
  .qf::-webkit-scrollbar { width: 8px; }
  .qf::-webkit-scrollbar-track { background: var(--panel); }
  .qf::-webkit-scrollbar-thumb { background: var(--border); }
  .qf-row {
    padding: 4px 0; border-top: 1px solid var(--border-2);
  }
  .qf-row:first-child { border-top: none; }
  .qf-row-toggle {
    display: grid; grid-template-columns: 14px 1fr auto;
    gap: 8px; align-items: center;
    width: 100%; padding: 4px 2px;
    background: transparent; border: none;
    cursor: pointer; text-align: left;
    color: var(--text); font: inherit;
  }
  .qf-row-toggle:hover .lbl { color: #fff; }
  .qf-row-toggle .caret {
    color: var(--muted); font-family: ui-monospace, monospace;
    font-size: 10px; line-height: 1;
    transition: transform 120ms ease;
    display: inline-block; width: 12px; text-align: center;
  }
  .qf-row.collapsed .qf-row-toggle .caret { transform: rotate(-90deg); color: var(--text); }
  .qf-row .lbl {
    font-size: 11px; letter-spacing: 2px; text-transform: uppercase;
    color: var(--text); font-weight: 600;
  }
  .qf-row-toggle .badge {
    color: var(--accent); background: rgba(88,166,255,0.10);
    border: 1px solid var(--accent);
    font-family: ui-monospace, Menlo, monospace;
    font-size: 10px; padding: 1px 7px;
    min-width: 20px; text-align: center;
    font-weight: 600;
    display: none;
  }
  .qf-row-toggle .badge.on { display: inline-block; }
  .qf-row-body {
    display: grid; grid-template-columns: 72px 1fr auto;
    gap: 12px; align-items: center;
    padding-top: 3px;
    max-height: 300px; overflow: hidden;
    transition: max-height 140ms ease, padding-top 140ms ease, opacity 100ms ease;
  }
  .qf-row.collapsed .qf-row-body {
    max-height: 0; padding-top: 0; opacity: 0; pointer-events: none;
  }
  .qf-row .chips {
    display: flex; flex-wrap: wrap; gap: 4px; align-items: center;
    grid-column: 2;
  }
  .qf-row .trail { display: flex; align-items: center; gap: 8px; color: var(--text); font-size: 11px; white-space: nowrap; grid-column: 3; }
  .chip {
    --c: var(--accent);
    background: transparent; border: 1px solid var(--muted);
    color: var(--text);
    padding: 4px 10px 4px 8px;
    font-size: 11px; font-weight: 500;
    font-family: ui-monospace, Menlo, monospace;
    letter-spacing: 1px; text-transform: uppercase;
    cursor: pointer;
    display: inline-flex; align-items: center; gap: 8px;
    transition: background 80ms ease, color 80ms ease, border-color 80ms ease;
  }
  .chip .ind {
    width: 8px; height: 8px; border: 1px solid var(--muted);
    background: transparent; display: inline-block; flex-shrink: 0;
    transition: background 80ms ease, border-color 80ms ease;
  }
  .chip .count {
    color: var(--muted); font-size: 10px;
    padding-left: 6px; border-left: 1px solid var(--border); margin-left: 2px;
    min-width: 24px; text-align: right;
    font-variant-numeric: tabular-nums;
  }
  .chip:hover { border-color: var(--text); color: #fff; }
  .chip:hover .ind { border-color: var(--text); }
  .chip:active { transform: translateY(1px); }
  .chip.on { background: rgba(88,166,255,0.10); border-color: var(--c); color: var(--c); }
  .chip.on .ind { background: var(--c); border-color: var(--c); }
  .chip.on .count { color: var(--c); border-left-color: var(--c); }
  .chip.danger { --c: var(--bad); }
  .chip.danger.on { background: rgba(248,81,73,0.10); }
  .chip.warn { --c: var(--warn); }
  .chip.warn.on { background: rgba(210,153,34,0.10); }
  .chip.good { --c: var(--good); }
  .chip.good.on { background: rgba(63,185,80,0.10); }
  .chip.clear { color: var(--dim); font-size: 10px; padding: 3px 8px; }
  .chip.clear .ind { display: none; }
  .chip.clear:hover { color: var(--bad); border-color: var(--bad); }

  .tablewrap {
    overflow-x: auto; overflow-y: auto;
    background: var(--bg);
    -webkit-overflow-scrolling: touch;
    min-width: 0;
  }
  table { min-width: 1000px; }
  table { width: 100%; border-collapse: collapse;
          font-family: ui-monospace, Menlo, Consolas, monospace; font-size: 12px; }
  thead th {
    position: sticky; top: 0;
    background: var(--panel); color: var(--muted);
    text-transform: uppercase; letter-spacing: 1px;
    font-weight: 500; font-size: 10px;
    text-align: left; padding: 6px 8px;
    border-bottom: 1px solid var(--border);
    white-space: nowrap;
  }
  tbody td {
    padding: 3px 8px; border-bottom: 1px solid var(--border-2);
    white-space: nowrap;
  }
  tbody tr { border-left: 2px solid transparent; }
  tbody tr:hover { background: var(--panel-2); }
  tbody tr.hit { background: rgba(88,166,255,0.03); border-left-color: var(--accent); }
  tbody td.n     { color: var(--dim); text-align: right; }
  tbody td.right { text-align: right; }
  tbody td.mac   { color: var(--muted); }
  tbody td.info  { color: var(--text); overflow: hidden; text-overflow: ellipsis; max-width: 260px; }
  tbody td.rssi.strong { color: var(--good); }
  tbody td.rssi.mid    { color: var(--warn); }
  tbody td.rssi.weak   { color: var(--bad); }
  tr.t-adv-ind      { color: var(--f-adv-ind); }
  tr.t-adv-direct   { color: var(--f-adv-direct); }
  tr.t-adv-nonconn  { color: var(--f-adv-nonconn); }
  tr.t-adv-scan     { color: var(--f-adv-scan); }
  tr.t-scan-req     { color: var(--f-scan-req); }
  tr.t-scan-rsp     { color: var(--f-scan-rsp); }
  tr.t-connect-req  { color: var(--f-connect-req); font-weight: 500; }
  tr.t-extended     { color: var(--f-extended); }
  tbody tr td.type  { font-weight: 500; }
  tbody td.atype {
    color: var(--muted); font-size: 10px;
    text-transform: uppercase; letter-spacing: 1px;
  }
  tbody td.atype.pub     { color: var(--f-addr-pub); }
  tbody td.atype.rnd-s   { color: var(--f-addr-rnd-s); }
  tbody td.atype.rnd-nrp { color: var(--f-addr-rnd-nrp); }
  tbody td.atype.rnd-rpa { color: var(--f-addr-rnd-rpa); }
  .tag {
    display: inline-block; padding: 1px 6px;
    border: 1px solid; font-size: 10px; letter-spacing: 1px;
    margin-right: 6px; text-transform: uppercase;
    color: var(--warn); border-color: var(--warn);
  }
  .tag.vendor { color: var(--accent); border-color: var(--accent); }

  .footer {
    grid-area: footer;
    display: flex; justify-content: space-between; align-items: center;
    padding: 5px 12px; background: var(--panel);
    border-top: 1px solid var(--border);
    font-family: ui-monospace, Menlo, monospace;
    font-size: 11px; color: var(--muted);
    gap: 12px;
  }
  .footer .right { display: flex; gap: 14px; flex-wrap: wrap; justify-content: flex-end; }
  .footer .v { color: var(--text); }
  .footer .v.good { color: var(--good); }
  .footer .v.bad  { color: var(--bad); }

  .scrim {
    position: fixed; inset: 0; background: rgba(0,0,0,0.55);
    opacity: 0; pointer-events: none;
    transition: opacity 0.15s ease; z-index: 40;
  }
  .scrim.open { opacity: 1; pointer-events: auto; }

  #save-status { font-family: ui-monospace, Menlo, monospace; font-size: 11px; color: var(--muted); }
  #save-status.ok { color: var(--good); }
  #save-status.err { color: var(--bad); }

  @media (max-width: 900px) {
    .app { grid-template-columns: 240px 1fr; }
    .banner { font-size: 6px; }
  }
  @media (max-width: 720px) {
    .app {
      grid-template-columns: 1fr;
      grid-template-areas: "topbar" "main" "footer";
    }
    .banner-stack { flex-direction: column; align-items: flex-start; gap: 4px; }
    .banner { font-size: 6px; display: block; overflow: hidden; }
    .banner-compact { display: none; }
    .status { grid-template-columns: repeat(3, auto); font-size: 10px; gap: 2px 10px; }
    .topbar { padding: 8px 10px; }
    .toolbar { padding: 6px 10px; gap: 4px; }
    .btn.settings { display: inline-flex; }
    .qf-row-body { grid-template-columns: 1fr; padding-left: 22px; }
    .qf-row .chips { grid-column: 1; flex-wrap: wrap; gap: 3px; }
    .qf-row .trail { grid-column: 1; padding-left: 0; justify-content: flex-start; }
    .chip {
      padding: 3px 7px 3px 6px; font-size: 10px;
      gap: 5px; letter-spacing: 0.5px;
    }
    .chip .ind { width: 6px; height: 6px; }
    .chip .count { padding-left: 4px; min-width: 18px; font-size: 9px; }
    .rail {
      position: fixed; top: 0; left: 0; bottom: 0;
      width: 88vw; max-width: 340px;
      transform: translateX(-100%);
      transition: transform 0.18s ease;
      z-index: 50; border-right: 1px solid var(--border);
    }
    .rail.open { transform: translateX(0); }
    .footer { flex-wrap: wrap; font-size: 10px; padding: 4px 8px; }
    .btn { padding: 8px 12px; }
  }
  @media (max-width: 420px) {
    .status { grid-template-columns: repeat(2, auto); }
    .qf { padding: 4px 8px; max-height: 130px; }
    .qf-row .lbl { font-size: 10px; padding-left: 2px; }
    tbody td.info { max-width: 140px; }
    .banner { font-size: 4.5px; }
  }
</style>

<div class="app">

  <div class="topbar">
    <div class="banner-wrap">
      <div class="banner-stack">
        <pre class="banner banner-1">  .oooooo.   ooooo     ooo ooooo          .oooooo..o ooooooooo.   oooooo   oooo
 d8P'  `Y8b  `888'     `8' `888'         d8P'    `Y8 `888   `Y88.  `888.   .8'
888      888  888       8   888          Y88bo.       888   .d88'   `888. .8'
888      888  888       8   888           `"Y8888o.   888ooo88P'     `888.8'
888      888  888       8   888  8888888      `"Y88b  888             `888'
`88b    d88'  `88.    .8'   888          oo     .d8P  888              888
 `Y8bood8P'     `YbodP'    o888o         8""88888P'  o888o            o888o</pre>
        <pre class="banner banner-2">oooooooooo.  ooooo        oooooooooooo  .oooooo..o ooooo      ooo ooooo oooooooooooo oooooooooooo
`888'   `Y8b `888'        `888'     `8 d8P'    `Y8 `888b.     `8' `888' `888'     `8 `888'     `8
 888     888  888          888         Y88bo.       8 `88b.    8   888   888          888
 888oooo888'  888          888oooo8     `"Y8888o.   8   `88b.  8   888   888oooo8     888oooo8
 888    `88b  888          888    "         `"Y88b  8     `88b.8   888   888    "     888    "
 888    .88P  888       o  888       o oo     .d8P  8       `888   888   888          888
o888bood8P'  o888ooooood8 o888ooooood8 8""88888P'  o8o        `8  o888o o888o        o888o</pre>
      </div>
      <span class="banner-compact">OUI-SPY // BLESNIFF</span>
    </div>
    <div class="status">
      <span>Out</span><span class="v good" id="statOut">--</span>
      <span>Win</span><span class="v" id="statWin">--</span>
      <span>Int</span><span class="v" id="statInt">--</span>
      <span>Up</span><span class="v" id="statUp">--</span>
      <span>Pps</span><span class="v" id="statPps">0</span>
      <span>Hits</span><span class="v good" id="statHits">0</span>
      <span>Drop</span><span class="v" id="statDrop">0</span>
    </div>
  </div>

  <div class="scrim" id="scrim"></div>

  <aside class="rail" id="rail">

    <section>
      <h3>Output</h3>
      <div class="radio-row">
        <input type="radio" name="out" id="outPcap"><label for="outPcap">PCAP</label>
        <input type="radio" name="out" id="outText"><label for="outText">Text</label>
      </div>
    </section>

    <section>
      <h3>Scan</h3>
      <label>Window (ms) &mdash; radio-on time per interval</label>
      <div class="slider-row">
        <input type="range" min="10" max="2000" step="10" value="100" id="scanWin"/>
        <span class="val" id="scanWinVal">100 ms</span>
      </div>
      <label>Interval (ms) &mdash; period between windows</label>
      <div class="slider-row">
        <input type="range" min="20" max="4000" step="10" value="100" id="scanInt"/>
        <span class="val" id="scanIntVal">100 ms</span>
      </div>
      <div class="warn-banner" id="winWarn" style="display:none">Window must be &le; interval.</div>
    </section>

    <section>
      <h3>Advert types</h3>
      <div class="check-row">
        <input type="checkbox" id="ftAdvInd"><label for="ftAdvInd">ADV_IND</label>
        <input type="checkbox" id="ftAdvDirect"><label for="ftAdvDirect">DIRECT</label>
        <input type="checkbox" id="ftAdvNonconn"><label for="ftAdvNonconn">NONCONN</label>
        <input type="checkbox" id="ftScanRsp"><label for="ftScanRsp">SCAN_RSP</label>
        <input type="checkbox" id="ftAdvScan"><label for="ftAdvScan">ADV_SCAN</label>
      </div>
      <h3 style="margin-top:10px">Address types</h3>
      <div class="check-row">
        <input type="checkbox" id="ftAddrPub"><label for="ftAddrPub">Public</label>
        <input type="checkbox" id="ftAddrRnd"><label for="ftAddrRnd">Random</label>
      </div>
    </section>

    <section>
      <h3>Access Point</h3>
      <label>SSID</label>
      <input type="text" id="apSsid" maxlength="32" />
      <label>Password (8-63 chars)</label>
      <input type="text" id="apPass" maxlength="63" />
      <div style="margin-top:8px">
        <button class="btn" id="apSave">Save AP &amp; Reboot</button>
      </div>
    </section>

    <section>
      <div style="display:grid;grid-template-columns:1fr 1fr;gap:6px">
        <button class="btn" id="btnReboot">Reboot</button>
        <button class="btn danger" id="btnReset">Factory</button>
        <button class="btn" id="btnDiscard">Discard</button>
        <button class="btn active" id="btnSave">Apply</button>
      </div>
      <div style="margin-top:6px"><span id="save-status">--</span></div>
    </section>

  </aside>

  <div class="main">
    <div class="toolbar">
      <input type="text" id="filter" placeholder="filter -- rssi>-60 | addr:aa:bb | name:airtag | mfr:apple | free text" />
      <button class="btn settings" id="btnSettings">Settings</button>
      <button class="btn active" id="followBtn">Follow</button>
      <button class="btn active" id="pauseBtn">Running</button>
      <button class="btn" id="clearViewBtn">Clear</button>
      <button class="btn" id="snapBtn">CSV</button>
      <button class="btn active" id="savePcapBtn">Save PCAP</button>
      <button class="btn danger" id="clearSessionBtn">Clear session</button>
      <button class="btn danger" id="clearRingBtn">Clear ring</button>
    </div>

    <div class="tablewrap" id="tablewrap">
      <table>
        <thead>
          <tr>
            <th class="hide-sm" style="width:54px">#</th>
            <th class="hide-sm" style="width:88px">Time</th>
            <th style="width:32px">Ch</th>
            <th style="width:52px">RSSI</th>
            <th style="width:130px">Adv Type</th>
            <th class="hide-sm" style="width:82px">Addr Type</th>
            <th style="width:140px">Address</th>
            <th style="width:150px">Name</th>
            <th class="hide-sm" style="width:120px">Svc</th>
            <th class="hide-sm" style="width:110px">Mfr</th>
            <th style="width:52px">Len</th>
          </tr>
        </thead>
        <tbody id="rows"></tbody>
      </table>
    </div>

    <div class="qf" id="qf">
      <div class="qf-row" data-group="type">
        <button class="qf-row-toggle" type="button">
          <span class="caret">&#9662;</span>
          <span class="lbl">Type</span>
          <span class="badge" data-badge="type">0</span>
        </button>
        <div class="qf-row-body">
          <div class="chips">
            <button class="chip" data-key="adv_ind" data-group="type"><span class="ind"></span>ADV_IND<span class="count" id="c-adv_ind">0</span></button>
            <button class="chip" data-key="adv_direct" data-group="type"><span class="ind"></span>DIRECT<span class="count" id="c-adv_direct">0</span></button>
            <button class="chip" data-key="adv_nonconn" data-group="type"><span class="ind"></span>NONCONN<span class="count" id="c-adv_nonconn">0</span></button>
            <button class="chip" data-key="adv_scan" data-group="type"><span class="ind"></span>ADV_SCAN<span class="count" id="c-adv_scan">0</span></button>
            <button class="chip warn" data-key="scan_req" data-group="type"><span class="ind"></span>SCAN_REQ<span class="count" id="c-scan_req">0</span></button>
            <button class="chip" data-key="scan_rsp" data-group="type"><span class="ind"></span>SCAN_RSP<span class="count" id="c-scan_rsp">0</span></button>
            <button class="chip good" data-key="connect_req" data-group="type"><span class="ind"></span>CONNECT<span class="count" id="c-connect_req">0</span></button>
            <button class="chip warn" data-key="extended" data-group="type"><span class="ind"></span>EXTENDED<span class="count" id="c-extended">0</span></button>
          </div>
          <div class="trail"></div>
        </div>
      </div>
      <div class="qf-row" data-group="addr">
        <button class="qf-row-toggle" type="button">
          <span class="caret">&#9662;</span>
          <span class="lbl">Address</span>
          <span class="badge" data-badge="addr">0</span>
        </button>
        <div class="qf-row-body">
          <div class="chips">
            <button class="chip" data-key="a_pub" data-group="addr"><span class="ind"></span>Public<span class="count" id="c-a_pub">0</span></button>
            <button class="chip" data-key="a_rnd_s" data-group="addr"><span class="ind"></span>Random-Static<span class="count" id="c-a_rnd_s">0</span></button>
            <button class="chip warn" data-key="a_rnd_nrp" data-group="addr"><span class="ind"></span>Random-NRP<span class="count" id="c-a_rnd_nrp">0</span></button>
            <button class="chip" data-key="a_rnd_rpa" data-group="addr"><span class="ind"></span>Random-RPA<span class="count" id="c-a_rnd_rpa">0</span></button>
          </div>
          <div class="trail"></div>
        </div>
      </div>
      <div class="qf-row" data-group="traits">
        <button class="qf-row-toggle" type="button">
          <span class="caret">&#9662;</span>
          <span class="lbl">Traits</span>
          <span class="badge" data-badge="traits">0</span>
        </button>
        <div class="qf-row-body">
          <div class="chips">
            <button class="chip" data-key="has_name" data-group="traits"><span class="ind"></span>Has-name<span class="count" id="c-has_name">0</span></button>
            <button class="chip" data-key="has_mfr" data-group="traits"><span class="ind"></span>Has-mfr<span class="count" id="c-has_mfr">0</span></button>
            <button class="chip" data-key="has_svc" data-group="traits"><span class="ind"></span>Has-svc<span class="count" id="c-has_svc">0</span></button>
            <button class="chip" data-key="has_tx" data-group="traits"><span class="ind"></span>Has-tx-pwr<span class="count" id="c-has_tx">0</span></button>
            <button class="chip good" data-key="connectable" data-group="traits"><span class="ind"></span>Connectable<span class="count" id="c-connectable">0</span></button>
          </div>
          <div class="trail"></div>
        </div>
      </div>
      <div class="qf-row" data-group="vendor">
        <button class="qf-row-toggle" type="button">
          <span class="caret">&#9662;</span>
          <span class="lbl">Vendor</span>
          <span class="badge" data-badge="vendor">0</span>
        </button>
        <div class="qf-row-body">
          <div class="chips" id="vendorChips"></div>
          <div class="trail">
            <input type="checkbox" id="hitsOnly" />
            <label for="hitsOnly">Hits only</label>
            <button class="chip clear" id="clearChipsBtn">Clear all</button>
          </div>
        </div>
      </div>
    </div>
  </div>

  <div class="footer">
    <span>ouispy-blesniff <b id="fwVer">--</b></span>
    <span class="right">
      <span>ws <b class="v" id="wsState">--</b></span>
      <span>total <b id="totalPkts">0</b></span>
      <span>shown <b id="rowCount">0</b></span>
      <span>session <b id="sessBytes">0</b></span>
      <span>flt <b id="fltState">off</b></span>
    </span>
  </div>

</div>

<script>
(function(){
  const $ = (id) => document.getElementById(id);
  const rows = $('rows');
  const wrap = $('tablewrap');
  const MAX_ROWS = 500;

  // --- Rail drawer (mobile) --------------------------------------------
  function toggleRail(force) {
    const rail = $('rail'); const scrim = $('scrim');
    const on = force !== undefined ? force : !rail.classList.contains('open');
    rail.classList.toggle('open', on);
    scrim.classList.toggle('open', on);
  }
  $('btnSettings').onclick = () => toggleRail(true);
  $('scrim').onclick = () => toggleRail(false);

  // --- Vendor DB -------------------------------------------------------
  // Same OUI list as ouispy-pcap. Matched against the first 3 bytes of the
  // advertising address (the high three bytes when printed MSB-first).
  // Match on any of: MAC OUI, Bluetooth SIG company ID (in mfr data),
  // 16-bit service UUID, or a case-insensitive substring of the local name.
  // BLE MACs randomize often -- CID / UUID / name are the reliable signals.
  const VENDORS = [
    { id:'ring',   name:'RING',   color:'var(--v-ring)',
      ouis:['00:0d:c5','14:cc:20','a4:77:33','b0:09:da','7c:8c:6c'],
      cids:['0171'],  // Amazon (Ring is Amazon-owned)
      svcs:[], names:['Ring'] },
    { id:'axon',   name:'AXON',   color:'var(--v-axon)',
      ouis:['00:25:df'],
      cids:['034d'],  // TASER International
      svcs:['fc81'], names:[] },
    { id:'flock',  name:'FLOCK',  color:'var(--v-flock)',
      ouis:['a4:cf:12','24:6f:28','3c:71:bf','48:e7:29','98:cd:ac'],
      cids:[], svcs:[], names:['Flock','Falcon','Raven'] },
    { id:'dji',    name:'DJI',    color:'var(--v-dji)',
      ouis:['0c:9a:e6','8c:58:23','04:a8:5a','58:b8:58','e4:7a:2c','60:60:1f','48:1c:b9','34:d2:62'],
      cids:['0bf3'], svcs:[], names:['DJI','Mavic','Phantom','Inspire'] },
    { id:'parrot', name:'PARROT', color:'var(--v-parrot)',
      ouis:['00:12:1c','00:26:7e','90:03:b7','90:3a:e6','a0:14:3d'],
      cids:['004d'], svcs:[], names:['Parrot','Anafi','Bebop'] },
    { id:'skydio', name:'SKYDIO', color:'var(--v-skydio)',
      ouis:['38:1d:14','24:69:8e'],
      cids:[], svcs:[], names:['Skydio'] },
    { id:'meta',   name:'META',   color:'var(--v-meta)',
      ouis:['7c:2a:9e','cc:66:0a','f4:03:43','5c:e9:1e','98:59:49','80:aa:1c','38:47:12','a4:c1:38','58:d5:6e','2c:41:a1','44:d9:e7','9c:d9:17'],
      cids:['0d53'],  // Luxottica (frame-side)
      svcs:['fd5f'], names:['Ray-Ban','Wayfarer','Oakley Meta'] },
  ];
  const vendorEnabled = new Set(VENDORS.map(v => v.id));
  const vendorHitCounts = {};
  VENDORS.forEach(v => {
    vendorHitCounts[v.id] = 0;
    $('vendorChips').insertAdjacentHTML('beforeend',
      '<button class="chip on" data-vendor="'+v.id+'" style="border-color:'+v.color+';color:'+v.color+'">'+
        '<span class="ind" style="background:'+v.color+';border-color:'+v.color+'"></span>'+v.name+
        '<span class="count" id="count-'+v.id+'">0</span>'+
      '</button>');
  });
  $('vendorChips').querySelectorAll('.chip').forEach(chip => {
    chip.addEventListener('click', () => {
      const id = chip.dataset.vendor;
      if (vendorEnabled.has(id)) vendorEnabled.delete(id); else vendorEnabled.add(id);
      chip.classList.toggle('on', vendorEnabled.has(id));
      const ind = chip.querySelector('.ind');
      ind.style.background = vendorEnabled.has(id) ? chip.style.color : 'transparent';
      if (typeof updateGroupBadges === 'function') updateGroupBadges();
      applyFilter();
    });
  });
  // Match on MAC OUI, Bluetooth SIG company ID, 16-bit service UUID (any of
  // the comma-separated list in `svc`), or a case-insensitive name substring.
  // Any signal wins -- BLE MACs are almost always randomized so CID/UUID/name
  // are what actually catches Meta glasses, Axon body cams, DJI drones, etc.
  function vendorFor(mac, cid, svcStr, name) {
    const prefix   = mac ? mac.slice(0, 8).toLowerCase() : '';
    const cidLower = cid ? cid.toLowerCase() : '';
    const svcs     = svcStr ? svcStr.toLowerCase().split(',').map(s => s.replace(/^0x/, '').trim()) : [];
    const nameL    = name ? name.toLowerCase() : '';
    for (const v of VENDORS) {
      if (!vendorEnabled.has(v.id)) continue;
      if (prefix && v.ouis.includes(prefix)) return v;
      if (cidLower && v.cids && v.cids.includes(cidLower)) return v;
      if (svcs.length && v.svcs && v.svcs.some(u => svcs.includes(u))) return v;
      if (nameL && v.names && v.names.some(n => nameL.includes(n.toLowerCase()))) return v;
    }
    return null;
  }

  // --- Advert type mapping ----------------------------------------------
  // Firmware sends `y` as e.g. "ADV_IND" / "SCAN_RSP". Map to chip keys + row class.
  function classifyType(y) {
    y = (y || '').toUpperCase();
    const keys = [];
    let cls = '';
    if (y === 'ADV_IND')            { keys.push('adv_ind');     cls = 't-adv-ind'; }
    else if (y === 'ADV_DIRECT')    { keys.push('adv_direct');  cls = 't-adv-direct'; }
    else if (y === 'ADV_NONCONN')   { keys.push('adv_nonconn'); cls = 't-adv-nonconn'; }
    else if (y === 'ADV_SCAN')      { keys.push('adv_scan');    cls = 't-adv-scan'; }
    else if (y === 'SCAN_REQ')      { keys.push('scan_req');    cls = 't-scan-req'; }
    else if (y === 'SCAN_RSP')      { keys.push('scan_rsp');    cls = 't-scan-rsp'; }
    else if (y === 'CONNECT_REQ')   { keys.push('connect_req'); cls = 't-connect-req'; }
    else                            { keys.push('extended');    cls = 't-extended'; }
    return { keys, cls };
  }

  function classifyAddrType(a) {
    a = (a || '').toLowerCase();
    if (a === 'pub')     return 'a_pub';
    if (a === 'rnd-s')   return 'a_rnd_s';
    if (a === 'rnd-nrp') return 'a_rnd_nrp';
    if (a === 'rnd-rpa') return 'a_rnd_rpa';
    return null;
  }
  function addrTypeCls(a) {
    a = (a || '').toLowerCase();
    if (a === 'pub')     return 'pub';
    if (a === 'rnd-s')   return 'rnd-s';
    if (a === 'rnd-nrp') return 'rnd-nrp';
    if (a === 'rnd-rpa') return 'rnd-rpa';
    return '';
  }

  // Trait bits from firmware match text_summary::traits(): 1=name, 2=mfr, 4=svc, 8=tx, 16=connectable.
  const TR_HAS_NAME=1, TR_HAS_MFR=2, TR_HAS_SVC=4, TR_HAS_TX=8, TR_CONNECTABLE=16;

  // --- Rendering: streaming append, cap at MAX_ROWS -------------------
  let paused = false;
  let n = 0;
  let hits = 0;
  const chipCounts = {};
  function bumpChip(k) {
    chipCounts[k] = (chipCounts[k] || 0) + 1;
    const el = $('c-' + k);
    if (el) el.textContent = chipCounts[k];
  }

  function escapeHtml(s) {
    return String(s).replace(/[&<>"']/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
  }
  function rssiCls(r) {
    return r > -55 ? 'strong' : r > -75 ? 'mid' : 'weak';
  }
  function fmtTime(ms) { return (ms/1000).toFixed(3); }
  function fmtUptime(sec) {
    const d = Math.floor(sec/86400);
    const h = Math.floor((sec%86400)/3600);
    const m = Math.floor((sec%3600)/60);
    const s = sec%60;
    if (d) return d+'d '+h+'h';
    if (h) return String(h).padStart(2,'0')+':'+String(m).padStart(2,'0');
    return String(m).padStart(2,'0')+':'+String(s).padStart(2,'0');
  }
  function fmtBytes(b) {
    if (b < 1024) return b+' B';
    if (b < 1024*1024) return (b/1024).toFixed(1)+' KB';
    return (b/1024/1024).toFixed(2)+' MB';
  }

  function pushPacket(p) {
    // p: {i,t,c,r,x?,y,a,m,l,f,n?,s?,u?,v?}
    const y    = p.y || '';
    const a    = p.a || '';
    const addr = p.m || '';
    const name = p.n || '';
    const svc  = p.s || '';
    const mfr  = p.v || '';
    const mfrHex = p.u || '';
    const tr   = p.f || 0;
    const ch   = (p.c == null || p.c < 0) ? '?' : p.c;

    const cls = classifyType(y);
    const keys = new Set(cls.keys);
    const ak = classifyAddrType(a); if (ak) keys.add(ak);
    if (tr & TR_HAS_NAME)     keys.add('has_name');
    if (tr & TR_HAS_MFR)      keys.add('has_mfr');
    if (tr & TR_HAS_SVC)      keys.add('has_svc');
    if (tr & TR_HAS_TX)       keys.add('has_tx');
    if (tr & TR_CONNECTABLE)  keys.add('connectable');
    keys.forEach(bumpChip);

    const vend = vendorFor(addr, mfrHex, svc, name);
    if (vend) {
      hits++;
      vendorHitCounts[vend.id]++;
      $('count-'+vend.id).textContent = vendorHitCounts[vend.id];
      $('statHits').textContent = hits;
    }

    n++;
    const rc = rssiCls(p.r);
    const vendorTag = vend
      ? '<span class="tag vendor" style="color:'+vend.color+';border-color:'+vend.color+'">'+vend.name+'</span>'
      : '';

    let mfrDisplay = '';
    if (mfrHex) {
      mfrDisplay = mfrHex + (mfr && mfr !== '?' ? ' ' + mfr : '');
    }

    const tr_el = document.createElement('tr');
    tr_el.className = cls.cls;
    if (vend) tr_el.classList.add('hit');
    tr_el.dataset.keys = [...keys].join(' ');
    tr_el.dataset.hit  = vend ? '1' : '0';
    tr_el.dataset.addr = addr.toLowerCase();
    tr_el.dataset.name = name.toLowerCase();
    tr_el.dataset.svc  = svc.toLowerCase();
    tr_el.dataset.mfr  = (mfrHex + ' ' + (mfr||'')).toLowerCase();
    tr_el.dataset.type = y.toLowerCase();
    tr_el.dataset.atype = a.toLowerCase();
    tr_el.dataset.rssi = String(p.r);
    tr_el.dataset.ch   = String(ch);
    tr_el.innerHTML =
      '<td class="n hide-sm">'+(p.i != null ? p.i : n)+'</td>'+
      '<td class="hide-sm">'+fmtTime(p.t || 0)+'</td>'+
      '<td>'+escapeHtml(String(ch))+'</td>'+
      '<td class="rssi '+rc+' right">'+p.r+'</td>'+
      '<td class="type">'+escapeHtml(y)+'</td>'+
      '<td class="atype '+addrTypeCls(a)+' hide-sm">'+escapeHtml(a)+'</td>'+
      '<td class="mac">'+vendorTag+escapeHtml(addr)+'</td>'+
      '<td class="info">'+escapeHtml(name)+'</td>'+
      '<td class="mac hide-sm">'+escapeHtml(svc)+'</td>'+
      '<td class="mac hide-sm">'+escapeHtml(mfrDisplay)+'</td>'+
      '<td class="right">'+p.l+'</td>';
    rows.appendChild(tr_el);
    while (rows.childElementCount > MAX_ROWS) rows.removeChild(rows.firstChild);
    applyRowFilter(tr_el);
  }

  // --- Follow (auto-scroll) --------------------------------------------
  let follow = true;
  let scrollByCode = false;
  function setFollow(on) {
    follow = on;
    const b = $('followBtn');
    b.textContent = on ? 'Follow' : 'Scroll to bottom to follow';
    b.classList.toggle('active', on);
    b.classList.toggle('paused', !on);
    if (on) {
      scrollByCode = true;
      requestAnimationFrame(() => { wrap.scrollTop = wrap.scrollHeight; });
    }
  }
  $('followBtn').onclick = () => setFollow(!follow);
  wrap.addEventListener('scroll', () => {
    if (scrollByCode) { scrollByCode = false; return; }
    const atBottom = (wrap.scrollHeight - wrap.scrollTop - wrap.clientHeight) <= 4;
    if (atBottom && !follow) setFollow(true);
    else if (!atBottom && follow) setFollow(false);
  }, { passive: true });

  const stickBottom = () => {
    if (!follow) return;
    scrollByCode = true;
    wrap.scrollTop = wrap.scrollHeight;
  };

  // --- Pause / Clear / Snapshot / Clear ring --------------------------
  $('pauseBtn').onclick = () => {
    paused = !paused;
    const b = $('pauseBtn');
    b.textContent = paused ? 'Paused' : 'Running';
    b.classList.toggle('active', !paused);
    b.classList.toggle('paused', paused);
  };
  $('clearViewBtn').onclick = () => {
    rows.innerHTML = ''; n = 0; hits = 0;
    $('statHits').textContent = '0';
    Object.keys(chipCounts).forEach(k => chipCounts[k] = 0);
    document.querySelectorAll('#qf .chip[data-key] .count').forEach(c => c.textContent = '0');
    VENDORS.forEach(v => { vendorHitCounts[v.id]=0; $('count-'+v.id).textContent='0'; });
    $('rowCount').textContent = '0';
  };
  $('clearRingBtn').onclick = async () => {
    try { await fetch('/api/clear', {method:'POST'}); } catch(e){}
  };
  $('savePcapBtn').onclick = () => {
    const stamp = new Date().toISOString().replace(/[:.]/g,'-').slice(0,19);
    const a = document.createElement('a');
    a.href = '/api/session.pcap?ts=' + Date.now();
    a.download = 'ouispy-blesniff-' + stamp + '.pcap';
    document.body.appendChild(a);
    a.click();
    a.remove();
  };
  $('clearSessionBtn').onclick = async () => {
    if (!confirm('Discard the recorded session PCAP?')) return;
    try { await fetch('/api/session/clear', {method:'POST'}); } catch(e){}
  };
  $('snapBtn').onclick = () => {
    const cols = ['idx','t_ms','ch','rssi','type','addr_type','address','name','svc','mfr','len'];
    const lines = [cols.join(',')];
    rows.querySelectorAll('tr').forEach(tr => {
      const c = tr.children;
      const q = (s) => '"'+String(s||'').replace(/"/g,'""')+'"';
      lines.push([c[0].textContent, c[1].textContent, c[2].textContent, c[3].textContent,
                  c[4].textContent, c[5].textContent, c[6].textContent,
                  q(c[7].textContent), q(c[8].textContent), q(c[9].textContent),
                  c[10].textContent].join(','));
    });
    const blob = new Blob([lines.join('\n')], {type:'text/csv'});
    const a = document.createElement('a');
    a.href = URL.createObjectURL(blob);
    a.download = 'ouispy-blesniff-' + new Date().toISOString().replace(/[:.]/g,'-') + '.csv';
    a.click();
  };

  // --- Chip filter -----------------------------------------------------
  const activeChips = new Set();
  document.querySelectorAll('#qf .chip[data-key]').forEach(chip => {
    chip.addEventListener('click', () => {
      const key = chip.dataset.key;
      if (activeChips.has(key)) activeChips.delete(key);
      else activeChips.add(key);
      chip.classList.toggle('on', activeChips.has(key));
      updateGroupBadges();
      applyFilter();
    });
  });
  $('clearChipsBtn').onclick = () => {
    activeChips.clear();
    document.querySelectorAll('#qf .chip[data-key]').forEach(c => c.classList.remove('on'));
    if (typeof vendorEnabled !== 'undefined') {
      vendorEnabled.clear();
      VENDORS.forEach(v => vendorEnabled.add(v.id));
      document.querySelectorAll('#vendorChips .chip').forEach(c => {
        c.classList.add('on');
        const ind = c.querySelector('.ind');
        if (ind) ind.style.background = c.style.color;
      });
    }
    updateGroupBadges();
    applyFilter();
  };

  // --- Collapsible groups ---------------------------------------------
  document.querySelectorAll('.qf-row-toggle').forEach(tog => {
    tog.addEventListener('click', () => {
      tog.closest('.qf-row').classList.toggle('collapsed');
    });
  });
  function updateGroupBadges() {
    document.querySelectorAll('.qf-row').forEach(row => {
      const g = row.dataset.group;
      let count = 0;
      if (g === 'vendor') {
        count = (typeof vendorEnabled !== 'undefined') ? (VENDORS.length - vendorEnabled.size) : 0;
      } else {
        count = row.querySelectorAll('.chip[data-key].on').length;
      }
      const badge = row.querySelector('[data-badge]');
      if (badge) {
        badge.textContent = count;
        badge.classList.toggle('on', count > 0);
      }
    });
  }
  updateGroupBadges();
  $('hitsOnly').onchange = () => applyFilter();
  $('filter').oninput = () => applyFilter();

  function parseTextFilter() {
    const q = $('filter').value.trim().toLowerCase();
    return {
      q,
      rssi:  q.match(/rssi\s*([<>=])\s*(-?\d+)/),
      type:  q.match(/type:(\S+)/),
      addr:  q.match(/addr:([0-9a-f:]+)/),
      name:  q.match(/name:(\S+)/),
      svc:   q.match(/svc:(\S+)/),
      mfr:   q.match(/mfr:(\S+)/),
      ch:    q.match(/ch:(\d+)/),
      free:  q.replace(/rssi\s*[<>=]\s*-?\d+/g,'')
              .replace(/(type|addr|name|svc|mfr|ch):\S+/g,'').trim()
    };
  }
  function rowMatch(tr, f, hitsOnly) {
    if (activeChips.size > 0) {
      const rowKeys = (tr.dataset.keys || '').split(' ');
      if (!rowKeys.some(k => activeChips.has(k))) return false;
    }
    if (hitsOnly && tr.dataset.hit !== '1') return false;
    if (f.rssi) {
      const r = parseInt(tr.dataset.rssi, 10);
      const op = f.rssi[1], v = +f.rssi[2];
      if (op === '>' && !(r > v)) return false;
      if (op === '<' && !(r < v)) return false;
      if (op === '=' && r !== v) return false;
    }
    if (f.type && !tr.dataset.type.includes(f.type[1]))   return false;
    if (f.addr && !tr.dataset.addr.includes(f.addr[1]))   return false;
    if (f.name && !tr.dataset.name.includes(f.name[1]))   return false;
    if (f.svc  && !tr.dataset.svc.includes(f.svc[1]))     return false;
    if (f.mfr  && !tr.dataset.mfr.includes(f.mfr[1]))     return false;
    if (f.ch   && tr.dataset.ch !== f.ch[1])              return false;
    if (f.free) {
      const hay = tr.dataset.type+' '+tr.dataset.atype+' '+tr.dataset.addr+' '+
                  tr.dataset.name+' '+tr.dataset.svc+' '+tr.dataset.mfr+
                  ' ch'+tr.dataset.ch+' rssi'+tr.dataset.rssi;
      if (!hay.includes(f.free)) return false;
    }
    return true;
  }
  function applyRowFilter(tr) {
    const f = parseTextFilter();
    const hitsOnly = $('hitsOnly').checked;
    tr.style.display = rowMatch(tr, f, hitsOnly) ? '' : 'none';
  }
  function applyFilter() {
    const f = parseTextFilter();
    const hitsOnly = $('hitsOnly').checked;
    const any = activeChips.size || hitsOnly || f.rssi || f.type || f.addr ||
                f.name || f.svc || f.mfr || f.ch || f.free;
    $('fltState').textContent = any ? 'on' : 'off';
    let shown = 0;
    rows.querySelectorAll('tr').forEach(tr => {
      const on = rowMatch(tr, f, hitsOnly);
      tr.style.display = on ? '' : 'none';
      if (on) shown++;
    });
    $('rowCount').textContent = shown;
  }

  // --- Batched render: keep DOM writes cheap under load ---------------
  let pending = [];
  let flushScheduled = false;
  function scheduleFlush() {
    if (flushScheduled) return;
    flushScheduled = true;
    requestAnimationFrame(() => {
      flushScheduled = false;
      const batch = pending;
      pending = [];
      for (const p of batch) pushPacket(p);
      let shown = 0;
      rows.querySelectorAll('tr').forEach(tr => { if (tr.style.display !== 'none') shown++; });
      $('rowCount').textContent = shown;
      stickBottom();
    });
  }
  function ingest(p) {
    if (paused) return;
    pending.push(p);
    if (pending.length > 400) pending.splice(0, pending.length - 400);
    scheduleFlush();
  }

  // --- WebSocket -------------------------------------------------------
  let ws = null;
  function connectWS() {
    const url = (location.protocol === 'https:' ? 'wss://' : 'ws://') + location.host + '/ws';
    ws = new WebSocket(url);
    ws.onopen  = () => { $('wsState').textContent = 'connected'; $('wsState').className = 'v good'; };
    ws.onclose = () => { $('wsState').textContent = 'disconnected'; $('wsState').className = 'v bad';
                          setTimeout(connectWS, 2000); };
    ws.onerror = () => { $('wsState').textContent = 'error'; $('wsState').className = 'v bad'; };
    ws.onmessage = (ev) => {
      let msg; try { msg = JSON.parse(ev.data); } catch(e) { return; }
      if (msg.type === 'status') {
        $('statOut').textContent = msg.out || '--';
        $('statUp').textContent = fmtUptime(msg.uptime || 0);
        $('statPps').textContent = msg.pps || 0;
        const drop = (msg.dropped_pcap || 0) + (msg.dropped_dash || 0);
        $('statDrop').textContent = drop;
        $('statDrop').className = 'v' + (drop > 0 ? ' bad' : '');
        $('totalPkts').textContent = msg.total || 0;
        $('sessBytes').textContent = fmtBytes(msg.session_bytes || 0);
        $('fwVer').textContent = msg.fw || '--';
        return;
      }
      if (msg.type === 'pkts' && Array.isArray(msg.p)) {
        for (const p of msg.p) ingest(p);
      } else if (msg.type === 'pkt') {
        ingest(msg);
      }
    };
  }

  // --- Config load / save ---------------------------------------------
  function markDirty() {
    $('save-status').textContent = 'unsaved';
    $('save-status').className = '';
  }
  document.querySelectorAll('.rail input, .rail select').forEach(el => {
    el.addEventListener('change', markDirty);
    el.addEventListener('input',  markDirty);
  });

  function updateScanLabels() {
    $('scanWinVal').textContent = $('scanWin').value + ' ms';
    $('scanIntVal').textContent = $('scanInt').value + ' ms';
    const win = +$('scanWin').value;
    const intv = +$('scanInt').value;
    $('winWarn').style.display = (win > intv) ? '' : 'none';
  }
  $('scanWin').oninput = updateScanLabels;
  $('scanInt').oninput = updateScanLabels;

  async function loadConfig() {
    try {
      const r = await fetch('/api/config');
      const c = await r.json();
      $('outPcap').checked = c.out === 0;
      $('outText').checked = c.out === 1;
      $('scanWin').value = c.scan_win;
      $('scanInt').value = c.scan_int;
      updateScanLabels();
      $('ftAdvInd').checked      = (c.ftmask & 0x01) !== 0;
      $('ftAdvDirect').checked   = (c.ftmask & 0x02) !== 0;
      $('ftAdvNonconn').checked  = (c.ftmask & 0x04) !== 0;
      $('ftScanRsp').checked     = (c.ftmask & 0x08) !== 0;
      $('ftAdvScan').checked     = (c.ftmask & 0x10) !== 0;
      $('ftAddrPub').checked     = (c.ftmask & 0x20) !== 0;
      $('ftAddrRnd').checked     = (c.ftmask & 0x40) !== 0;
      $('apSsid').value = c.ap_ssid || '';
      $('apPass').value = c.ap_pass || '';
      $('statWin').textContent = c.scan_win + 'ms';
      $('statInt').textContent = c.scan_int + 'ms';
      $('save-status').textContent = 'saved';
      $('save-status').className = 'ok';
    } catch (e) {
      $('save-status').textContent = 'load failed';
      $('save-status').className = 'err';
    }
  }

  $('btnDiscard').onclick = () => loadConfig();

  $('btnSave').onclick = async () => {
    let ftmask = 0;
    if ($('ftAdvInd').checked)     ftmask |= 0x01;
    if ($('ftAdvDirect').checked)  ftmask |= 0x02;
    if ($('ftAdvNonconn').checked) ftmask |= 0x04;
    if ($('ftScanRsp').checked)    ftmask |= 0x08;
    if ($('ftAdvScan').checked)    ftmask |= 0x10;
    if ($('ftAddrPub').checked)    ftmask |= 0x20;
    if ($('ftAddrRnd').checked)    ftmask |= 0x40;
    const body = {
      out:      $('outPcap').checked ? 0 : 1,
      scan_win: parseInt($('scanWin').value, 10),
      scan_int: parseInt($('scanInt').value, 10),
      ftmask:   ftmask
    };
    $('save-status').textContent = 'applying...';
    $('save-status').className = '';
    try {
      const r = await fetch('/api/config', {
        method: 'POST',
        headers: {'content-type':'application/json'},
        body: JSON.stringify(body)
      });
      if (r.ok) {
        $('save-status').textContent = 'applied';
        $('save-status').className = 'ok';
        $('statWin').textContent = body.scan_win + 'ms';
        $('statInt').textContent = body.scan_int + 'ms';
      } else {
        $('save-status').textContent = 'error';
        $('save-status').className = 'err';
      }
    } catch(e) {
      $('save-status').textContent = 'error';
      $('save-status').className = 'err';
    }
  };

  $('apSave').onclick = async () => {
    const body = { ssid: $('apSsid').value, pass: $('apPass').value };
    try {
      const r = await fetch('/api/ap', {
        method: 'POST',
        headers: {'content-type':'application/json'},
        body: JSON.stringify(body)
      });
      if (r.ok) setTimeout(() => fetch('/api/reboot', {method:'POST'}), 300);
    } catch(e){}
  };
  $('btnReboot').onclick = async () => {
    try { await fetch('/api/reboot', {method:'POST'}); } catch(e){}
  };
  $('btnReset').onclick = async () => {
    if (!confirm('Factory reset all settings and reboot?')) return;
    try { await fetch('/api/reset', {method:'POST'}); } catch(e){}
  };

  loadConfig();
  connectWS();
})();
</script>
)HTML";
