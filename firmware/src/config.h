#pragma once
#include <Arduino.h>
#include "secrets.h"   // copy include/secrets.h.example -> include/secrets.h

// ---------------- Feature flags ----------------
#ifndef ENABLE_OLED
#define ENABLE_OLED 0            // 1 = SSD1306 128x64 status display at 0x3C
#endif
#ifndef NOTIFY_BACKEND_NTFY
#define NOTIFY_BACKEND_NTFY 1    // 1 = ntfy.sh push, 0 = log to Serial only
#endif

// ---------------- Pin map (FIXED, see wiring doc) ----------------
#define PIN_I2C_SDA     21
#define PIN_I2C_SCL     22
#define PIN_IR_BEAM     27   // INPUT_PULLUP, LOW = beam broken
#define PIN_CUP_SWITCH  26   // INPUT_PULLUP, LOW = cup present
#define PIN_BTN_A       32   // dispense now
#define PIN_BTN_B       33   // taken / acknowledge
#define PIN_BUZZER      25   // active buzzer, HIGH = on
#define PIN_NEOPIXEL    13
#define NEOPIXEL_COUNT  8    // 1..8

// ---------------- I2C addresses ----------------
#define MCP0_ADDR  0x20      // motors 1-4
#define MCP1_ADDR  0x21      // motor 5
#define RTC_ADDR   0x68
#define OLED_ADDR  0x3C
#define I2C_CLOCK_HZ 400000

// ---------------- Motors ----------------
#define NUM_SLOTS            5
#define HALF_STEPS_PER_REV   4096
#define STEP_DELAY_MS        1        // min delay between half-steps
#define STEPS_PER_PILL       4096     // one full disk revolution per pill
#define DISPENSE_FORWARD     true
#define AGITATE_DEGREES      15
#define AGITATE_CYCLES       3

// ---------------- Drop detection / retry ----------------
#define DROP_DEBOUNCE_MS     2
#define DROP_SETTLE_MS       400      // after first break, keep listening for a 2nd pill
#define DROP_TIMEOUT_MS      1500     // wait after a rotation before declaring "no drop"
#define AGITATE_WAIT_MS      600
#define MAX_RETRIES          3        // agitate+retry attempts per pill
#define INTER_PILL_DELAY_MS  300

// ---------------- Schedule / state ----------------
#define SCHEDULE_PATH        "/schedule.json"
#define MAX_EVENTS           16
#define MAX_PILLS_PER_DOSE   9
#define CATCHUP_MINUTES      60       // dispense late after outage up to this long
#define NOT_TAKEN_MINUTES    30
#define DEFAULT_LOW_THRESHOLD 5
#define BUTTON_DEBOUNCE_MS   30
#define ERROR_DISPLAY_MS     10000
#define OFFLINE_NOTIFY_MS    (5UL * 60UL * 1000UL)

// ---------------- Time ----------------
#define TZ_STRING   "UTC0"            // POSIX TZ, e.g. "EST5EDT,M3.2.0,M11.1.0"
#define NTP_SERVER1 "pool.ntp.org"
#define NTP_SERVER2 "time.nist.gov"
#define RTC_RESYNC_MS (6UL * 3600UL * 1000UL)

// ---------------- Notifications ----------------
#ifndef NTFY_SERVER
#define NTFY_SERVER "https://ntfy.sh"
#endif
#ifndef NTFY_TOPIC
#define NTFY_TOPIC  "pill-dispenser-unset"
#endif

// Slot labels used in UI/logs
static const char SLOT_LABELS[NUM_SLOTS] = {'A', 'B', 'C', 'D', 'E'};
