#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_task_wdt.h>
#include "config.h"
#include "MotorBank.h"
#include "DropSensor.h"
#include "Dispenser.h"
#include "Schedule.h"
#include "Clock.h"
#include "State.h"
#include "Notify.h"
#include "WebUI.h"
#include "Indicators.h"

#if ENABLE_OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
static Adafruit_SSD1306 oled(128, 64, &Wire, -1);
static bool oledOk = false;
#endif

static MotorBank motors;
static DropSensor sensor;
static Dispenser dispenser;
static Schedule schedule;
static Clock rtcClock;
static State state;
static Indicators ind;
static WebUI web;
static AppStatus app;

static bool motorsOk = false;
static uint32_t errorUntil = 0;
static bool cupLifted = false;
static uint32_t cupAbsentSince = 0;
static bool notTakenSent = false;
static bool clockAlertSent = false;
static bool timeValid = false;

// RAM copy of the manual lockout, so it also works while the clock is invalid (NVS holds the epoch copy).
static uint32_t lastDoseMs[NUM_SLOTS] = {0, 0, 0, 0, 0};

// queued web dispense request (handled in loop, never inside the HTTP handler)
static volatile int8_t reqSlot = -1;
static volatile uint8_t reqCount = 0;

// WiFi offline tracking
static bool wifiWasUp = false;
static uint32_t wifiDownSince = 0;

// ---------------- Buttons ----------------
struct Button {
  uint8_t pin;
  bool stable;
  bool lastRaw;
  uint32_t t;
  bool fired;      // hold action already triggered for this press
  explicit Button(uint8_t p) : pin(p), stable(HIGH), lastRaw(HIGH), t(0), fired(false) {}
};
static Button btnA(PIN_BTN_A);
static Button btnB(PIN_BTN_B);

static bool pressed(Button& b) {   // true once per debounced press; also tracks releases
  bool raw = digitalRead(b.pin);
  uint32_t now = millis();
  if (raw != b.lastRaw) { b.lastRaw = raw; b.t = now; }
  if (now - b.t >= BUTTON_DEBOUNCE_MS && raw != b.stable) {
    b.stable = raw;
    if (raw == HIGH) b.fired = false;
    return raw == LOW;
  }
  return false;
}

static bool heldFor(Button& b, uint32_t ms) {   // true once per press, after it has been held ms
  if (b.stable == LOW && !b.fired && millis() - b.t >= ms) { b.fired = true; return true; }
  return false;
}

// ---------------- Dose logic ----------------
static void raiseError() { errorUntil = millis() + ERROR_DISPLAY_MS; }
static void note(const char* why) { app.lastNote = why; Serial.printf("[dose] %s\n", why); }

static void acknowledge(const char* why) {
  if (!state.pending()) return;
  state.setPending(false);
  cupLifted = false;
  notTakenSent = false;
  Serial.printf("[dose] taken (%s)\n", why);
}

