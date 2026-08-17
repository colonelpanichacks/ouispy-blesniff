#pragma once

#include <Arduino.h>
#include "scan.h"

namespace session_pcap {

constexpr size_t DESIRED_CAP    = 2 * 1024 * 1024;
constexpr size_t FALLBACK_CAP   = 64 * 1024;
constexpr size_t GLOBAL_HDR_LEN = 24;

bool     init();
void     append(const scan::Frame& f);
void     clear();

size_t   size();
size_t   capacity();
uint32_t dropped();

// Copy up to `len` bytes starting at `offset` into `out`. Returns bytes copied.
size_t   read_chunk(size_t offset, uint8_t* out, size_t len);

} // namespace session_pcap
