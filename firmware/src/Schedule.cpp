#include "Schedule.h"
#include <LittleFS.h>

bool Schedule::begin() {
  if (!LittleFS.begin(true)) {
    Serial.println("[sched] LittleFS mount failed");
    setDefaults();
    return false;
  }
  if (!load()) {
    Serial.println("[sched] no valid schedule, writing defaults");
    setDefaults();
    save();
  }
  return true;
}

void Schedule::setDefaults() {
  events_.clear();
  DoseEvent a; a.hour = 8;  a.minute = 0; a.counts[0] = 1; a.counts[2] = 2;   // A x1, C x2
  DoseEvent b; b.hour = 20; b.minute = 0; b.counts[1] = 1;                    // B x1
  events_.push_back(a);
  events_.push_back(b);
}

bool Schedule::load() {
  File f = LittleFS.open(SCHEDULE_PATH, "r");
  if (!f) return false;
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) return false;
  return setFromJson(doc.as<JsonVariantConst>());
}

bool Schedule::save() const {
  File f = LittleFS.open(SCHEDULE_PATH, "w");
  if (!f) return false;
  JsonDocument doc;
  toJson(doc);
  size_t n = serializeJson(doc, f);
  f.close();
  return n > 0;
}

bool Schedule::hasTime(uint16_t mod) const {
  for (const auto& e : events_) if (e.minuteOfDay() == mod) return true;
  return false;
}

bool Schedule::add(const DoseEvent& e) {
  if (events_.size() >= MAX_EVENTS || hasTime(e.minuteOfDay())) return false;
  events_.push_back(e);
  return true;
}

bool Schedule::remove(size_t index) {
  if (index >= events_.size()) return false;
  events_.erase(events_.begin() + index);
  return true;
}

void Schedule::toJson(JsonDocument& doc) const {
  JsonArray arr = doc["events"].to<JsonArray>();
  for (const auto& e : events_) {
    JsonObject o = arr.add<JsonObject>();
    o["hour"] = e.hour;
    o["minute"] = e.minute;
    JsonArray c = o["counts"].to<JsonArray>();
    for (uint8_t i = 0; i < NUM_SLOTS; i++) c.add(e.counts[i]);
    o["enabled"] = e.enabled;
  }
}

bool Schedule::setFromJson(JsonVariantConst root) {
  JsonArrayConst arr = root["events"].as<JsonArrayConst>();
  if (arr.isNull()) return false;
  std::vector<DoseEvent> tmp;
  for (JsonVariantConst v : arr) {
    JsonObjectConst o = v.as<JsonObjectConst>();
    int h = o["hour"] | -1;
    int m = o["minute"] | -1;
    if (h < 0 || h > 23 || m < 0 || m > 59) return false;
    DoseEvent e;
    e.hour = (uint8_t)h;
    e.minute = (uint8_t)m;
    JsonArrayConst c = o["counts"].as<JsonArrayConst>();
    for (size_t i = 0; i < NUM_SLOTS; i++) {
      int n = c[i] | 0;
      if (n < 0) n = 0;
      if (n > MAX_PILLS_PER_DOSE) n = MAX_PILLS_PER_DOSE;
      e.counts[i] = (uint8_t)n;
    }
    e.enabled = o["enabled"] | true;
    for (const auto& x : tmp) if (x.minuteOfDay() == e.minuteOfDay()) return false;  // duplicate time
    tmp.push_back(e);
    if (tmp.size() > MAX_EVENTS) return false;
  }
  events_ = tmp;
  return true;
}
