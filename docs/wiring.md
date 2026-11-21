# Pill Dispenser - Wiring Guide

5-slot automatic pill dispenser. ESP32 DevKit V1 (30-pin) + 2x MCP23017 I/O expanders driving 5x 28BYJ-48 steppers through ULN2003 boards.

This pin map is fixed; firmware is written against it. Do not change pins or I2C addresses without updating the firmware.

A block-level diagram is also in `wiring.svg`.

---

## 1. Overview block diagram

```
                         POWER (5 V domain)
 ┌──────────┐   ┌──────┐   ┌────────┐   ┌─────────┐   ┌─────────────┐
 │ 5V 3A    ├──►│Barrel├──►│ Fuse   ├──►│ Power   ├──►│ Bulk cap    │
 │ adapter  │   │ jack │   │ 2A slow│   │ switch  │   │ 470-1000 uF │
 └──────────┘   └──────┘   └────────┘   └─────────┘   └──────┬──────┘
                                                             │
        ═══════════════════════ 5V RAIL ═════════════════════╪═════════════
          │            │              │             │        │ (GND star point
          │VIN         │VCC           │VCC          │5V      │  at the cap)
          ▼            ▼              ▼             ▼
   ┌────────────┐  5x ULN2003     NeoPixel      Buzzer (+)
   │ ESP32      │  boards A-E     strip         via NPN
   │ DevKit V1  │  (+5V for motor + 330R data   (GPIO25)
   │            │   common)       + 1000uF)
   │  3V3 ──────┼──┐              ▲ GPIO13
   │  GPIO21 SDA│  │              │
   │  GPIO22 SCL│  │   3.3 V RAIL (from ESP32 on-board regulator)
   └─┬──┬──┬────┘  │   ═══════════╪════════════════════════════════
     │  │  │       │              │
     │  │  │       ├──► MCP23017 #0, #1, DS3231, OLED (VCC)
     │  │  │       ├──► 4.7k pull-ups (SDA, SCL)
     │  │  │       └──► IR emitter (via resistor), IR receiver
     │  │  │
     │  │  └─ GPIO27 ◄── IR break-beam receiver (pill drop)
     │  └──── GPIO26 ◄── cup microswitch
     └─────── GPIO32/33 ◄── buttons A / B

              I2C BUS (SDA GPIO21 / SCL GPIO22, 3.3 V, 4.7k pull-ups)
 ESP32 ───┬───────────────┬───────────────┬───────────────┬──────────────
          │               │               │               │
   ┌──────┴─────┐  ┌──────┴─────┐  ┌──────┴─────┐  ┌──────┴─────┐
   │ MCP23017#0 │  │ MCP23017#1 │  │  DS3231    │  │ SSD1306    │
   │   0x20     │  │   0x21     │  │  RTC 0x68  │  │ OLED 0x3C  │
   │            │  │            │  │  +CR2032   │  │ (optional) │
   └─┬──┬──┬──┬─┘  └─────┬──────┘  └────────────┘  └────────────┘
     │  │  │  │          │
   GPA0-3 GPA4-7 GPB0-3 GPB4-7   GPA0-3
     │     │     │      │          │
   ULN-A  ULN-B ULN-C  ULN-D      ULN-E
     │     │     │      │          │
   Motor1 Motor2 Motor3 Motor4    Motor5
   (28BYJ-48 x5, one energized at a time)

 UI:  Button A (GPIO32), Button B (GPIO33), Buzzer (GPIO25), NeoPixel (GPIO13),
      OLED (I2C), cup switch (GPIO26)
 Sensor: IR break-beam (GPIO27)
```

---

## 2. ESP32 pin map

