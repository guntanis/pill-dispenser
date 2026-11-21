#include "WebUI.h"
#include <WiFi.h>

static const char INDEX_HTML[] PROGMEM = R"HTML(<!doctype html>
<html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Pill Dispenser</title>
<style>
body{font:15px sans-serif;max-width:720px;margin:1em auto;padding:0 .6em}
table{border-collapse:collapse}td,th{padding:2px 4px;border:1px solid #ccc}
input[type=number]{width:3.5em}button{margin:2px;padding:4px 8px}
.warn{color:#b00;font-weight:bold}.ok{color:#080}pre{margin:0}
</style></head><body>
<h2>Pill Dispenser</h2>
<pre id="st">loading...</pre>
<button onclick="ack()">Dose taken (acknowledge)</button>
<h3>Dispense now</h3><div id="dn"></div>
<h3>Pill counts</h3><div id="ct"></div>
<h3>Schedule</h3>
<table id="sc"></table>
<button onclick="addRow()">Add event</button>
<button onclick="saveSched()">Save schedule</button>
<div id="msg"></div>
<script>
var L='ABCDE',built=false;
function p2(n){return (n<10?'0':'')+n}
function msg(t,bad){var m=document.getElementById('msg');m.className=bad?'warn':'ok';m.textContent=t}
function j(u,o){return fetch(u,o).then(function(r){return r.text().then(function(t){
 var d={};try{d=JSON.parse(t)}catch(e){}
 if(!r.ok)throw new Error(d.error||t);return d})})}
function post(u,b){return j(u,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(b||{})})}
function status(){j('/api/status').then(function(s){
 var t='Time: '+s.time+'  (NTP '+(s.ntp?'ok':'no')+', RTC '+(s.rtc?'ok':'missing')+')\n';
 t+='WiFi RSSI: '+s.rssi+'  Cup: '+(s.cup?'present':'ABSENT')+'  Busy: '+s.busy+'\n';
 t+='Dose pending: '+(s.pending?'YES':'no')+'\n';
 if(s.last)t+='Last dose: requested '+s.last.requested+', dropped '+s.last.dropped+', jams '+s.last.jams+', doubles '+s.last.doubles+(s.last.refused?' (refused/aborted)':'')+'\n';
 document.getElementById('st').textContent=t;
 if(!built){built=true;build(s)}
}).catch(function(e){document.getElementById('st').textContent='error: '+e.message})}
function build(s){
 var d='',c='';
 for(var i=0;i<5;i++){
  d+='<button onclick="disp('+i+')">'+L[i]+'</button>';
  c+=L[i]+': <input type="number" min="0" max="999" id="c'+i+'" value="'+s.counts[i]+'"> ';
 }
 d+=' count <input type="number" id="dc" min="1" max="9" value="1">';
 c+='<br>Low threshold: <input type="number" id="lw" min="0" max="255" value="'+s.low+'"> <button onclick="saveCounts()">Save counts</button>';
 document.getElementById('dn').innerHTML=d;document.getElementById('ct').innerHTML=c;
 loadSched();
}
function disp(i){post('/api/dispense',{slot:i,count:+document.getElementById('dc').value||1}).then(function(){msg('Dispense queued')}).catch(function(e){msg(e.message,1)})}
function ack(){post('/api/ack').then(function(){msg('Acknowledged')}).catch(function(e){msg(e.message,1)})}
function saveCounts(){var a=[];for(var i=0;i<5;i++)a.push(+document.getElementById('c'+i).value||0);
 post('/api/counts',{counts:a,low:+document.getElementById('lw').value||0}).then(function(){msg('Counts saved')}).catch(function(e){msg(e.message,1)})}
function rowHtml(e){
 var h='<tr class="r"><td><input type="time" value="'+p2(e.hour)+':'+p2(e.minute)+'"></td>';
 for(var i=0;i<5;i++)h+='<td><input type="number" min="0" max="9" value="'+e.counts[i]+'"></td>';
 h+='<td><input type="checkbox"'+(e.enabled?' checked':'')+'></td><td><button onclick="del(this)">x</button></td></tr>';
 return h}
function del(b){b.closest('tr').remove()}
function addRow(){document.getElementById('sc').insertAdjacentHTML('beforeend',rowHtml({hour:12,minute:0,counts:[0,0,0,0,0],enabled:true}))}
function loadSched(){j('/api/schedule').then(function(d){
 var h='<tr><th>Time</th><th>A</th><th>B</th><th>C</th><th>D</th><th>E</th><th>On</th><th></th></tr>';
 d.events.forEach(function(e){h+=rowHtml(e)});
 document.getElementById('sc').innerHTML=h})}
function saveSched(){
 var ev=[].map.call(document.querySelectorAll('#sc tr.r'),function(tr){
  var x=tr.querySelectorAll('input'),hm=(x[0].value||'00:00').split(':');
  var c=[];for(var k=1;k<=5;k++)c.push(+x[k].value||0);
  return {hour:+hm[0],minute:+hm[1],counts:c,enabled:x[6].checked}});
 post('/api/schedule',{events:ev}).then(function(){msg('Schedule saved')}).catch(function(e){msg(e.message,1)})}
status();setInterval(status,5000);
</script></body></html>)HTML";

