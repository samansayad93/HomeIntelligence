/* =========================================================================
   Inteligence — dashboard logic
   Connects directly to the MQTT broker over WebSocket (no server needed),
   subscribes to the device's sensor topics, and publishes commands.
   ========================================================================= */
"use strict";

/* ----------------------------- sensor definitions ----------------------------- */
/* Each sensor maps a device topic suffix (relative to the base topic) to a card. */

const ADC_MAX = 4095;            // ESP32 12-bit ADC
const MQ2_THRESHOLD = 600;       // matches config.h MQ2_THRESHOLD
const STALE_MS = 30000;          // mark a sensor stale after 30s without data
const CHART_POINTS = 40;

const ICONS = {
  ldr: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="4"/><path d="M12 2v2M12 20v2M4.93 4.93l1.41 1.41M17.66 17.66l1.41 1.41M2 12h2M20 12h2M6.34 17.66l-1.41 1.41M19.07 4.93l-1.41 1.41"/></svg>',
  temp: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M14 14.76V3.5a2.5 2.5 0 0 0-5 0v11.26a4.5 4.5 0 1 0 5 0z"/></svg>',
  hum: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 2.69l5.66 5.66a8 8 0 1 1-11.31 0z"/></svg>',
  pir: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="5" r="2"/><path d="M7 13c1.5-1.5 3-2 5-2s3.5.5 5 2"/><path d="M4 21c0-4 3.5-6 8-6s8 2 8 6"/></svg>',
  mq2: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 2c1 3 4 5 4 9a4 4 0 0 1-8 0c0-1 .3-2 .8-2.8C9 9 11 7 12 2z"/></svg>'
};

const SENSORS = [
  {
    kind: "LDR", suffix: "sensor/LDR", name: "Light", icon: ICONS.ldr, unit: "",
    parse: v => parseInt(v, 10),
    render: (el, val) => {
      const pct = Math.round(Math.min(val, ADC_MAX) / ADC_MAX * 100);
      el.num.textContent = val;
      el.sub.textContent = `${pct}% of range`;
      el.bar.style.width = pct + "%";
      el.bar.className = pct > 66 ? "" : pct > 33 ? "warn" : "danger";
      setChip(el.chip, pct > 66 ? "HIGH" : pct > 33 ? "MID" : "LOW", pct > 66 ? "ok" : pct > 33 ? "info" : "warn");
    }
  },
  {
    kind: "TEMP", suffix: "sensor/TEMP", name: "Temperature", icon: ICONS.temp, unit: "°C",
    parse: v => parseFloat(v),
    render: (el, val) => {
      el.num.textContent = val.toFixed(1);
      el.sub.textContent = val < 18 ? "cold" : val > 26 ? "warm" : "comfortable";
      setChip(el.chip, val > 30 ? "HOT" : val > 26 ? "WARM" : val < 12 ? "COLD" : "OK", val > 30 || val < 12 ? "warn" : "ok");
    }
  },
  {
    kind: "HUM", suffix: "sensor/HUM", name: "Humidity", icon: ICONS.hum, unit: "%",
    parse: v => parseFloat(v),
    render: (el, val) => {
      el.num.textContent = val.toFixed(1);
      el.sub.textContent = val < 30 ? "dry" : val > 60 ? "humid" : "balanced";
      setChip(el.chip, val > 70 ? "HIGH" : val < 25 ? "LOW" : "OK", val > 70 || val < 25 ? "warn" : "ok");
    }
  },
  {
    kind: "PIR", suffix: "sensor/PIR", name: "Motion", icon: ICONS.pir, unit: "",
    parse: v => parseInt(v, 10) === 1,
    render: (el, val) => {
      el.num.textContent = val ? "MOTION" : "CLEAR";
      el.sub.textContent = val ? "movement detected" : "no movement";
      el.card.classList.toggle("is-active", val);
      setChip(el.chip, val ? "ACTIVE" : "IDLE", val ? "active" : "ok");
    }
  },
  {
    kind: "MQ2", suffix: "sensor/MQ2", name: "Gas", icon: ICONS.mq2, unit: "",
    parse: v => parseInt(v, 10),
    render: (el, val) => {
      const danger = val >= MQ2_THRESHOLD;
      const pct = Math.round(Math.min(val, 1500) / 1500 * 100);
      el.num.textContent = val;
      el.sub.textContent = `threshold ${MQ2_THRESHOLD}`;
      el.bar.style.width = pct + "%";
      el.bar.className = danger ? "danger" : val > MQ2_THRESHOLD * 0.7 ? "warn" : "";
      el.card.classList.toggle("is-danger", danger);
      setChip(el.chip, danger ? "DANGER" : "SAFE", danger ? "danger" : "ok");
    }
  }
];

