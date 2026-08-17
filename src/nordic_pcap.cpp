#include "nordic_pcap.h"

namespace nordic_pcap {

size_t build_frame(const scan::Frame& f, uint8_t* out) {
    uint8_t* p = out;

    // -------- 10-byte LE-LL-WITH-PHDR pseudo-header --------
    // Byte 0: RF channel index (0..39). 0xFF means unknown; Wireshark tolerates
    // any value in the 0..39 range, so map unknown to 39 (a valid adv channel).
    uint8_t ch = (f.channel <= 39) ? f.channel : 39;
    *p++ = ch;
    *p++ = (uint8_t)f.rssi;   // signal power (int8 dBm)
    *p++ = 0;                 // noise power (unused; flag bit not set anyway)
    *p++ = 0;                 // access-address offenses
    // Bytes 4..7: reference access address (u32 LE) — advertising AA
    *p++ = (uint8_t)(ADV_ACCESS_ADDR      );
    *p++ = (uint8_t)(ADV_ACCESS_ADDR >>  8);
    *p++ = (uint8_t)(ADV_ACCESS_ADDR >> 16);
    *p++ = (uint8_t)(ADV_ACCESS_ADDR >> 24);
    // Bytes 8..9: flags (u16 LE)
    *p++ = (uint8_t)(PHDR_FLAGS      );
    *p++ = (uint8_t)(PHDR_FLAGS >>  8);

    // -------- 4-byte access address (on-air, little-endian) --------
    *p++ = (uint8_t)(ADV_ACCESS_ADDR      );
    *p++ = (uint8_t)(ADV_ACCESS_ADDR >>  8);
    *p++ = (uint8_t)(ADV_ACCESS_ADDR >> 16);
    *p++ = (uint8_t)(ADV_ACCESS_ADDR >> 24);

    // -------- 2-byte LL header --------
    // Byte 0: [ChSel(1) | TxAdd(1) | RxAdd(1) | RFU(1) | PDU type(4)]
    //         bit 0-3 = PDU type
    //         bit 6   = TxAdd (1 if random address)
    //         bit 7   = RxAdd (1 if directed advert)
    uint8_t hdr0 = f.ll_pdu_type & 0x0F;
    bool tx_random = (f.addr_type != scan::ADDR_PUBLIC && f.addr_type != scan::ADDR_UNKNOWN);
    if (tx_random) hdr0 |= 0x40;
    if (f.ll_pdu_type == scan::LL_ADV_DIRECT_IND) hdr0 |= 0x80;
    // Byte 1: length in low 6 bits (address + AdvData)
    uint8_t len_field = (uint8_t)((6 + f.payload_len) & 0x3F);
    *p++ = hdr0;
    *p++ = len_field;

    // -------- 6-byte advertising address (little-endian on wire) --------
    // scan::Frame.addr is stored MSB-last in our conventional order (addr[5] = MSB),
    // matching how BLE spec describes address bytes. On the wire the address is
    // little-endian, so byte 0 of the AA is our addr[0]. Same convention already
    // used in ouispy-pcap for MAC serialization — write bytes in stored order.
    for (int i = 0; i < 6; ++i) *p++ = f.addr[i];

    // -------- AdvData --------
    if (f.payload_len) {
        memcpy(p, f.payload, f.payload_len);
        p += f.payload_len;
    }

    // -------- 3-byte CRC (synthesized zero — we don't have the real one) --------
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;

    return (size_t)(p - out);
}

} // namespace nordic_pcap
