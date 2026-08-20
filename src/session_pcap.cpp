#include "session_pcap.h"
#include "nordic_pcap.h"

#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace session_pcap {

namespace {

struct __attribute__((packed)) PcapGlobal {
    uint32_t magic;
    uint16_t vmaj, vmin;
    int32_t  tz;
    uint32_t sig;
    uint32_t snaplen;
    uint32_t linktype;
};

struct __attribute__((packed)) PcapRec {
    uint32_t ts_sec, ts_usec;
    uint32_t incl_len, orig_len;
};

// Tiered PSRAM allocation. First entry that succeeds wins; nothing falls
// through to DRAM (that path OOM-crashed the ESP32 previously).
constexpr size_t CAP_TIERS[] = { 6 * 1024 * 1024, 4 * 1024 * 1024, 2 * 1024 * 1024 };

uint8_t*             g_buf     = nullptr;
size_t               g_cap     = 0;
size_t               g_used    = 0;
uint32_t             g_dropped = 0;
SemaphoreHandle_t    g_lock    = nullptr;

volatile State       g_state          = State::IDLE;
volatile uint32_t    g_downloads      = 0;

void write_global_header_locked() {
    PcapGlobal g{};
    g.magic    = nordic_pcap::PCAP_MAGIC;
    g.vmaj     = nordic_pcap::PCAP_VER_MAJOR;
    g.vmin     = nordic_pcap::PCAP_VER_MINOR;
    g.snaplen  = nordic_pcap::PCAP_SNAPLEN;
    g.linktype = nordic_pcap::PCAP_LINKTYPE;
    memcpy(g_buf, &g, sizeof(g));
    g_used = sizeof(g);
}

// Walk record boundaries forward until we've skipped at least `bytes_to_drop`.
size_t next_boundary_after_locked(size_t bytes_to_drop) {
    size_t o = GLOBAL_HDR_LEN;
    size_t dropped = 0;
    while (o + sizeof(PcapRec) <= g_used) {
        PcapRec rec;
        memcpy(&rec, g_buf + o, sizeof(rec));
        size_t rec_total = sizeof(rec) + rec.incl_len;
        if (o + rec_total > g_used) break;
        dropped += rec_total;
        o += rec_total;
        if (dropped >= bytes_to_drop) return o;
    }
    return o;
}

void reset_ring_locked() {
    write_global_header_locked();
    g_dropped = 0;
}

} // namespace

bool init() {
    if (g_buf) return true;
    g_lock = xSemaphoreCreateMutex();
    if (!g_lock) return false;

    for (size_t i = 0; i < sizeof(CAP_TIERS) / sizeof(CAP_TIERS[0]); ++i) {
        uint8_t* p = (uint8_t*)heap_caps_malloc(CAP_TIERS[i],
                                                MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (p) {
            g_buf = p;
            g_cap = CAP_TIERS[i];
            Serial.printf("[session_pcap] tier %u ok: %u bytes in PSRAM\n",
                          (unsigned)i, (unsigned)g_cap);
            break;
        }
        Serial.printf("[session_pcap] tier %u FAILED (%u bytes)\n",
                      (unsigned)i, (unsigned)CAP_TIERS[i]);
    }

    if (!g_buf) {
        Serial.println("[session_pcap] disabled -- no PSRAM tier available");
        vSemaphoreDelete(g_lock);
        g_lock = nullptr;
        return false;
    }

    xSemaphoreTake(g_lock, portMAX_DELAY);
    reset_ring_locked();
    g_state = State::IDLE;
    xSemaphoreGive(g_lock);
    return true;
}

State       state()      { return g_state; }
const char* state_name() {
    switch (g_state) {
        case State::IDLE:      return "idle";
        case State::RECORDING: return "recording";
        case State::PAUSED:    return "paused";
        case State::STOPPED:   return "stopped";
    }
    return "?";
}

bool cmd_record() {
    if (!g_buf || !g_lock) return false;
    // Legal from IDLE, PAUSED, STOPPED (i.e. anything but already RECORDING).
    if (g_state == State::RECORDING) return false;
    xSemaphoreTake(g_lock, portMAX_DELAY);
    reset_ring_locked();
    g_state = State::RECORDING;
    xSemaphoreGive(g_lock);
    return true;
}

bool cmd_pause() {
    if (!g_buf || !g_lock) return false;
    if (g_state != State::RECORDING) return false;
    g_state = State::PAUSED;
    return true;
}

bool cmd_resume() {
    if (!g_buf || !g_lock) return false;
    if (g_state != State::PAUSED) return false;
    g_state = State::RECORDING;
    return true;
}

bool cmd_stop() {
    if (!g_buf || !g_lock) return false;
    if (g_state != State::RECORDING && g_state != State::PAUSED) return false;
    g_state = State::STOPPED;
    return true;
}

void append(const scan::Frame& f) {
    if (!g_buf || !g_lock) return;
    if (g_state != State::RECORDING) return;

    // Serialize into a local stage buffer, then move it under the lock.
    static uint8_t stage[nordic_pcap::FRAME_OVERHEAD + scan::MAX_PAYLOAD];
    size_t body = nordic_pcap::build_frame(f, stage);
    const size_t rec_len = sizeof(PcapRec) + body;

    if (rec_len > g_cap - GLOBAL_HDR_LEN) return;

    xSemaphoreTake(g_lock, portMAX_DELAY);

    // Re-check state after taking the lock -- a stop() could have raced in.
    if (g_state != State::RECORDING) {
        xSemaphoreGive(g_lock);
        return;
    }

    if (g_used + rec_len > g_cap) {
        // Reclaim roughly half the buffer -- amortizes memmove cost.
        const size_t want_free = g_cap / 2;
        const size_t drop_to = next_boundary_after_locked(want_free);
        if (drop_to > GLOBAL_HDR_LEN && drop_to <= g_used) {
            const size_t moved = g_used - drop_to;
            memmove(g_buf + GLOBAL_HDR_LEN, g_buf + drop_to, moved);
            g_used = GLOBAL_HDR_LEN + moved;
            g_dropped++;
        } else {
            write_global_header_locked();
        }
    }

    PcapRec rec{};
    rec.ts_sec   = f.ts_sec;
    rec.ts_usec  = f.ts_usec;
    rec.incl_len = (uint32_t)body;
    rec.orig_len = (uint32_t)body;
    memcpy(g_buf + g_used, &rec, sizeof(rec));
    memcpy(g_buf + g_used + sizeof(rec), stage, body);
    g_used += rec_len;

    xSemaphoreGive(g_lock);
}

size_t   size()      { return g_used; }
size_t   capacity()  { return g_cap; }
uint32_t dropped()   { return g_dropped; }

size_t read_chunk(size_t offset, uint8_t* out, size_t len) {
    if (!g_buf || !g_lock) return 0;
    // Downloads are only served in STOPPED. If the state has moved on (Record
    // wiped the ring) we return 0 and the chunked response terminates cleanly.
    if (g_state != State::STOPPED) return 0;
    xSemaphoreTake(g_lock, portMAX_DELAY);
    size_t copied = 0;
    if (g_state == State::STOPPED && offset < g_used) {
        const size_t remain = g_used - offset;
        const size_t n = (len < remain) ? len : remain;
        memcpy(out, g_buf + offset, n);
        copied = n;
    }
    xSemaphoreGive(g_lock);
    return copied;
}

void download_begin() { g_downloads++; }
void download_end()   { if (g_downloads) g_downloads--; }
uint32_t downloads_in_flight() { return g_downloads; }

} // namespace session_pcap
