#pragma once

#include <Arduino.h>

namespace config {

// Filter mask bits — matched against per-advert traits at capture time.
// Advert type bits use the wire-level LE LL PDU type numbering.
constexpr uint8_t FT_ADV_IND       = 0x01;
constexpr uint8_t FT_ADV_DIRECT    = 0x02;
constexpr uint8_t FT_ADV_NONCONN   = 0x04;
constexpr uint8_t FT_SCAN_RSP      = 0x08;
constexpr uint8_t FT_ADV_SCAN_IND  = 0x10;
// Address-type gate (all-on default): drop random-address adverts when off, etc.
constexpr uint8_t FT_ADDR_PUBLIC   = 0x20;
constexpr uint8_t FT_ADDR_RANDOM   = 0x40;
// Reserved/spare
constexpr uint8_t FT_SPARE_7       = 0x80;

constexpr uint8_t FT_DEFAULT =
    FT_ADV_IND | FT_ADV_DIRECT | FT_ADV_NONCONN | FT_SCAN_RSP | FT_ADV_SCAN_IND |
    FT_ADDR_PUBLIC | FT_ADDR_RANDOM;

// USB output is text-only (line summaries + CMD replies). PCAP binary
// capture lives on the dashboard exclusively -- GET /api/session.pcap.
struct Config {
    uint16_t scan_window_ms;   // 10..scan_interval_ms
    uint16_t scan_interval_ms; // 20..4000
    uint8_t  ft_mask;
    char     ap_ssid[33];
    char     ap_pass[64];
};

void        load();
void        save();
void        reset_defaults();
Config&     get();

void set_scan_window(uint16_t ms);
void set_scan_interval(uint16_t ms);
void set_ftmask(uint8_t m);
void set_ap(const char* ssid, const char* pass);

const char* FW_VERSION();

} // namespace config
