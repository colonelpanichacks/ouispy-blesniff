# OUI-SPY BLESNIFF

Passive **BLE advertising** sniffer for the **Seeed Studio XIAO ESP32-S3**.

Sister firmware to [ouispy-pcap](https://github.com/colonelpanichacks/ouispy-pcap), same UX and flow but pointed at the 2.4&nbsp;GHz BLE advertising channels (37 / 38 / 39) instead of Wi-Fi 802.11.

> **Advertisements only.** The ESP32 radio only exposes BLE advertising events through its Bluetooth stack. Connected-device pairing traffic, encryption negotiation, and post-connection PDUs are NOT captured — for that you need an nRF52 sniffer or Ubertooth.
>
> What you *do* get: beacons, iBeacons, Eddystone, scan requests / responses, wearables broadcasting, AirTags, Meta glasses probes, Ring doorbell adverts, drone Remote ID adverts — everything on air on 37/38/39 with RSSI, address type, name, service UUIDs, manufacturer data.

---

## Feature checklist

- NimBLE passive scan across 37/38/39, complete PDU capture with RSSI
- **USB-CDC text summary** — human-readable one-liner per advert (scriptable)
- **On-device dashboard** on `ouispy-blesniff` / `sniffuntothem` at `192.168.4.1` — live advert table, filter chips, session PCAP download (Wireshark-ready `LINKTYPE_BLUETOOTH_LE_LL_WITH_PHDR` / 256)
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

Join the `ouispy-blesniff` / `sniffuntothem` Wi-Fi, open `http://192.168.4.1`, and click **Save PCAP** on the toolbar. The download is `LINKTYPE_BLUETOOTH_LE_LL_WITH_PHDR` (256) so Wireshark keeps channel + RSSI.

The USB-CDC PCAP binary streaming path has been removed — ESP32-S3 Arduino USB CDC is not reliable for high-rate binary streaming, and the dashboard's PSRAM snapshot download parses cleanly regardless of capture rate. See `tools/README.md`.

---

## Serial command protocol

Newline-terminated ASCII, prefix `CMD:`.

| Command | Effect |
|---|---|
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
