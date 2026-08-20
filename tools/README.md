# Wireshark integration

**PCAP capture is served by the on-device dashboard.** Point your browser at
`http://192.168.4.1` after joining the `ouispy-blesniff` / `sniffuntothem`
Wi-Fi, click **Save PCAP** on the toolbar, and open the downloaded file in
Wireshark.

The USB-CDC PCAP streaming path (previously `ouispy_blesniff_pipe.py` +
`ouispy_blesniff_extcap.py`) has been removed: ESP32-S3 Arduino USB CDC is
not reliable for high-rate binary streaming, and the dashboard path uses an
immutable PSRAM snapshot which parses cleanly regardless of capture rate.

USB CDC still emits human-readable text summaries (one line per advert) and
responds to `CMD:STATUS` / `CMD:VERSION` for scripting.
