#include "DropSensor.h"

void DropSensor::begin() {
  pinMode(PIN_IR_BEAM, INPUT_PULLUP);
  lastRaw_ = stable_ = (digitalRead(PIN_IR_BEAM) == LOW);
  lastChange_ = millis();
}

void DropSensor::arm() {
  count_ = 0;
  armed_ = true;
  lastBreakAt_ = millis();
}

void DropSensor::poll() {
  bool raw = (digitalRead(PIN_IR_BEAM) == LOW);
  uint32_t now = millis();
  if (raw != lastRaw_) {
    lastRaw_ = raw;
    lastChange_ = now;
  }
  if (raw != stable_ && (now - lastChange_) >= DROP_DEBOUNCE_MS) {
    stable_ = raw;
    if (stable_ && armed_) {          // new confirmed break
      if (count_ < 255) count_++;
      lastBreakAt_ = now;
    }
  }
}

uint8_t DropSensor::waitForDrop(uint32_t timeoutMs) {
  uint32_t t0 = millis();
  for (;;) {
    poll();
    uint32_t now = millis();
    if (count_ > 0) {
      if (now - lastBreakAt_ >= DROP_SETTLE_MS) break;
    } else if (now - t0 >= timeoutMs) {
      break;
    }
    delay(1);
  }
  return count_;
}
