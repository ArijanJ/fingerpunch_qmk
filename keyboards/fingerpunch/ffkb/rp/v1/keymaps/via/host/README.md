# FFKB host notifications

This keymap retains VIA and adds a private, polled raw HID command channel.
No USB interfaces, usage IDs, report sizes, or VIA protocol versions change.
Events currently describe MIDI mode, joystick mode, gaming layout, SDVX encoder
mode, and the Linux native drag-scroll toggle. Event emission never sends USB
reports. Normal keys, notes, encoder ticks, and automatic layer changes do not
generate notifications.

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
toggle MIDI mode ten times slowly and confirm ten consecutive sequence numbers.
Stop the listener, toggle twenty times, then restart: eight newest events
should arrive in two batches with twelve additional drops (assuming an empty
queue beforehand). Keyboard input must remain responsive without the listener.

With the listener stopped, open VIA and verify connection, remapping, and
lighting controls if available, including ten seconds of slider movement.
Only then try simultaneous polling and VIA as an explicit compatibility test;
if it fails, stop polling and use the separate-access workflow above.
