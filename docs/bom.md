# Pill Dispenser - Bill of Materials

All prices are **approximate 2026 ballpark figures in USD** (unit price ranges, typical hobby retailers/marketplaces). Part numbers are well-known generic types or search phrases, not distributor SKUs. "Check your parts bin" = you said you already have ESP32s, Arduinos and general electronic components, so look there first.

## Controller

| Qty | Part | Spec | Example part / search phrase | Unit price | Notes |
|---|---|---|---|---|---|
| 1 | ESP32 dev board | ESP32-WROOM-32, 30-pin DevKit V1 | "ESP32 DevKit V1 30 pin" | $6-10 | **Check your parts bin.** Pin map assumes the 30-pin layout. |

## I/O and drivers

| Qty | Part | Spec | Example part / search phrase | Unit price | Notes |
|---|---|---|---|---|---|
| 2 | I2C GPIO expander | MCP23017, 16-bit, DIP-28 or breakout | "MCP23017-E/SP DIP-28" or "MCP23017 I2C breakout" | $2-4 (DIP) / $4-8 (breakout) | DIP is cheapest and fits perfboard; breakouts are easier. Addresses 0x20 and 0x21. |
| 4+ | Ceramic capacitor | 100 nF, 50 V | "100nF ceramic capacitor assortment" | $0.10-0.30 | 2 for the MCP23017s, spares and optional switch debounce. Check your parts bin. |
| 2 | Resistor | 4.7 kΩ, 1/4 W | "4.7k resistor" | $0.05 | I2C pull-ups. Check your parts bin. |
| 1 | NPN transistor | 2N2222A / 2N3904 / BC337 | "2N3904 transistor" | $0.10-0.30 | Buzzer driver. Check your parts bin. |
| 1 | Resistor | 1 kΩ | "1k resistor" | $0.05 | Buzzer base resistor. |
| 1 | Resistor | 330 Ω | "330 ohm resistor" | $0.05 | NeoPixel data line. |
| 1 | Resistor (optional) | 100 Ω | "100 ohm resistor" | $0.05 | Only for a bare IR LED emitter. |
| 1 | Level shifter (optional) | 74AHCT125 (DIP-14 or SOIC breakout) | "74AHCT125 level shifter" | $1-3 | Only if 3.3 V data to the NeoPixel is flaky. |
| 1 | Resistor assortment | 1/4 W metal-film kit | "metal film resistor assortment kit" | $6-10 | Covers all of the above if you don't already have them. |

## Motors

| Qty | Part | Spec | Example part / search phrase | Unit price | Notes |
|---|---|---|---|---|---|
| 5 | Stepper motor + driver | 28BYJ-48, 5 V unipolar, with ULN2003 driver board | "28BYJ-48 5V with ULN2003 driver board 5-pack" | $12-20 per 5-pack ($2.5-4 each) | **Check your parts bin** for any you already have. Buy the 5 V version, not 12 V. |

## Sensors

| Qty | Part | Spec | Example part / search phrase | Unit price | Notes |
|---|---|---|---|---|---|
| 1 | IR break-beam pair | 3-wire module, 3.3-5 V, 940 nm | "IR break beam sensor 3mm 3-5V emitter receiver pair" | $3-8 | Pill-drop sensor. Verify output goes LOW when the beam is broken. Alt: bare 940 nm IR LED + phototransistor ($0.50-1). |
| 1 | Microswitch | Lever-type snap-action, SPDT/SPST, 3 A | "micro limit switch lever 3-pin" | $0.50-2 | Cup-present switch. Check your parts bin. |
| 1 | RTC module | DS3231 I2C, 3.3 V compatible | "DS3231 RTC module I2C" | $2-6 | Prefer a module without the charging circuit, or remove it (see wiring.md). |
| 1 | RTC backup battery | CR2032 coin cell | "CR2032 battery" | $1-2 | Or LIR2032 rechargeable if the module has a charge circuit. |

## UI

| Qty | Part | Spec | Example part / search phrase | Unit price | Notes |
|---|---|---|---|---|---|
| 1 | OLED display (optional) | 0.96 in SSD1306 128x64, I2C, 0x3C | "0.96 inch OLED I2C SSD1306 128x64 4-pin" | $3-6 | Make sure it is the 4-pin I2C version. |
| 2 | Pushbutton | Momentary, panel mount 12-16 mm or tact switch | "12mm momentary push button panel mount" | $1-3 | Button A and B. Check your parts bin for tact switches. |
| 1 | Buzzer | Active piezo, 5 V (module type), ~30 mA | "active buzzer 5V module" | $0.50-2 | Driven through the NPN. |
| 1 | NeoPixel strip / ring | WS2812B, 5 V, ~8 pixels | "WS2812B 8 LED strip" or "WS2812B 8 ring" | $3-8 | Budgeted at 8 pixels (480 mA max). |
| 1 | Electrolytic cap | 100-1000 µF, 10 V+ | "1000uF 16V electrolytic capacitor" | $0.30-1 | At the NeoPixel strip. Check your parts bin. |

