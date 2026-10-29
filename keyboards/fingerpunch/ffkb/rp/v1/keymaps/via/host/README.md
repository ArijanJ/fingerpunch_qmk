# FFKB host notifications

This keymap retains VIA and adds a private, polled raw HID command channel.
No USB interfaces, usage IDs, report sizes, or VIA protocol versions change.
Events currently describe MIDI mode, joystick mode, gaming layout, SDVX encoder
mode, the Linux native drag-scroll toggle, and MIDI root/octave changes. Event
emission never sends USB reports. Normal keys, notes, encoder ticks outside MIDI
mode, and automatic layer changes do not generate notifications.

## MIDI root and octave knobs

Toggle MIDI mode with the X+H combo. While MIDI mode is active:

- **Left encoder:** clockwise raises the root by one semitone,
  counterclockwise lowers it. B/C crosses octaves smoothly: C4 → B3 downward,
  B4 → C5 upward. The host's octave display follows that crossing.
- **Right encoder:** clockwise raises the octave; counterclockwise lowers it.
- **Left middle thumb** (normally the NAV/Space key): hold for sustain.
  `MIDI_SUSTAIN` sends MIDI channel 1 CC 64 value 127 on press and 0 on release.
  Leaving MIDI mode also clears an active pedal. The synth handles sustain;
  note-off events are still sent normally. Outside MIDI mode, this thumb keeps
  its existing binding. If VIA retains the old MIDI-layer thumb assignment,
  set this key to Any `0x7E98` (matrix row 5, column 3; MIDI layer 13).
- **Home row:** the piano's ten white-key positions. **Top row:** black-key
  positions with the familiar two/three grouping. At root C4:

  ```text
  Upper:  —  C#  D#  —  F#  G#  A#  —  C#  D#
  Lower:  C   D   E  F   G   A   B  C   D   E
  ```

  The upper row is aligned to the ten lower playable keys; the outermost
  column on each side also stays unused. Both rows transpose together with
  the root knob. Lower offsets are `0,2,4,5,7,9,11,12,14,16`;
  upper offsets are `—,1,3,—,6,8,10,—,13,15`. Together they cover every
  semitone from the root through root + 16, allowing major, minor, and other
  scales manually. Blank positions never emit a note. Startup is C4 (MIDI 60).
  Shared pitches after transposition remain held until the last key is released.
- Root octaves range from -1 to 7, keeping every note key in MIDI range for
  every root. Turning past an octave limit leaves the setting unchanged.
- Held notes keep their original pitch until release, even after knob changes.
  Subsequent presses use the new root/octave. Leaving MIDI mode releases all
  tracked notes. Settings survive mode toggles but reset to C4 on firmware reboot.
- **Trackball left/right:** decreases/increases expression (MIDI CC 11),
  including for sounding notes. **Trackball down/up:** decreases/increases
  velocity for subsequent note presses. Both axes work together diagonally.
  Each 40 motion units changes the value by one; partial motion accumulates.
  Those are processed sensor counts: keyboard DPI and enabled/applicable
  Fingerpunch acceleration affect sensitivity before MIDI processing. OS mouse
  speed does not. `MIDI_TRACKBALL_STEP` in `keymap.c` sets the MIDI-specific
  threshold; lower values respond faster, higher values respond more finely.
  Expression is bounded to 0–127 and velocity to 1–127. Startup values are
  expression 127 and velocity 100; values survive MIDI mode toggles.
  MIDI mode consumes pointer/wheel movement before navigation, drag-scroll,
  and joystick handling; mouse button bits are preserved. Outside MIDI mode,
  the trackball retains its existing behavior. No host event IDs changed.

Run `--listen` to print translated changes or `--notify` for desktop feedback:
`MIDI root: C# · octave 4`, `MIDI octave: 5 · root C#`. Enabling MIDI mode also
reports the current root. The host uses MIDI 60 = C4; some DAWs label octaves
differently, though the transmitted note numbers are the same.

## Build and flash

From the QMK repository root:

```sh
qmk compile -kb fingerpunch/ffkb/rp/v1 -km via
```

Flash `fingerpunch_ffkb_rp_v1_via.uf2` using your usual RP2040 bootloader
procedure. The separately built `*_no_via.uf2` is for build verification only;
it has no VIA or host channel.

## Linux setup

Run these commands from this keymap directory. Install the distribution's
HIDAPI hidraw runtime and `notify-send` first (usually libnotify). The Python
package is **hid**, which exposes `hid.Device`, rather than the similarly named
Python package **hidapi**.

