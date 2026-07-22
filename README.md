# Inteligence — ESP32 Home Intelligence Hub

A Wi-Fi-connected, MQTT-integrated home automation and sensing node for the
**ESP32** (Arduino framework via PlatformIO). It reads a set of environmental
sensors, publishes them over MQTT, and can **capture and replay RF (433 & 315 MHz)
and infrared signals** — turning the board into a bridge between physical remotes,
sensors, and your smart-home broker.

On first boot it opens a captive setup portal, so no Wi-Fi or MQTT credentials are
hard-coded into the firmware.

---

## Table of Contents

- [Features](#features)
- [Hardware](#hardware)
- [Wiring](#wiring)
- [Software Architecture](#software-architecture)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
- [First-Boot Provisioning](#first-boot-provisioning)
- [Configuration](#configuration)
- [Usage](#usage)
  - [Serial & MQTT Commands](#serial--mqtt-commands)
  - [MQTT Topics](#mqtt-topics)
- [RF — Dual Band (433 & 315 MHz)](#rf--dual-band-433--315-mhz)
- [Persistent Storage](#persistent-storage)
- [Dependencies](#dependencies)
- [Known Issues & To-Do](#known-issues--to-do)

---

## Features

- **Provisioning portal** — configure Wi-Fi + MQTT from a phone/laptop on first
  boot; no rebuild required to change networks.
- **MQTT telemetry** — publishes light, temperature, humidity, gas, and motion
  readings on a configurable base topic.
- **MQTT control** — the same commands available over Serial are accepted over an
  MQTT command topic.
- **RF capture & replay** — record 433 *or* 315 MHz signals by name and replay
  them later. Both bands are handled simultaneously (see
  [RF — Dual Band](#rf--dual-band-433--315-mhz)).
- **IR capture & replay** — record infrared signals (NEC / SONY / SAMSUNG or raw)
  and replay them.
- **Local persistence** — all saved RF/IR signals and the provisioning profile are
  stored in LittleFS and survive reboots.
- **Built-in alarm** — a buzzer that can be triggered manually, by gas threshold,
  or by motion.

## Hardware

**Board:** ESP32 DevKit (any common 38-pin variant).

| Part | Purpose |
|------|---------|
| DHT11 | Temperature & humidity |
| LDR (photoresistor) | Ambient light level |
| MQ-2 | Combustible gas / smoke |
| HC-SR501 (PIR) | Motion detection |
| Active buzzer | Alarm |
| 433 MHz TX + RX pair (e.g. FS1000A / XY-MK-5V) | 433 MHz RF |
| 315 MHz TX + RX pair | 315 MHz RF |
| IR receiver (e.g. TSOP38238) + IR LED | Infrared capture & transmit |

> The ESP32's ADC1 input-only pins (GPIO 34/35) are used for the analog sensors.

## Wiring

| Device | ESP32 GPIO | Notes |
|--------|-----------|-------|
| DHT11 data | **33** | 10 kΩ pull-up recommended |
| LDR (analog) | **34** | Input-only ADC1 pin |
| MQ-2 (analog AO) | **35** | Input-only ADC1 pin |
| PIR OUT | **13** | |
| Buzzer (+) | **12** | Active-high buzzer |
| IR receiver DATA | **14** | |
| IR LED (transmit) | **25** | |
| 433 MHz RX DATA | **27** | |
| 433 MHz TX DATA | **26** | |
| 315 MHz RX DATA | **4** | *Adjustable — see config* |
| 315 MHz TX DATA | **5** | *Adjustable — see config* |

All modules share the ESP32's 3.3 V / 5 V and GND. Note that most 433/315 and IR
transmitter/receiver boards run at **5 V**; power them from `VCC` (5 V), and level-
shift the TX *data* lines (GPIO 26 / 5 / 25) if your module is not 5 V-tolerant.
Receiver DATA lines are safe to read directly.

---

## Software Architecture

The firmware is organized into independent modules, each with a header in
`include/<Module>/` and an implementation in `src/<Module>/`. A single `Sensors`
module orchestrates the others from `loop()`.

| Module | Responsibility |
|--------|----------------|
| `main` | Setup/loop entry; routes MQTT commands to the serial handler |
| `Sensors` | Periodic sensor reads, RF/IR receive polling, alarm ticking |
| `Provisioning` | Captive Wi-Fi AP + web form to capture Wi-Fi/MQTT creds |
| `MQTT` | Wi-Fi STA connect, PubSubClient connect/reconnect, publish/subscribe |
| `Serial` | Text command parser (shared by Serial and MQTT) + `help` |
| `RF` | Dual-band (433/315) RF capture & replay via RCSwitch |
| `IR` | Infrared capture & replay via IRremoteESP8266 |
| `Alarm` | Buzzer on/off + 2 s duty-cycle while alarming |
| `DHT`, `LDR`, `MQ2`, `PIR` | Individual sensor drivers |
| `Memory` | LittleFS JSON load/save for RF, IR, and provisioning data |
| `config` | Central pin map, MQTT topics, timing constants |

### Runtime loop

`setup()` → begins Serial → runs provisioning portal if not yet provisioned → sets
up sensors + MQTT → prints help.

`loop()` → `handleSerial()` → `handleMQTT()` → `handleSensors()`.

`handleSensors()` polls for an in-progress RF/IR capture, ticks the alarm buzzer,
optionally checks PIR motion, and every 10 s prints and publishes the sensor suite
(LDR, temp, humidity, PIR). MQ-2 is read on its own 10 s cadence after a 40 s
warm-up.

## Project Structure

```
.
├── platformio.ini          # Build config, library deps, LittleFS filesystem
├── config.h (include/)     # Pins, topics, intervals (single source of truth)
├── include/                # Headers per module
│   └── <Module>/*.h
├── src/
│   ├── main.cpp
│   └── <Module>/*.cpp
├── lib/                    # (project-local libraries, currently empty)
├── test/
└── note.md                 # Developer to-do / known-bugs scratchpad
```

---

## Getting Started

1. **Install PlatformIO**

   ```bash
   python -m pip install platformio
   ```

   (Or use the PlatformIO IDE extension for VS Code.)

2. **Build**

   ```bash
   pio run
   ```

3. **Upload to the board**

   ```bash
   pio run -t upload
   ```

4. **Open the serial monitor** (115200 baud)

   ```bash
   pio device monitor
   ```

> LittleFS is created/formatted automatically on first run — there is no `data/`
> folder to upload. The provisioning and signal files are written at runtime.

## First-Boot Provisioning

When the board boots without a saved profile it starts a Wi-Fi access point:

| | |
|---|---|
| **SSID** | `Inteligence-Setup` |
| **Password** | `123456` |
| **Setup URL** | `http://192.168.4.1` |

1. Connect to the `Inteligence-Setup` Wi-Fi network.
2. Open `http://192.168.4.1` in a browser.
3. Fill in your Wi-Fi SSID/password and MQTT broker host/port (and optional user/
   pass), then **Save & Restart**.
4. The board reboots, joins your Wi-Fi, and connects to the broker.

If no provisioning profile exists, the firmware falls back to the `WIFI_SSID` /
`MQTT_HOST` constants in `config.h`. To re-provision, delete `/provisioning.json`
from LittleFS (or erase flash).

## Configuration

All tunables live in [`include/config.h`](include/config.h):

- **Pins** — the wiring table above.
- **MQTT** — `MQTT_BASE_TOPIC` (`homeinteligence`), broker host/port, client ID,
  credentials (used only as a fallback; the portal overrides them).
- **Timing** — sensor interval (10 s), MQ-2 preheat (40 s), MQ-2 threshold (600),
  buzzer duty cycle (2 s).

---

## Usage

The same text commands work two ways:
- typed into the **Serial monitor** at 115200, or
- published to the **MQTT command topic** `homeinteligence/command`.

### Serial & MQTT Commands

| Command | Action |
|---------|--------|
| `rf rx <name>` | Wait for an RF signal (433 **or** 315) and save it as `<name>` |
| `rf tx <name>` | Transmit the RF signal saved as `<name>` (on its original band) |
| `rf list` | List saved RF signals (name, band, code, bits, protocol, pulse) |
| `ir rx <name>` | Wait for an IR signal and save it as `<name>` |
| `ir tx <name>` | Transmit the IR signal saved as `<name>` |
| `ir list` | List saved IR signals |
| `buzzer on` / `buzzer off` | Start / stop the alarm buzzer |
| `motion on` / `motion off` | Start / stop the motion detection |
| `help` | Print the command list |

**Examples**

```
rf rx living_room_light      # press your 433/315 remote; it's captured + saved
rf tx living_room_light      # replay it later (correct band chosen automatically)
ir rx tv_power
ir tx tv_power
buzzer off
```

### MQTT Topics

Published to (base = `homeinteligence`):

| Topic | Payload | Trigger |
|-------|---------|---------|
| `homeinteligence/sensor/LDR` | light reading | every 10 s |
| `homeinteligence/sensor/TEMP` | °C | every 10 s |
| `homeinteligence/sensor/HUM` | % humidity | every 10 s |
| `homeinteligence/sensor/PIR` | `0`/`1` | every 10 s |
| `homeinteligence/sensor/MQ2` | gas reading | every 10 s (after warm-up) |
| `homeinteligence/alarm` | `MQ2 threshold exceeded!` / `Motion Detected!` | when an incident occured (also starts the buzzer) |

Subscribed to:

| Topic | Payload |
|-------|---------|
| `homeinteligence/command` | any command from the table above |

So to toggle the buzzer from your broker, publish `buzzer on` to
`homeinteligence/command`.

---

## RF — Dual Band (433 & 315 MHz)

RF supports **both 433 MHz and 315 MHz simultaneously**. Because a receiver module
is frequency-specific (a 433 module cannot decode a 315 MHz signal and vice-versa),
the firmware drives **two independent `RCSwitch` instances** — one per band — each
on its own RX/TX pin pair:

| Band | RX pin | TX pin |
|------|--------|--------|
| 433 MHz | GPIO 27 | GPIO 26 |
| 315 MHz | GPIO 4 | GPIO 5 |

- **Capture:** `rf rx <name>` listens on *both* receivers; whichever band actually
  receives the signal is recorded. The console confirms the band, e.g.
  `Saved RF signal: living_room_light (433 MHz)`.
- **Replay:** `rf tx <name>` automatically uses the band that signal was captured
  on — no need to specify it.

Each saved signal stores `code`, `bits`, `protocol`, `pulse`, and `band`, so it is
always replayed faithfully on the right hardware. If the default 315 pins (4/5)
clash with your board, change `RF_RX_315_PIN` / `RF_TX_315_PIN` in
[`config.h`](include/config.h).

---

## Persistent Storage

LittleFS holds three JSON files (created on demand):

| File | Contents |
|------|----------|
| `/provisioning.json` | Wi-Fi + MQTT credentials from the portal |
| `/rf_signals.json` | Saved RF signals (up to 20) |
| `/ir_signals.json` | Saved IR signals (up to 20, raw buffer up to 200 samples) |

Old RF captures saved before dual-band support are loaded with a default band of
**433 MHz**, so existing recordings keep working.

## Dependencies

Declared in [`platformio.ini`](platformio.ini):

- `adafruit/DHT sensor library` + `adafruit/Adafruit Unified Sensor` — DHT11
- `sui77/rc-switch` — 433/315 MHz RF
- `crankyoldgit/IRremoteESP8266` — IR send/receive
- `bblanchon/ArduinoJson` — JSON config & signal storage
- `knolleary/PubSubClient` — MQTT client

PlatformIO installs these automatically on first build.

---

## Known Issues & To-Do

Tracked in [`note.md`](note.md):

- **IR TX does not currently work** (RX is functional).
- MQ-2 driver is flagged for cleanup but operates correctly.
- RF functionality is the active area of work (dual-band support recently added).

Planned hardening:

- Proper error reporting for DHT reads (currently NaN is published as `0`).
- Robust error handling for LittleFS file open failures.

---

*No license has been set for this project. Add one before sharing or deploying.*
