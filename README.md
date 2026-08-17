# OUI-SPY BLESNIFF

Passive **BLE advertising** sniffer for the **Seeed Studio XIAO ESP32-S3**.

Sister firmware to [ouispy-pcap](https://github.com/colonelpanichacks/ouispy-pcap), same UX and flow but pointed at the 2.4&nbsp;GHz BLE advertising channels (37 / 38 / 39) instead of Wi-Fi 802.11.

> **Advertisements only.** The ESP32 radio only exposes BLE advertising events through its Bluetooth stack. Connected-device pairing traffic, encryption negotiation, and post-connection PDUs are NOT captured — for that you need an nRF52 sniffer or Ubertooth.
>
> What you *do* get: beacons, iBeacons, Eddystone, scan requests / responses, wearables broadcasting, AirTags, Meta glasses probes, Ring doorbell adverts, drone Remote ID adverts — everything on air on 37/38/39 with RSSI, address type, name, service UUIDs, manufacturer data.

---

## Feature checklist

- NimBLE passive scan across 37/38/39, complete PDU capture with RSSI
- **Two output modes on one USB-CDC**, runtime-toggle:
  - **PCAP binary** (default) — Wireshark-ready stream using `LINKTYPE_BLUETOOTH_LE_LL_WITH_PHDR` (256) so channel + RSSI survive
  - **Text summary** — human-readable one-liner per advert
- **On-device dashboard** on `ouispy-blesniff` / `sniffuntothem` at `192.168.4.1` — live advert table, filter chips, session PCAP download
- **Chip filters**: advertising type (ADV_IND / ADV_NONCONN / ADV_SCAN / SCAN_REQ / SCAN_RSP / CONNECT_REQ / EXTENDED), traits (name-present / mfr-data / service-data), vendor identify against the OUI Database (RING, AXON, FLOCK SAFETY, DJI, PARROT, SKYDIO, META/RAY-BAN)
- **Server-side 2 MB PSRAM session buffer** with browser download via `GET /api/session.pcap`
- Configurable scan window / interval from the dashboard, filters persist to NVS

---

## Flash it

Included as **Mode 5** (or whichever slot follows PCAP) in the OUI-SPY Unified Blue **Dev** channel:

**https://colonelpanichacks.github.io/oui-spy-unified-blue/**

Pick **Dev / Experimental**, click **Connect & Flash**. To build the standalone locally:

```bash
pio run -e seeed_xiao_esp32s3 -t upload
pio device monitor -b 115200
```

---

## Wireshark integration

### Option A — extcap plugin (one-click)

```bash
mkdir -p ~/.config/wireshark/extcap
cp tools/ouispy_blesniff_extcap.py ~/.config/wireshark/extcap/
chmod +x ~/.config/wireshark/extcap/ouispy_blesniff_extcap.py
```

Open Wireshark → capture list → **OUI-SPY BLESNIFF** → pick serial port → start.

### Option B — bare pipe

```bash
python3 tools/ouispy_blesniff_pipe.py /dev/tty.usbmodem* | wireshark -k -i -
python3 tools/ouispy_blesniff_pipe.py /dev/tty.usbmodem* | tshark -i - -w capture.pcapng
```

---

## Serial command protocol

Newline-terminated ASCII, prefix `CMD:`.

| Command | Effect |
|---|---|
| `CMD:MODE PCAP` | Switch USB output to PCAP binary |
| `CMD:MODE TEXT` | Switch USB output to text summaries |
| `CMD:WINDOW <ms>` | Set BLE scan window (10-2000) |
| `CMD:INTERVAL <ms>` | Set BLE scan interval (window ≤ interval, 20-4000) |
| `CMD:STATUS` | Print device state as one JSON line |
| `CMD:VERSION` | Firmware version string |

---

## Hardware

**Board:** Seeed Studio XIAO ESP32-S3

| Pin | Function |
|---|---|
| GPIO 3 | Buzzer (PWM, optional beep on hit) |
| GPIO 21 | User LED (inverted logic — LOW = ON) |
| GPIO 0 | BOOT button |

---

## License

MIT
