#include "scan.h"
#include "config.h"

#include <NimBLEDevice.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>

namespace scan {

namespace {

struct Ring {
    Frame*        slots;
    size_t        capacity;
    volatile size_t head;
    volatile size_t tail;
    volatile uint32_t dropped;
    portMUX_TYPE  mux;
};

Ring ring_pcap = { nullptr, 0, 0, 0, 0, portMUX_INITIALIZER_UNLOCKED };
Ring ring_dash = { nullptr, 0, 0, 0, 0, portMUX_INITIALIZER_UNLOCKED };

volatile uint32_t g_total          = 0;
volatile uint32_t g_this_sec       = 0;
volatile uint32_t g_per_sec        = 0;
volatile uint32_t g_last_pps_ms    = 0;
volatile uint32_t g_frame_idx      = 0;

bool ring_alloc(Ring& r, size_t slot_count, bool prefer_psram) {
    r.capacity = slot_count;
    r.head = 0; r.tail = 0; r.dropped = 0;
    size_t bytes = slot_count * sizeof(Frame);
    if (prefer_psram && psramFound()) {
        r.slots = (Frame*)ps_malloc(bytes);
        if (r.slots) return true;
    }
    r.slots = (Frame*)malloc(bytes);
    return r.slots != nullptr;
}

inline size_t ring_next(const Ring& r, size_t i) {
    return (i + 1) % r.capacity;
}

void ring_push(Ring& r, const Frame& f) {
    portENTER_CRITICAL_ISR(&r.mux);
    size_t next_head = ring_next(r, r.head);
    if (next_head == r.tail) {
        r.tail = ring_next(r, r.tail);
        r.dropped++;
    }
    r.slots[r.head] = f;
    r.head = next_head;
    portEXIT_CRITICAL_ISR(&r.mux);
}

bool ring_pop(Ring& r, Frame* out) {
    bool got = false;
    portENTER_CRITICAL(&r.mux);
    if (r.tail != r.head) {
        *out = r.slots[r.tail];
        r.tail = ring_next(r, r.tail);
        got = true;
    }
    portEXIT_CRITICAL(&r.mux);
    return got;
}

// NimBLE HCI advType (from advertisement report) → wire LL PDU type.
uint8_t map_hci_advtype_to_ll(uint8_t hci) {
    switch (hci) {
        case 0: return LL_ADV_IND;         // BLE_HCI_ADV_TYPE_ADV_IND
        case 1: return LL_ADV_DIRECT_IND;  // BLE_HCI_ADV_TYPE_ADV_DIRECT_IND_HD
        case 2: return LL_ADV_SCAN_IND;    // BLE_HCI_ADV_TYPE_ADV_SCAN_IND
        case 3: return LL_ADV_NONCONN_IND; // BLE_HCI_ADV_TYPE_ADV_NONCONN_IND
        case 4: return LL_SCAN_RSP;        // BLE_HCI_ADV_TYPE_SCAN_RSP
        default: return LL_ADV_IND;
    }
}

// Random address subtype is encoded in the top two bits of MSB (byte 5, per BLE spec).
// NimBLE surfaces the address in the same byte order the user expects (byte 5 is MSB).
uint8_t classify_random_addr(const uint8_t addr[6]) {
    uint8_t hi2 = (addr[5] >> 6) & 0x03;
    switch (hi2) {
        case 0b00: return ADDR_RANDOM_NRP;
        case 0b01: return ADDR_RANDOM_RPA;
        case 0b11: return ADDR_RANDOM_STATIC;
        default:   return ADDR_RANDOM_STATIC; // 0b10 reserved; treat as static
    }
}

uint8_t ftbit_for_ll(uint8_t ll_pdu_type) {
    switch (ll_pdu_type) {
        case LL_ADV_IND:         return config::FT_ADV_IND;
        case LL_ADV_DIRECT_IND:  return config::FT_ADV_DIRECT;
        case LL_ADV_NONCONN_IND: return config::FT_ADV_NONCONN;
        case LL_SCAN_RSP:        return config::FT_SCAN_RSP;
        case LL_ADV_SCAN_IND:    return config::FT_ADV_SCAN_IND;
        default:                 return 0xFF;
    }
}

class Cb : public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* dev) override {
        if (!dev) return;