// Dispense counts[] one slot at a time. eventMinute < 0 = ad-hoc (no per-event bookkeeping).
// manual = web or button, which also enforces the per-slot lockout.
// counts[] is clamped in place by the daily cap. Returns false if nothing was dispensed.
static bool startDose(uint8_t counts[NUM_SLOTS], int eventMinute, bool manual) {
  if (!motorsOk) { note("motor driver not found, refusing"); raiseError(); return false; }
  if (!dispenser.cupPresent()) { note("cup not present, refusing"); raiseError(); return false; }

  sensor.poll();
  if (sensor.beamBroken()) {
    note("beam blocked before dispensing, refusing");
    Notify::send(EVT_SENSOR, "Beam blocked before dispensing; check the funnel and the sensor");
    raiseError();
    return false;
  }

  struct tm t;
  bool timeOk = rtcClock.now(t);
  uint32_t today = timeOk ? Clock::dateKey(t) : 0;
  uint32_t nowEpoch = timeOk ? (uint32_t)time(nullptr) : 0;

  // Per-slot daily cap (all paths) and manual lockout (web/button only).
  bool any = false;
  for (uint8_t s = 0; s < NUM_SLOTS; s++) {
    if (!counts[s]) continue;
    uint16_t done = state.dispensedToday(s, today);
    if (done + counts[s] > MAX_PILLS_PER_SLOT_PER_DAY) {
      uint8_t allowed = done >= MAX_PILLS_PER_SLOT_PER_DAY ? 0 : (uint8_t)(MAX_PILLS_PER_SLOT_PER_DAY - done);
      Notify::send(EVT_LIMIT, String("Slot ") + SLOT_LABELS[s] + ": daily cap of " + MAX_PILLS_PER_SLOT_PER_DAY
                              + " reached, " + allowed + " of " + counts[s] + " allowed");
      counts[s] = allowed;
      raiseError();
    }
    if (manual && counts[s]) {
      bool locked = false;
      if (lastDoseMs[s] && millis() - lastDoseMs[s] < MANUAL_LOCKOUT_MINUTES * 60000UL) locked = true;
      uint32_t last = state.lastDoseAt(s);
      if (nowEpoch && last && nowEpoch >= last && nowEpoch - last < MANUAL_LOCKOUT_MINUTES * 60UL) locked = true;
      if (locked) {
        Notify::send(EVT_LIMIT, String("Slot ") + SLOT_LABELS[s] + ": manual dispense refused, last dose less than "
                                + MANUAL_LOCKOUT_MINUTES + " min ago");
        counts[s] = 0;
        raiseError();
      }
    }
    if (counts[s]) any = true;
  }
  if (!any) {
    note("nothing to dispense after limits");
    // A scheduled event refused by the cap must not retry every minute for the catch-up window.
    if (eventMinute >= 0 && timeOk) state.setLastDispensed((uint16_t)eventMinute, today);
    return false;
  }

  if (eventMinute >= 0 && timeOk) {
    // Mark BEFORE moving so a reboot mid-dose can never double-dose.
    state.setLastDispensed((uint16_t)eventMinute, today);
  }

  app.busy = true;
  app.lastNote = "";
  DispenseResult total;
  for (uint8_t s = 0; s < NUM_SLOTS; s++) {
    if (!counts[s]) continue;
    DispenseResult r = dispenser.dispense(s, counts[s]);
    total.requested += r.requested;
    total.dropped += r.dropped;
    total.jams += r.jams;
    total.doubles += r.doubles;
    if (r.refused) total.refused = true;
    if (r.dropped) {
      state.consume(s, r.dropped);
      state.addDispensedToday(s, r.dropped, today);
      state.setLastDoseAt(s, nowEpoch);
      lastDoseMs[s] = millis() ? millis() : 1;
    }

    if (r.dropped < r.requested) {
      String m = String("Slot ") + SLOT_LABELS[s] + ": only " + r.dropped + "/" + r.requested + " dispensed";
      if (r.cupMissing) m += " (cup removed)";
      Notify::send(EVT_JAM, m);
      raiseError();
    }
    if (r.doubles) {
      Notify::send(EVT_DOUBLE, String("Slot ") + SLOT_LABELS[s] + ": double drop detected, check the cup");
      raiseError();
    }
    if (r.dropped > 0 && state.isLow(s)) {
      Notify::send(EVT_LOW_PILLS, String("Slot ") + SLOT_LABELS[s] + " has " + state.count(s) + " left");
    }
    if (r.cupMissing) break;
  }
  app.busy = false;
  app.hasLast = true;
  app.last = total;

  sensor.poll();
  if (sensor.beamBroken()) {
    Notify::send(EVT_SENSOR, "Beam still blocked after dispensing; a pill may be stuck in the funnel");
    raiseError();
  }

  if (total.dropped > 0) {
    state.setPending(true, nowEpoch);
    notTakenSent = false;
    cupLifted = false;
    Notify::send(EVT_DOSE_READY, String(total.dropped) + " pill(s) in the cup");
  }
  return total.dropped > 0;
}

