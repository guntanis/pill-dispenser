#pragma once
#include <Arduino.h>

enum NotifyEvent {
  EVT_DOSE_READY,
  EVT_DOSE_NOT_TAKEN,
  EVT_JAM,
  EVT_DOUBLE,
  EVT_LOW_PILLS,
  EVT_OFFLINE_RTC
};

namespace Notify {
  // Always logs to Serial; pushes to the configured backend if WiFi is up.
  // Blocking (short HTTP timeout). Messages are dropped when offline (see README TODO).
  void send(NotifyEvent evt, const String& detail = "");
}