function setChip(chip, label, state) {
  chip.textContent = label;
  chip.dataset.state = state;
}

/* ----------------------------- state ----------------------------- */
const STORAGE_KEY = "intelligence.web.settings.v1";

let client = null;
let baseTopic = "homeintelligence";
let device = "";                  // device name — inserted into every topic path
let intentionallyClosed = false;
const cards = {};      // kind -> { card, num, sub, chip, bar, time, chart, ts }
/* live device-reported signal lists (rf / ir). Not cached: fetched fresh from the
   device over MQTT every time the dashboard connects. Each RF/list|IR/list message
   carries one signal object, merged by index. */
const deviceSignals = { rf: [], ir: [] };

/* ----------------------------- dom ----------------------------- */
const $ = (sel, root = document) => root.querySelector(sel);
const $$ = (sel, root = document) => Array.from(root.querySelectorAll(sel));

const elStatus = $("#connStatus");
const elStatusLabel = $(".connstatus__label", elStatus);
const form = $("#connForm");
const connectBtn = $("#connectBtn");
const disconnectBtn = $("#disconnectBtn");
const elConnHint = $("#connHint");
const elLog = $("#log");
const elLogFilter = $("#logFilter");
const elLastUpdate = $("#lastUpdate");
const elAlarmBanner = $("#alarmBanner");
const elAlarmTitle = $("#alarmTitle");
const elAlarmTime = $("#alarmTime");
const elCmdTopicLabel = $("#cmdTopicLabel");

/* ----------------------------- persistence ----------------------------- */
function loadJSON(key, fallback) {
  try { return JSON.parse(localStorage.getItem(key)) ?? fallback; }
  catch { return fallback; }
}
function saveJSON(key, val) {
  try { localStorage.setItem(key, JSON.stringify(val)); } catch { /* ignore quota */ }
}
function loadSettings() {
  return Object.assign(
    { protocol: "ws", host: "", port: "9001", path: "/mqtt", baseTopic: "homeintelligence", device: "", clientId: "", username: "", password: "" },
    loadJSON(STORAGE_KEY, {})
  );
}
function saveSettings(s) { saveJSON(STORAGE_KEY, s); }

/* ----------------------------- build sensor cards ----------------------------- */
function buildCards() {
  const grid = $("#sensorGrid");
  grid.innerHTML = "";
  SENSORS.forEach(s => {
    const card = document.createElement("div");
    card.className = "sensor";
    card.dataset.kind = s.kind;
    const hasChart = s.kind !== "PIR";
    card.innerHTML = `
      <div class="sensor__top">
        <span class="sensor__name"><span class="sensor__ico">${s.icon}</span>${s.name}</span>
        <span class="sensor__chip" data-state="">—</span>
      </div>
      <div class="sensor__value">
        <span class="sensor__num">--</span>
        <span class="sensor__unit">${s.unit}</span>
      </div>
      <div class="sensor__sub">waiting…</div>
      <div class="sensor__bar"><i></i></div>
      ${hasChart ? '<div class="sensor__chart"><canvas></canvas></div>' : ''}
      <div class="sensor__time">never</div>
    `;
    grid.appendChild(card);

    const entry = {
      card,
      num: $(".sensor__num", card),
      sub: $(".sensor__sub", card),
      chip: $(".sensor__chip", card),
      bar: $(".sensor__bar > i", card),
      time: $(".sensor__time", card),
      chart: null,
      ts: 0
    };
    if (hasChart) entry.chart = makeChart($("canvas", card), s.kind);
    cards[s.kind] = entry;
  });
}

