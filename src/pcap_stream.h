#pragma once

#include <Arduino.h>
#include "scan.h"

// USB output is text-only (one-line human-readable per advert). PCAP binary
// capture lives on the dashboard exclusively -- GET /api/session.pcap in
// web_dashboard.cpp / session_pcap.h. The dashboard path is bulletproof;
// the USB CDC layer on ESP32-S3 could not be made reliable for high-rate
// binary streaming (residual byte-boundary corruption under load).
namespace pcap_stream {

void write_frame_text(const scan::Frame& f);

} // namespace pcap_stream
