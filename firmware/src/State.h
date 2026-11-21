#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

// Persistent (NVS) state: last-dispensed dates, pill counts, pending dose.
class State {
public:
  void begin();

  // Per-event last dispensed date (yyyymmdd), keyed by event minute-of-day.
  uint32_t lastDispensed(uint16_t minuteOfDay);
  void setLastDispensed(uint16_t minuteOfDay, uint32_t dateKey);

  uint16_t count(uint8_t slot);
  void setCount(uint8_t slot, uint16_t n);
  void consume(uint8_t slot, uint16_t n);       // saturating decrement
  uint8_t lowThreshold();
  void setLowThreshold(uint8_t t);
  bool isLow(uint8_t slot) { return count(slot) <= lowThreshold(); }
  bool anyLow();

  bool pending() const { return pending_; }
  uint32_t pendingSince() const { return pendingSince_; }   // epoch secs (0 if clock was invalid)
  void setPending(bool p, uint32_t sinceEpoch = 0);

private:
  Preferences prefs_;
  bool pending_ = false;
  uint32_t pendingSince_ = 0;
};
