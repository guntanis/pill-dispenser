# 5-slot pill dispenser firmware (ESP32, PlatformIO + Arduino)

## Build and flash
```
cp include/secrets.h.example include/secrets.h   # then edit (gitignored)
pio run                    # build
pio run -t upload          # flash firmware
pio run -t uploadfs        # flash data/ to LittleFS (optional; schedule.json is auto-created on first boot)
pio device monitor         # 115200 baud
```
Feature flags (in `src/config.h`, or `build_flags = -DENABLE_OLED=1`): `ENABLE_OLED`, `NOTIFY_BACKEND_NTFY`.
Set `TZ_STRING` in `config.h` to your POSIX timezone (default `UTC0`).

## WiFi, web credentials and notifications
`include/secrets.h` defines `WIFI_SSID`, `WIFI_PASS`, `NTFY_TOPIC`, `WEB_USER` and `WEB_PASS`. The build fails
if `WEB_USER`/`WEB_PASS` are missing; the web UI never runs without auth.
Install the ntfy app and subscribe to the same topic (use an unguessable name; ntfy.sh topics are public).
Self-hosted server: define `NTFY_SERVER`.
Events: DOSE_READY, DOSE_NOT_TAKEN (after `NOT_TAKEN_MINUTES`), JAM, DOUBLE, LOW_PILLS, OFFLINE_RTC
(sent when WiFi returns after more than 5 min offline), LIMIT (daily cap or manual lockout refused pills),
SENSOR (beam blocked before or after a dose), CLOCK_INVALID (no valid time, schedule suspended),
CONFIG_CHANGED (schedule or counts changed via the web, with the client IP).
Web UI: `http://<device-ip>/` (IP printed on Serial); the browser asks for `WEB_USER`/`WEB_PASS`.

## Safeguards
Everything here is enforced in firmware regardless of how a dose was requested (schedule, web, button).

| Safeguard | Setting | What it does |
|---|---|---|
| Once per event per day | | An event's "last dispensed" date is written to NVS before the motor moves. A reboot or power blip mid-dose cannot dose it again. |
| Catch-up window | `CATCHUP_MINUTES` (60) | A missed event fires late after an outage up to this long, then is skipped for the day. |
| One dose at a time | | No scheduled dose starts while a previous one is pending (not yet acknowledged or cup not lifted). |
| Per-dose cap | `MAX_PILLS_PER_DOSE` (9) | Clamped when the schedule is saved and on `POST /api/dispense`. |
| Per-slot daily cap | `MAX_PILLS_PER_SLOT_PER_DAY` (10) | Counts pills actually detected per hopper per calendar day, persisted in NVS. A request over the cap is clamped and a LIMIT notification is sent. While the clock is invalid the counter never resets. |
| Manual lockout | `MANUAL_LOCKOUT_MINUTES` (30) | Web and button dispenses of the same slot are refused within this window of the last dose from that slot. Kept both in RAM and NVS. Scheduled doses are not subject to it. |
| Button A hold | `BTN_A_HOLD_MS` (1000) | Button A must be held; a tap does nothing. |
| Cup present | | Checked before the dose and between pills. Dispense aborts if the cup is removed. |
| Beam sanity | | Refuses to dispense if the beam already reads "broken" (dead emitter, blocked funnel). Alerts if it is still broken after a dose (pill stuck in the throat). |
| Motor driver present | | Refuses to dispense if either MCP23017 was not found at boot. Coil lines are driven low at boot and after every move; only one motor is ever energized. |
| Clock validity | `CLOCK_INVALID_NOTIFY_MS` (5 min) | Scheduled doses only run with a valid time (RTC or NTP, year >= 2024). If no valid time this long after boot, a CLOCK_INVALID notification is sent and the indicator stays red. Button A is refused while the clock is invalid. |
| Watchdog | `WDT_TIMEOUT_S` (30) | ESP32 task watchdog, fed from the main loop and from every motor half-step. A hang (for example an I2C lock-up mid-move) reboots the device; the once-per-day mark was already written so it does not re-dose. |
| Web auth | `WEB_USER`, `WEB_PASS` | HTTP digest auth on every route including `/`. |
| Config audit | | Every schedule or count change via the API sends a CONFIG_CHANGED notification with the client IP. |