| ESP32 pin | Function | Connects to | Notes |
|---|---|---|---|
| VIN | 5 V power in | 5 V rail (after fuse, switch, bulk cap) | Feeds the on-board 3.3 V regulator. Do **not** also connect USB-powered 5 V to the rail without a diode (see Power tips). |
| GND | Ground | Star ground point | Use any GND pin; make the star connection short and thick. |
| 3V3 | 3.3 V out | 3.3 V rail: MCP23017 x2, DS3231, OLED, pull-ups, IR parts | Regulator is ~800 mA class (AMS1117); our 3.3 V loads are well under 100 mA besides the ESP32 itself. |
| GPIO21 | I2C SDA | SDA bus: MCP23017 #0/#1, DS3231, OLED | 4.7 kΩ pull-up to 3.3 V (one pair for the bus). 3.3 V logic. |
| GPIO22 | I2C SCL | SCL bus (same devices) | 4.7 kΩ pull-up to 3.3 V. 3.3 V logic. |
| GPIO27 | Pill drop sensor | IR break-beam receiver output | Input, internal pull-up. Active LOW (LOW = beam broken = pill detected). 3.3 V logic. |
| GPIO26 | Cup-present switch | Microswitch, other side to GND | Input, internal pull-up. Closed (LOW) = cup present, as wired by the firmware polarity. |
| GPIO32 | Button A "dispense now" | Pushbutton to GND | Input, internal pull-up. Active LOW. |
| GPIO33 | Button B "taken / acknowledge" | Pushbutton to GND | Input, internal pull-up. Active LOW. |
| GPIO25 | Buzzer drive | 1 kΩ to base of NPN; NPN switches buzzer to GND | Output, HIGH = buzz. 3.3 V logic. |
| GPIO13 | NeoPixel data | 330 Ω series resistor, then strip DIN | Output. 3.3 V data to 5 V strip (see section 9 for level shifting). |
| EN | Reset | Optional reset button to GND | Leave as board default. |

### Pins deliberately avoided

| Pins | Why |
|---|---|
| GPIO 6-11 | Wired to the on-module SPI flash. Using them crashes the chip. |
| GPIO 0, 2, 12, 15 (strapping) | Their level at boot selects boot mode / flash voltage (GPIO12 = flash voltage; a high level there can prevent boot). Pull-ups/downs or attached loads can cause failed boots or boot-log noise. |
| GPIO 34-39 | Input-only and have **no internal pull-ups/pull-downs**, so they would need external resistors for switches. |
| GPIO1/3 | UART0 (USB serial) - kept free for programming and logs. |

---

## 3. MCP23017 pin assignments

Both chips: VDD = 3.3 V, VSS = GND, RESET tied to 3.3 V, 100 nF ceramic across VDD/VSS placed close to the chip, SDA/SCL to the I2C bus, INTA/INTB unused (leave open).

| Chip | Address | A2 A1 A0 |
|---|---|---|
| MCP23017 #0 | 0x20 | GND GND GND |
| MCP23017 #1 | 0x21 | GND GND VCC (3.3 V) |

| MCP23017 pin | Chip | ULN2003 board input | Motor / slot |
|---|---|---|---|
| GPA0 | #0 (0x20) | A: IN1 | Motor 1 |
| GPA1 | #0 | A: IN2 | Motor 1 |
| GPA2 | #0 | A: IN3 | Motor 1 |
| GPA3 | #0 | A: IN4 | Motor 1 |
| GPA4 | #0 | B: IN1 | Motor 2 |
| GPA5 | #0 | B: IN2 | Motor 2 |
| GPA6 | #0 | B: IN3 | Motor 2 |
| GPA7 | #0 | B: IN4 | Motor 2 |
| GPB0 | #0 | C: IN1 | Motor 3 |
| GPB1 | #0 | C: IN2 | Motor 3 |
| GPB2 | #0 | C: IN3 | Motor 3 |
| GPB3 | #0 | C: IN4 | Motor 3 |
| GPB4 | #0 | D: IN1 | Motor 4 |
| GPB5 | #0 | D: IN2 | Motor 4 |
| GPB6 | #0 | D: IN3 | Motor 4 |
| GPB7 | #0 | D: IN4 | Motor 4 |
| GPA0 | #1 (0x21) | E: IN1 | Motor 5 |
| GPA1 | #1 | E: IN2 | Motor 5 |
| GPA2 | #1 | E: IN3 | Motor 5 |
| GPA3 | #1 | E: IN4 | Motor 5 |
| GPA4-7, GPB0-7 | #1 | spare | Unused. Leave unconnected (configured as outputs LOW or inputs by firmware). |

**Why 3.3 V is enough for the ULN2003:** each ULN2003 input has an internal ~2.7 kΩ series resistor; at a 3.3 V drive that is roughly 0.8-0.9 mA per input, and the part's guaranteed turn-on input voltage (V_I(on)) is about 2.4 V at full rated load, so 3.3 V has comfortable margin. The 28BYJ-48 coils draw far less than the ULN2003's 500 mA per channel limit. The MCP23017 can source about 25 mA per pin, so each pin has an enormous margin. The ULN2003 inputs have internal pull-downs, so motors stay off during the MCP23017 power-up default (all pins as inputs), but still have firmware initialise the pins as outputs LOW as soon as possible.

