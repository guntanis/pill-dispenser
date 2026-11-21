#pragma once
#include <Arduino.h>
#include "config.h"

// IR break-beam: LOW = beam broken. Counts distinct (debounced) breaks.
class DropSensor {
public:
  void begin();
  void arm();                         // reset counter and start counting
  void poll();                        // call often (motor tick does this)
  bool armed() const { return armed_; }
  uint8_t count() const { return count_; }
  bool beamBroken() const { return stable_; }
  // Blocks (yielding) until: no break within timeoutMs -> 0, or a break was seen
  // and DROP_SETTLE_MS passed without further breaks. Returns number of breaks.
  uint8_t waitForDrop(uint32_t timeoutMs);

private:
  bool armed_ = false;
  bool stable_ = false;    // true = broken
  bool lastRaw_ = false;
  uint32_t lastChange_ = 0;
  uint32_t lastBreakAt_ = 0;
  uint8_t count_ = 0;
};