        // Filter early — cheap byte checks. Advert-type gate.
        uint8_t ll = map_hci_advtype_to_ll(dev->getAdvType());
        uint8_t ft = config::get().ft_mask;
        uint8_t typebit = ftbit_for_ll(ll);
        if (typebit != 0xFF && (ft & typebit) == 0) return;

        NimBLEAddress a = dev->getAddress();
        uint8_t addr_type_raw = a.getType();

        // NimBLE stores the address with m_address[5] = MSB and m_address[0] = LSB
        // (verified against library's toString(): it prints m_address[5..0]).
        // Match that convention for the Frame — addr[5] is MSB, addr[0] is LSB.
        // The on-wire LE-LL AA is transmitted LSB first, so nordic_pcap writes
        // addr[0..5] in stored order (correctly LSB first).
        const uint8_t* nat = a.getNative();
        uint8_t addr[6];
        memcpy(addr, nat, 6);

        uint8_t addr_type;
        if (addr_type_raw == 0 || addr_type_raw == 2) {
            addr_type = ADDR_PUBLIC;
            if ((ft & config::FT_ADDR_PUBLIC) == 0) return;
        } else if (addr_type_raw == 1 || addr_type_raw == 3) {
            addr_type = classify_random_addr(addr);
            if ((ft & config::FT_ADDR_RANDOM) == 0) return;
        } else {
            addr_type = ADDR_UNKNOWN;
        }

        Frame f;
        f.idx  = ++g_frame_idx;
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        f.ts_sec  = (uint32_t)tv.tv_sec;
        f.ts_usec = (uint32_t)tv.tv_usec;
        f.channel = 0xFF;                     // NimBLE doesn't expose per-advert channel
        f.rssi    = (int8_t)dev->getRSSI();
        f.tx_power = dev->haveTXPower() ? (int8_t)dev->getTXPower() : INT8_MIN;
        f.ll_pdu_type = ll;
        f.addr_type   = addr_type;
        memcpy(f.addr, addr, 6);

        size_t plen = dev->getPayloadLength();
        if (plen > MAX_PAYLOAD) plen = MAX_PAYLOAD;
        f.payload_len = (uint16_t)plen;
        if (plen > 0) memcpy(f.payload, dev->getPayload(), plen);

        ring_push(ring_pcap, f);
        ring_push(ring_dash, f);

        g_total++;
        g_this_sec++;
    }
};

Cb g_cb;

void start_scan() {
    NimBLEScan* s = NimBLEDevice::getScan();
    s->stop();
    s->setActiveScan(false);           // passive — never emit SCAN_REQ
    s->setInterval(config::get().scan_interval_ms);
    s->setWindow(config::get().scan_window_ms);
    s->setDuplicateFilter(false);      // capture every advert, even repeats
    s->setAdvertisedDeviceCallbacks(&g_cb, /*wantDuplicates=*/true);
    s->start(0, nullptr, false);
}

} // namespace

bool init() {
    bool have_psram = psramFound();
    size_t pcap_slots = have_psram ? 256 : 16;
    size_t dash_slots = have_psram ? 64  : 8;
    if (!ring_alloc(ring_pcap, pcap_slots, true)) return false;
    if (!ring_alloc(ring_dash, dash_slots, true)) return false;

    NimBLEDevice::init("");
    // No setPower — passive scan never TXes, so TX-power tuning is moot.
    start_scan();
    return true;
}

void apply_scan_params() {
    start_scan();
}

bool pop_pcap(Frame* out)      { return ring_pop(ring_pcap, out); }
bool pop_dashboard(Frame* out) { return ring_pop(ring_dash, out); }

uint32_t total_adverts() { return g_total; }
uint32_t dropped_pcap()  { return ring_pcap.dropped; }
uint32_t dropped_dash()  { return ring_dash.dropped; }

uint32_t adverts_per_sec() {
    uint32_t now = millis();
    if (now - g_last_pps_ms >= 1000) {
        g_per_sec = g_this_sec;
        g_this_sec = 0;
        g_last_pps_ms = now;
    }
    return g_per_sec;
}

void clear_ring() {
    portENTER_CRITICAL(&ring_pcap.mux);
    ring_pcap.head = ring_pcap.tail = 0; ring_pcap.dropped = 0;
    portEXIT_CRITICAL(&ring_pcap.mux);
    portENTER_CRITICAL(&ring_dash.mux);
    ring_dash.head = ring_dash.tail = 0; ring_dash.dropped = 0;
    portEXIT_CRITICAL(&ring_dash.mux);
}

} // namespace scan