```sh
python3 -m venv host/.venv
host/.venv/bin/pip install -r host/requirements.txt
sudo install -m 0644 host/70-qmk-link.rules /etc/udev/rules.d/70-qmk-link.rules
sudo udevadm control --reload-rules
```

Unplug and reconnect the keyboard so the access rule applies. Run the listener
as your logged-in desktop user, with access to the session D-Bus:

```sh
host/.venv/bin/python host/qmk_link.py --ping
# proto=1 report_len=32 queue=8
host/.venv/bin/python host/qmk_link.py --state
host/.venv/bin/python host/qmk_link.py --ack 42
host/.venv/bin/python host/qmk_link.py --notify
```

`--listen` logs events without notifications. `--notify` also listens.
Ctrl-C stops the process. Use `--serial` to select among multiple matching
boards. Notification text lives in `notifications.json`; keys are hexadecimal
`code:arg` pairs. Remove a pair to silence it, or pass `--config /path/file.json`.
MIDI root/octave events use `15:*` and `16:*` templates with `{note}`, `{octave}`,
and `{number}` placeholders. An exact `code:arg` entry overrides the template.
Set a MIDI template to an empty string to silence it; omitted MIDI templates use
the built-in root/octave message. These messages are translated on the host.
The listener invokes `notify-send` with a fixed argument list; it does not run
shell commands supplied by the keyboard.

## VIA coexistence limitation

**Stop the listener while using VIA until simultaneous operation has been
verified with your client.** Polling correlates this listener's replies but
does not prevent VIA from receiving them and treating them as incorrect VIA
responses. Keeping the listener running while VIA is closed preserves normal
VIA remapping functionality when you later stop the listener and open VIA.

The implementation does not detect VIA, coordinate with it, or guarantee
concurrent access. If concurrent use is required, the VIA client must filter
private replies or all traffic must go through a coordinating host process.

## Optional desktop service

After the manual checks pass, install a persistent copy (including its own
virtual environment; do not copy an existing virtual environment):

```sh
mkdir -p "$HOME/.local/share/qmk-link" "$HOME/.config/systemd/user"
cp host/qmk_link.py host/notifications.json host/requirements.txt "$HOME/.local/share/qmk-link/"
python3 -m venv "$HOME/.local/share/qmk-link/.venv"
"$HOME/.local/share/qmk-link/.venv/bin/pip" install -r "$HOME/.local/share/qmk-link/requirements.txt"
cp host/qmk-link.service "$HOME/.config/systemd/user/"
systemctl --user daemon-reload
systemctl --user enable --now qmk-link.service
```

The service follows `graphical-session.target`; desktop environments must
activate that target and export the desktop environment to the user manager.
Check logs with `journalctl --user -u qmk-link.service`.
Use `systemctl --user stop qmk-link.service` before opening VIA and
`systemctl --user start qmk-link.service` afterward.

## Protocol version 1

All reports are 32 bytes; host writes include an additional zero report-ID
prefix, which HIDAPI consumes. Replies have command byte ORed with `0x80`;
unused bytes are zero. Unimplemented commands fall through to VIA.

| Command | Request after command | Reply after command |
| --- | --- | --- |
| `20` PING | none | version=1, report length=32, queue capacity=8 |
| `21` GET_EVENTS | none | count, dropped low, dropped high, up to 7 events |
| `22` GET_STATE | none | highest active/default layer combined, highest default layer, modifier mask, host_ack |
| `23` SET_HOST_ACK | one byte | stored byte |

An event is `(code, arg, sequence high, sequence low)`. The eight-entry queue
drops the oldest event on overflow. Both sequence and cumulative drop counter
wrap modulo 65536 and reset on firmware reboot. Modifiers include regular,
weak, and enabled oneshot modifiers. `host_ack` is only a volatile byte for
testing host-to-keyboard writes; it does not acknowledge event delivery or
control LEDs.

| Event code | Argument |
| --- | --- |
| `10` MIDI mode | 0 off, 1 on |
| `11` Joystick mode | 0 off, 1 on |
| `12` Gaming toggle | resulting default layer: 0 base, 2 gaming, 4 Canary |
| `13` SDVX encoders | 0 off, 1 on |
| `14` Native keyboard drag scrolling | 0 host scrolling, 1 native scrolling |
| `15` MIDI root changed / MIDI mode enabled | complete root MIDI note number |
| `16` MIDI octave changed | complete root MIDI note number |

The new event codes are additive; the version-1 wire format is unchanged. Both
MIDI pitch events carry the complete root number so each message can independently
derive pitch class (`number % 12`) and octave (`number // 12 - 1`).

