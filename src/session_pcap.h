#pragma once

#include <Arduino.h>
#include "scan.h"

namespace session_pcap {

// Try 6 MB first, then 4 MB, then 2 MB in PSRAM. If all fail the module is
// disabled -- we never fall back to plain malloc() (DRAM), which was the OOM
// crash path in the pre-Tier-1 build.
constexpr size_t GLOBAL_HDR_LEN = 24;

enum class State : uint8_t {
    IDLE      = 0,   // boot state -- append() no-op, ring empty
    RECORDING = 1,   // append() writes into ring
    PAUSED    = 2,   // append() no-op, ring preserved, resumable
    STOPPED   = 3    // append() no-op, ring finalized, download enabled
};

bool     init();

// State machine. Return true on success. On rejected transition returns
// false; caller (web layer) turns that into HTTP 409.
bool     cmd_record();   // any -> RECORDING (clears ring on entry)
bool     cmd_pause();    // RECORDING -> PAUSED
bool     cmd_resume();   // PAUSED -> RECORDING
bool     cmd_stop();     // RECORDING|PAUSED -> STOPPED

State    state();
const char* state_name();       // "idle"|"recording"|"paused"|"stopped"

void     append(const scan::Frame& f);

size_t   size();                // bytes currently held in ring
size_t   capacity();            // actual allocated cap (from CAP_TIERS)
uint32_t dropped();

// Downloads only permitted from STOPPED. read_chunk() takes the session
// mutex per chunk and memcpys straight out of the live ring. The STOPPED
// guarantee is what makes this safe -- if the state ever leaves STOPPED
// mid-download (Record is the only such transition, and it wipes the ring)
// read_chunk() returns 0 for the remainder so the response truncates cleanly.
//
// Design decision: Record wins over an in-flight download rather than being
// refused. The download can be retried; the user's intent to start a fresh
// capture is higher priority than preserving a stale byte stream. The
// download-in-progress counter is tracked so state transitions can observe it
// (currently informational; kept for future policy changes).
size_t   read_chunk(size_t offset, uint8_t* out, size_t len);

void     download_begin();      // increments in-flight downloader count
void     download_end();
uint32_t downloads_in_flight();

} // namespace session_pcap
