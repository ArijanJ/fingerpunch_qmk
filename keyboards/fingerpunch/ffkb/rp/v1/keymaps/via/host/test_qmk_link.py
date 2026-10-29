import subprocess
import tempfile
import unittest
from pathlib import Path

from qmk_link import ACK, EVENTS, PING, Keyboard, parse_events


class FakeDevice:
    def __init__(self, reports):
        self.reports = list(reports)
        self.written = []

    def write(self, packet):
        self.written.append(packet)
        return len(packet)

    def read(self, size, timeout):
        return self.reports.pop(0) if self.reports else b""


class HostTests(unittest.TestCase):
    def test_request_skips_foreign_reports_and_pads_report_id(self):
        kb = Keyboard(None, 0, 0)
        reply = bytes([ACK | 0x80, 42]) + bytes(30)
        kb.dev = FakeDevice([bytes([1]) + bytes(31), reply])
        self.assertEqual(kb.request(ACK, b"*"), reply)
        self.assertEqual(kb.dev.written, [bytes([0, ACK, 42]) + bytes(30)])

    def test_event_endianness_and_validation(self):
        report = bytes([EVENTS | 0x80, 1, 0x34, 0x12, 0x10, 1, 0xab, 0xcd]) + bytes(24)
        self.assertEqual(parse_events(report), ([(0x10, 1, 0xabcd)], 0x1234))
        for invalid in (report[:5], bytes([EVENTS | 0x80, 8]) + bytes(30)):
            with self.assertRaises(ValueError):
                parse_events(invalid)

    def test_unknown_firmware_and_timeout_fail(self):
        for reports, error in (([bytes([255]) + bytes(31)], ValueError), ([], OSError)):
            kb = Keyboard(None, 0, 0)
            kb.dev = FakeDevice(reports)
            with self.assertRaises(error):
                kb.request(PING)

    def test_bad_handshake_closes_device(self):
        class Device(FakeDevice):
            closed = False

            def close(self):
                self.closed = True

        device = Device([bytes([PING | 0x80, 2, 32, 8]) + bytes(28)])

        class Hid:
            @staticmethod
            def enumerate(vid, pid):
                return [dict(usage_page=0xff60, usage=0x61, path=b"fake")]

            @staticmethod
            def Device(path):
                return device

        kb = Keyboard(Hid, 0, 0)
        with self.assertRaises(ValueError):
            kb.open()
        self.assertTrue(device.closed)
        self.assertIsNone(kb.dev)


class FirmwareTests(unittest.TestCase):
    def test_actual_module_queue_dispatch_wrap_and_stub(self):
        source = Path(__file__).resolve().parents[1]
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "quantum.h").write_text('''
#include <stdint.h>
static const uint32_t layer_state = 8, default_layer_state = 2;
static inline uint8_t get_highest_layer(uint32_t s) { uint8_t n=0; while(s >>= 1) ++n; return n; }
static inline uint8_t get_mods(void) { return 1; }
static inline uint8_t get_weak_mods(void) { return 2; }
static inline uint8_t get_oneshot_mods(void) { return 4; }
''')
            (root / "usb_descriptor.h").write_text("#define RAW_EPSIZE 32\n")
            (root / "via.h").write_text("")
            (root / "raw_hid.h").write_text("void raw_hid_send(uint8_t *data, uint8_t length);\n")
            harness = root / "test.c"
            harness.write_text('''
#include <assert.h>
#include <string.h>
#include "via_host_link.h"
static uint8_t reply[32];
static unsigned sends;
void raw_hid_send(uint8_t *data, uint8_t length);
void raw_hid_send(uint8_t *data, uint8_t length) { assert(length == 32); memcpy(reply, data, 32); ++sends; }
static void request(uint8_t cmd, uint8_t arg) { uint8_t data[32] = {cmd, arg}; assert(via_command_kb(data, 32)); assert(reply[0] == (cmd | 0x80)); }
int main(void) {
    uint8_t foreign[32] = {1};
    assert(!via_command_kb(foreign, 32)); assert(foreign[0] == 1);
    assert(!via_command_kb(foreign, 0)); assert(sends == 0);
    request(HL_CMD_PING, 0); assert(reply[1] == 1 && reply[2] == 32 && reply[3] == 8);
    for (int i=0; i<20; ++i) hl_push_event(HL_EV_MIDI, i);
    assert(sends == 1); /* no USB send during event production */
    request(HL_CMD_GET_EVENTS, 0); assert(reply[1] == 7 && reply[2] == 12);
    for (int i=0; i<7; ++i) { assert(reply[5+4*i] == 12+i); assert(reply[7+4*i] == 13+i); }
    request(HL_CMD_GET_EVENTS, 0); assert(reply[1] == 1 && reply[5] == 19 && reply[7] == 20);
    request(HL_CMD_GET_EVENTS, 0); assert(reply[1] == 0 && reply[2] == 12 && reply[4] == 0);
    request(HL_CMD_SET_HOST_ACK, 42); assert(reply[1] == 42);
    request(HL_CMD_GET_STATE, 0); assert(reply[1] == 3 && reply[2] == 1 && reply[3] == 7 && reply[4] == 42);
    for (unsigned i=0; i<65536; ++i) hl_push_event(HL_EV_MIDI, 1);
    request(HL_CMD_GET_EVENTS, 0); assert(reply[2] == 4 && reply[3] == 0);
    request(HL_CMD_GET_EVENTS, 0); assert(reply[6] == 0 && reply[7] == 20);
    return 0;
}
''')
            base = ["cc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-Wmissing-prototypes",
                    "-I", str(root), "-I", str(source)]
            binary = root / "test"
            subprocess.run(base + ["-DVIA_ENABLE", str(source / "via_host_link.c"), str(harness), "-o", str(binary)], check=True)
            subprocess.run([str(binary)], check=True)
            harness.write_text('#include "via_host_link.h"\nint main(void) { hl_push_event(1, 2); return 0; }\n')
            subprocess.run(base + [str(source / "via_host_link.c"), str(harness), "-o", str(binary)], check=True)
            subprocess.run([str(binary)], check=True)


if __name__ == "__main__":
    unittest.main()
