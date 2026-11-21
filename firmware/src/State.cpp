#include "State.h"

void State::begin() {
  prefs_.begin("pill", false);
  pending_ = prefs_.getBool("pend", false);
  pendingSince_ = prefs_.getUInt("pendTs", 0);
}

uint32_t State::lastDispensed(uint16_t m) {
  char k[10]; snprintf(k, sizeof(k), "ld%u", m);
  return prefs_.getUInt(k, 0);
}

void State::setLastDispensed(uint16_t m, uint32_t d) {
  char k[10]; snprintf(k, sizeof(k), "ld%u", m);
  prefs_.putUInt(k, d);
}

uint16_t State::count(uint8_t slot) {
  char k[8]; snprintf(k, sizeof(k), "cnt%u", slot);
  return prefs_.getUShort(k, 0);
}

void State::setCount(uint8_t slot, uint16_t n) {
  char k[8]; snprintf(k, sizeof(k), "cnt%u", slot);
  prefs_.putUShort(k, n);
}

void State::consume(uint8_t slot, uint16_t n) {
  uint16_t c = count(slot);
  setCount(slot, c > n ? c - n : 0);
}

// Daily counters live under "dd<slot>" with the date they belong to in "ddate".
// A different date resets all five; dateKey 0 never resets them.
static void rollDay(Preferences& p, uint32_t dateKey) {
  if (dateKey == 0) return;
  if (p.getUInt("ddate", 0) == dateKey) return;
  for (uint8_t s = 0; s < NUM_SLOTS; s++) {
    char k[8]; snprintf(k, sizeof(k), "dd%u", s);
    p.putUShort(k, 0);
  }
  p.putUInt("ddate", dateKey);
}

uint16_t State::dispensedToday(uint8_t slot, uint32_t dateKey) {
  rollDay(prefs_, dateKey);
  char k[8]; snprintf(k, sizeof(k), "dd%u", slot);
  return prefs_.getUShort(k, 0);
}

void State::addDispensedToday(uint8_t slot, uint16_t n, uint32_t dateKey) {
  uint16_t c = dispensedToday(slot, dateKey);
  char k[8]; snprintf(k, sizeof(k), "dd%u", slot);
  uint32_t v = (uint32_t)c + n;
  if (v > 65535u) v = 65535u;
  prefs_.putUShort(k, (uint16_t)v);
}

uint32_t State::lastDoseAt(uint8_t slot) {
  char k[8]; snprintf(k, sizeof(k), "la%u", slot);
  return prefs_.getUInt(k, 0);
}

void State::setLastDoseAt(uint8_t slot, uint32_t epoch) {
  char k[8]; snprintf(k, sizeof(k), "la%u", slot);
  prefs_.putUInt(k, epoch);
}

uint8_t State::lowThreshold() { return prefs_.getUChar("low", DEFAULT_LOW_THRESHOLD); }
void State::setLowThreshold(uint8_t t) { prefs_.putUChar("low", t); }

bool State::anyLow() {
  for (uint8_t s = 0; s < NUM_SLOTS; s++) if (isLow(s)) return true;
  return false;
}

void State::setPending(bool p, uint32_t since) {
  pending_ = p;
  pendingSince_ = p ? since : 0;
  prefs_.putBool("pend", pending_);
  prefs_.putUInt("pendTs", pendingSince_);
}