This is best-effort notification delivery. GET_EVENTS dequeues before receipt
is confirmed, so transport failures can lose events. Sends occur only in the
host-triggered hook, and the platform USB send can still wait for its timeout.
On connection the listener validates the handshake; transport errors reconnect
after one second and restart loss tracking. The initial loss message reports
the firmware's cumulative count since boot, including after reconnects. A full
counter wrap between polls cannot be distinguished from zero additional loss.
Old queued toggles may display when the listener starts. This protocol has no
boot/session identifier and is not intended for reliable action execution.

## Verification

```sh
python3 -m unittest discover -s host -v
qmk compile -kb fingerpunch/ffkb/rp/v1 -km via
qmk compile -kb fingerpunch/ffkb/rp/v1 -km via -e VIA_ENABLE=no -e TARGET=fingerpunch_ffkb_rp_v1_via_no_via
```

The tests compile the actual C module with small QMK stubs and exercise
dispatch, FIFO retention, overflow, sequence/counter wrap, state, and the
non-VIA stub. Host tests exercise packet framing, parsing, handshake rejection,
foreign reports, and timeouts. They do not simulate USB or prove coexistence.

After flashing, check PING and state/ack round trips. With `--listen` running,
toggle joystick mode ten times slowly and confirm ten consecutive sequence numbers.
Stop the listener, toggle joystick mode twenty times, then restart: eight newest events
should arrive in two batches with twelve additional drops (assuming an empty
queue beforehand). Keyboard input must remain responsive without the listener.

With the listener stopped, open VIA and verify connection, remapping, and
lighting controls if available, including ten seconds of slider movement.
Only then try simultaneous polling and VIA as an explicit compatibility test;
if it fails, stop polling and use the separate-access workflow above.

For MIDI, enable the mode and verify C4, turn the left knob one step clockwise
for C#4, then the right knob clockwise for C#5. Play the note row and check pitches
in your DAW/MIDI monitor. Hold a note, turn each knob, then release: the original
note must stop. Repeat while exiting MIDI mode. Confirm both octave limits and
that other layers' knob actions still work. If using the optional service, copy
the updated `qmk_link.py` and `notifications.json` to `~/.local/share/qmk-link/`
and restart `qmk-link.service` after flashing the new firmware.

For the root-boundary regression, start from C4 and turn the left knob one step
counterclockwise: the root must be B3 (MIDI 59), not B4 (71). One step clockwise
returns to C4 (60). Crossing upward from B4 must produce C5. The listener already
derives octave from the full root number; this fix requires firmware flashing,
not a host-listener change.

Also roll the trackball left/right while holding a MIDI note and confirm CC 11
changes in your MIDI monitor. Roll up/down and play new notes to check velocity.
The cursor should stay still in MIDI mode; leave MIDI mode and verify normal
pointer movement and navigation-layer arrow keys again.

For sustain, watch CC 64 in your MIDI monitor: thumb press must send 127 and
release 0. With a sustain-capable synth, hold the thumb, play/release a note,
then release the thumb. Exit MIDI mode while holding it and verify CC 64 returns
to 0. This feature needs only a firmware flash/VIA key assignment; no listener
changes are required.

### VIA and the piano upper row

VIA saves its dynamic keymap in EEPROM; flashing may leave its saved top row
empty instead of loading the new firmware defaults. Back up your VIA layout
before resetting EEPROM to load the new layout, or assign the upper row through
VIA's Any/custom-keycode entry. Above the ten existing playable lower keys,
use these assignments left to right:

```text
Upper:  KC_NO  0x7E8F  0x7E90  KC_NO  0x7E92  0x7E93  0x7E94  KC_NO  0x7E96  0x7E97
```

The old minor-row keycode numbers were preserved and renamed `MIDI_BLACK_0`
through `MIDI_BLACK_9`. Slots 0, 3, and 7 are now silent even if VIA still has
those old numeric assignments. If you already loaded the previous minor row,
its saved assignments therefore produce the new piano pattern after flashing.
The lower row starts at `0x7E84`. Recheck numeric values if the custom-keycode
enums change. No host event IDs or listener updates are needed.

To check the layout, release all notes, select C, and verify C# and D# on the
upper row followed by F#, G#, and A#. Combine both rows to play every chromatic
step. Repeat at C#, D, and B: all pitches should transpose together. Keep the
synth/DAW transpose neutral and disable any scale quantizer when checking.
Notes held across a root change intentionally retain their old pitch.
Automated tests check actual note-on output, silent positions, and complete
chromatic coverage at all twelve roots and supported octaves; listening tests
remain manual.