---

## 4. Power subsystem

```
 5V 3A wall adapter (center-positive, 5.5x2.1 mm)
        │+                                        │-
        ▼                                         │
   ┌─────────┐                                    │
   │ Barrel  │                                    │
   │  jack   │                                    │
   └──┬───┬──┘                                    │
      │+  │-──────────────────────────────────────┤
      │                                           │
   [ FUSE 2A slow-blow, inline holder ]           │
      │                                           │
   [ SPST rocker switch ]                         │
      │                                           │
      ├────────────┬────────────► +5V RAIL        │
      │          ──┴── C1 470-1000uF/10V+         │
      │          ──┬── (observe polarity)         │
      │            │                              │
      └────────────┴──────────────────────────────┴──► GND STAR POINT
                                                      (all grounds return here)

 +5V RAIL ──► ESP32 VIN
          ──► ULN2003 boards A-E "+" (VCC) pins (x5, via JST-XH or a bus bar)
          ──► NeoPixel 5V (with its own 1000uF cap at the strip)
          ──► Buzzer (+)

 ESP32 3V3 pin ──► 3.3V RAIL ──► MCP23017 x2, DS3231, OLED, pull-ups, IR parts
```

Notes:
- Order: jack, fuse, switch, bulk cap. Fuse first so a short anywhere downstream is protected.
- Choose a **2 A slow-blow** fuse (peak draw is about 1.1 A; see budget). It protects the 22 AWG wiring from the 3 A adapter.
- Optional: a 1N5819 Schottky in series (or a P-MOSFET reverse-protection circuit) after the switch if a wrong-polarity adapter is a risk. The 0.3-0.4 V drop is tolerable for the 5 V domain but costs motor torque; skip it if you use a labelled, center-positive adapter.
- Common ground: all GNDs (ESP32, MCP23017s, ULN2003 boards, NeoPixel, sensors) meet at one star point near the bulk cap. Run motor return currents on their own wire to the star point, not through the logic ground.
- Never power from USB and the 5 V rail at the same time without a Schottky diode in series with the rail feeding VIN (or unplug the adapter while flashing).

---

## 5. I2C devices

```
  3.3V RAIL
     │            │
   [4.7k]       [4.7k]       (one pair for the whole bus)
     │            │
 SDA ┴────────────┼───────────── GPIO21 (ESP32)
 SCL ─────────────┴───────────── GPIO22 (ESP32)
     │   │
     │   ├────────────────────────────────────────────┐
     │   │                                            │
 ┌───┴───┴───────────┐ ┌───────────────────┐  ┌───────┴─────────┐ ┌───────────────┐
 │ MCP23017 #0 (0x20)│ │ MCP23017 #1 (0x21)│  │ DS3231 (0x68)   │ │ SSD1306 (0x3C)│
 │ SDA(13) SCL(12)   │ │ SDA(13) SCL(12)   │  │ SDA SCL VCC GND │ │ SDA SCL VCC GND│
 │ VDD(9)  VSS(10)   │ │ VDD(9)  VSS(10)   │  │ CR2032 backup   │ │               │
 │ RESET(18) -> 3V3  │ │ RESET(18) -> 3V3  │  └─────────────────┘ └───────────────┘
 │ A0 A1 A2 -> GND   │ │ A0 -> 3V3         │
 │ 100nF VDD-VSS     │ │ A1 A2 -> GND      │
 └───────────────────┘ │ 100nF VDD-VSS     │
                       └───────────────────┘
```

MCP23017 (DIP-28) key pins: VDD pin 9, VSS pin 10, SCL pin 12, SDA pin 13, A0 pin 15, A1 pin 16, A2 pin 17, RESET pin 18, GPB0-7 pins 1-8, GPA0-7 pins 21-28. Breakout boards differ, so verify against your board's silkscreen.

Cautions:
- **Pull-ups:** many DS3231 and OLED breakouts already carry their own 4.7 kΩ-10 kΩ pull-ups to VCC. In parallel these lower the effective value. Total effective pull-up should stay above roughly 1.5-2 kΩ. If the bus looks over-loaded, remove one of the board resistors or omit your extra pair.
- **DS3231 modules (ZS-042 style):** the CR2032 charging circuit on cheap boards can charge a non-rechargeable CR2032 and leak. Either remove the charging resistor/diode, use an LIR2032 rechargeable cell, or buy a module without that circuit. These modules also have an AT24C32 EEPROM that shows up on the scanner at 0x57 - expected, harmless.
- Everything on the I2C bus is powered from 3.3 V so there is no level conflict with the ESP32.
- Keep the bus short (under about 30 cm) and at 100-400 kHz.

