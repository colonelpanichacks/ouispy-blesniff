#include "pcap_stream.h"
#include "config.h"
#include "nordic_pcap.h"
#include "text_summary.h"

namespace pcap_stream {

namespace {

bool header_emitted = false;

struct __attribute__((packed)) PcapGlobal {
    uint32_t magic;
    uint16_t vmaj;
    uint16_t vmin;
    int32_t  thiszone;
    uint32_t sigfigs;
    uint32_t snaplen;
    uint32_t linktype;
};

struct __attribute__((packed)) PcapRec {
    uint32_t ts_sec;
    uint32_t ts_usec;
    uint32_t incl_len;
    uint32_t orig_len;
};

void write_global_header() {
    PcapGlobal g{};
    g.magic     = nordic_pcap::PCAP_MAGIC;
    g.vmaj      = nordic_pcap::PCAP_VER_MAJOR;
    g.vmin      = nordic_pcap::PCAP_VER_MINOR;
    g.thiszone  = 0;
    g.sigfigs   = 0;
    g.snaplen   = nordic_pcap::PCAP_SNAPLEN;
    g.linktype  = nordic_pcap::PCAP_LINKTYPE;
    Serial.write((const uint8_t*)&g, sizeof(g));
    header_emitted = true;
}

} // namespace

void begin() {
    header_emitted = false;
    ensure_header_for_current_mode();
}

void on_mode_changed() {
    header_emitted = false;
    ensure_header_for_current_mode();
}

void ensure_header_for_current_mode() {
    if (config::get().out_mode == config::OUT_PCAP && !header_emitted) {
        write_global_header();
    }
}

void write_frame_pcap(const scan::Frame& f) {
    // Stage the full record in a single BSS buffer, then one Serial.write().
    // Frame body max: FRAME_OVERHEAD + MAX_PAYLOAD == ~281 bytes. Plus PcapRec.
    static uint8_t stage[sizeof(PcapRec) + nordic_pcap::FRAME_OVERHEAD + scan::MAX_PAYLOAD];

    size_t body = nordic_pcap::build_frame(f, stage + sizeof(PcapRec));
    PcapRec rec{};
    rec.ts_sec   = f.ts_sec;
    rec.ts_usec  = f.ts_usec;
    rec.incl_len = (uint32_t)body;
    rec.orig_len = (uint32_t)body;
    memcpy(stage, &rec, sizeof(rec));
    Serial.write(stage, sizeof(rec) + body);
}

void write_frame_text(const scan::Frame& f) {
    char line[320];
    size_t n = text_summary::format_line(f, line, sizeof(line));
    if (n > 0) Serial.write((const uint8_t*)line, n);
}

} // namespace pcap_stream
