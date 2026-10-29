# Keymap handoff and maintenance

## Current implementation

- Target: `fingerpunch/ffkb/rp/v1`, keymap `via`, RP2040, VID/PID
  `0xFE2B/0xFFBB`. VIA is enabled, not Vial; the configured board is non-split.
- `keymap.c` owns MIDI input and mode state; `midi_pitch.c/.h` own pitch and
  chord generation. Enter MIDI mode with X+H; exit via `MIDI_MODE` on the bottom
  left edge. Chords replaced the MIDI-layer X/H keys. MIDI uses layer 13.
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
- Bottom row has ten chord keys: degrees I–VII then I–III one octave higher.
  Thumb `K34`, immediately left of sustain, cycles triads -> diatonic sevenths
  -> dominant/minor sevenths. Last left thumb `K36` toggles major/natural minor.
  Triads/sevenths stack diatonic thirds. In the third setting, every chord uses
  `0,4,7,10` in major or `0,3,7,10` in minor (explicit user preference).
  Major/triads are defaults; toggling MIDI preserves these settings. The piano
  rows do not change mapping with chord scale. High voicings shift down a whole
  octave if their highest note exceeds 127; no individual-note clamping.
- Held notes are stored in `midi_key_notes[30][4]` and `midi_key_note_count[30]`
  (piano slots 0..19, chord slots 20..29). `midi_note_refs[128]` is the combined
  pitch ownership pool. Only a 0->1 reference sends note-on; 1->0 sends note-off.
  Release uses the saved snapshot even after root/type/scale changes. Counts
  define active array entries; cleared entries use `MIDI_PITCH_NONE`.
  This prepares for a future arpeggiator, which is not implemented. Sustain
  latched by the synth is not represented as physically held in these arrays.
- New appended keycodes: chords `0x7E99..0x7EA2`, chord cycle `0x7EA3`, scale
  toggle `0x7EA4`. MIDI exit is existing `0x7E83`. Saved VIA layouts need these
  assignments on layer 13; preserve prior IDs and user mappings.
- In MIDI mode, both middle thumbs are sustain: left `K35` (normally `NAV_SPC`,
  third thumb argument) is `MIDI_SUSTAIN` / `0x7E98`; right `K38` (normally
  `SPC_NUM`, sixth argument) is `MIDI_SUSTAIN_RIGHT` / `0x7EA5`.
  `midi_sustain` is a two-bit ownership mask, not a bool. Send channel-1 CC 64
  value 127 when the first thumb presses, 0 when the last releases. Mode exit
  clears both bits and releases the pedal. Use distinct keycodes for correct
  overlap handling. Sustain belongs to the synth, not firmware note retention.
  No host-link event or listener update is involved.
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
- Host commands are `0x20..0x23`; events `0x10..0x18`. Root/octave events
  `0x15/0x16` both carry the complete root MIDI number. Host translation uses
  MIDI 60=C4 and sharp spellings. Trackball expression/velocity currently emit
  no host events. GET_STATE does not expose MIDI root, expression, or velocity.
  Chord-type event `0x17` carries 0/1/2 and scale event `0x18` carries 0 major /
  1 natural minor. MIDI entry emits current root/type/scale along with mode-on.
  `host/notifications.json` has the new labels; update installed host copies.
  These additive events keep protocol version 1 and the existing framing.
- `host/README.md` contains setup, protocol, VIA keycode assignments, service
  installation, and physical checks. `host/VALIDATION.md` records actual results.

## Verification and working state

From this keymap directory: `python3 -m unittest discover -s host -v`.
Nine tests currently pass, including actual C key-processing branches with
stubs, exact pitch coverage at all roots/octaves, silent slots, held-note
release, root-boundary regression, trackball behavior, sustain press/release and
mode-exit cleanup, all chord shapes/scales/degrees at all roots, ownership-array
cleanup, dual-pedal overlap, physical thumb assignments, and host notifications.
These tests do not prove USB/VIA coexistence or audible hardware behavior.

From the QMK repository root:

```sh
qmk compile -kb fingerpunch/ffkb/rp/v1 -km via
qmk compile -kb fingerpunch/ffkb/rp/v1 -km via -e VIA_ENABLE=no -e TARGET=fingerpunch_ffkb_rp_v1_via_no_via
```

The latest VIA build passed and its root-level `fingerpunch_ffkb_rp_v1_via.uf2`
includes the root-boundary fix, both sustain keys, and chords. See `host/VALIDATION.md`
for the latest non-VIA build status.
Existing board metadata diagnostics, FORCE_NKRO deprecation, and serial LTO
warnings are known. The agent has not flashed firmware or verified hardware;
the user has been testing it, but the latest fix is awaiting confirmation.

There are uncommitted chord/control/host/documentation changes; preserve them.
Do not assume generated UF2s or `/tmp` test environments are committed or durable.
VIA may retain saved keymaps across flashes. Back up mappings before EEPROM
reset; prefer assigning changed keys through VIA when preserving user mappings.
Important physical-layout correction from the user: the eight trailing layout
arguments are LEFT KNOB PRESS, THREE LEFT THUMBS, THREE RIGHT THUMBS, RIGHT
KNOB PRESS. K32 and K3B are knob presses, not outer thumbs. The major/minor
switch was mistakenly assigned to K32; it is now on the last left thumb K36
(fourth trailing argument, normally OSSPC_BROWSE, matrix row 5 column 5).
The three left thumbs are chord-type K34, sustain K35, scale K36. Both knob
presses are unused in MIDI mode. Confirm saved VIA layer 13 moves Any 0x7EA4
from the left knob to K36. The scale handler/pitches were already correct:
C-E-G vs C-Eb-G. It affects only new bottom-row chords, not piano/held/sustained
notes. Host event 0x18 carries 1 minor / 0 major.

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