function makeChart(canvas, kind) {
  const ctx = canvas.getContext("2d");
  const grad = ctx.createLinearGradient(0, 0, 0, 40);
  const color = ({ TEMP: "#f59e0b", HUM: "#38bdf8", LDR: "#fbbf24", MQ2: "#34d399" })[kind] || "#4f8cff";
  grad.addColorStop(0, color + "55");
  grad.addColorStop(1, color + "00");
  return new Chart(ctx, {
    type: "line",
    data: { labels: [], datasets: [{ data: [], borderColor: color, backgroundColor: grad, borderWidth: 1.6, fill: true, tension: .35, pointRadius: 0 }] },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      animation: false,
      plugins: { legend: { display: false }, tooltip: { enabled: false } },
      scales: { x: { display: false }, y: { display: false, grace: "10%" } },
      elements: { line: { borderJoinStyle: "round" } }
    }
  });
}

function pushChart(chart, value) {
  if (!chart) return;
  const d = chart.data;
  d.labels.push("");
  d.datasets[0].data.push(value);
  if (d.labels.length > CHART_POINTS) { d.labels.shift(); d.datasets[0].data.shift(); }
  chart.update("none");
}

/* ----------------------------- MQTT ----------------------------- */
function readForm() {
  const fd = new FormData(form);
  return {
    protocol: fd.get("protocol"),
    host: (fd.get("host") || "").trim(),
    port: (fd.get("port") || "").trim(),
    path: (fd.get("path") || "").trim() || "/mqtt",
    baseTopic: (fd.get("baseTopic") || "").trim() || "homeintelligence",
    device: (fd.get("device") || "").trim(),
    clientId: (fd.get("clientId") || "").trim(),
    username: (fd.get("username") || "").trim(),
    password: fd.get("password") || ""
  };
}

function deviceBase() {
  // Every device topic is <baseTopic>/<device>/<suffix>; the device name is the
  // segment the firmware inserts in topicFor(), so it must match here exactly.
  return device ? `${baseTopic}/${device}` : baseTopic;
}

function connect() {
  const s = readForm();
  if (!s.host) { flashHint("Enter your broker host first."); form.host.focus(); return; }
  if (!s.device) { flashHint("Enter the device name (matches the device's provisioning name)."); form.device.focus(); return; }
  saveSettings(s);
  baseTopic = s.baseTopic.replace(/^\/+|\/+$/g, "");
  device = s.device.replace(/^\/+|\/+$/g, "");
  elCmdTopicLabel.textContent = `${deviceBase()}/command`;

  const port = s.port || (s.protocol === "wss" ? "443" : "9001");
  const path = s.path.startsWith("/") ? s.path : "/" + s.path;
  const url = `${s.protocol}://${s.host}:${port}${path}`;
  const clientId = s.clientId || `inteligence-web-${Math.random().toString(16).slice(2, 8)}`;

  const opts = { clientId, clean: true, reconnectPeriod: 4000, connectTimeout: 8000 };
  if (s.username) { opts.username = s.username; opts.password = s.password; }

  setStatus("connecting", "Connecting…");
  log("sys", url, `connecting as ${clientId}`);
  intentionallyClosed = false;

  try {
    client = mqtt.connect(url, opts);
  } catch (err) {
    setStatus("error", "Error");
    log("sys", "connect", String(err));
    return;
  }

  client.on("connect", () => {
    setStatus("connected", "Connected");
    const subs = [`${deviceBase()}/sensor/#`, `${deviceBase()}/alarm`, `${deviceBase()}/RF/list`, `${deviceBase()}/IR/list`];
    client.subscribe(subs, (err) => {
      if (err) { log("sys", "subscribe", String(err)); return; }
      log("sys", "subscribe", subs.join(", "));
      /* Signals are never cached — fetch them fresh from the device on every connect. */
      deviceSignals.rf = [];
      deviceSignals.ir = [];
      renderSignalLists();
      sendCommand("rf list");
      sendCommand("ir list");
    });
    setButtons(true);
  });

  client.on("reconnect", () => setStatus("reconnecting", "Reconnecting…"));
  client.on("close", () => {
    if (!intentionallyClosed) setStatus("disconnected", "Disconnected");
  });
  client.on("error", (err) => {
    setStatus("error", "Connection error");
    log("sys", "error", String(err));
  });

  client.on("message", (topic, payload) => {
    const msg = payload.toString();
    handleMessage(topic, msg);
  });

  setButtons(true);
}

