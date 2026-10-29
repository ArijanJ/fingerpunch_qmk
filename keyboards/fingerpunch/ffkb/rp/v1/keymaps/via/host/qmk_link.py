#!/usr/bin/env python3
"""Best-effort notification bridge for the FFKB private VIA command channel."""

import argparse
import json
from pathlib import Path
import subprocess
import sys
import time

REPORT_LEN = 32
PING, EVENTS, STATE, ACK = 0x20, 0x21, 0x22, 0x23
EV_MIDI_ROOT, EV_MIDI_OCTAVE = 0x15, 0x16
EV_MIDI_CHORD, EV_MIDI_SCALE = 0x17, 0x18
NOTE_NAMES = ("C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B")


def parse_events(report):
    if len(report) != REPORT_LEN or report[0] != EVENTS | 0x80 or report[1] > 7:
        raise ValueError("invalid event report")
    events = []
    for offset in range(4, 4 + report[1] * 4, 4):
        events.append((report[offset], report[offset + 1],
                       int.from_bytes(report[offset + 2:offset + 4], "big")))
    return events, int.from_bytes(report[2:4], "little")


class Keyboard:
    def __init__(self, hid, vid, pid, serial=None):
        self.hid, self.vid, self.pid, self.serial = hid, vid, pid, serial
        self.dev = None

    def open(self):
        devices = [d for d in self.hid.enumerate(self.vid, self.pid)
                   if d.get("usage_page") == 0xFF60 and d.get("usage") == 0x61
                   and (self.serial is None or d.get("serial_number") == self.serial)]
        if not devices:
            raise OSError("raw HID interface 0xFF60/0x61 not found")
        if len(devices) != 1:
            raise ValueError("multiple keyboards found; select one with --serial")
        self.dev = self.hid.Device(path=devices[0]["path"])
        try:
            report = self.request(PING)
            if tuple(report[1:4]) != (1, REPORT_LEN, 8):
                raise ValueError(f"unsupported host-link handshake: {list(report[1:4])}")
        except Exception:
            self.close()
            raise

    def close(self):
        if self.dev is not None:
            self.dev.close()
            self.dev = None

    def request(self, cmd, payload=b"", timeout_ms=300):
        if self.dev is None:
            raise OSError("device is not open")
        if len(payload) > REPORT_LEN - 1:
            raise ValueError("payload too large")
        packet = bytes([0, cmd]) + payload + bytes(REPORT_LEN - 1 - len(payload))
        if self.dev.write(packet) != len(packet):
            raise OSError("short HID write")
        deadline = time.monotonic() + timeout_ms / 1000
        while time.monotonic() < deadline:
            remaining = max(1, int((deadline - time.monotonic()) * 1000))
            report = bytes(self.dev.read(REPORT_LEN, timeout=remaining))
            if not report:
                break
            if len(report) != REPORT_LEN:
                raise ValueError("short HID report")
            if report[0] == 0xFF:
                raise ValueError("firmware rejected a command; check flashed firmware")
            if report[0] == cmd | 0x80:
                return report
        # Do not retry a dequeuing request on this handle: a late reply could
        # otherwise be confused with the next request for the same command.
        raise OSError(f"timeout waiting for command 0x{cmd:02x}")


def event_message(messages, code, arg):
    message = messages.get(f"{code:02x}:{arg:02x}")
    if code in (EV_MIDI_ROOT, EV_MIDI_OCTAVE) and 0 <= arg <= 127:
        if message is None:
            message = messages.get(f"{code:02x}:*", "MIDI root: {note} · octave {octave}")
        return (message.replace("{note}", NOTE_NAMES[arg % 12])
                .replace("{octave}", str(arg // 12 - 1))
                .replace("{number}", str(arg)))
    return message


def notify(message):
    if message:
        subprocess.run(["notify-send", "--app-name=QMK", "--expire-time=2000",
                        "Keyboard", message], check=False, timeout=3)


def listen(kb, messages, notifications):
    while True:
        try:
            if kb.dev is None:
                kb.open()
            last_seq, last_dropped = None, 0
            while True:
                events, dropped = parse_events(kb.request(EVENTS))
                lost = (dropped - last_dropped) & 0xFFFF
                if lost:
                    print(f"[lost {lost} event(s) to queue overflow]", flush=True)
                last_dropped = dropped
                for code, arg, seq in events:
                    if last_seq is not None and seq != ((last_seq + 1) & 0xFFFF):
                        print(f"[sequence gap: {last_seq} -> {seq}]", flush=True)
                    last_seq = seq
                    message = event_message(messages, code, arg)
                    detail = f" — {message}" if message else ""
                    print(f"event code=0x{code:02x} arg=0x{arg:02x} seq={seq}{detail}", flush=True)
                    if notifications:
                        try:
                            notify(message)
                        except (OSError, subprocess.TimeoutExpired) as exc:
                            print(f"notification failed: {exc}", file=sys.stderr)
                if not events:
                    time.sleep(0.05)
        except (OSError, kb.hid.HIDException) as exc:
            print(f"link unavailable: {exc}; retrying in 1s", file=sys.stderr, flush=True)
            kb.close()
            time.sleep(1)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--vid", type=lambda s: int(s, 0), default=0xFE2B)
    parser.add_argument("--pid", type=lambda s: int(s, 0), default=0xFFBB)
    parser.add_argument("--serial")
    parser.add_argument("--ping", action="store_true")
    parser.add_argument("--state", action="store_true")
    parser.add_argument("--ack", type=lambda s: int(s, 0))
    parser.add_argument("--listen", action="store_true")
    parser.add_argument("--notify", action="store_true", help="notify while listening")
    parser.add_argument("--config", type=Path, default=Path(__file__).with_name("notifications.json"))
    args = parser.parse_args()
    if args.ack is not None and not 0 <= args.ack <= 255:
        parser.error("--ack must be between 0 and 255")
    if not (args.ping or args.state or args.ack is not None or args.listen or args.notify):
        parser.error("choose --ping, --state, --ack, --listen, or --notify")
    try:
        import hid
    except ImportError:
        parser.exit(1, "Install host/requirements.txt in a virtual environment first.\n")
    kb = Keyboard(hid, args.vid, args.pid, args.serial)
    try:
        messages = json.loads(args.config.read_text()) if args.listen or args.notify else {}
        if not isinstance(messages, dict) or any(not isinstance(v, str) for v in messages.values()):
            raise ValueError("notification config must map event keys to strings")
        if args.ping or args.state or args.ack is not None:
            kb.open()
        if args.ping:
            r = kb.request(PING)
            print(f"proto={r[1]} report_len={r[2]} queue={r[3]}")
        if args.state:
            r = kb.request(STATE)
            print(f"layer={r[1]} default={r[2]} mods=0x{r[3]:02x} ack={r[4]}")
        if args.ack is not None:
            print(f"ack={kb.request(ACK, bytes([args.ack]))[1]}")
        if args.listen or args.notify:
            listen(kb, messages, args.notify)
    except KeyboardInterrupt:
        pass
    except (OSError, ValueError, hid.HIDException) as exc:
        parser.exit(1, f"{exc}\n")
    finally:
        kb.close()


if __name__ == "__main__":
    main()
