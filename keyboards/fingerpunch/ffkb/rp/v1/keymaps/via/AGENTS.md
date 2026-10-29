# Keep the host listener in sync

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
