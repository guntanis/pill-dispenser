#include "Clock.h"
#include <Wire.h>
#include <sys/time.h>
#include <esp_sntp.h>

static volatile bool g_ntpSynced = false;
static void onNtpSync(struct timeval*) { g_ntpSynced = true; }

bool Clock::begin() {
  sntp_set_time_sync_notification_cb(onNtpSync);
  configTzTime(TZ_STRING, NTP_SERVER1, NTP_SERVER2);   // sets TZ; SNTP starts, syncs once WiFi is up

  rtcOk_ = rtc_.begin(&Wire);
  if (!rtcOk_) {
    Serial.println("[clock] DS3231 not found");
    return false;
  }
  rtcLostPower_ = rtc_.lostPower();
  if (rtcLostPower_) {
    Serial.println("[clock] RTC lost power, time invalid until NTP sync");
    return true;
  }
  DateTime d = rtc_.now();       // RTC holds LOCAL time
  struct tm t = {};
  t.tm_year = d.year() - 1900;
  t.tm_mon = d.month() - 1;
  t.tm_mday = d.day();
  t.tm_hour = d.hour();
  t.tm_min = d.minute();
  t.tm_sec = d.second();
  t.tm_isdst = -1;
  time_t ts = mktime(&t);
  if (d.year() >= 2024 && ts > 0) {
    struct timeval tv = {ts, 0};
    settimeofday(&tv, nullptr);
    Serial.printf("[clock] seeded from RTC: %s\n", iso().c_str());
  }
  return true;
}

void Clock::tick() {
  if (millis() - lastTick_ < 1000) return;
  lastTick_ = millis();
  if (g_ntpSynced && rtcOk_ && (!rtcWritten_ || millis() - lastRtcWrite_ >= RTC_RESYNC_MS)) {
    struct tm t;
    if (now(t)) {
      rtc_.adjust(DateTime(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec));
      rtcWritten_ = true;
      rtcLostPower_ = false;
      lastRtcWrite_ = millis();
      Serial.printf("[clock] RTC updated from NTP: %s\n", iso().c_str());
    }
  }
}

bool Clock::now(struct tm& out) const {
  time_t ts = time(nullptr);
  localtime_r(&ts, &out);
  return out.tm_year + 1900 >= 2024;
}

bool Clock::ntpSynced() const { return g_ntpSynced; }

String Clock::iso() const {
  struct tm t;
  if (!now(t)) return "unset";
  char buf[24];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
  return String(buf);
}
