#include "pcap_stream.h"
#include "text_summary.h"

namespace pcap_stream {

void write_frame_text(const scan::Frame& f) {
    char line[320];
    size_t n = text_summary::format_line(f, line, sizeof(line));
    if (n > 0) Serial.write((const uint8_t*)line, n);
}

} // namespace pcap_stream