function disconnect() {
  intentionallyClosed = true;
  if (client) {
    try { client.end(true); } catch { /* ignore */ }
    client = null;
  }
  setStatus("disconnected", "Disconnected");
  setButtons(false);
  log("sys", "disconnect", "client closed");
}

function handleMessage(topic, msg) {
  const prefix = deviceBase() + "/";
  if (!topic.startsWith(prefix)) return;
  const suffix = topic.slice(prefix.length);
  const now = Date.now();
  log("in", topic, msg);

  if (suffix === "alarm") {
    handleAlarmMessage(msg, now);
    return;
  }

  if (suffix === "RF/list") {
    handleSignalList("rf", msg);
    return;
  }

  if (suffix === "IR/list") {
    handleSignalList("ir", msg);
    return;
  }

  const sensor = SENSORS.find(s => s.suffix === suffix);
  if (!sensor) { return; }

  const entry = cards[sensor.kind];
  if (!entry) return;
  const val = sensor.parse(msg);
  if (val === null || Number.isNaN(val)) { return; }

  sensor.render(entry, val);
  entry.ts = now;
  entry.card.classList.remove("is-stale");
  entry.time.textContent = "just now";
  pushChart(entry.chart, typeof val === "boolean" ? (val ? 1 : 0) : val);

  elLastUpdate.textContent = `last update ${formatClock(now)}`;
}

/* ----------------------------- commands ----------------------------- */
function sendCommand(cmd) {
  cmd = (cmd || "").trim();
  if (!cmd) return;
  if (!client || !client.connected) { flashHint("Not connected to the broker."); return; }
  const topic = `${deviceBase()}/command`;
  client.publish(topic, cmd);
  log("out", topic, cmd);
}

function captureSignal(kind, name) {
  if (!name) { flashHint(`Enter a name for the ${kind} signal.`); return false; }
  sendCommand(`${kind} rx ${name}`);
  return true;
}
function transmitSignal(kind, name) {
  if (!name) { flashHint(`Enter a ${kind} signal name to transmit.`); return false; }
  sendCommand(`${kind} tx ${name}`);
  return true;
}
function deleteSignal(kind, name) {
  if (!name) { flashHint(`Enter a ${kind} signal name to delete.`); return false; }
  sendCommand(`${kind} del ${name}`);
  return true;
}

/* render the device-reported signal tables + per-kind autocomplete options.
   Signals are never cached: the tables reflect whatever the device last reported. */
