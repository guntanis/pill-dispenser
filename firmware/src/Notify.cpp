#include "Notify.h"
#include "config.h"

#if NOTIFY_BACKEND_NTFY
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#endif

namespace {
struct Meta { const char* title; const char* prio; const char* tags; };
Meta meta(NotifyEvent e) {
  switch (e) {
    case EVT_DOSE_READY:     return {"Dose ready", "default", "pill"};
    case EVT_DOSE_NOT_TAKEN: return {"Dose NOT taken", "high", "warning"};
    case EVT_JAM:            return {"Dispenser jam", "high", "rotating_light"};
    case EVT_DOUBLE:         return {"Double drop", "high", "warning"};
    case EVT_LOW_PILLS:      return {"Low pills", "default", "package"};
    case EVT_OFFLINE_RTC:    return {"Was offline (RTC clock)", "low", "satellite"};
  }
  return {"Pill dispenser", "default", "pill"};
}
}  // namespace

namespace Notify {
void send(NotifyEvent evt, const String& detail) {
  Meta m = meta(evt);
  Serial.printf("[notify] %s: %s\n", m.title, detail.c_str());
#if NOTIFY_BACKEND_NTFY
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[notify] WiFi down, not sent");
    return;
  }
  String url = String(NTFY_SERVER) + "/" + NTFY_TOPIC;
  HTTPClient http;
  WiFiClientSecure secure;
  WiFiClient plain;
  bool ok;
  if (url.startsWith("https://")) {
    secure.setInsecure();                    // TODO: pin CA cert instead
    ok = http.begin(secure, url);
  } else {
    ok = http.begin(plain, url);
  }
  if (!ok) return;
  http.setTimeout(5000);
  http.addHeader("Title", m.title);
  http.addHeader("Priority", m.prio);
  http.addHeader("Tags", m.tags);
  int code = http.POST(detail.length() ? detail : String(m.title));
  Serial.printf("[notify] ntfy HTTP %d\n", code);
  http.end();
#endif
}
}  // namespace Notify
