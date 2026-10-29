# Validation, 2026-10-29

| Check | Result |
| --- | --- |
| `qmk compile -kb fingerpunch/ffkb/rp/v1 -km via` | Exit 0; linked and created `fingerpunch_ffkb_rp_v1_via.uf2`. No duplicate-symbol or missing-prototype errors. |
| Same build with `-e VIA_ENABLE=no -e TARGET=fingerpunch_ffkb_rp_v1_via_no_via` | Exit 0; linked and created a separately named UF2. |
| `python3 -m unittest discover -s host -v` | Five tests passed, including compiling/executing the actual firmware module with test stubs. |
| `git diff --check` | Passed. |
| Install `host/requirements.txt` into temporary venv | Successfully installed `hid==1.0.8`; confirmed `Device.read(size, timeout=None)` API. |
| Run host `--ping` against hardware | Exit 1: `unable to open device`. HID enumeration finds a matching device, but access is unavailable in this environment. |
| VIA connect/remapping/lighting-slider regression | Pending physical test after flashing. |
| Ten event toggles with listener active | Pending physical test; FIFO behavior covered by C test. |
| Twenty toggles without listener, then drain newest eight | Pending physical test; overflow/drop accounting covered by C test. |
| VIA and listener simultaneously | Pending; not guaranteed by polling. See README limitation. |

Both QMK builds report existing board metadata diagnostics (missing
`keyboard.json`, layout metadata and header-defined `LAYOUT_ffkb`), the existing
`FORCE_NKRO` deprecation, and serial LTO compilation warnings. These did not
prevent either build from succeeding. An initial build caught a missing
`usb_descriptor.h` include in the new module; that was fixed before the
successful builds.

No firmware was flashed, system udev rules installed, or desktop service
activated by this implementation session. The temporary Python environment
used for host-library verification is `/tmp/ffkb-host-link-venv`.