function renderSignalLists() {
  renderSignalList("rf", $("#rfSignals"));
  renderSignalList("ir", $("#irSignals"));
  refreshDatalists();
}
function renderSignalList(kind, el) {
  if (!el) return;
  el.innerHTML = "";

  const list = deviceSignals[kind] || [];
  if (!list.length) {
    el.innerHTML = `<span class="siglist__empty">No saved signals — capture one above or press List.</span>`;
    return;
  }

  const table = document.createElement("table");
  table.className = `sigtable sigtable--${kind}`;

  if (kind === "rf") {
    table.innerHTML = `<thead><tr><th>Name</th><th>Freq</th><th></th></tr></thead>`;
  } else {
    table.innerHTML = `<thead><tr><th>Name</th><th></th></tr></thead>`;
  }

  const tbody = document.createElement("tbody");
  list.forEach(s => { if (s) tbody.appendChild(buildSignalRow(kind, s)); });
  table.appendChild(tbody);
  el.appendChild(table);
}

/* build one table row for a device-reported signal. Clicking the row (or its
   Transmit button) replays the signal; the Delete button removes it.
   IR: {index,name,protocol,value,bits}   RF: {index,name,code,bits,protocol,pulse,band} */
function buildSignalRow(kind, s) {
  const name = s && s.name ? String(s.name) : "(unnamed)";
  const tr = document.createElement("tr");
  tr.className = `sigrow sigrow--${kind}`;
  tr.title = `${kind} tx ${name} — click to transmit`;

  const nameCell = `<td class="sigrow__name">${escapeHTML(name)}</td>`;
  const freqCell = (kind === "rf")
    ? `<td class="sigrow__freq">${s.band != null ? escapeHTML(`${s.band} MHz`) : "—"}</td>`
    : "";
  const actionCell =
    `<td class="sigrow__action">` +
      `<button type="button" class="btn btn--sm sigrow__tx sigrow__tx--${kind}" title="${kind} tx ${escapeHTML(name)}">Transmit</button>` +
      `<button type="button" class="btn btn--sm btn--ghost sigrow__del" title="${kind} del ${escapeHTML(name)} — delete">Delete</button>` +
    `</td>`;

  tr.innerHTML = nameCell + freqCell + actionCell;
  const transmit = (e) => { e.stopPropagation(); transmitSignal(kind, name); };
  const del = (e) => {
    e.stopPropagation();
    if (confirm(`Delete the ${kind.toUpperCase()} signal "${name}" from the device?`)) deleteSignal(kind, name);
  };
  tr.addEventListener("click", transmit);
  $(".sigrow__tx", tr).addEventListener("click", transmit);
  $(".sigrow__del", tr).addEventListener("click", del);
  return tr;
}

/* parse a device-published RF/list or IR/list payload and refresh that section.
   Each message carries one signal object (streamed one at a time by the firmware);
   it is merged into the running list by index. */
function handleSignalList(kind, msg) {
  let data;
  try { data = JSON.parse(msg); }
  catch { log("sys", `${kind}/list`, "invalid JSON, ignored"); return; }

  let list;
  if (Array.isArray(data)) {
    list = data;
  } else if (data && typeof data === "object") {
    list = mergeSignalObject(kind, data);
  } else {
    return;
  }
  list = list.filter(s => s && typeof s === "object" && (s.name != null || s.index != null));

  deviceSignals[kind] = list;
  renderSignalLists();
  log("sys", `${kind}/list`, `${list.length} signal${list.length === 1 ? "" : "s"} loaded`);
}

/* merge one streamed signal object into the running device list, keyed by index */
function mergeSignalObject(kind, obj) {
  const cur = Array.isArray(deviceSignals[kind]) ? deviceSignals[kind].slice() : [];
  const at = (obj.index != null) ? cur.findIndex(s => s.index === obj.index) : -1;
  if (at >= 0) cur[at] = obj; else cur.push(obj);
  return cur;
}
function refreshDatalists() {
  for (const kind of ["rf", "ir"]) {
    const dl = $(`#${kind}SignalNames`);
    if (dl) {
      const names = (deviceSignals[kind] || []).map(s => s && s.name).filter(Boolean);
      dl.innerHTML = names.map(n => `<option value="${escapeHTML(n)}"></option>`).join("");
    }
  }
}

