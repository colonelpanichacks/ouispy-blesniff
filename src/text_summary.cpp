#include "text_summary.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>

namespace text_summary {

// AD types (Assigned Numbers / Generic Access Profile)
namespace ad {
constexpr uint8_t FLAGS                    = 0x01;
constexpr uint8_t INCOMPLETE_16BIT_UUIDS   = 0x02;
constexpr uint8_t COMPLETE_16BIT_UUIDS     = 0x03;
constexpr uint8_t INCOMPLETE_32BIT_UUIDS   = 0x04;
constexpr uint8_t COMPLETE_32BIT_UUIDS     = 0x05;
constexpr uint8_t INCOMPLETE_128BIT_UUIDS  = 0x06;
constexpr uint8_t COMPLETE_128BIT_UUIDS    = 0x07;
constexpr uint8_t SHORTENED_LOCAL_NAME     = 0x08;
constexpr uint8_t COMPLETE_LOCAL_NAME      = 0x09;
constexpr uint8_t TX_POWER_LEVEL           = 0x0A;
constexpr uint8_t SERVICE_DATA_16          = 0x16;
constexpr uint8_t SERVICE_DATA_32          = 0x20;
constexpr uint8_t SERVICE_DATA_128         = 0x21;
constexpr uint8_t MANUFACTURER_SPECIFIC    = 0xFF;
} // namespace ad

const char* ll_type_name(uint8_t t) {
    switch (t) {
        case scan::LL_ADV_IND:         return "ADV_IND";
        case scan::LL_ADV_DIRECT_IND:  return "ADV_DIRECT";
        case scan::LL_ADV_NONCONN_IND: return "ADV_NONCONN";
        case scan::LL_SCAN_REQ:        return "SCAN_REQ";
        case scan::LL_SCAN_RSP:        return "SCAN_RSP";
        case scan::LL_CONNECT_IND:     return "CONNECT_REQ";
        case scan::LL_ADV_SCAN_IND:    return "ADV_SCAN";
        default:                       return "ADV_?";
    }
}

const char* addr_type_short(uint8_t a) {
    switch (a) {
        case scan::ADDR_PUBLIC:        return "pub";
        case scan::ADDR_RANDOM_STATIC: return "rnd-s";
        case scan::ADDR_RANDOM_NRP:    return "rnd-nrp";
        case scan::ADDR_RANDOM_RPA:    return "rnd-rpa";
        default:                       return "unk";
    }
}

void format_addr(const uint8_t addr[6], char* out18) {
    // Address printed MSB-first (addr[5] is MSB, per BLE spec convention).
    snprintf(out18, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
             addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
}

// AD-structure iterator: for each (type, len, data), invoke `visit(type, data, dlen)`.
// dlen is the AD data length (excludes the type byte).
template <typename V>
void for_each_ad(const scan::Frame& f, V visit) {
    size_t i = 0;
    while (i < f.payload_len) {
        uint8_t ad_len = f.payload[i];
        if (ad_len == 0) { i++; continue; }
        if (i + 1 + ad_len > f.payload_len) return;
        uint8_t ad_type = f.payload[i + 1];
        const uint8_t* d = f.payload + i + 2;
        uint8_t dlen = ad_len - 1;
        if (!visit(ad_type, d, dlen)) return;
        i += 1 + ad_len;
    }
}

void extract_name(const scan::Frame& f, char* out, size_t out_sz) {
    out[0] = 0;
    for_each_ad(f, [&](uint8_t type, const uint8_t* d, uint8_t dlen) -> bool {
        if (type == ad::COMPLETE_LOCAL_NAME || type == ad::SHORTENED_LOCAL_NAME) {
            size_t n = dlen;
            if (n >= out_sz) n = out_sz - 1;
            for (size_t k = 0; k < n; ++k) {
                uint8_t c = d[k];
                out[k] = (c >= 0x20 && c < 0x7F) ? (char)c : '.';
            }
            out[n] = 0;
            return false;
        }
        return true;
    });
}

uint16_t manufacturer_id(const scan::Frame& f) {
    uint16_t id = 0xFFFF;
    for_each_ad(f, [&](uint8_t type, const uint8_t* d, uint8_t dlen) -> bool {
        if (type == ad::MANUFACTURER_SPECIFIC && dlen >= 2) {
            id = (uint16_t)d[0] | ((uint16_t)d[1] << 8);
            return false;
        }
        return true;
    });
    return id;
}

const char* mfr_shortname(uint16_t id) {
    switch (id) {
        // Big consumer
        case 0x004C: return "Apple";
        case 0x0006: return "Microsoft";
        case 0x00E0: return "Google";
        case 0x0075: return "Samsung";
        case 0x0171: return "Amazon";
        case 0x038F: return "Xiaomi";
        case 0x0087: return "Garmin";
        case 0x00D2: return "Sonos";
        case 0x008A: return "Bose";
        case 0x2C00: return "GoPro";
        // Silicon / dev
        case 0x0059: return "Nordic";
        case 0x0131: return "Cypress";
        case 0x02E5: return "Espressif";
        case 0x000F: return "Broadcom";
        case 0x0002: return "Nokia";
        case 0x0157: return "Anhui Huami";
        case 0x0499: return "Ruuvi";
        // Surveillance / drones / smartglasses (from Detector OUI Database)
        case 0x034D: return "Axon/TASER";
        case 0x0D53: return "Luxottica (Meta/Ray-Ban)";
        case 0x0BF3: return "DJI";
        case 0x004D: return "Parrot";
        default:     return "?";
    }
}

size_t extract_service_uuids(const scan::Frame& f, char* out, size_t out_sz) {
    size_t written = 0;
    bool   first   = true;
    auto emit = [&](uint16_t u) {
        char buf[10];
        int n = snprintf(buf, sizeof(buf), first ? "0x%04X" : ",0x%04X", u);
        if (n > 0 && written + (size_t)n < out_sz) {
            memcpy(out + written, buf, n);
            written += n;
            first = false;
        }
    };
    for_each_ad(f, [&](uint8_t type, const uint8_t* d, uint8_t dlen) -> bool {
        if (type == ad::INCOMPLETE_16BIT_UUIDS || type == ad::COMPLETE_16BIT_UUIDS) {
            for (size_t k = 0; k + 1 < dlen; k += 2) {
                uint16_t u = (uint16_t)d[k] | ((uint16_t)d[k+1] << 8);
                emit(u);
            }
        }
        return true;
    });
    if (written < out_sz) out[written] = 0;
    return written;
}

uint8_t traits(const scan::Frame& f) {
    uint8_t t = 0;
    if (f.tx_power != INT8_MIN) t |= TR_HAS_TXPOWER;
    if (f.ll_pdu_type == scan::LL_ADV_IND || f.ll_pdu_type == scan::LL_ADV_DIRECT_IND) {
        t |= TR_CONNECTABLE;
    }
    for_each_ad(f, [&](uint8_t type, const uint8_t*, uint8_t) -> bool {
        if (type == ad::COMPLETE_LOCAL_NAME || type == ad::SHORTENED_LOCAL_NAME) t |= TR_HAS_NAME;
        if (type == ad::MANUFACTURER_SPECIFIC)                                    t |= TR_HAS_MFR;
        if (type == ad::SERVICE_DATA_16 || type == ad::SERVICE_DATA_32 ||
            type == ad::SERVICE_DATA_128)                                         t |= TR_HAS_SVC_DATA;
        return true;
    });
    return t;
}

size_t format_line(const scan::Frame& f, char* out, size_t out_sz) {
    char addr[18]; format_addr(f.addr, addr);
    char name[32] = {0};
    extract_name(f, name, sizeof(name));
    char svc[80]; svc[0] = 0;
    extract_service_uuids(f, svc, sizeof(svc));
    uint16_t mfr = manufacturer_id(f);

    // Channel: firmware doesn't get per-advert channel; print 0xFF as "?"
    char chbuf[4];
    if (f.channel <= 39) snprintf(chbuf, sizeof(chbuf), "%u", f.channel);
    else                 strlcpy(chbuf, "?", sizeof(chbuf));

    size_t off = snprintf(out, out_sz, "[Ch%s RSSI%ddBm] %s %s:%s",
                          chbuf, (int)f.rssi,
                          ll_type_name(f.ll_pdu_type),
                          addr_type_short(f.addr_type),
                          addr);
    if (off >= out_sz) return out_sz - 1;

    if (name[0]) {
        int n = snprintf(out + off, out_sz - off, " name=\"%s\"", name);
        if (n > 0) off += (size_t)n;
    }
    if (svc[0] && off < out_sz) {
        int n = snprintf(out + off, out_sz - off, " svc=%s", svc);
        if (n > 0) off += (size_t)n;
    }
    if (mfr != 0xFFFF && off < out_sz) {
        int n = snprintf(out + off, out_sz - off, " mfr=%04X(%s)", mfr, mfr_shortname(mfr));
        if (n > 0) off += (size_t)n;
    }
    if (off < out_sz) {
        out[off++] = '\n';
        if (off < out_sz) out[off] = 0;
    }
    return off;
}

} // namespace text_summary
