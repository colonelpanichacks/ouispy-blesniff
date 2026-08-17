#pragma once

#include <Arduino.h>

namespace scan {

// Legacy LE advertising PDUs are capped at 37 bytes (6 addr + 31 AdvData).
// Extended advertising can be larger; 256 covers both comfortably.
constexpr uint16_t MAX_PAYLOAD = 256;

// LE LL PDU type numbering (wire-level), used everywhere downstream.
// NimBLE's HCI advType enum is different — mapped in scan.cpp.
constexpr uint8_t LL_ADV_IND         = 0;
constexpr uint8_t LL_ADV_DIRECT_IND  = 1;
constexpr uint8_t LL_ADV_NONCONN_IND = 2;
constexpr uint8_t LL_SCAN_REQ        = 3;
constexpr uint8_t LL_SCAN_RSP        = 4;
constexpr uint8_t LL_CONNECT_IND     = 5;
constexpr uint8_t LL_ADV_SCAN_IND    = 6;

// Address type classification (refined for random subtypes).
constexpr uint8_t ADDR_PUBLIC        = 0;
constexpr uint8_t ADDR_RANDOM_STATIC = 1;
constexpr uint8_t ADDR_RANDOM_NRP    = 2;   // non-resolvable private
constexpr uint8_t ADDR_RANDOM_RPA    = 3;   // resolvable private
constexpr uint8_t ADDR_UNKNOWN       = 0xFF;

struct Frame {
    uint32_t idx;
    uint32_t ts_sec;
    uint32_t ts_usec;
    uint8_t  channel;        // 0xFF if unknown; NimBLE doesn't expose per-advert channel
    int8_t   rssi;
    int8_t   tx_power;       // INT8_MIN if not present in advert
    uint8_t  ll_pdu_type;    // LL_* above
    uint8_t  addr_type;      // ADDR_* above
    uint8_t  addr[6];        // advertising address, big-endian human order
    uint16_t payload_len;    // bytes of AdvData (post-address)
    uint8_t  payload[MAX_PAYLOAD];
};

bool     init();

bool     pop_pcap(Frame* out);
bool     pop_dashboard(Frame* out);

uint32_t total_adverts();
uint32_t dropped_pcap();
uint32_t dropped_dash();
uint32_t adverts_per_sec();

// Restart the underlying NimBLE scan with the current config window/interval.
// Cheap enough to call from the config-apply path.
void     apply_scan_params();

void     clear_ring();

} // namespace scan