---

## 6. One motor channel (repeat x5)

```
   MCP23017 (3.3V logic)                ULN2003 driver board            28BYJ-48 (5V)
   ┌────────────┐                       ┌──────────────────┐
   │ GPx0 ──────┼──── IN1 ─────────────►│ IN1       OUT1 ──┼──┐
   │ GPx1 ──────┼──── IN2 ─────────────►│ IN2       OUT2 ──┼──┤   5-pin JST/Molex
   │ GPx2 ──────┼──── IN3 ─────────────►│ IN3       OUT3 ──┼──┼── connector on the
   │ GPx3 ──────┼──── IN4 ─────────────►│ IN4       OUT4 ──┼──┤   motor plugs directly
   └────────────┘                       │ "+" (VCC) ◄──┐   │  │   into the board socket
                                        │ "-" (GND)    │   │  │   (red wire = +5V common)
                                        └──────┬───────┴───┘  │
                                               │              │
                                  5V RAIL ─────┘    GND ──► star point
```

- Per channel: 4 signal wires from the MCP23017 (twist with a ground, or use a 6-pin JST-XH: IN1-IN4, +5V, GND), plus 5 V and GND at the board's power pins. Remove the board's ON/OFF jumper only if you need to cut motor power; leave it on for normal use.
- The motor plugs straight into the ULN2003 board's white 5-pin socket.
- Repeat for A (#0 GPA0-3), B (#0 GPA4-7), C (#0 GPB0-3), D (#0 GPB4-7), E (#1 GPA0-3).
- Firmware energizes only one motor at a time, so the 5 V rail sees one motor (~240 mA) plus ULN2003 board LEDs.
- Firmware should de-energize all coils after each move; a held coil wastes ~240 mA and heats the motor.

---

## 7. Drop sensor (IR break-beam, GPIO27)

Recommended: a **3-wire break-beam module pair** (emitter + receiver), powered from 3.3 V.

```
  3.3V ────────────┬────────────────────────┐
                   │                        │
             EMITTER module            RECEIVER module
             (red = V+, black = GND)   (red = V+, black = GND, white/yellow = OUT)
                   │                        │
  GND ─────────────┴──┬─────────────────────┤
                      │                     OUT ──┬──────► GPIO27
                      │                           │
                      │                  (internal pull-up on GPIO27;
                      └──────────────────  open-collector output pulls LOW)
```

- Output is LOW when the beam is broken (pill falling through) - matches the active-LOW input.
- **Verify polarity** with a multimeter during bring-up; some modules are inverted. If yours reads LOW when the beam is clear, tell the firmware author (or choose a different module).
- 3.3 V operation: common 3-5 V break-beam modules work at 3.3 V but with less range. Mount the pair 15-30 mm apart across the chute.

Discrete alternative (bare 940 nm IR LED + matching phototransistor):
- Emitter: 3.3 V -> **100 Ω** -> IR LED anode, cathode -> GND. Current is about (3.3 - 1.3) / 100 = ~20 mA, which is fine for continuous operation. Use 150 Ω (~13 mA) if you want to be conservative.
- Receiver: phototransistor collector -> GPIO27 (with internal pull-up), emitter -> GND. This reads LOW when the beam is **present** (inverted from what firmware expects). To get the break-LOW behavior add a small NPN inverter stage, or just use the module pair. For this reason the module pair is the recommended route.
- Shield the receiver from room light with a short black heat-shrink tube.

---

## 8. Cup switch and buttons

```
  3.3V (internal pull-up, ~45k, inside ESP32)
     │
     ├──── GPIO26 ◄──┬── Cup microswitch (NO) ─── GND      LOW = pressed (cup present)
     │               └── 100nF to GND (optional debounce)
     ├──── GPIO32 ◄──┬── Button A "dispense now" ─── GND   LOW = pressed
     │               └── 100nF to GND (optional)
     └──── GPIO33 ◄──┬── Button B "taken / ack"  ─── GND   LOW = pressed
                     └── 100nF to GND (optional)
```

- No external resistors required: GPIO26/32/33 all support internal pull-ups. (That is exactly why the input-only pins 34-39 are avoided.)
- Run these on twisted pair with GND, use JST-XH connectors, and debounce in firmware (20-50 ms) in addition to or instead of the optional capacitors.
- Cup switch: use a lever-style microswitch (e.g. a mini snap-action switch) wired between GPIO26 and GND using the NO contact. If the firmware polarity expects the opposite, use the NC contact.

---

## 9. Buzzer (GPIO25) - transistor-driven

Decision: **drive through an NPN transistor**. Typical active piezo buzzers (5 V module-type) draw 20-35 mA, which exceeds the 12 mA comfort level for an ESP32 GPIO, and running it from 5 V gives a louder tone.

```
  +5V RAIL ─────────────┐
                         │
                  ┌──────┴──────┐
                  │ Active      │
                  │ buzzer  (+) │      (observe polarity; module buzzers
                  │         (-) │       are marked +)
                  └──────┬──────┘
                         │
                         C
  GPIO25 ──[ 1 kΩ ]──── B   2N2222A or 2N3904 (or BC337)
                         E
                         │
                        GND

  Optional: 10 kΩ from B to GND to hold the buzzer off while the ESP32 boots.
  Optional: 1N4148 flyback diode across the buzzer is unnecessary for a piezo buzzer, only for magnetic ones.
```

- Base current: (3.3 - 0.7) / 1 kΩ = 2.6 mA, which saturates the transistor for a load of 30 mA.
- Active (self-oscillating) buzzer: GPIO HIGH = sound, so it works with simple digital output or slow PWM patterns.
- If you pick a 3.3 V-rated active buzzer drawing <12 mA, you may instead drive it directly: GPIO25 -> 100 Ω -> buzzer(+) -> GND. The transistor circuit above is the documented default.

---

## 10. NeoPixel strip (GPIO13)

```
  +5V RAIL ───────────────┬─────────────────────► strip 5V
                          │
                        ──┴── C 100-1000 uF / 10V+  (mounted at the strip's 5V/GND pads)
                        ──┬──
  GND ────────────────────┴─────────────────────► strip GND

  GPIO13 ──[ 330 Ω ]────────────────────────────► strip DIN
```

- The 330 Ω resistor goes close to the strip's DIN, not close to the ESP32.
- Connect GND first, 5 V second, data last when hot-plugging; avoid hot-plugging the strip at all.
- **Level shifting:** WS2812B nominally wants V_IH of 0.7 x VDD = 3.5 V on a 5 V supply, so 3.3 V data is out of spec but usually works for short runs (< 30 cm) and small strips. If you see flicker or wrong colors, the options are:
  - a 74AHCT125 (or 74HCT245) buffer powered from 5 V, with the 3.3 V data on its input: ESP32 GPIO13 -> 74AHCT125 input, output -> 330 Ω -> DIN. Tie the unused inputs to GND, OE pin to GND.
  - a sacrificial first pixel powered at about 4.3 V (a diode drop below 5 V), which lowers the V_IH threshold for the rest of the chain.
- Brightness: cap at 30-50% in firmware. It keeps current down and extends the life of the PSU margin (see budget).

---

## 11. Power budget (5 V rail)

Assumes 8 NeoPixels (adjust to your strip). Currents are worst-case figures at the 5 V input; 3.3 V loads are fed by the linear regulator on the DevKit so they cost the same current at 5 V.

| Load | Typical | Peak / budget | Notes |
|---|---|---|---|
| ESP32 DevKit V1 (WiFi TX) | 80-120 mA | 250 mA | Peak during WiFi transmit bursts. |
| One 28BYJ-48 energized | 160-200 mA | 240 mA | Only one motor at a time. |
| ULN2003 board LEDs / idle (x5) | ~5 mA | 10 mA | LEDs light only on active coils. |
| NeoPixels, 8 pixels | 150 mA | 480 mA | 60 mA/pixel at full-white max; 480 mA at max. |
| Buzzer (active, 5 V) | 0 | 30 mA | Only when sounding. |
| OLED SSD1306 | 10 mA | 20 mA | Optional. |
| MCP23017 x2 + DS3231 | 2 mA | 5 mA | Negligible. |
| IR emitter + receiver | 20 mA | 25 mA | 100 Ω at 3.3 V; ~20 mA through the LED. |
| **Total** | ~0.5 A | **~1.1 A** | |

Why a 5 V 3 A adapter is comfortable:
- Worst-case simultaneous peak is about 1.1 A, so there is nearly 3x headroom at 3 A.
- Even if you extend to 30 pixels (1.8 A at full white), you remain within 3 A, but derate the strip in firmware.
- Only one motor runs at a time, so the 5-motor total (about 1.2 A) never materializes.
- A 2 A slow-blow fuse sits above the 1.1 A peak and below the adapter limit.
- DevKit regulator: ESP32 (~250 mA) + 3.3 V peripherals (~50 mA) is about 300 mA against the AMS1117's 800 mA rating. It dissipates (5 - 3.3) x 0.3 = ~0.5 W - warm but fine.

---

## 12. Wiring and assembly tips

- **Separate motor wiring from I2C.** Keep motor/ULN2003 leads and the SDA/SCL wires at least 2-3 cm apart, and cross them at right angles if they must meet.
- **Twist I2C pairs:** twist SDA with GND and SCL with GND (or SDA/SCL/GND/3.3V as a bundle) for any run longer than about 10 cm.
- **JST-XH everywhere** modules are removable: each ULN2003 board (6-pin: IN1-4, +5V, GND), I2C devices (4-pin: 3V3, GND, SDA, SCL), sensors (3-pin), buttons (2-pin), NeoPixel (3-pin). Key them so a connector cannot be inserted backwards.
- **Label each ULN2003 board** with its slot letter A-E (and the matching wire bundle at both ends) so MCP23017 pins and motors never get crossed.
- **Strain relief:** zip-tie or hot-glue cable bundles to the enclosure near each connector; add a grommet or strain-relief knot where the power cable enters the case; never let the motor's short leads carry tension.
- Colour code: red = +5 V, orange = +3.3 V, black = GND, yellow = SCL, green = SDA.
- Use 22 AWG for 5 V and GND distribution, 26-28 AWG is fine for signals.
- Place the 100 nF decoupling caps directly at each MCP23017's VDD/VSS pins; put the bulk cap near the barrel jack and the NeoPixel cap at the strip.
- Solder on perfboard rather than breadboarding for the final build; breadboard contacts will fail under motor vibration.
- Add a ferrule or solder-tinned ends for screw terminals if used.
- Mark the power-switch position and fuse value on the enclosure.

---

## 13. Bring-up checklist

Do these in order, and stop at the first failure.

1. **Visual inspection:** polarity of the bulk cap, MCP23017 orientation (pin 1 notch), no solder bridges.
2. **Continuity (power off, adapter unplugged):**
   - 5 V rail to GND: should **not** beep (no short). Expect a brief resistance rise from the capacitor charging.
   - 3.3 V rail to GND: no short.
   - Each ULN2003 IN1-IN4 goes to the intended MCP23017 pin (use the table in section 3).
   - GND continuity across all boards to the star point.
   - SDA and SCL not shorted to each other or to power.
3. **Power-up without modules** (ESP32 only, then add boards): plug in the adapter, switch on.
   - Measure **5.0 V (+/- 0.25 V)** at the rail and at the ULN2003 "+" pins.
   - Measure **3.3 V (+/- 0.1 V)** on the ESP32 3V3 pin and at the MCP23017 VDD pins.
   - Nothing warm; fuse intact.
4. **Flash a minimal I2C scanner.** Expect **0x20, 0x21, 0x68** (and **0x3C** if the OLED is fitted; possibly 0x57 from the RTC module's EEPROM). Anything missing: check the address pins (A0 on chip #1 must be tied high), pull-ups, and SDA/SCL swap.
5. **Test one motor:** step motor 1 (ULN-A) slowly through the 4-step sequence; it should turn smoothly. Then check that coils de-energize afterwards (board LEDs off). A jittering or buzzing motor with no rotation usually means IN1-IN4 order is wrong.
6. **Test the sensor:** read GPIO27 while passing a finger (or a pill) through the beam. Expect HIGH with a clear beam and LOW when broken. Check the polarity at this step.
7. **Test inputs:** press the cup switch (GPIO26), button A (GPIO32), and button B (GPIO33); each should read LOW when pressed.
8. **Test outputs:** buzzer on/off (GPIO25), NeoPixel red/green/blue test pattern (GPIO13), OLED text.
9. **Test RTC:** read the time, power-cycle, confirm it keeps time on the CR2032.
10. **All five motors, one at a time:** B through E on MCP23017 pins as per the table. Confirm only the intended motor moves each time, and then run a full dispense cycle with sensor confirmation.
11. **Thermal check:** after a few minutes of cycling, the ESP32's regulator and the ULN2003 chips should be warm, not hot.