These make the firmware harder to misuse. They do not make it a medical device; see the safety section in the root README.

## Dispense, retry and double-drop logic
For each pill: arm the beam sensor, turn the disk `STEPS_PER_PILL` half-steps (default one full rev), and
poll the beam on every half-step so a fast drop mid-rotation is not missed. After the move it waits up to
`DROP_TIMEOUT_MS`. Once a break is seen it keeps listening for `DROP_SETTLE_MS`; each debounced break is one drop.
- 0 drops: agitate (rock +/-15 deg, 3 cycles), listen `AGITATE_WAIT_MS`, then rotate again. Up to `MAX_RETRIES`
  retries. If it still sees nothing, that slot stops, a JAM notification is sent and the error indicator shows.
- 1 drop: success. 2 or more drops: counted as dropped, flagged DOUBLE (notification + error indicator).
- The cup switch must read "present" before and between pills; otherwise the dispense is refused.
- Pill counts are decremented by pills actually detected.

Dose lifecycle: scheduled/manual dose -> pending (persisted in NVS) -> acknowledged by button B, `POST /api/ack`,
or the cup being lifted and replaced. The event's "last dispensed date" is written before the motor moves, so a
reboot cannot double-dose. After an outage an event still fires up to `CATCHUP_MINUTES` late. No new scheduled dose
starts while one is pending. Button A (held for `BTN_A_HOLD_MS`) dispenses the earliest enabled event not yet
dispensed today and marks it done, subject to the manual lockout.

## Pin map
| Function | Pin / address |
|---|---|
| I2C SDA / SCL | GPIO21 / GPIO22 |
| MCP23017 #0 | 0x20: GPA0-3 motor1, GPA4-7 motor2, GPB0-3 motor3, GPB4-7 motor4 (IN1-IN4) |
| MCP23017 #1 | 0x21: GPA0-3 motor5 (rest spare) |
| DS3231 RTC | 0x68 |
| SSD1306 OLED (optional) | 0x3C |
| IR break-beam | GPIO27, INPUT_PULLUP, LOW = broken |
| Cup switch | GPIO26, INPUT_PULLUP, LOW = cup present |
| Button A (dispense now) | GPIO32, INPUT_PULLUP |
| Button B (taken / ack) | GPIO33, INPUT_PULLUP |
| Buzzer (active) | GPIO25, HIGH = on |
| NeoPixel data | GPIO13 (count in `NEOPIXEL_COUNT`) |

## HTTP API
All routes require HTTP digest auth with `WEB_USER`/`WEB_PASS`.
`GET /api/status` (includes `timeValid`, `today[5]` pills dispensed per slot, `capPerDay`, `lockoutMin`, `note`),
`GET|POST /api/schedule` (`{"events":[{hour,minute,counts:[5],enabled}]}`),
`POST /api/dispense` (`{slot:0-4,count}`), `POST /api/ack`, `POST /api/counts` (`{counts:[5],low:n}`).

## TODO / stubbed
- Not compiled or tested on hardware yet (PlatformIO was not available when this was written).
- Web auth is digest over plain HTTP (no TLS). Keep the device on your LAN; do not port-forward it.
- ntfy TLS uses `setInsecure()` (no certificate pinning).
- Notifications are dropped when offline (no queue/retry).
- Disk geometry: `STEPS_PER_PILL` assumes one pill per full revolution; tune `STEP_DELAY_MS`, debounce and settle times on real hardware.
- Dispense blocks the main loop (web server is served only between moves; `delay()` keeps WiFi alive).
- Time zone is a compile-time string; RTC stores local time.
- OLED screen is a minimal status dump.
- Buzzer patterns are simple; no mute/quiet hours.
- No unit tests; no motor homing/position tracking between slots.
