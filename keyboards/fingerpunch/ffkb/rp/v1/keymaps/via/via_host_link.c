#include "via_host_link.h"

#ifdef VIA_ENABLE
#    include <string.h>
#    include "quantum.h"
#    include "raw_hid.h"
#    include "usb_descriptor.h"
#    include "via.h"

_Static_assert(RAW_EPSIZE == 32, "Host link protocol requires 32-byte reports");
_Static_assert(4 + HL_MAX_EVENTS_PER_REPORT * 4 == RAW_EPSIZE, "Invalid event packet size");

typedef struct {
    uint8_t code, arg;
    uint16_t seq;
} hl_event_t;

static hl_event_t queue[HL_EVENT_QUEUE_SIZE];
static uint8_t head, count, host_ack;
static uint16_t next_seq = 1, dropped;

void hl_push_event(uint8_t code, uint8_t arg) {
    if (!code) return;
    if (count == HL_EVENT_QUEUE_SIZE) {
        head = (head + 1) % HL_EVENT_QUEUE_SIZE;
        --count;
        ++dropped;
    }
    queue[(head + count) % HL_EVENT_QUEUE_SIZE] = (hl_event_t){code, arg, next_seq++};
    ++count;
}

bool via_command_kb(uint8_t *data, uint8_t length) {
    if (length != RAW_EPSIZE) return false;
    uint8_t cmd = data[0];
    if (cmd < HL_CMD_PING || cmd > HL_CMD_SET_HOST_ACK) return false;
    uint8_t value = data[1];
    memset(data, 0, length);
    data[0] = cmd | HL_REPLY_FLAG;
    switch (cmd) {
        case HL_CMD_PING:
            data[1] = HL_PROTO_VERSION;
            data[2] = RAW_EPSIZE;
            data[3] = HL_EVENT_QUEUE_SIZE;
            break;
        case HL_CMD_GET_EVENTS: {
            uint8_t n = count < HL_MAX_EVENTS_PER_REPORT ? count : HL_MAX_EVENTS_PER_REPORT;
            data[1] = n;
            data[2] = dropped & 0xff;
            data[3] = dropped >> 8;
            for (uint8_t i = 0; i < n; ++i) {
                hl_event_t event = queue[head];
                uint8_t offset = 4 + i * 4;
                data[offset] = event.code;
                data[offset + 1] = event.arg;
                data[offset + 2] = event.seq >> 8;
                data[offset + 3] = event.seq & 0xff;
                head = (head + 1) % HL_EVENT_QUEUE_SIZE;
                --count;
            }
            break;
        }
        case HL_CMD_GET_STATE:
            data[1] = get_highest_layer(layer_state | default_layer_state);
            data[2] = get_highest_layer(default_layer_state);
            data[3] = get_mods() | get_weak_mods();
#    ifndef NO_ACTION_ONESHOT
            data[3] |= get_oneshot_mods();
#    endif
            data[4] = host_ack;
            break;
        case HL_CMD_SET_HOST_ACK:
            data[1] = host_ack = value;
            break;
    }
    /* VIA returns immediately after this hook reports the request handled. */
    raw_hid_send(data, length);
    return true;
}
#else
void hl_push_event(uint8_t code, uint8_t arg) {
    (void)code;
    (void)arg;
}
#endif