/* toggle visual state for buzzer/motion buttons */
function markToggle(cmd) {
  const btn = $(`.btn--seg[data-cmd="${cssEscape(cmd)}"]`);
  if (!btn) return;
  const group = btn.closest(".togglepair");
  $$(".btn--seg", group).forEach(b => b.classList.remove("is-on"));
  btn.classList.add("is-on");
}

/* ----------------------------- alarm ----------------------------- */
/* The alarm topic carries two kinds of message:
   - arm/disarm status ("Motion Detection: ON/OFF"): update the motion buttons.
   - real alarm events ("Motion Detected!", "MQ2 threshold exceeded!"): show the
     banner and mark the buzzer on. The buzzer must only light up for an actual
     alarm — arming/disarming is just a status change. */
function handleAlarmMessage(msg, ts) {
  const lower = (msg || "").toLowerCase();

  if (lower.includes("motion detection: on")) {
    markToggle("motion on");
    return;
  }
  if (lower.includes("motion detection: off")) {
    markToggle("motion off");
    return;
  }

  showAlarm(msg, ts);
  markToggle("buzzer on");
}

function showAlarm(msg, ts) {
  elAlarmTitle.textContent = msg;
  elAlarmTime.textContent = formatClock(ts);
  elAlarmBanner.classList.remove("hidden");
  // restart the flash animation
  elAlarmBanner.style.animation = "none";
  void elAlarmBanner.offsetWidth;
  elAlarmBanner.style.animation = "";
}
function dismissAlarm() { elAlarmBanner.classList.add("hidden"); }

/* ----------------------------- log ----------------------------- */
function log(dir, topic, payload) {
  const dirLabel = dir === "in" ? "RX" : dir === "out" ? "TX" : "•";
  const row = document.createElement("div");
  row.className = "log__row";
  row.dataset.dir = dir;
  row.innerHTML =
    `<span class="log__t">${formatClock(Date.now())}</span>` +
    `<span class="log__topic">${escapeHTML(topic)} <span class="payload">${escapeHTML(payload)}</span></span>` +
    `<span class="log__dir ${dir}">${dirLabel}</span>`;
  elLog.appendChild(row);
  // cap length
  while (elLog.children.length > 200) elLog.removeChild(elLog.firstChild);
  applyLogFilter();
  elLog.scrollTop = elLog.scrollHeight;
}
function applyLogFilter() {
  const f = elLogFilter.value;
  $$(".log__row", elLog).forEach(r => {
    r.style.display = (f === "all" || r.dataset.dir === f) ? "" : "none";
  });
}

/* ----------------------------- status / helpers ----------------------------- */
function setStatus(state, label) {
  elStatus.dataset.state = state;
  elStatusLabel.textContent = label;
}
function setButtons(connected) {
  connectBtn.disabled = connected;
  disconnectBtn.disabled = !connected;
}
function flashHint(text) {
  elConnHint.textContent = text;
  elConnHint.style.color = "var(--warn)";
  clearTimeout(flashHint._t);
  flashHint._t = setTimeout(() => {
    elConnHint.style.color = "";
    elConnHint.textContent = "Tip: broker needs a WebSocket listener (Mosquitto: port 9001, path /mqtt).";
  }, 3500);
}

