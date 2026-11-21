#include "MotorBank.h"
#include <Wire.h>

// Half-step sequence, bit0..3 = IN1..IN4
static const uint8_t HALF_STEP_SEQ[8] = {0x1, 0x3, 0x2, 0x6, 0x4, 0xC, 0x8, 0x9};

bool MotorBank::begin() {
  const uint8_t addrs[2] = {MCP0_ADDR, MCP1_ADDR};
  bool ok = true;
  for (uint8_t c = 0; c < 2; c++) {
    if (!mcp_[c].begin_I2C(addrs[c], &Wire)) {
      Serial.printf("[motor] MCP23017 @0x%02X not found\n", addrs[c]);
      ok = false;
      continue;
    }
    for (uint8_t p = 0; p < 16; p++) mcp_[c].pinMode(p, OUTPUT);
    shadow_[c] = 0;
    mcp_[c].writeGPIOAB(0);
  }
  return ok;
}

void MotorBank::writeNibble(uint8_t slot, uint8_t nibble) {
  uint8_t c = chipOf(slot), s = shiftOf(slot);
  shadow_[c] = (uint16_t)((shadow_[c] & ~(0xFu << s)) | ((uint16_t)(nibble & 0xF) << s));
  mcp_[c].writeGPIOAB(shadow_[c]);
}

void MotorBank::move(uint8_t slot, uint32_t n, bool forward) {
  if (slot >= NUM_SLOTS) return;
  if (active_ >= 0 && active_ != (int8_t)slot) release((uint8_t)active_);
  active_ = (int8_t)slot;
  for (uint32_t i = 0; i < n; i++) {
    phase_[slot] = forward ? (phase_[slot] + 1) & 7 : (phase_[slot] + 7) & 7;
    writeNibble(slot, HALF_STEP_SEQ[phase_[slot]]);
    if (tick_) tick_();
    delay(STEP_DELAY_MS);   // also yields to WiFi / RTOS
  }
}

void MotorBank::step(uint8_t slot, uint32_t halfSteps, bool forward) {
  move(slot, halfSteps, forward);
  release(slot);
}

void MotorBank::rotateDegrees(uint8_t slot, float degrees) {
  bool fwd = degrees >= 0;
  float a = fabsf(degrees);
  step(slot, (uint32_t)(a * HALF_STEPS_PER_REV / 360.0f + 0.5f), fwd);
}

void MotorBank::agitate(uint8_t slot) {
  uint32_t n = (uint32_t)(AGITATE_DEGREES * HALF_STEPS_PER_REV / 360.0f + 0.5f);
  move(slot, n, true);
  for (uint8_t i = 0; i < AGITATE_CYCLES; i++) {
    move(slot, 2 * n, false);
    move(slot, 2 * n, true);
  }
  move(slot, n, false);   // net zero displacement
  release(slot);
}

void MotorBank::release(uint8_t slot) {
  if (slot >= NUM_SLOTS) return;
  writeNibble(slot, 0);
  if (active_ == (int8_t)slot) active_ = -1;
}

void MotorBank::releaseAll() {
  for (uint8_t s = 0; s < NUM_SLOTS; s++) writeNibble(s, 0);
  active_ = -1;
}
