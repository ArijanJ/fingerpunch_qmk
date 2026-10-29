# Keymap handoff and maintenance

## Current implementation

- Target: `fingerpunch/ffkb/rp/v1`, keymap `via`, RP2040, VID/PID
  `0xFE2B/0xFFBB`. VIA is enabled, not Vial; the configured board is non-split.
- `keymap.c` owns MIDI input and mode state; `midi_pitch.c/.h` own pitch
  mapping/transposition. MIDI mode toggles with the X+H combo and uses layer 13.
- In MIDI mode, the **left encoder changes root by one semitone** and the
  **right encoder changes octave by twelve semitones**. Clockwise raises values.
  Root starts at MIDI 60/C4 and is bounded to 0..107 (C-1..B7); settings are
  volatile, persist across mode toggles, and reset on firmware reboot.
- Important regression: root-down from C4 previously wrapped to B4, sounding
  almost an octave higher. It is fixed to C4 -> B3; upward B4 -> C5 also crosses
  smoothly. Preserve absolute-note transposition; do not reintroduce same-octave
  pitch-class wrapping. Root changes may legitimately update the displayed octave.
- The home row is the piano white-key pattern. The top row is the piano black-key
  pattern, **not the earlier full natural-minor row**. At C:

  ```text
  Upper:  —  C# D#  —  F# G# A#  —  C# D#
  Lower:  C   D  E  F   G  A  B  C   D  E
  ```

  Both rows transpose together. Upper slots 0, 3, 7 are silent, including old
  saved VIA assignments; `MIDI_PITCH_NONE` is 255, never a playable note.
  Existing custom-keycode numbers were retained when `MIDI_MINOR_*` symbols
  became `MIDI_BLACK_*`. Append new keycodes rather than shifting existing IDs.
- Held keys retain their note-on pitch after transposition. Shared pitches use
  one note-on and remain sounding until the last matching key is released.
  Leaving MIDI mode releases tracked notes. Preserve these behaviors.
- In MIDI mode, the left middle thumb (`K35`, normally `NAV_SPC`; third thumb
  argument in `LAYOUT_ffkb`) is `MIDI_SUSTAIN`, appended at `0x7E98`.
  It sends channel-1 CC 64 value 127 on press and 0 on release. MIDI-mode exit
  clears an active pedal. Sustain is implemented by the receiving synth, not
  firmware note retention. No host-link event or listener update is involved.
- In MIDI mode, trackball **left/right decreases/increases expression (CC 11)**;
  **down/up decreases/increases velocity of future notes**. Threshold is 40
  processed movement units per step; remainders reset on mode changes.
  Expression starts at 127 (range 0..127); velocity starts at 100 (range 1..127).
  Firmware sensor DPI and applicable Fingerpunch acceleration affect sensitivity;
  OS mouse speed does not. MIDI consumes motion before joystick/nav/drag-scroll,
  preserving button bits. Other modes retain existing trackball/encoder actions.
- `via_host_link.c/.h` implement a polled eight-event queue through
  `via_command_kb()`. Do not define another `raw_hid_receive()` or send unsolicited
  raw HID reports. Keep reports at 32 bytes, usage page/ID `0xFF60/0x61`, and
  preserve VIA's USB interface and protocol version.
- Host commands are `0x20..0x23`; events `0x10..0x16`. Root/octave events
  `0x15/0x16` both carry the complete root MIDI number. Host translation uses
  MIDI 60=C4 and sharp spellings. Trackball expression/velocity currently emit
  no host events. GET_STATE does not expose MIDI root, expression, or velocity.
- `host/README.md` contains setup, protocol, VIA keycode assignments, service
  installation, and physical checks. `host/VALIDATION.md` records actual results.

## Verification and working state

From this keymap directory: `python3 -m unittest discover -s host -v`.
Seven tests currently pass, including actual C key-processing branches with
stubs, exact pitch coverage at all roots/octaves, silent slots, held-note
release, root-boundary regression, trackball behavior, sustain press/release and
mode-exit cleanup, and host translation.
These tests do not prove USB/VIA coexistence or audible hardware behavior.

From the QMK repository root:

```sh
qmk compile -kb fingerpunch/ffkb/rp/v1 -km via
qmk compile -kb fingerpunch/ffkb/rp/v1 -km via -e VIA_ENABLE=no -e TARGET=fingerpunch_ffkb_rp_v1_via_no_via
```

The latest VIA build passed and its root-level `fingerpunch_ffkb_rp_v1_via.uf2`
includes the root-boundary fix and sustain key. The non-VIA variant passed earlier,
before the latest trackball/piano/root/sustain changes; rebuild it to claim current
full verification.
Existing board metadata diagnostics, FORCE_NKRO deprecation, and serial LTO
warnings are known. The agent has not flashed firmware or verified hardware;
the user has been testing it, but the latest fix is awaiting confirmation.

There are uncommitted changes, including new `midi_pitch.c/.h`; preserve them.
Do not assume generated UF2s or `/tmp` test environments are committed or durable.
VIA may retain saved keymaps across flashes. Back up mappings before EEPROM
reset; prefer assigning changed keys through VIA when preserving user mappings.

## Keep the host listener in sync

When changing USB identity, host-link commands/events, their argument meanings,
or layer numbers used by events, update the host listener in the same change.
Ordinary key rearrangements need no host update unless they change emitted events.

- **USB VID/PID:** Check the keyboard's effective USB configuration (currently
  `../../info.json`). Update defaults in `host/qmk_link.py` and the lowercase
  vendor/product values in `host/70-qmk-link.rules`.
- **Command/event IDs:** Treat `via_host_link.h` and `via_host_link.c` as the
  firmware source of truth. Update command constants, parsing, and handshake
  expectations in `host/qmk_link.py` when affected. Keep private commands in
  `0x20–0x3F`; preserve VIA's handler and protocol.
- **Event arguments/layers:** Check `hl_push_event()` calls and `keymap_layers`
  in `keymap.c`. Update `host/notifications.json`, whose keys are hexadecimal
  `code:arg` pairs. For example, gaming event `0x12` with default layer 2 uses
  `"12:02"`; moving that layer to 5 requires `"12:05"`. These are semantic
  event/layer values, not custom keycode enum positions.
- **Protocol changes:** Keep the 32-byte report size and existing USB interface.
  For incompatible wire-format or semantic changes, bump `HL_PROTO_VERSION`
  and update the host's version validation and parser together. Do not change
  `VIA_PROTOCOL_VERSION` to version this feature.
- Update affected tests in `host/test_qmk_link.py` and protocol/setup notes in
  `host/README.md`. Run `python3 -m unittest discover -s host -v`; compile the
  firmware when firmware changes. Report hardware checks as pending unless run.

If a user installed the optional service, remind them to copy updated host files
to `~/.local/share/qmk-link/` and restart `qmk-link.service`; repository edits do
not update that copy. USB ID changes also require reinstalling the udev rule,
reloading rules, and reconnecting the keyboard. Firmware changes require flashing.
Keep the listener stopped while using VIA until simultaneous operation is verified.