function formatClock(ts) {
  const d = new Date(ts);
  return d.toLocaleTimeString([], { hour: "2-digit", minute: "2-digit", second: "2-digit" });
}
function relativeTime(ts) {
  const s = Math.round((Date.now() - ts) / 1000);
  if (s < 2) return "just now";
  if (s < 60) return `${s}s ago`;
  const m = Math.floor(s / 60);
  if (m < 60) return `${m}m ago`;
  return `${Math.floor(m / 60)}h ago`;
}
function escapeHTML(s) {
  return String(s).replace(/[&<>"']/g, c => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" }[c]));
}
function cssEscape(s) { return window.CSS?.escape ? CSS.escape(s) : s.replace(/"/g, '\\"'); }

/* tick: refresh relative times + staleness */
setInterval(() => {
  for (const kind in cards) {
    const e = cards[kind];
    if (!e.ts) continue;
    e.time.textContent = relativeTime(e.ts);
    if (Date.now() - e.ts > STALE_MS) e.card.classList.add("is-stale");
  }
}, 1000);

/* ----------------------------- wire up the UI ----------------------------- */
function applySettingsToForm() {
  const s = loadSettings();
  for (const k of ["protocol", "host", "port", "path", "baseTopic", "device", "clientId", "username", "password"]) {
    if (s[k] !== undefined && form.elements[k]) form.elements[k].value = s[k];
  }
}

function init() {
  buildCards();
  applySettingsToForm();
  renderSignalLists();
  setStatus("disconnected", "Disconnected");
  setButtons(false);

  // connection form
  form.addEventListener("submit", (e) => { e.preventDefault(); connect(); });
  disconnectBtn.addEventListener("click", disconnect);

  // collapsible connection card
  $("#connToggle").addEventListener("click", () => {
    const btn = $("#connToggle");
    const open = btn.getAttribute("aria-expanded") === "true";
    btn.setAttribute("aria-expanded", String(!open));
    $("#connBody").classList.toggle("hidden", open);
  });

  // direct command buttons (buzzer / motion)
  $$(".btn--seg[data-cmd]").forEach(btn => {
    btn.addEventListener("click", () => {
      const cmd = btn.dataset.cmd;
      sendCommand(cmd);
      markToggle(cmd);
    });
  });

  // Default safe state: buzzer off, motion disarmed. These stop-actions are
  // always available, so show them selected until real state arrives (a retained
  // arm/disarm message or an alarm event will switch the highlight as needed).
  markToggle("buzzer off");
  markToggle("motion off");

  // RF
  const rfName = $("#rfName");
  $("#rfCapture").addEventListener("click", () => captureSignal("rf", rfName.value.trim()));
  $("#rfTransmit").addEventListener("click", () => transmitSignal("rf", rfName.value.trim()));
  $("#rfDelete").addEventListener("click", () => deleteSignal("rf", rfName.value.trim()));
  $("#rfList").addEventListener("click", () => sendCommand("rf list"));

  // IR
  const irName = $("#irName");
  $("#irCapture").addEventListener("click", () => captureSignal("ir", irName.value.trim()));
  $("#irTransmit").addEventListener("click", () => transmitSignal("ir", irName.value.trim()));
  $("#irDelete").addEventListener("click", () => deleteSignal("ir", irName.value.trim()));
  $("#irList").addEventListener("click", () => sendCommand("ir list"));

  // enter-to-capture on the name fields
  rfName.addEventListener("keydown", e => { if (e.key === "Enter") { e.preventDefault(); captureSignal("rf", rfName.value.trim()); } });
  irName.addEventListener("keydown", e => { if (e.key === "Enter") { e.preventDefault(); captureSignal("ir", irName.value.trim()); } });

  // raw command
  const rawCmd = $("#rawCmd");
  $("#rawSend").addEventListener("click", () => { sendCommand(rawCmd.value); rawCmd.value = ""; });
  rawCmd.addEventListener("keydown", e => { if (e.key === "Enter") { e.preventDefault(); sendCommand(rawCmd.value); rawCmd.value = ""; } });

  // log
  elLogFilter.addEventListener("change", applyLogFilter);
  $("#logClear").addEventListener("click", () => { elLog.innerHTML = ""; });

  // alarm dismiss
  $("#alarmDismiss").addEventListener("click", dismissAlarm);

  log("sys", "ready", "dashboard loaded — enter broker details and Connect.");
}

document.addEventListener("DOMContentLoaded", init);
