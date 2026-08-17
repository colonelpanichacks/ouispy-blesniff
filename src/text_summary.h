#pragma once

#include <Arduino.h>
#include "scan.h"

namespace text_summary {

const char* ll_type_name(uint8_t ll_pdu_type);
const char* addr_type_short(uint8_t addr_type);

void        format_addr(const uint8_t addr[6], char* out18);
void        extract_name(const scan::Frame& f, char* out, size_t out_sz);
uint16_t    manufacturer_id(const scan::Frame& f);  // returns 0xFFFF if absent
const char* mfr_shortname(uint16_t id);

// Fill `svc_out` with comma-joined 16-bit service UUID hex values ("0xFFE0,0x180F").
// Returns bytes written (0 if none).
size_t      extract_service_uuids(const scan::Frame& f, char* out, size_t out_sz);

// Traits bitfield surfaced to dashboard (bit-per-trait).
constexpr uint8_t TR_HAS_NAME     = 0x01;
constexpr uint8_t TR_HAS_MFR      = 0x02;
constexpr uint8_t TR_HAS_SVC_DATA = 0x04;
constexpr uint8_t TR_HAS_TXPOWER  = 0x08;
constexpr uint8_t TR_CONNECTABLE  = 0x10;
uint8_t     traits(const scan::Frame& f);

// out: buffer of at least 256 bytes; returns bytes written (not counting NUL)
size_t      format_line(const scan::Frame& f, char* out, size_t out_sz);

} // namespace text_summary
