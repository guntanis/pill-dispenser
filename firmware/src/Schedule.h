#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "config.h"

struct DoseEvent {
  uint8_t hour = 0;
  uint8_t minute = 0;
  uint8_t counts[NUM_SLOTS] = {0, 0, 0, 0, 0};
  bool enabled = true;
  uint16_t minuteOfDay() const { return hour * 60 + minute; }
  uint16_t totalPills() const { uint16_t s = 0; for (uint8_t c : counts) s += c; return s; }
};

class Schedule {
public:
  bool begin();                      // mounts LittleFS, loads or creates defaults
  bool load();
  bool save() const;
  bool add(const DoseEvent& e);      // false if full or duplicate time
  bool remove(size_t index);
  size_t size() const { return events_.size(); }
  const DoseEvent& at(size_t i) const { return events_[i]; }
  void toJson(JsonDocument& doc) const;               // {"events":[...]}
  bool setFromJson(JsonVariantConst root);            // validate + replace
  void setDefaults();

private:
  std::vector<DoseEvent> events_;
  bool hasTime(uint16_t mod) const;
};
