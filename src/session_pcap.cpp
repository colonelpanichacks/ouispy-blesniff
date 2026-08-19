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

uint8_t*             g_buf     = nullptr;
size_t               g_cap     = 0;
size_t               g_used    = 0;
uint32_t             g_dropped = 0;
SemaphoreHandle_t    g_lock    = nullptr;

// Immutable snapshot for /api/session.pcap downloads. Live ring can shift
// under us via memmove; snapshot preserves a coherent view for the whole
// response. Overwritten on each snapshot_take() call.
uint8_t*             g_snap      = nullptr;
size_t               g_snap_cap  = 0;
size_t               g_snap_size = 0;

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

} // namespace

bool init() {
    if (g_buf) return true;
    g_lock = xSemaphoreCreateMutex();
    if (!g_lock) return false;

    g_buf = (uint8_t*)heap_caps_malloc(DESIRED_CAP, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (g_buf) {
        g_cap = DESIRED_CAP;
    } else {
        g_buf = (uint8_t*)malloc(FALLBACK_CAP);
        if (!g_buf) { vSemaphoreDelete(g_lock); g_lock = nullptr; return false; }
        g_cap = FALLBACK_CAP;
    }
    xSemaphoreTake(g_lock, portMAX_DELAY);
    write_global_header_locked();
    g_dropped = 0;
    xSemaphoreGive(g_lock);
    return true;
}

void clear() {
    if (!g_buf) return;
    xSemaphoreTake(g_lock, portMAX_DELAY);
    write_global_header_locked();
    g_dropped = 0;
    xSemaphoreGive(g_lock);
}

void append(const scan::Frame& f) {
    if (!g_buf || !g_lock) return;

    // Serialize into a local stage buffer, then move it under the lock.
    static uint8_t stage[nordic_pcap::FRAME_OVERHEAD + scan::MAX_PAYLOAD];
    size_t body = nordic_pcap::build_frame(f, stage);
    const size_t rec_len = sizeof(PcapRec) + body;

    if (rec_len > g_cap - GLOBAL_HDR_LEN) return;

    xSemaphoreTake(g_lock, portMAX_DELAY);

    if (g_used + rec_len > g_cap) {
        // Reclaim roughly half the buffer — amortizes memmove cost.
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
    xSemaphoreTake(g_lock, portMAX_DELAY);
    size_t copied = 0;
    if (offset < g_used) {
        const size_t remain = g_used - offset;
        const size_t n = (len < remain) ? len : remain;
        memcpy(out, g_buf + offset, n);
        copied = n;
    }
    xSemaphoreGive(g_lock);
    return copied;
}

size_t snapshot_take() {
    if (!g_buf || !g_lock) return 0;
    xSemaphoreTake(g_lock, portMAX_DELAY);
    const size_t sz = g_used;
    if (sz == 0) { xSemaphoreGive(g_lock); g_snap_size = 0; return 0; }
    // Reuse existing snap allocation if it fits; otherwise realloc in PSRAM.
    if (!g_snap || g_snap_cap < sz) {
        if (g_snap) heap_caps_free(g_snap);
        g_snap = (uint8_t*) heap_caps_malloc(sz, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (!g_snap) {
            g_snap = (uint8_t*) malloc(sz);  // DRAM fallback for tiny buffers
        }
        if (!g_snap) { g_snap_cap = 0; g_snap_size = 0; xSemaphoreGive(g_lock); return 0; }
        g_snap_cap = sz;
    }
    memcpy(g_snap, g_buf, sz);
    g_snap_size = sz;
    xSemaphoreGive(g_lock);
    return sz;
}

size_t snapshot_size() { return g_snap_size; }

size_t snapshot_read(size_t offset, uint8_t* out, size_t len) {
    if (!g_snap || offset >= g_snap_size) return 0;
    const size_t remain = g_snap_size - offset;
    const size_t n = (len < remain) ? len : remain;
    memcpy(out, g_snap + offset, n);
    return n;
}

} // namespace session_pcap
