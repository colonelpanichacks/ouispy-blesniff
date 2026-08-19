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

// Take an immutable snapshot of the current buffer so a slow download won't
// desync when the live ring shifts. Snapshot lives in PSRAM (up to
// DESIRED_CAP bytes); it is overwritten on the next snapshot_take() call.
// Returns the snapshot size in bytes, or 0 on failure/empty.
size_t   snapshot_take();
size_t   snapshot_size();
size_t   snapshot_read(size_t offset, uint8_t* out, size_t len);

} // namespace session_pcap
