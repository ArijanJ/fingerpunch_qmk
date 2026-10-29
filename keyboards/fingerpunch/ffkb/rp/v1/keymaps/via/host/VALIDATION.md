# Validation, 2026-10-29

| Check | Result |
| --- | --- |
| `qmk compile -kb fingerpunch/ffkb/rp/v1 -km via` | Exit 0; linked and created `fingerpunch_ffkb_rp_v1_via.uf2`. No duplicate-symbol or missing-prototype errors. |
| Same build with `-e VIA_ENABLE=no -e TARGET=fingerpunch_ffkb_rp_v1_via_no_via` | Passed earlier and created a separately named UF2, before the latest trackball, piano-layout, and root-boundary changes. Current full non-VIA firmware rebuild is pending; automated host-link stub test still passes. |
| `python3 -m unittest discover -s host -v` | Seven tests passed, including compiling/executing the actual firmware module, MIDI pitch module, and MIDI key-processing branches with test stubs. |
| `git diff --check` | Passed. |
| Install `host/requirements.txt` into temporary venv | Successfully installed `hid==1.0.8`; confirmed `Device.read(size, timeout=None)` API. |
| Run host `--ping` against hardware | Exit 1: `unable to open device`. HID enumeration finds a matching device, but access is unavailable in this environment. |
| VIA connect/remapping/lighting-slider regression | Pending physical test after flashing. |
| Ten event toggles with listener active | Pending physical test; FIFO behavior covered by C test. |
| Twenty toggles without listener, then drain newest eight | Pending physical test; overflow/drop accounting covered by C test. |
| VIA and listener simultaneously | Pending; not guaranteed by polling. See README limitation. |
| MIDI root/octave pitch range and held-note release | Automated tests passed: original note-off pitch survives transposition and MIDI mode exit; all roots/octaves stay within MIDI range. Physical knob/DAW verification pending. |
| Host MIDI note translation | Automated checks passed for C4, C#4, B3, negative octaves, upper MIDI note, and notification templates. |
| MIDI trackball expression/velocity | VIA firmware rebuild passed. Automated checks cover accumulated motion, CC 11 updates, velocity on subsequent notes, upper/lower bounds, mode-toggle remainder reset, and consumed motion with preserved buttons. Physical trackball/DAW testing pending. |
| Piano white/black rows | Tests check actual note-on values, silent upper slots, and complete 17-semitone coverage at 12 roots × 9 octaves, plus shared-pitch release behavior. Physical/audible verification pending. |
| VIA piano-row assignment values | Lower row starts `0x7E84`; upper slots retain `0x7E8E`–`0x7E97`, with slots 0, 3, and 7 silent. Existing numeric IDs preserved. |
| Root octave-boundary regression | Fixed same-octave pitch-class wrapping (C4 down incorrectly selected B4). Tests verify C4 → B3, B3 → C4, C4 → C#4, both range limits, and octave-knob steps. Host event arguments still carry the complete MIDI root number; no wire-format change. Physical verification pending. |
| MIDI sustain thumb | Automated checks passed for CC 64=127 on press, 0 on release, duplicate-press suppression, ignored inactive presses, and mode-exit cleanup. Physical synth/pedal verification pending. |

The recorded QMK builds report existing board metadata diagnostics (missing
`keyboard.json`, layout metadata and header-defined `LAYOUT_ffkb`), the existing
`FORCE_NKRO` deprecation, and serial LTO compilation warnings. These did not
prevent either build from succeeding. An initial build caught a missing
`usb_descriptor.h` include in the new module; that was fixed before the
successful builds.

The agent did not flash firmware, install system udev rules, or activate the
desktop service. The user has been testing MIDI and reported the root-boundary
problem; the latest fix has not yet been confirmed on hardware. Do not infer
which firmware/service copy is installed from generated repository artifacts.
The temporary Python environment used for host-library verification was
`/tmp/ffkb-host-link-venv`; it may not survive a future session.

## Handoff

Latest VIA UF2: repository-root `fingerpunch_ffkb_rp_v1_via.uf2`, rebuilt after
the absolute-root fix and sustain thumb. Current source and new `midi_pitch.c/.h` have uncommitted
changes. Firmware tests still use stubs for hardware-dependent calls. Remaining
checks are physical MIDI/trackball behavior, VIA regression/coexistence, and a
fresh full non-VIA build if required. See the keymap's `AGENTS.md` for the current
controls and maintenance rules.