void WebUI::begin(Schedule* s, State* st, Clock* c, AppStatus* app, WebHooks hooks) {
  sched_ = s; state_ = st; clock_ = c; app_ = app; hooks_ = hooks;
  server_.on("/", HTTP_GET, [this]() { server_.send_P(200, "text/html", INDEX_HTML); });
  server_.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
  server_.on("/api/schedule", HTTP_GET, [this]() { handleGetSchedule(); });
  server_.on("/api/schedule", HTTP_POST, [this]() { handlePostSchedule(); });
  server_.on("/api/dispense", HTTP_POST, [this]() { handleDispense(); });
  server_.on("/api/ack", HTTP_POST, [this]() { handleAck(); });
  server_.on("/api/counts", HTTP_POST, [this]() { handleCounts(); });
  server_.onNotFound([this]() { sendError(404, "not found"); });
  server_.begin();
  Serial.println("[web] server started on :80");
}

void WebUI::sendError(int code, const char* msg) {
  JsonDocument d;
  d["error"] = msg;
  String out;
  serializeJson(d, out);
  sendJson(code, out);
}

bool WebUI::readBody(JsonDocument& doc) {
  if (!server_.hasArg("plain")) return false;
  return !deserializeJson(doc, server_.arg("plain"));
}

void WebUI::handleStatus() {
  JsonDocument d;
  d["time"] = clock_->iso();
  d["ntp"] = clock_->ntpSynced();
  d["rtc"] = clock_->rtcPresent();
  d["rssi"] = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;
  d["cup"] = app_->cup;
  d["busy"] = app_->busy;
  d["pending"] = state_->pending();
  d["pendingSince"] = state_->pendingSince();
  d["low"] = state_->lowThreshold();
  JsonArray c = d["counts"].to<JsonArray>();
  for (uint8_t i = 0; i < NUM_SLOTS; i++) c.add(state_->count(i));
  if (app_->hasLast) {
    JsonObject l = d["last"].to<JsonObject>();
    l["requested"] = app_->last.requested;
    l["dropped"] = app_->last.dropped;
    l["jams"] = app_->last.jams;
    l["doubles"] = app_->last.doubles;
    l["refused"] = app_->last.refused;
  }
  String out;
  serializeJson(d, out);
  sendJson(200, out);
}

void WebUI::handleGetSchedule() {
  JsonDocument d;
  sched_->toJson(d);
  String out;
  serializeJson(d, out);
  sendJson(200, out);
}

void WebUI::handlePostSchedule() {
  JsonDocument d;
  if (!readBody(d)) { sendError(400, "bad json"); return; }
  if (!sched_->setFromJson(d.as<JsonVariantConst>())) { sendError(400, "invalid schedule (range/duplicate time)"); return; }
  if (!sched_->save()) { sendError(500, "save failed"); return; }
  sendJson(200, "{\"ok\":true}");
}

void WebUI::handleDispense() {
  JsonDocument d;
  if (!readBody(d)) { sendError(400, "bad json"); return; }
  int slot = d["slot"] | -1;
  int count = d["count"] | 1;
  if (slot < 0 || slot >= NUM_SLOTS || count < 1 || count > MAX_PILLS_PER_DOSE) { sendError(400, "bad slot/count"); return; }
  const char* err = hooks_.requestDispense ? hooks_.requestDispense((uint8_t)slot, (uint8_t)count) : "unavailable";
  if (err) { sendError(409, err); return; }
  sendJson(202, "{\"ok\":true}");
}

void WebUI::handleAck() {
  if (hooks_.requestAck) hooks_.requestAck();
  sendJson(200, "{\"ok\":true}");
}

void WebUI::handleCounts() {
  JsonDocument d;
  if (!readBody(d)) { sendError(400, "bad json"); return; }
  JsonArrayConst c = d["counts"].as<JsonArrayConst>();
  if (!c.isNull()) {
    for (uint8_t i = 0; i < NUM_SLOTS; i++) {
      int n = c[i] | 0;
      if (n < 0) n = 0;
      if (n > 65535) n = 65535;
      state_->setCount(i, (uint16_t)n);
    }
  }
  if (!d["low"].isNull()) {
    int l = d["low"] | DEFAULT_LOW_THRESHOLD;
    state_->setLowThreshold((uint8_t)constrain(l, 0, 255));
  }
  sendJson(200, "{\"ok\":true}");
}
