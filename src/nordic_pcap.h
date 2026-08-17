#pragma once

#include <Arduino.h>
#include "scan.h"

// Builds LINKTYPE_BLUETOOTH_LE_LL_WITH_PHDR (256) payloads for both the
// USB PCAP stream and the in-memory session PCAP buffer.
namespace nordic_pcap {

// Sizes:
//  10-byte LE-LL-WITH-PHDR pseudo-header
// + 4-byte access address
// + 2-byte LL header (PDU type / flags / length)
// + 6-byte advertising address
// + N-byte AdvData
// + 3-byte CRC (synthesized zero)
constexpr size_t PHDR_LEN         = 10;
constexpr size_t ACCESS_ADDR_LEN  = 4;
constexpr size_t LL_HDR_LEN       = 2;
constexpr size_t ADV_ADDR_LEN     = 6;
constexpr size_t CRC_LEN          = 3;

// Fixed overhead around the AdvData payload.
constexpr size_t FRAME_OVERHEAD =
    PHDR_LEN + ACCESS_ADDR_LEN + LL_HDR_LEN + ADV_ADDR_LEN + CRC_LEN;

constexpr uint32_t PCAP_MAGIC        = 0xA1B2C3D4;
constexpr uint16_t PCAP_VER_MAJOR    = 2;
constexpr uint16_t PCAP_VER_MINOR    = 4;
constexpr uint32_t PCAP_LINKTYPE     = 256;  // LINKTYPE_BLUETOOTH_LE_LL_WITH_PHDR
constexpr uint32_t PCAP_SNAPLEN      = 512;

// Advertising channel access address (little-endian on wire: D6 BE 89 8E).
constexpr uint32_t ADV_ACCESS_ADDR   = 0x8E89BED6;

// Pseudo-header flags: dewhitened | signal-power-valid | ref-AA-valid |
//                      CRC-checked | CRC-valid = 0x0613
// (CRC-valid is fine to advertise; Wireshark won't try to recompute against our zeros)
constexpr uint16_t PHDR_FLAGS        = 0x0613;

// Serialize one scan::Frame into a LE_LL_WITH_PHDR buffer.
// `out` must have room for FRAME_OVERHEAD + f.payload_len bytes.
// Returns bytes written.
size_t build_frame(const scan::Frame& f, uint8_t* out);

// Convenience: the total serialized size for a Frame.
inline size_t frame_size(const scan::Frame& f) {
    return FRAME_OVERHEAD + f.payload_len;
}

} // namespace nordic_pcap