static void copyCounts(const DoseEvent& e, uint8_t out[NUM_SLOTS]) {
  for (uint8_t i = 0; i < NUM_SLOTS; i++) out[i] = e.counts[i];
}

static void schedulerTick(const struct tm& t) {
  if (state.pending()) return;   // previous dose not yet taken
  uint32_t today = Clock::dateKey(t);
  int nowMin = t.tm_hour * 60 + t.tm_min;
  for (size_t i = 0; i < schedule.size(); i++) {
    const DoseEvent& e = schedule.at(i);
    if (!e.enabled || e.totalPills() == 0) continue;
    int mod = e.minuteOfDay();
    if (nowMin < mod || nowMin - mod > CATCHUP_MINUTES) continue;
    if (state.lastDispensed(mod) == today) continue;
    uint8_t counts[NUM_SLOTS];
    copyCounts(e, counts);
    Serial.printf("[sched] event %02u:%02u due\n", e.hour, e.minute);
    startDose(counts, mod, false);   // if refused (no cup / sensor) it retries next minute within the catch-up window
    return;
  }
}

// Button A (held): dispense the earliest enabled event that has not been dispensed today,
// and mark it done so the scheduler will not dose it again. Subject to the manual lockout.
static void manualDispenseNext() {
  if (state.pending()) { note("dose pending, button A ignored"); ind.beep(300); return; }
  struct tm t;
  if (!rtcClock.now(t)) { note("clock invalid, button A refused"); ind.beep(300); raiseError(); return; }
  uint32_t today = Clock::dateKey(t);
  int best = -1, bestMod = 100000;
  for (size_t i = 0; i < schedule.size(); i++) {
    const DoseEvent& e = schedule.at(i);
    if (!e.enabled || e.totalPills() == 0) continue;
    int mod = e.minuteOfDay();
    if (state.lastDispensed(mod) == today) continue;
    if (mod < bestMod) { bestMod = mod; best = (int)i; }
  }
  if (best < 0) { note("all of today's events already dispensed"); ind.beep(300); return; }
  uint8_t counts[NUM_SLOTS];
  copyCounts(schedule.at(best), counts);
  startDose(counts, bestMod, true);
}

// ---------------- Web hooks ----------------
static const char* hookDispense(uint8_t slot, uint8_t count) {
  if (app.busy || reqSlot >= 0) return "busy";
  if (state.pending()) return "a dose is pending, acknowledge it first";
  if (!app.cup) return "cup not present";
  reqCount = count;
  reqSlot = (int8_t)slot;
  return nullptr;
}
static void hookAck() { acknowledge("web"); }

// ---------------- Display ----------------
#if ENABLE_OLED
static void drawOled() {
  if (!oledOk) return;
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);
  oled.println(rtcClock.iso());
  oled.printf("WiFi:%s NTP:%s\n", WiFi.status() == WL_CONNECTED ? "ok" : "--", rtcClock.ntpSynced() ? "ok" : "--");
  oled.printf("Cup:%s Dose:%s\n", app.cup ? "yes" : "NO", state.pending() ? "PENDING" : "-");
  for (uint8_t i = 0; i < NUM_SLOTS; i++) oled.printf("%c:%u ", SLOT_LABELS[i], state.count(i));
  oled.display();
}
#endif

// ---------------- Arduino ----------------
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n== Pill dispenser ==");

  pinMode(PIN_BTN_A, INPUT_PULLUP);
  pinMode(PIN_BTN_B, INPUT_PULLUP);
  ind.begin();

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  Wire.setClock(I2C_CLOCK_HZ);

  state.begin();
  sensor.begin();
  motorsOk = motors.begin();         // also drives every coil line low
  dispenser.begin(&motors, &sensor);
  if (!motorsOk) raiseError();
  schedule.begin();

  rtcClock.begin();

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) { delay(250); Serial.print('.'); }
  Serial.println();
  wifiWasUp = WiFi.status() == WL_CONNECTED;
  if (wifiWasUp) Serial.printf("[wifi] %s\n", WiFi.localIP().toString().c_str());
  else { wifiDownSince = millis(); Serial.println("[wifi] offline, running on RTC"); }

  WebHooks hooks;
  hooks.requestDispense = hookDispense;
  hooks.requestAck = hookAck;
  web.begin(&schedule, &state, &rtcClock, &app, hooks);

