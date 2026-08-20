#include "web_dashboard.h"
#include "scan.h"
#include "config.h"
#include "dashboard_html.h"
#include "text_summary.h"
#include "pcap_stream.h"
#include "session_pcap.h"

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace web_dashboard {

namespace {

AsyncWebServer   server(80);
AsyncWebSocket   ws("/ws");
TaskHandle_t     dash_task_h = nullptr;
uint32_t         boot_ms = 0;

size_t append_pkt_json(const scan::Frame& f, char* out, size_t cap) {
    char addr[18];
    text_summary::format_addr(f.addr, addr);
    char name[32] = {0};
    text_summary::extract_name(f, name, sizeof(name));
    char svc[80]; svc[0] = 0;
    text_summary::extract_service_uuids(f, svc, sizeof(svc));
    uint16_t mfr = text_summary::manufacturer_id(f);
    uint8_t  tr  = text_summary::traits(f);

    StaticJsonDocument<512> doc;
    doc["i"] = f.idx;
    doc["t"] = (uint32_t)(millis() - boot_ms);
    doc["c"] = (int)(f.channel <= 39 ? f.channel : -1);
    doc["r"] = (int)f.rssi;
    if (f.tx_power != INT8_MIN) doc["x"] = (int)f.tx_power;
    doc["y"] = text_summary::ll_type_name(f.ll_pdu_type);
    doc["a"] = text_summary::addr_type_short(f.addr_type);
    doc["m"] = addr;
    doc["l"] = f.payload_len;
    doc["f"] = tr;                                   // traits bitfield
    if (name[0]) doc["n"] = name;
    if (svc[0])  doc["s"] = svc;
    if (mfr != 0xFFFF) {
        char mbuf[16];
        snprintf(mbuf, sizeof(mbuf), "%04X", mfr);
        doc["u"] = mbuf;                             // mfr id hex
        doc["v"] = text_summary::mfr_shortname(mfr); // mfr shortname
    }

    size_t n = measureJson(doc);
    if (n + 2 > cap) return 0;
    return serializeJson(doc, out, cap);
}

void send_status() {
    if (ws.count() == 0) return;
    StaticJsonDocument<384> doc;
    doc["type"] = "status";
    doc["uptime"] = (uint32_t)((millis() - boot_ms) / 1000);
    doc["pps"]    = scan::adverts_per_sec();
    doc["total"]  = scan::total_adverts();
    doc["dropped_pcap"] = scan::dropped_pcap();
    doc["dropped_dash"] = scan::dropped_dash();
    doc["session_bytes"] = (uint32_t)session_pcap::size();
    doc["fw"] = config::FW_VERSION();

    char buf[400];
    size_t n = serializeJson(doc, buf, sizeof(buf));
    ws.textAll(buf, n);
}

static constexpr size_t BATCH_CAP        = 8192;
static constexpr size_t BATCH_FLUSH_WATER = 6144;
static constexpr uint32_t BATCH_TICK_MS  = 30;
static constexpr int MAX_DRAIN_PER_TICK  = 120;

void flush_batch(char* buf, size_t& pos, uint16_t& count) {
    if (count == 0) return;
    buf[pos++] = ']';
    buf[pos++] = '}';
    if (ws.count() > 0 && ws.availableForWriteAll()) {
        ws.textAll(buf, pos);
    }
    pos = 0;
    count = 0;
}

void begin_batch(char* buf, size_t& pos) {
    memcpy(buf, "{\"type\":\"pkts\",\"p\":[", 20);
    pos = 20;
}

void dashboard_task(void*) {
    uint32_t last_status = 0;
    uint32_t last_flush  = 0;
    static char batch[BATCH_CAP];
    size_t pos = 0;
    uint16_t count = 0;
    begin_batch(batch, pos);

    for (;;) {
        scan::Frame f;
        int drained = 0;
        while (drained < MAX_DRAIN_PER_TICK && scan::pop_dashboard(&f)) {
            if (count > 0) {
                if (pos + 1 >= BATCH_CAP) break;
                batch[pos++] = ',';
            }
            size_t n = append_pkt_json(f, batch + pos, BATCH_CAP - pos - 2);
            if (n == 0) {
                if (count > 0) pos--;
                break;
            }
            pos += n;
            count++;
            drained++;
            if (pos >= BATCH_FLUSH_WATER) break;
        }

        uint32_t now = millis();
        bool tick_expired = (now - last_flush) >= BATCH_TICK_MS;
        if (count > 0 && (tick_expired || pos >= BATCH_FLUSH_WATER)) {
            flush_batch(batch, pos, count);
            begin_batch(batch, pos);
            last_flush = now;
        }

        if (now - last_status > 1000) {
            send_status();
            ws.cleanupClients();
            last_status = now;
        }
        vTaskDelay(pdMS_TO_TICKS(drained >= MAX_DRAIN_PER_TICK ? 2 : 20));
    }
}

void handle_get_config(AsyncWebServerRequest* req) {
    StaticJsonDocument<512> doc;
    const auto& c = config::get();
    doc["scan_win"] = c.scan_window_ms;
    doc["scan_int"] = c.scan_interval_ms;
    doc["ftmask"]   = c.ft_mask;
    doc["ap_ssid"]  = c.ap_ssid;
    doc["ap_pass"]  = c.ap_pass;
    String body;
    serializeJson(doc, body);
    req->send(200, "application/json", body);
}

bool accumulate_body(AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t index, size_t total) {
    if (total == 0 || total > 8192) return false;
    if (index == 0 && req->_tempObject == nullptr) {
        req->_tempObject = malloc(total);
    }
    if (req->_tempObject == nullptr) return false;
    memcpy((uint8_t*)req->_tempObject + index, data, len);
    return (index + len) >= total;
}

void handle_post_config(AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t index, size_t total) {
    if (!accumulate_body(req, data, len, index, total)) return;
    StaticJsonDocument<640> doc;
    DeserializationError err = deserializeJson(doc, (const uint8_t*)req->_tempObject, total);
    if (err) { req->send(400, "application/json", "{\"error\":\"json\"}"); return; }

    bool need_apply_scan = false;

    if (doc.containsKey("scan_win")) {
        uint16_t v = doc["scan_win"];
        if (v != config::get().scan_window_ms) { config::set_scan_window(v); need_apply_scan = true; }
    }
    if (doc.containsKey("scan_int")) {
        uint16_t v = doc["scan_int"];
        if (v != config::get().scan_interval_ms) { config::set_scan_interval(v); need_apply_scan = true; }
    }
    if (doc.containsKey("ftmask")) {
        uint8_t m = doc["ftmask"];
        if (m != config::get().ft_mask) { config::set_ftmask(m); }
    }

    if (need_apply_scan) scan::apply_scan_params();

    req->send(200, "application/json", "{\"ok\":true}");
}

void handle_post_ap(AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t index, size_t total) {
    if (!accumulate_body(req, data, len, index, total)) return;
    StaticJsonDocument<256> doc;
    if (deserializeJson(doc, (const uint8_t*)req->_tempObject, total)) { req->send(400); return; }
    const char* ssid = doc["ssid"] | "";
    const char* pass = doc["pass"] | "";
    config::set_ap(ssid, pass);
    req->send(200, "application/json", "{\"ok\":true}");
}

void handle_reboot(AsyncWebServerRequest* req) {
    req->send(200, "application/json", "{\"ok\":true}");
    delay(200);
    ESP.restart();
}

void handle_reset(AsyncWebServerRequest* req) {
    config::reset_defaults();
    req->send(200, "application/json", "{\"ok\":true}");
    delay(200);
    ESP.restart();
}

void handle_clear(AsyncWebServerRequest* req) {
    scan::clear_ring();
    req->send(200, "application/json", "{\"ok\":true}");
}

void handle_session_clear(AsyncWebServerRequest* req) {
    session_pcap::clear();
    req->send(200, "application/json", "{\"ok\":true}");
}

void handle_session_pcap(AsyncWebServerRequest* req) {
    // Snapshot the live ring so the download can't desync when the writer
    // memmoves the buffer mid-transfer.
    const size_t total = session_pcap::snapshot_take();
    if (total == 0) { req->send(204, "application/vnd.tcpdump.pcap", ""); return; }

    AsyncWebServerResponse* r = req->beginChunkedResponse(
        "application/vnd.tcpdump.pcap",
        [](uint8_t* buf, size_t maxLen, size_t index) -> size_t {
            return session_pcap::snapshot_read(index, buf, maxLen);
        });
    char filename[64];
    snprintf(filename, sizeof(filename), "attachment; filename=\"ouispy-blesniff-%lu.pcap\"",
             (unsigned long)(millis() / 1000));
    r->addHeader("Content-Disposition", filename);
    r->addHeader("Cache-Control", "no-store");
    req->send(r);
}

} // namespace

uint32_t connected_clients() { return ws.count(); }

bool init() {
    boot_ms = millis();

    server.on("/", HTTP_GET, [](AsyncWebServerRequest* req){
        AsyncWebServerResponse* r = req->beginResponse_P(200, "text/html", (const uint8_t*)INDEX_HTML, strlen_P(INDEX_HTML));
        r->addHeader("Cache-Control", "no-store");
        req->send(r);
    });
    server.on("/api/config", HTTP_GET, handle_get_config);
    server.on("/api/config", HTTP_POST,
        [](AsyncWebServerRequest* req){}, nullptr, handle_post_config);
    server.on("/api/ap", HTTP_POST,
        [](AsyncWebServerRequest* req){}, nullptr, handle_post_ap);
    server.on("/api/reboot", HTTP_POST, handle_reboot);
    server.on("/api/reset", HTTP_POST, handle_reset);
    server.on("/api/clear", HTTP_POST, handle_clear);
    server.on("/api/session.pcap", HTTP_GET, handle_session_pcap);
    server.on("/api/session/clear", HTTP_POST, handle_session_clear);

    server.onNotFound([](AsyncWebServerRequest* req){ req->send(404, "text/plain", "not found"); });

    server.addHandler(&ws);
    server.begin();

    xTaskCreatePinnedToCore(&dashboard_task, "dash", 10240, nullptr, 3, &dash_task_h, 1);
    return true;
}

void tick() {
    ws.cleanupClients();
}

} // namespace web_dashboard
