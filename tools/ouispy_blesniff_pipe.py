#!/usr/bin/env python3
"""
ouispy_blesniff_pipe.py — read PCAP-over-USB-CDC from an OUI-SPY BLESNIFF
board and write raw pcap bytes to stdout.

    python3 ouispy_blesniff_pipe.py /dev/tty.usbmodem* | wireshark -k -i -
    python3 ouispy_blesniff_pipe.py /dev/tty.usbmodem* | tshark -i - -w capture.pcapng

Emits LINKTYPE_BLUETOOTH_LE_LL_WITH_PHDR (256). If no port is given,
auto-picks a single matching serial port; errors if zero or many match.
Sends CMD:MODE PCAP\\n on connect so the device is in the right output
mode even if it was left in TEXT.
"""

import argparse
import glob
import os
import signal
import sys
import time

BAUD = 115200
PCAP_MAGIC = b"\xd4\xc3\xb2\xa1"


def pick_port(pattern):
    if pattern and os.path.exists(pattern) and not any(c in pattern for c in "*?["):
        return pattern
    candidates = sorted(glob.glob(pattern)) if pattern else (
        sorted(glob.glob("/dev/tty.usbmodem*"))
        + sorted(glob.glob("/dev/tty.usbserial*"))
        + sorted(glob.glob("/dev/ttyUSB*"))
        + sorted(glob.glob("/dev/ttyACM*"))
    )
    if not candidates:
        sys.exit(f"ouispy_blesniff_pipe: no serial port matched {pattern or 'auto'}")
    if len(candidates) > 1:
        sys.exit("ouispy_blesniff_pipe: multiple ports matched:\n  " + "\n  ".join(candidates)
                 + "\nSpecify one explicitly.")
    return candidates[0]


def open_port(port):
    try:
        import serial
    except ImportError:
        sys.exit("ouispy_blesniff_pipe: pyserial not installed. `pip install pyserial`")
    ser = serial.Serial(port, BAUD, timeout=0.2, write_timeout=1.0)
    ser.reset_input_buffer()
    try:
        ser.write(b"CMD:MODE PCAP\n")
        ser.flush()
    except Exception:
        pass
    return ser


def find_magic(ser, deadline):
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
    sys.exit("ouispy_blesniff_pipe: timed out waiting for pcap magic. "
             "Check the cable, the board is powered, and firmware is running.")


def main():
    ap = argparse.ArgumentParser(description="OUI-SPY BLESNIFF serial pipe")
    ap.add_argument("port", nargs="?",
                    help="Serial device (default: autodetect single match)")
    ap.add_argument("--sync-timeout", type=float, default=5.0,
                    help="Seconds to wait for pcap magic before giving up")
    args = ap.parse_args()

    port = pick_port(args.port)
    print(f"ouispy_blesniff_pipe: {port}", file=sys.stderr)

    ser = open_port(port)
    signal.signal(signal.SIGPIPE, signal.SIG_DFL)

    prefix = find_magic(ser, time.time() + args.sync_timeout)
    out = sys.stdout.buffer
    try:
        out.write(prefix)
        out.flush()
        while True:
            data = ser.read(4096)
            if not data:
                continue
            out.write(data)
            out.flush()
    except (BrokenPipeError, KeyboardInterrupt):
        return 0
    except Exception as e:
        print(f"ouispy_blesniff_pipe: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
