#!/usr/bin/env python3
"""
ouispy_blesniff_extcap.py — Wireshark extcap plugin for the OUI-SPY
BLESNIFF board. Drop into your extcap directory and OUI-SPY BLESNIFF
appears in the Wireshark interface list.

    macOS:   ~/.config/wireshark/extcap/
    Linux:   ~/.local/lib/wireshark/extcap/  or  ~/.config/wireshark/extcap/
    Windows: %APPDATA%\\Wireshark\\extcap\\

Emits LINKTYPE_BLUETOOTH_LE_LL_WITH_PHDR (256) — Wireshark dissects it
natively.
"""

import argparse
import glob
import signal
import sys
import time

VERSION = "0.1.0"
DISPLAY = "OUI-SPY BLESNIFF"
IFACE = "ouispy-blesniff"
DLT = 256           # LINKTYPE_BLUETOOTH_LE_LL_WITH_PHDR
BAUD = 115200
PCAP_MAGIC = b"\xd4\xc3\xb2\xa1"


def list_serial_ports():
    patterns = ("/dev/tty.usbmodem*", "/dev/tty.usbserial*",
                "/dev/ttyUSB*", "/dev/ttyACM*")
    out = []
    for p in patterns:
        out.extend(sorted(glob.glob(p)))
    if sys.platform.startswith("win"):
        for i in range(1, 32):
            out.append(f"COM{i}")
    return out


def cmd_interfaces():
    print(f"extcap {{version={VERSION}}}{{help=https://github.com/colonelpanichacks/ouispy-blesniff}}")
    print(f"interface {{value={IFACE}}}{{display={DISPLAY}}}")


def cmd_dlts():
    print(f"dlt {{number={DLT}}}{{name=BLUETOOTH_LE_LL_WITH_PHDR}}{{display=Bluetooth Low Energy Link Layer with PHDR}}")


def cmd_config():
    ports = list_serial_ports()
    print("arg {number=0}{call=--serial-port}{display=Serial port}"
          "{type=selector}{tooltip=USB-CDC serial device for the OUI-SPY BLESNIFF board}"
          "{required=true}")
    if ports:
        for i, p in enumerate(ports):
            print(f"value {{arg=0}}{{value={p}}}{{display={p}}}{{default={'true' if i == 0 else 'false'}}}")
    else:
        print("value {arg=0}{value=}{display=(no serial ports detected)}{default=true}")

    print("arg {number=1}{call=--force-mode}{display=Force PCAP mode on connect}"
          "{type=boolean}{default=true}"
          "{tooltip=Send CMD:MODE PCAP so the device switches from TEXT if needed}")

    print("arg {number=2}{call=--sync-timeout}{display=Sync timeout (s)}"
          "{type=integer}{default=5}{range=1,60}"
          "{tooltip=How long to wait for the pcap magic on the serial stream}")


def open_port(port, force_mode):
    try:
        import serial
    except ImportError:
        sys.exit("ouispy_blesniff_extcap: pyserial not installed. `pip install pyserial`")
    ser = serial.Serial(port, BAUD, timeout=0.2, write_timeout=1.0)
    ser.reset_input_buffer()
    if force_mode:
        try:
            ser.write(b"CMD:MODE PCAP\n")
            ser.flush()
        except Exception:
            pass
    return ser


def sync_to_magic(ser, deadline):
    window = b""
    while time.time() < deadline:
        chunk = ser.read(4096)
        if not chunk:
            continue
        window += chunk
        idx = window.find(PCAP_MAGIC)
        if idx >= 0:
            return window[idx:]
        if len(window) > 65536:
            window = window[-4096:]
    raise RuntimeError("timed out waiting for pcap magic (is the board plugged in and running?)")


def cmd_capture(fifo, port, force_mode, sync_timeout):
    if not port:
        sys.exit("ouispy_blesniff_extcap: no serial port selected")
    ser = open_port(port, force_mode)
    signal.signal(signal.SIGPIPE, signal.SIG_DFL)
    with open(fifo, "wb") as out:
        prefix = sync_to_magic(ser, time.time() + sync_timeout)
        out.write(prefix)
        out.flush()
        try:
            while True:
                data = ser.read(4096)
                if data:
                    out.write(data)
                    out.flush()
        except (BrokenPipeError, KeyboardInterrupt):
            return


def main():
    ap = argparse.ArgumentParser(add_help=False)
    ap.add_argument("--extcap-interfaces", action="store_true")
    ap.add_argument("--extcap-dlts", action="store_true")
    ap.add_argument("--extcap-config", action="store_true")
    ap.add_argument("--capture", action="store_true")
    ap.add_argument("--extcap-version", nargs="?", const="")
    ap.add_argument("--extcap-interface", default="")
    ap.add_argument("--fifo", default="")
    ap.add_argument("--serial-port", default="")
    ap.add_argument("--force-mode", default="true")
    ap.add_argument("--sync-timeout", type=float, default=5.0)
    args, _ = ap.parse_known_args()

    if args.extcap_version is not None:
        print(f"extcap {{version={VERSION}}}")
        return 0
    if args.extcap_interfaces:
        cmd_interfaces()
        return 0
    if args.extcap_dlts:
        cmd_dlts()
        return 0
    if args.extcap_config:
        cmd_config()
        return 0
    if args.capture:
        cmd_capture(
            args.fifo,
            args.serial_port,
            args.force_mode.lower() in ("true", "1", "yes"),
            args.sync_timeout,
        )
        return 0

    print(__doc__.strip(), file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