## Power

| Qty | Part | Spec | Example part / search phrase | Unit price | Notes |
|---|---|---|---|---|---|
| 1 | Wall adapter | 5 V, 3 A, barrel 5.5x2.1 mm, center-positive | "5V 3A power supply 5.5x2.1mm" | $8-14 | Reputable brand; regulated. |
| 1 | Barrel jack | 5.5x2.1 mm panel mount (or PCB mount) | "DC barrel jack 5.5 2.1 panel mount" | $1-2 | Must match the adapter plug. |
| 1 | Bulk capacitor | 470-1000 µF, 10 V or 16 V electrolytic | "1000uF 16V electrolytic capacitor" | $0.30-1 | At the rail. Check your parts bin. |
| 1 | Inline fuse + holder | 2 A slow-blow, 5x20 mm glass | "inline fuse holder 5x20mm" + "2A slow blow fuse" | $2-5 | Or a 2 A resettable PTC fuse as an alternative. |
| 1 | Power switch | SPST rocker, 3 A+ | "SPST rocker switch 3A panel mount" | $1-3 | Check your parts bin. |
| 1 | Schottky diode (optional) | 1N5819 | "1N5819 diode" | $0.10-0.30 | Prevents USB/adapter backfeed. |

## Connectors and hardware

| Qty | Part | Spec | Example part / search phrase | Unit price | Notes |
|---|---|---|---|---|---|
| 1 kit | JST-XH connector kit | 2.54 mm, 2-6 pin, housings + pins + headers | "JST XH 2.54mm connector kit" | $8-14 | Crimping tool needed ($15-30) if you don't own one. |
| 1 pack | Dupont jumper wires | M-F, M-M, F-F assortment | "Dupont jumper wire kit 40pin" | $4-8 | **Check your parts bin.** Handy for prototyping. |
| 1-2 | Perfboard / protoboard | ~7x9 cm or larger, double-sided | "double sided perfboard 7x9cm" | $4-8 | **Check your parts bin.** |
| 1 kit | Hookup wire | 22 AWG silicone, multi-color | "22 AWG silicone wire kit" | $10-15 | **Check your parts bin.** |
| 1 pack | Pin headers / sockets | 2.54 mm male + female, 15-pin for ESP32 | "female header 2.54mm 15 pin" | $3-6 | So the ESP32 and DIP chips are removable. |
| 1 pack | IC sockets (optional) | DIP-28 for MCP23017 | "DIP-28 IC socket" | $1-3 | |
| 1 kit | Screws | M3 and M4 assortment, 6-20 mm, pan/socket head | "M3 M4 screw assortment kit" | $8-12 | For the 3D-printed modules. |
| 1 pack | Heat-set inserts | M3 (and M4 if used), brass | "M3 heat set threaded inserts" | $6-10 | Installed with a soldering iron. |
| 1 pack | Heat shrink tubing | assorted | "heat shrink tubing kit" | $5-8 | Check your parts bin. |
| 1 pack | Cable ties / strain relief | small nylon | "nylon cable ties small" | $3-5 | |

## Consumables

| Qty | Part | Spec | Example part / search phrase | Unit price | Notes |
|---|---|---|---|---|---|
| 1 | Desiccant packets | silica gel, food/medicine-safe, 1-5 g | "silica gel desiccant packets 2g" | $5-8 | Keep pills dry; replace periodically. |
| 1 spool | TPU filament | 95A, 1.75 mm, 500 g-1 kg | "TPU 95A 1.75mm filament" | $18-28 | Wiper blade. Check your parts bin. |
| 1 spool | PETG filament | 1.75 mm, 1 kg | "PETG 1.75mm 1kg filament" | $16-25 | Main structure. Check your parts bin. |

---

## Rough total (2026 ballpark)

| Scope | Approx. total |
|---|---|
| Electronics only (controller to power, new parts) | $75-130 |
| Plus connectors and hardware | $125-210 |
| Plus consumables (desiccant, TPU, PETG) | **$150-250** |
| If your parts bin covers the ESP32, resistors, caps, wire, perfboard and filament | roughly **$60-110** |

Prices vary with shipping and vendor; treat them as planning figures, not quotes.
