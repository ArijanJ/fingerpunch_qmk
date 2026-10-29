#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Private IDs for this firmware; not reserved by upstream VIA. */
#define HL_PROTO_VERSION 1
#define HL_CMD_PING 0x20
#define HL_CMD_GET_EVENTS 0x21
#define HL_CMD_GET_STATE 0x22
#define HL_CMD_SET_HOST_ACK 0x23
#define HL_REPLY_FLAG 0x80
#define HL_EVENT_QUEUE_SIZE 8
#define HL_MAX_EVENTS_PER_REPORT 7

#define HL_EV_MIDI 0x10
#define HL_EV_JOYSTICK 0x11
#define HL_EV_GAMING 0x12 /* arg: resulting default layer */
#define HL_EV_SDVX 0x13
#define HL_EV_NATIVE_SCROLL 0x14
#define HL_EV_MIDI_ROOT 0x15   /* arg: complete root MIDI note number */
#define HL_EV_MIDI_OCTAVE 0x16 /* arg: complete root MIDI note number */

/* Main-loop/key-processing context only. Best-effort, drop oldest on overflow. */
void hl_push_event(uint8_t code, uint8_t arg);

#ifdef VIA_ENABLE
bool via_command_kb(uint8_t *data, uint8_t length);
#endif
