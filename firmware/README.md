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

## WiFi and notifications
`include/secrets.h` defines `WIFI_SSID`, `WIFI_PASS`, `NTFY_TOPIC`. Install the ntfy app and subscribe to
the same topic (use an unguessable name; ntfy.sh topics are public). Self-hosted server: define `NTFY_SERVER`.
Events: DOSE_READY, DOSE_NOT_TAKEN (after `NOT_TAKEN_MINUTES`), JAM, DOUBLE, LOW_PILLS, OFFLINE_RTC
(sent when WiFi returns after more than 5 min offline). Web UI: `http://<device-ip>/` (IP printed on Serial).

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
starts while one is pending. Button A dispenses the next upcoming enabled event (wrapping to the first) and marks
it done for today.

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
`GET /api/status`, `GET|POST /api/schedule` (`{"events":[{hour,minute,counts:[5],enabled}]}`),
`POST /api/dispense` (`{slot:0-4,count}`), `POST /api/ack`, `POST /api/counts` (`{counts:[5],low:n}`).
No authentication (LAN only).

## TODO / stubbed
- Not compiled or tested on hardware yet (PlatformIO was not available when this was written).
- No web UI/API authentication; do not expose to the internet.
- ntfy TLS uses `setInsecure()` (no certificate pinning).
- Notifications are dropped when offline (no queue/retry).
- Disk geometry: `STEPS_PER_PILL` assumes one pill per full revolution; tune `STEP_DELAY_MS`, debounce and settle times on real hardware.
- Dispense blocks the main loop (web server is served only between moves; `delay()` keeps WiFi alive).
- Time zone is a compile-time string; RTC stores local time.
- OLED screen is a minimal status dump.
- Buzzer patterns are simple; no mute/quiet hours.
- No unit tests; no motor homing/position tracking between slots.
