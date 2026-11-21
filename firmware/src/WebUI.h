#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include "Schedule.h"
#include "State.h"
#include "Clock.h"
#include "Dispenser.h"

// Shared, main-owned runtime status shown by /api/status.
struct AppStatus {
  bool busy = false;
  bool cup = false;
  bool hasLast = false;
  DispenseResult last;       // aggregate of last dose (slot field unused)
  String lastNote;
};

struct WebHooks {
  // Return nullptr on success (request queued) or a static error string.
  const char* (*requestDispense)(uint8_t slot, uint8_t count) = nullptr;
  void (*requestAck)() = nullptr;
};

class WebUI {
public:
  void begin(Schedule* s, State* st, Clock* c, AppStatus* app, WebHooks hooks);
  void handle() { server_.handleClient(); }

private:
  WebServer server_{80};
  Schedule* sched_ = nullptr;
  State* state_ = nullptr;
  Clock* clock_ = nullptr;
  AppStatus* app_ = nullptr;
  WebHooks hooks_;

  bool auth();               // false = 401 already sent, handler must return
  void sendJson(int code, const String& body) { server_.send(code, "application/json", body); }
  void sendError(int code, const char* msg);
  bool readBody(JsonDocument& doc);
  void handleStatus();
  void handleGetSchedule();
  void handlePostSchedule();
  void handleDispense();
  void handleAck();
  void handleCounts();
};
