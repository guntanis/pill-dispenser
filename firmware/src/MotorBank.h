#pragma once
#include <Arduino.h>
#include <Adafruit_MCP23X17.h>
#include "config.h"

// Drives 5x 28BYJ-48 (ULN2003) through two MCP23017 expanders.
// Only one motor is ever energized at a time; coils are released after a move.
class MotorBank {
public:
  bool begin();
  void step(uint8_t slot, uint32_t halfSteps, bool forward = true);  // blocking, releases after
  void rotateDegrees(uint8_t slot, float degrees);                   // sign = direction
  void agitate(uint8_t slot);                                        // rock +/-AGITATE_DEGREES
  void release(uint8_t slot);
  void releaseAll();
  // Called after every half-step (used to poll the drop sensor mid-rotation).
  void setTickCallback(void (*cb)()) { tick_ = cb; }

private:
  Adafruit_MCP23X17 mcp_[2];
  uint16_t shadow_[2] = {0, 0};   // bits 0-7 = GPA, 8-15 = GPB
  uint8_t phase_[NUM_SLOTS] = {0};
  int8_t active_ = -1;
  void (*tick_)() = nullptr;

  static uint8_t chipOf(uint8_t slot) { return slot < 4 ? 0 : 1; }
  static uint8_t shiftOf(uint8_t slot) { return slot < 4 ? slot * 4 : 0; }
  void writeNibble(uint8_t slot, uint8_t nibble);
  void move(uint8_t slot, uint32_t n, bool forward);
};
