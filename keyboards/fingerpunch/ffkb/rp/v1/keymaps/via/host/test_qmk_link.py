import subprocess
import tempfile
import unittest
from pathlib import Path

from qmk_link import ACK, EVENTS, PING, Keyboard, event_message, parse_events


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
    def test_chord_mode_notifications(self):
        import json
        messages = json.loads(Path(__file__).with_name("notifications.json").read_text())
        self.assertIn("triads", event_message(messages, 0x17, 0))
        self.assertIn("diatonic sevenths", event_message(messages, 0x17, 1))
        self.assertIn("minor sevenths", event_message(messages, 0x17, 2))
        self.assertIn("major", event_message(messages, 0x18, 0))
        self.assertIn("natural minor", event_message(messages, 0x18, 1))

    def test_midi_names_octaves_and_templates(self):
        messages = {"15:*": "root {note}, octave {octave}, MIDI {number}",
                    "16:*": "octave {octave}, root {note}"}
        self.assertEqual(event_message(messages, 0x15, 60), "root C, octave 4, MIDI 60")
        self.assertEqual(event_message(messages, 0x15, 61), "root C#, octave 4, MIDI 61")
        self.assertEqual(event_message(messages, 0x15, 59), "root B, octave 3, MIDI 59")
        self.assertEqual(event_message(messages, 0x16, 59), "octave 3, root B")
        self.assertEqual(event_message(messages, 0x16, 0), "octave -1, root C")
        self.assertEqual(event_message(messages, 0x16, 127), "octave 9, root G")
        self.assertIsNone(event_message(messages, 0x16, 128))
        self.assertEqual(event_message({"15:*": ""}, 0x15, 60), "")
        self.assertEqual(event_message({"15:3c": "custom"}, 0x15, 60), "custom")

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
    def test_midi_layer_thumb_assignments(self):
        import re
        keymap = (Path(__file__).resolve().parents[1] / "keymap.c").read_text()
        layout = re.search(r"\[LAYER_MIDI\]\s*=\s*LAYOUT_ffkb\((.*?)\n\s*\)", keymap, re.S)
        self.assertIsNotNone(layout)
        keys = [key.strip() for key in layout.group(1).split(",") if key.strip()]
        self.assertEqual(len(keys), 44)
        self.assertEqual(keys[36:40], ["XXXXXXX", "MIDI_CHORD_NEXT", "MIDI_SUSTAIN", "MIDI_SCALE_TOGGLE"])
        self.assertEqual(keys[41], "MIDI_SUSTAIN_RIGHT")
        self.assertEqual(keys[43], "XXXXXXX")
        self.assertEqual(keys[24], "MIDI_MODE")

    def test_pitch_ranges_and_held_notes_use_original_pitch(self):
        source = Path(__file__).resolve().parents[1]
        keymap = (source / "keymap.c").read_text()
        # Compile the actual MIDI state/toggle and note-processing branches.
        state = "// Channel 1;" + keymap.split("// Channel 1;", 1)[1].split("const uint16_t PROGMEM keymaps", 1)[0]
        handler = keymap.split("bool process_record_user(uint16_t keycode, keyrecord_t *record) {", 1)[1].split("    if (keycode >= GC_LSU)", 1)[0]
        pointing = keymap.split("report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {", 1)[1].split("    if (is_joystick_mode)", 1)[0]
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            harness = root / "test.c"
            harness.write_text('''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include "midi_pitch.h"
#include "via_host_link.h"
#define LAYER_MIDI 13
#define MIDI_MODE 99
#define MIDI_NOTE_0 100
#define MIDI_NOTE_9 109
#define MIDI_BLACK_0 110
#define MIDI_BLACK_1 111
#define MIDI_BLACK_2 112
#define MIDI_BLACK_9 119
#define MIDI_SUSTAIN 120
#define MIDI_CHORD_0 121
#define MIDI_CHORD_9 130
#define MIDI_CHORD_NEXT 131
#define MIDI_SCALE_TOGGLE 132
#define MIDI_SUSTAIN_RIGHT 133
typedef struct { struct { bool pressed; } event; } keyrecord_t;
static int midi_device;
static uint8_t last_on, last_off, last_velocity, last_cc, last_cc_value;
static unsigned off_count, cc_count, on_count;
static uint8_t last_event_code, last_event_arg;
typedef struct { int16_t x, y, h, v; uint8_t buttons; } report_mouse_t;
static void midi_send_noteoff(int *d, int c, uint8_t n, int v) { (void)d; (void)c; (void)v; last_off=n; ++off_count; }
static void midi_send_noteon(int *d, int c, uint8_t n, int v) { (void)d; (void)c; last_velocity=v; last_on=n; ++on_count; }
static void midi_send_cc(int *d, int c, int cc, int v) { (void)d; (void)c; last_cc=cc; last_cc_value=v; ++cc_count; }
static void layer_on(int layer) { (void)layer; }
static void layer_off(int layer) { (void)layer; }
void hl_push_event(uint8_t code, uint8_t arg) { last_event_code=code; last_event_arg=arg; }
''' + state + '\nstatic report_mouse_t process_pointer(report_mouse_t mouse_report) {' + pointing + '''
return mouse_report;
}
''' + '\nstatic bool process_note(uint16_t keycode, keyrecord_t *record) {' + handler + '''
return true;
}
int main(void) {
    keyrecord_t press = {.event.pressed=true}, release = {.event.pressed=false};
    assert(midi_pitch_root() == 60);
    assert(midi_pitch_change_root(false));
    assert(midi_pitch_root() == 59 && midi_pitch_note(0) == 59); /* C4 -> B3 */
    assert(midi_pitch_change_root(true));
    assert(midi_pitch_root() == 60); /* B3 -> C4 */
    assert(midi_pitch_change_root(true));
    assert(midi_pitch_root() == 61); /* C4 -> C#4 */
    assert(midi_pitch_change_root(false));
    process_note(MIDI_SUSTAIN, &press); assert(cc_count == 0 && !midi_sustain);
    process_note(MIDI_MODE, &press);
    assert(last_cc == 11 && last_cc_value == 127);
    unsigned sustain_cc = cc_count;
    process_note(MIDI_SUSTAIN, &press);
    assert(last_cc == 64 && last_cc_value == 127 && midi_sustain);
    process_note(MIDI_SUSTAIN, &press); assert(cc_count == sustain_cc + 1);
    process_note(MIDI_SUSTAIN, &release);
    assert(last_cc == 64 && last_cc_value == 0 && !midi_sustain);
    /* Either pedal can hold sustain; releasing one must not release the other. */
    sustain_cc = cc_count;
    process_note(MIDI_SUSTAIN, &press);
    process_note(MIDI_SUSTAIN_RIGHT, &press);
    process_note(MIDI_SUSTAIN, &release);
    assert(midi_sustain == 2 && last_cc_value == 127 && cc_count == sustain_cc + 1);
    process_note(MIDI_SUSTAIN_RIGHT, &release);
    assert(!midi_sustain && last_cc_value == 0 && cc_count == sustain_cc + 2);
    sustain_cc = cc_count;
    process_note(MIDI_SUSTAIN_RIGHT, &press);
    process_note(MIDI_SUSTAIN, &press);
    process_note(MIDI_SUSTAIN_RIGHT, &release);
    assert(midi_sustain == 1 && cc_count == sustain_cc + 1);
    process_note(MIDI_SUSTAIN, &release);
    assert(!midi_sustain && cc_count == sustain_cc + 2);
    process_note(MIDI_SUSTAIN, &press);
    process_note(MIDI_SUSTAIN_RIGHT, &press);
    process_note(MIDI_MODE, &press);
    assert(last_cc == 64 && last_cc_value == 0 && !midi_sustain);
    sustain_cc = cc_count;
    process_note(MIDI_SUSTAIN, &release); assert(cc_count == sustain_cc);
    process_note(MIDI_SUSTAIN_RIGHT, &release); assert(cc_count == sustain_cc);
    process_note(MIDI_SUSTAIN_RIGHT, &press); assert(!midi_sustain && cc_count == sustain_cc);
    process_note(MIDI_MODE, &press);
    unsigned initial_cc = cc_count;
    /* The scale toggle acts on press, changes chord thirds, and reports it. */
    process_note(MIDI_SCALE_TOGGLE, &press);
    assert(midi_chord_is_minor() && last_event_code == HL_EV_MIDI_SCALE && last_event_arg == 1);
    process_note(MIDI_SCALE_TOGGLE, &release); assert(midi_chord_is_minor());
    process_note(MIDI_CHORD_0, &press);
    assert(midi_key_notes[20][0] == 60 && midi_key_notes[20][1] == 63 && midi_key_notes[20][2] == 67);
    process_note(MIDI_CHORD_0, &release);
    process_note(MIDI_SCALE_TOGGLE, &press);
    assert(!midi_chord_is_minor() && last_event_code == HL_EV_MIDI_SCALE && last_event_arg == 0);
    /* Diatonic C major triad and Em share E/G; single C is shared too. */
    process_note(MIDI_CHORD_0, &press);
    assert(midi_key_note_count[20] == 3);
    assert(midi_key_notes[20][0] == 60 && midi_key_notes[20][1] == 64 && midi_key_notes[20][2] == 67);
    unsigned chord_ons = on_count, chord_offs = off_count;
    process_note(MIDI_NOTE_0, &press); assert(on_count == chord_ons && midi_note_refs[60] == 2);
    process_note(MIDI_CHORD_0 + 2, &press); assert(on_count == chord_ons + 1);
    process_note(MIDI_CHORD_0, &release); assert(off_count == chord_offs);
    process_note(MIDI_CHORD_0 + 2, &release); assert(off_count == chord_offs + 3);
    process_note(MIDI_NOTE_0, &release); assert(off_count == chord_offs + 4);
    for (unsigned note=0; note<128; ++note) assert(midi_note_refs[note] == 0);

    /* Chords keep their original snapshots through settings changes. */
    process_note(MIDI_CHORD_0, &press);
    process_note(MIDI_SCALE_TOGGLE, &press);
    process_note(MIDI_CHORD_NEXT, &press);
    assert(midi_chord_is_minor() && midi_chord_type() == MIDI_CHORD_SEVENTH);
    midi_pitch_change_root(true);
    midi_pitch_change_octave(true);
    assert(midi_key_notes[20][0] == 60 && midi_key_notes[20][2] == 67);
    chord_offs = off_count;
    process_note(MIDI_CHORD_0, &release); assert(off_count == chord_offs + 3);
    midi_pitch_change_octave(false); midi_pitch_change_root(false);
    uint8_t notes[4];
    assert(midi_chord_notes(0, notes) == 4);
    assert(notes[0] == 60 && notes[1] == 63 && notes[2] == 67 && notes[3] == 70);
    process_note(MIDI_CHORD_NEXT, &press);
    assert(midi_chord_type() == MIDI_CHORD_DOMINANT);
    assert(midi_chord_notes(0, notes) == 4 && notes[1] == 63 && notes[3] == 70);
    process_note(MIDI_SCALE_TOGGLE, &press);
    assert(!midi_chord_is_minor());
    assert(midi_chord_notes(0, notes) == 4 && notes[1] == 64 && notes[3] == 70);
    process_note(MIDI_CHORD_NEXT, &press);
    assert(midi_chord_type() == MIDI_CHORD_TRIAD);
    process_note(MIDI_CHORD_0 + 6, &press); /* B diminished */
    assert(midi_key_notes[26][0] == 71 && midi_key_notes[26][1] == 74 && midi_key_notes[26][2] == 77);
    process_note(MIDI_SUSTAIN, &press);
    process_note(MIDI_MODE, &press);
    assert(!midi_sustain && last_cc == 64 && last_cc_value == 0);
    for (unsigned note=0; note<128; ++note) assert(midi_note_refs[note] == 0);
    for (unsigned key=0; key<MIDI_PLAY_KEY_COUNT; ++key) assert(midi_key_note_count[key] == 0);
    process_note(MIDI_MODE, &press);
    initial_cc = cc_count;
    report_mouse_t mouse = process_pointer((report_mouse_t){.x=-39, .y=-39, .h=3, .v=4, .buttons=1});
    assert(mouse.x == 0 && mouse.y == 0 && mouse.h == 0 && mouse.v == 0 && mouse.buttons == 1);
    assert(cc_count == initial_cc && midi_velocity == 100);
    process_pointer((report_mouse_t){.x=-1, .y=-1});
    assert(last_cc == 11 && last_cc_value == 126 && midi_velocity == 101);
    process_pointer((report_mouse_t){.x=40, .y=40});
    assert(last_cc_value == 127 && midi_velocity == 100);
    process_pointer((report_mouse_t){.x=-32768, .y=32767});
    assert(last_cc_value == 0 && midi_velocity == 1);
    process_pointer((report_mouse_t){.x=-32768, .y=32767});
    assert(midi_expression == 0 && midi_velocity == 1);
    /* A single opposite step works immediately after saturation. */
    midi_trackball_x = midi_trackball_y = 0;
    process_pointer((report_mouse_t){.x=40, .y=-40});
    assert(midi_expression == 1 && midi_velocity == 2);
    midi_trackball_x = midi_trackball_y = 0;
    process_pointer((report_mouse_t){.x=32767, .y=-32768});
    assert(midi_expression == 127 && midi_velocity == 127);
    process_pointer((report_mouse_t){.x=39});
    process_note(MIDI_MODE, &press);
    process_note(MIDI_MODE, &press);
    assert(midi_trackball_x == 0 && midi_trackball_y == 0);
    process_pointer((report_mouse_t){.x=-1});
    assert(midi_expression == 127);
    process_note(MIDI_NOTE_0, &press); assert(last_on == 60);
    assert(last_velocity == 127);
    /* Empty black-key positions never emit notes. */
    unsigned shared_off = off_count;
    unsigned initial_on = on_count;
    process_note(MIDI_BLACK_0, &press); assert(last_on == 60);
    process_note(MIDI_BLACK_0, &release); assert(off_count == shared_off && on_count == initial_on);
    process_note(MIDI_NOTE_0, &release); assert(off_count == shared_off + 1);
    /* Transposition can make two held keys share a pitch. */
    process_note(MIDI_NOTE_0 + 1, &press); assert(last_on == 62);
    midi_pitch_change_root(true);
    shared_off = off_count;
    initial_on = on_count;
    process_note(MIDI_BLACK_1, &press); assert(on_count == initial_on);
    process_note(MIDI_NOTE_0 + 1, &release); assert(off_count == shared_off);
    process_note(MIDI_BLACK_1, &release); assert(off_count == shared_off + 1 && last_off == 62);
    midi_pitch_change_root(false);
    process_note(MIDI_BLACK_2, &press); assert(last_on == 63);
    process_note(MIDI_BLACK_2, &release); assert(last_off == 63);
    process_note(MIDI_NOTE_0, &press);
    midi_pitch_change_root(true); midi_pitch_change_octave(true);
    assert(midi_pitch_root() == 73);
    process_note(MIDI_NOTE_0, &release); assert(last_off == 60);
    process_note(MIDI_NOTE_0, &press); assert(last_on == 73);
    midi_pitch_change_root(true);
    process_note(MIDI_MODE, &press); assert(last_off == 73);
    unsigned before = off_count;
    process_note(MIDI_NOTE_0, &release); assert(off_count == before);
    process_note(MIDI_MODE, &press);
    while (midi_pitch_change_root(false)) {}
    assert(midi_pitch_root() == 0 && !midi_pitch_change_root(false));
    assert(!midi_pitch_change_octave(false));
    for (unsigned octave=0; octave<9; ++octave) {
        for (unsigned pitch=0; pitch<12; ++pitch) {
            const uint8_t major[10] = {0,2,4,5,7,9,11,12,14,16};
            const uint8_t black[10] = {255,1,3,255,6,8,10,255,13,15};
            uint8_t current_root = midi_pitch_root();
            assert(current_root == octave * 12 + pitch);
            uint32_t chromatic = 0;
            for (unsigned i=0; i<MIDI_SCALE_NOTE_COUNT; ++i) {
                assert(midi_pitch_note(i) == current_root + major[i]);
                assert(midi_pitch_note(i) <= 127);
                chromatic |= 1u << major[i];
                process_note(MIDI_NOTE_0 + i, &press);
                assert(last_on == current_root + major[i]);
                process_note(MIDI_NOTE_0 + i, &release);
                unsigned ons = on_count, offs = off_count;
                process_note(MIDI_BLACK_0 + i, &press);
                if (black[i] == 255) {
                    assert(midi_pitch_note(i + MIDI_SCALE_NOTE_COUNT) == MIDI_PITCH_NONE);
                    assert(on_count == ons);
                } else {
                    assert(midi_pitch_note(i + MIDI_SCALE_NOTE_COUNT) == current_root + black[i]);
                    assert(last_on == current_root + black[i] && last_on <= 127);
                    assert(on_count == ons + 1);
                    chromatic |= 1u << black[i];
                }
                process_note(MIDI_BLACK_0 + i, &release);
                assert(off_count == offs + (black[i] != 255));
            }
            assert(chromatic == ((1u << 17) - 1)); /* every semitone, no gaps */
            /* Every chord shape follows the chosen scale, at every root. */
            for (unsigned scale=0; scale<2; ++scale) {
                const uint8_t major_scale[7] = {0,2,4,5,7,9,11};
                const uint8_t minor_scale[7] = {0,2,3,5,7,8,10};
                const uint8_t *steps = scale ? minor_scale : major_scale;
                assert(midi_chord_is_minor() == (bool)scale);
                for (unsigned type=0; type<3; ++type) {
                    assert(midi_chord_type() == (midi_chord_type_t)type);
                    for (unsigned degree=0; degree<10; ++degree) {
                        uint8_t count = midi_chord_notes(degree, notes);
                        assert(count == (type == 0 ? 3 : 4));
                        int expected[4];
                        for (unsigned voice=0; voice<count; ++voice) {
                            unsigned d = degree + voice * 2;
                            if (type == 2) {
                                const uint8_t shape[4] = {0, scale ? 3 : 4, 7, 10};
                                expected[voice] = current_root + (degree/7)*12 + steps[degree%7] + shape[voice];
                            } else expected[voice] = current_root + (d/7)*12 + steps[d%7];
                        }
                        int shift = expected[count-1] > 127 ? 12 : 0;
                        process_note(MIDI_CHORD_0 + degree, &press);
                        assert(midi_key_note_count[20+degree] == count);
                        for (unsigned voice=0; voice<count; ++voice) {
                            assert(notes[voice] == expected[voice]-shift && notes[voice] <= 127);
                            assert(midi_key_notes[20+degree][voice] == notes[voice]);
                        }
                        process_note(MIDI_CHORD_0 + degree, &release);
                    }
                    midi_chord_cycle_type();
                }
                midi_chord_toggle_minor();
            }
            if (current_root < 107) assert(midi_pitch_change_root(true));
        }
    }
    assert(!midi_pitch_change_octave(true));
    assert(midi_pitch_root() == 107 && !midi_pitch_change_root(true));
    for (unsigned i=0; i<11; ++i) assert(midi_pitch_change_root(false));
    assert(midi_pitch_root() == 96);
    assert(midi_pitch_change_root(false)); assert(midi_pitch_root() == 95);
    assert(midi_pitch_change_root(true)); assert(midi_pitch_root() == 96);
    assert(midi_pitch_change_octave(false)); assert(midi_pitch_root() == 84);
    assert(midi_pitch_change_octave(true)); assert(midi_pitch_root() == 96);
    return 0;
}
''')
            binary = root / "test"
            subprocess.run(["cc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-I", str(source),
                            str(source / "midi_pitch.c"), str(harness), "-o", str(binary)], check=True)
            subprocess.run([str(binary)], check=True)

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