#if ENABLE_OLED
  oledOk = oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
#endif

  // Watchdog last, after the blocking WiFi wait. Fed in loop() and on every motor half-step.
  esp_task_wdt_init(WDT_TIMEOUT_S, true);
  esp_task_wdt_add(NULL);
}

void loop() {
  static int64_t lastMinuteKey = -1;
  static uint32_t lastOled = 0;

  esp_task_wdt_reset();
  web.handle();
  rtcClock.tick();
  sensor.poll();                     // keeps beamBroken() current between doses
  app.cup = dispenser.cupPresent();

  struct tm t;
  timeValid = rtcClock.now(t);

  // WiFi up/down bookkeeping
  bool up = WiFi.status() == WL_CONNECTED;
  if (up && !wifiWasUp) {
    if (millis() - wifiDownSince >= OFFLINE_NOTIFY_MS) {
      Notify::send(EVT_OFFLINE_RTC, "Back online after running on the RTC clock");
    }
    wifiWasUp = true;
  } else if (!up && wifiWasUp) {
    wifiWasUp = false;
    wifiDownSince = millis();
  }

  // Clock sanity: no valid time means no scheduled doses; say so once.
  if (!clockAlertSent && !timeValid && millis() >= CLOCK_INVALID_NOTIFY_MS) {
    clockAlertSent = true;
    Notify::send(EVT_CLOCK_INVALID, "No valid time (RTC lost power and no NTP); scheduled doses are suspended");
  }

  // Buttons
  pressed(btnA);                     // tracks state; A acts on hold only
  if (heldFor(btnA, BTN_A_HOLD_MS)) { Serial.println("[btn] A held"); manualDispenseNext(); }
  if (pressed(btnB)) { Serial.println("[btn] B"); acknowledge("button B"); }

  // Web-queued dispense
  if (reqSlot >= 0) {
    uint8_t counts[NUM_SLOTS] = {0, 0, 0, 0, 0};
    counts[reqSlot] = reqCount;
    reqSlot = -1;
    startDose(counts, -1, true);
  }

  // Cup lifted: present -> absent -> present while a dose is pending = taken
  if (state.pending()) {
    if (!app.cup) {
      if (cupAbsentSince == 0) cupAbsentSince = millis();
      if (millis() - cupAbsentSince >= 200) cupLifted = true;
    } else {
      cupAbsentSince = 0;
      if (cupLifted) acknowledge("cup lifted");
    }
  } else {
    cupAbsentSince = 0;
  }

  // Scheduler: once per minute, only with a valid clock
  if (timeValid) {
    int64_t key = (int64_t)Clock::dateKey(t) * 1440 + t.tm_hour * 60 + t.tm_min;
    if (key != lastMinuteKey) {
      lastMinuteKey = key;
      schedulerTick(t);
    }
  }

  // Dose not taken
  if (state.pending() && !notTakenSent && state.pendingSince() > 0 && timeValid) {
    if ((uint32_t)time(nullptr) - state.pendingSince() >= NOT_TAKEN_MINUTES * 60UL) {
      notTakenSent = true;
      Notify::send(EVT_DOSE_NOT_TAKEN, String("Dose not taken after ") + NOT_TAKEN_MINUTES + " min");
    }
  }

  // Indicators
  if (millis() < errorUntil || !timeValid) ind.setMode(IND_ERROR);
  else if (state.pending()) ind.setMode(IND_DOSE_READY);
  else if (state.anyLow()) ind.setMode(IND_LOW_PILLS);
  else ind.setMode(IND_IDLE);
  ind.tick();

#if ENABLE_OLED
  if (millis() - lastOled >= 1000) { lastOled = millis(); drawOled(); }
#else
  (void)lastOled;
#endif

  delay(2);
}
