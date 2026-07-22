# Inteligence — Web Dashboard

A self-contained, server-less web dashboard for the **Inteligence** ESP32 hub.
It connects **directly from your browser to your MQTT broker over WebSocket**,
shows live sensor readings, and sends control commands to the device — exactly
the same commands that work over Serial.

```
┌────────────┐   WebSocket (ws/wss)   ┌──────────────┐   TCP 1883   ┌────────────┐
│  Browser   │ ─────────────────────▶ │  MQTT broker │ ───────────▶ │   ESP32    │
│  dashboard │ ◀───────────────────── │  (Mosquitto) │ ◀─────────── │  (firmware) │
└────────────┘   sensor/+alarm  (sub) └──────────────┘  command (pub)└────────────┘
        publish → command                       publish → sensor/*
```

No backend, no build step, no API keys. Open `index.html` (or serve the folder)
and point it at your broker.

---

## What it does

**Live telemetry** — subscribes to the device's sensor topics and renders a card
per sensor with the current value, a status chip, a normalized bar, and a rolling
sparkline:

| Card | Topic | Payload | Shown as |
|------|-------|---------|----------|
| Light | `…/sensor/LDR` | raw ADC (0–4095) | value + % of range + sparkline |
| Temperature | `…/sensor/TEMP` | °C | value + comfort chip + sparkline |
| Humidity | `…/sensor/HUM` | % | value + comfort chip + sparkline |
| Motion | `…/sensor/PIR` | `0` / `1` | CLEAR / MOTION state |
| Gas | `…/sensor/MQ2` | raw ADC (0–4095) | value + threshold bar + SAFE/DANGER |

(`…` = your base topic, default `homeinteligence`.)

**Alarm banner** — when a message arrives on `…/alarm`
(`MQ2 threshold exceeded!` or `motion detected`), a dismissible banner appears.

**Control** — publishes to `…/command`. Buttons cover every device command:

| UI action | Command published |
|-----------|-------------------|
| Buzzer On / Off | `buzzer on` / `buzzer off` |
| Motion Arm / Disarm | `motion on` / `motion off` |
| RF Capture | `rf rx <name>` |
| RF Transmit | `rf tx <name>` |
| RF List | `rf list` |
| IR Capture / Transmit / List | `ir rx|tx <name>` / `ir list` |
| Raw command box | any text (e.g. `help`) |

> **Note on `list`:** the device prints `rf list` / `ir list` results to **Serial
> only** — they are not published over MQTT, so they appear in your serial
> monitor, not on this page. Names you capture *through the dashboard* are
> remembered locally (in the browser) and offered as autocomplete for transmit.

**Activity log** — a terminal-style panel showing every incoming (`RX`) and
outgoing (`TX`) MQTT message, with filter and clear.

---

## Prerequisites: enable WebSocket on your broker

Browsers cannot speak raw MQTT/TCP — they need **MQTT over WebSocket**. Your
broker must expose a WebSocket listener. For **Mosquitto**, add a `listener`:

```conf
# /etc/mosquitto/mosquitto.conf  (or your docker config)
listener 1883           # normal MQTT (used by the ESP32)
listener 9001           # WebSocket (used by this dashboard)
protocol websockets
allow_anonymous true
```

Then restart Mosquitto (`systemctl restart mosquitto`). The dashboard's defaults
match this: **port 9001, path `/mqtt`**.

> If your Mosquitto is older and ignores the path, leave the path field as
> `/mqtt` — it is harmless. For TLS-terminated front-ends (e.g. behind nginx),
> switch the protocol to `wss` and set the port accordingly (443).

---

## Run it

### Option A — just open the file
Double-click `index.html`. Works for `ws://` against a broker on your LAN.
(Some browsers restrict `file://` for `wss://`/mixed content — serve it if so.)

### Option B — serve the folder (recommended)
Any static server works. From the `Website/` directory:

```bash
# Python (built-in)
python3 -m http.server 8080

# or Node
npx serve .
```

Then open <http://localhost:8080>.

---

## Connecting

1. In the **Broker Connection** card, enter your broker's **host**
   (e.g. `192.168.1.10`). Port defaults to `9001`, path to `/mqtt`, base topic to
   `homeinteligence` — change them only if your setup differs.
2. Add a **username / password** if your broker requires auth.
3. Click **Connect**. The status pill turns green and the log shows the
   subscriptions.
4. Sensor cards populate within ~10 s (the device publishes every 10 s; MQ-2
   after a 40 s warm-up).

Settings are remembered in `localStorage`, so reconnecting later is one click.

---

## Files

```
Website/
├── index.html              # dashboard markup
├── css/style.css           # dark dashboard theme
├── js/
│   ├── app.js              # MQTT connect/sub/publish, rendering, controls
│   └── lib/
│       ├── mqtt.min.js     # MQTT.js (browser WebSocket client) — bundled
│       └── chart.umd.min.js# Chart.js — bundled
└── README.md               # this file
```

The two libraries are **bundled locally** so the dashboard works on a LAN with no
internet. To upgrade, replace the files under `js/lib/`
(`mqtt.min.js` from `unpkg.com/mqtt`, `chart.umd.min.js` from `cdn.jsdelivr.net/npm/chart.js`).

---

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| Stuck on "Connecting…" / red status | Broker host wrong, or no WebSocket listener on that port. Verify port 9001 + `protocol websockets` in Mosquitto. |
| `wss` certificate errors | Use a valid cert, or use plain `ws` on the LAN. Browsers block mixed content if the page is `https` but the broker is `ws`. |
| Connected but no sensor data | Base topic mismatch. It must equal `MQTT_BASE_TOPIC` in `include/config.h` (default `homeinteligence`). |
| Commands don't trigger the device | Confirm the device is subscribed (serial prints `MQTT subscribed: homeinteligence/command`) and online. |
| `rf list` results missing | Expected — see the note above; they go to Serial only. |
