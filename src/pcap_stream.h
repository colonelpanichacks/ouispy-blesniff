#pragma once

#include <Arduino.h>
#include "scan.h"

namespace pcap_stream {

void begin();

// Called when the output mode toggles between PCAP/TEXT at runtime.
// Forces the pcap global header to be re-emitted on the next PCAP write.
void on_mode_changed();

// Emit the global pcap header once for the current output mode, if needed.
void ensure_header_for_current_mode();

void write_frame_pcap(const scan::Frame& f);
void write_frame_text(const scan::Frame& f);

} // namespace pcap_stream
