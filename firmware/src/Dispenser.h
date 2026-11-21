#pragma once
#include <Arduino.h>
#include "config.h"
#include "MotorBank.h"
#include "DropSensor.h"

struct DispenseResult {
  uint8_t slot = 0;
  uint8_t requested = 0;
  uint8_t dropped = 0;     // pills detected (a double drop counts as 2)
  uint8_t jams = 0;        // failed attempts (each retry that saw nothing)
  uint8_t doubles = 0;     // pills where >1 drop detected
  bool refused = false;    // not run at all / aborted
  bool cupMissing = false;
};

class Dispenser {
public:
  void begin(MotorBank* motors, DropSensor* sensor);
  bool cupPresent() const;                       // microswitch LOW = present
  DispenseResult dispense(uint8_t slot, uint8_t count);

private:
  MotorBank* motors_ = nullptr;
  DropSensor* sensor_ = nullptr;
};
