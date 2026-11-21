#include "Dispenser.h"
#include <esp_task_wdt.h>

static DropSensor* g_sensor = nullptr;
// Runs after every half-step: poll the beam and feed the watchdog, so a long dose
// does not trip it but a hung I2C write (no more ticks) does.
static void sensorTick() {
  if (g_sensor) g_sensor->poll();
  esp_task_wdt_reset();
}

void Dispenser::begin(MotorBank* motors, DropSensor* sensor) {
  motors_ = motors;
  sensor_ = sensor;
  g_sensor = sensor;
  pinMode(PIN_CUP_SWITCH, INPUT_PULLUP);
}

bool Dispenser::cupPresent() const { return digitalRead(PIN_CUP_SWITCH) == LOW; }

DispenseResult Dispenser::dispense(uint8_t slot, uint8_t count) {
  DispenseResult r;
  r.slot = slot;
  r.requested = count;
  if (slot >= NUM_SLOTS) { r.refused = true; return r; }
  if (!cupPresent()) { r.refused = true; r.cupMissing = true; return r; }

  motors_->setTickCallback(sensorTick);   // poll beam during every half-step
  for (uint8_t p = 0; p < count; p++) {
    if (!cupPresent()) { r.refused = true; r.cupMissing = true; break; }

    sensor_->arm();
    motors_->step(slot, STEPS_PER_PILL, DISPENSE_FORWARD);
    uint8_t n = sensor_->waitForDrop(DROP_TIMEOUT_MS);

    uint8_t attempts = 0;
    while (n == 0) {
      r.jams++;
      Serial.printf("[disp] slot %c: no drop (attempt %u)\n", SLOT_LABELS[slot], attempts + 1);
      if (attempts >= MAX_RETRIES) break;
      attempts++;
      motors_->agitate(slot);
      n = sensor_->waitForDrop(AGITATE_WAIT_MS);       // pill may fall during agitation
      if (n == 0) {
        motors_->step(slot, STEPS_PER_PILL, DISPENSE_FORWARD);
        n = sensor_->waitForDrop(DROP_TIMEOUT_MS);
      }
    }

    if (n == 0) break;                 // gave up: stop spinning this slot
    if (n >= 2) { r.doubles++; Serial.printf("[disp] slot %c: DOUBLE drop (%u)\n", SLOT_LABELS[slot], n); }
    r.dropped += n;
    delay(INTER_PILL_DELAY_MS);
  }
  motors_->setTickCallback(nullptr);
  motors_->release(slot);
  return r;
}
