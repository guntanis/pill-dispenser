#pragma once
#include <Arduino.h>
#include <time.h>
#include <RTClib.h>
#include "config.h"

// System time (localtime via TZ) is the single source of truth.
// Boot: seeded from DS3231 if valid. When WiFi+NTP syncs, system time is
// corrected and written back to the DS3231.
class Clock {
public:
  bool begin();                   // returns true if RTC found
  void tick();                    // call from loop()
  bool now(struct tm& out) const; // false if time not yet valid
  bool ntpSynced() const;
  bool rtcPresent() const { return rtcOk_; }
  bool rtcLostPower() const { return rtcLostPower_; }
  String iso() const;             // "YYYY-MM-DD HH:MM:SS" or "unset"
  static uint32_t dateKey(const struct tm& t) {
    return (uint32_t)(t.tm_year + 1900) * 10000 + (uint32_t)(t.tm_mon + 1) * 100 + t.tm_mday;
  }

private:
  RTC_DS3231 rtc_;
  bool rtcOk_ = false;
  bool rtcLostPower_ = false;
  bool rtcWritten_ = false;
  uint32_t lastRtcWrite_ = 0;
  uint32_t lastTick_ = 0;
};
