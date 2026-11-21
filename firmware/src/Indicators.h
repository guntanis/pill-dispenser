#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"

enum IndicatorMode { IND_IDLE, IND_DOSE_READY, IND_ERROR, IND_LOW_PILLS };

class Indicators {
public:
  Indicators() : strip_(NEOPIXEL_COUNT, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800) {}
  void begin();
  void setMode(IndicatorMode m);       // no-op if unchanged
  IndicatorMode mode() const { return mode_; }
  void tick();                         // non-blocking, call every loop
  void beep(uint16_t ms);              // short blocking-free-ish confirmation beep

private:
  Adafruit_NeoPixel strip_;
  IndicatorMode mode_ = IND_IDLE;
  uint32_t modeStart_ = 0;
  uint32_t lastPixel_ = 0;
  uint32_t beepUntil_ = 0;
  void fill(uint8_t r, uint8_t g, uint8_t b);
};
