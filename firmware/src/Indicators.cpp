#include "Indicators.h"
#include <math.h>

void Indicators::begin() {
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
  strip_.begin();
  strip_.setBrightness(80);
  strip_.clear();
  strip_.show();
}

void Indicators::setMode(IndicatorMode m) {
  if (m == mode_) return;
  mode_ = m;
  modeStart_ = millis();
  lastPixel_ = 0;
}

void Indicators::beep(uint16_t ms) { beepUntil_ = millis() + ms; }

void Indicators::fill(uint8_t r, uint8_t g, uint8_t b) {
  for (uint16_t i = 0; i < NEOPIXEL_COUNT; i++) strip_.setPixelColor(i, strip_.Color(r, g, b));
  strip_.show();
}

void Indicators::tick() {
  uint32_t now = millis();
  uint32_t since = now - modeStart_;

  // ---- buzzer ----
  bool buzz = now < beepUntil_;
  switch (mode_) {
    case IND_DOSE_READY: {            // double beep every 3 s
      uint32_t ph = since % 3000;
      buzz = buzz || ph < 150 || (ph >= 300 && ph < 450);
      break;
    }
    case IND_ERROR: {                 // three short beeps, once
      if (since < 1000) { uint32_t ph = since % 300; buzz = buzz || ph < 120; }
      break;
    }
    case IND_LOW_PILLS:               // single chirp when entered
      if (since < 100) buzz = true;
      break;
    default: break;
  }
  digitalWrite(PIN_BUZZER, buzz ? HIGH : LOW);

  // ---- pixels (rate limited: show() blocks interrupts briefly) ----
  if (now - lastPixel_ < 30) return;
  lastPixel_ = now;
  switch (mode_) {
    case IND_IDLE:
      fill(0, 6, 0);
      break;
    case IND_DOSE_READY: {
      float k = (sinf(now / 300.0f) + 1.0f) * 0.5f;
      uint8_t v = 10 + (uint8_t)(k * 245);
      fill(0, v / 3, v);              // pulsing blue
      break;
    }
    case IND_ERROR:
      fill(255, 0, 0);
      break;
    case IND_LOW_PILLS:
      fill(255, 100, 0);              // amber
      break;
  }
}
