#pragma once

//#define VIAL_KEYBOARD_UID {0x4B, 0x25, 0xFF, 0x63, 0xAE, 0x22, 0x5A, 0x5A}

//#define VIAL_KEY_OVERRIDE_ENTRIES 20
//#define VIAL_COMBO_ENTRIES 20
//#define VIAL_TAP_DANCE_ENTRIES 20

#define PWM33XX_LIFTOFF_DISTANCE 4

#define POINTING_DEVICE_AUTO_MOUSE_ENABLE
#define AUTO_MOUSE_DEFAULT_LAYER 11

#define FP_POINTING_ACCELERATION_ENABLE
#define AUTO_MOUSE_TIME 500

#define FP_AUTO_MOUSE_TRACKBALL_SENSITIVITY 0.1
#define AUTO_MOUSE_THRESHOLD 0

#define AUTO_MOUSE_DEBOUNCE 25

#undef FP_AUDIO_MOUSE_BUTTONS
#undef FP_AUDIO_CUT_COPY_PASTE
#undef FP_AUDIO_SAVE

#define DYNAMIC_KEYMAP_LAYER_COUNT 16

#define ENCODER_DEFAULT_POS 0x3

#ifndef __arm__
/* Disable unused features. */
#    define NO_ACTION_ONESHOT
#endif // __arm__

#define TAPPING_TERM 150

// Enable rapid switch from tap to hold, disables double tap hold auto-repeat.
#define TAPPING_FORCE_HOLD

#define HOLD_ON_OTHER_KEY_PRESS_PER_KEY
#define QUICK_TAP_TERM 120
#define QUICK_TAP_TERM_PER_KEY

#define ONESHOT_TAP_TOGGLE 3  /* Tapping this number of times holds the key until tapped once again. */
//#define ONESHOT_TIMEOUT 5000  /* Time (in ms) before the one shot key is released */

#define COMBO_TERM 20

#define SPECULATIVE_HOLD

#define FORCE_NKRO

#define USB_POLLING_INTERVAL_MS 1
#define QMK_KEYS_PER_SCAN 12

/* Charybdis-specific features. */

// #define EECONFIG_USER_DATA_SIZE 20

#ifdef POINTING_DEVICE_ENABLE
#undef CHARYBDIS_AUTO_SNIPING_ON_LAYER

// Automatically enable the pointer layer when moving the trackball.  See also:
// - `CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_TIMEOUT_MS`
// - `CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_THRESHOLD`

#define CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_ENABLE
#define CHARYBDIS_AUTO_POINTER_LAYER_TRIGGER_THRESHOLD 1

#define WHEEL_EXTENDED_REPORT
#define MOUSE_EXTENDED_REPORT
#define POINTING_DEVICE_HIRES_SCROLL_ENABLE
#define POINTING_DEVICE_HIRES_SCROLL_MULTIPLIER 12
#define POINTING_DEVICE_HIRES_SCROLL_EXPONENT 10
#define CHARYBDIS_DRAGSCROLL_BUFFER_SIZE 5
//#define POINTING_DEVICE_TASK_THROTTLE_MS 4

// #define CHARYBDIS_MINIMUM_DEFAULT_DPI 225
// #define CHARYBDIS_DEFAULT_DPI_CONFIG_STEP 75

// #define CHARYBDIS_MINIMUM_SNIPING_DPI 200
// #define CHARYBDIS_SNIPING_DPI_CONFIG_STEP 50

//#define MACCEL_DEBUG
/*
 * Requires enabling float support for printf!
 */
#undef PRINTF_SUPPORT_DECIMAL_SPECIFIERS
#define PRINTF_SUPPORT_DECIMAL_SPECIFIERS 1

// DEFAULTS
// #define MACCEL_TAKEOFF 2.0      // lower/higher value = curve takes off more smoothly/abruptly
// #define MACCEL_GROWTH_RATE 0.25 // lower/higher value = curve reaches its upper limit slower/faster
// #define MACCEL_OFFSET 2.2       // lower/higher value = acceleration kicks in earlier/later
// #define MACCEL_LIMIT 0.2        // lower limit of accel curve (minimum acceleration factor)

// old, can work
#define MACCEL_TAKEOFF 1.8      // lower/higher value = curve takes off more smoothly/abruptly
#define MACCEL_GROWTH_RATE 0.1 // lower/higher value = curve reaches its upper limit slower/faster
#define MACCEL_OFFSET 2.5       // lower/higher value = acceleration kicks in earlier/later
#define MACCEL_LIMIT 0.2       // lower limit of accel curve (minimum acceleration factor)

//#define MACCEL_TAKEOFF 0.6      // lower/higher value = curve takes off more smoothly/abruptly
//#define MACCEL_GROWTH_RATE 0.1 // lower/higher value = curve reaches its upper limit slower/faster
//#define MACCEL_OFFSET 6.6       // lower/higher value = acceleration kicks in earlier/later
//#define MACCEL_LIMIT 0.1       // lower limit of accel curve (minimum acceleration factor)

#endif // POINTING_DEVICE_ENABLE

//#define PERMISSIVE_HOLD

#define FLOW_TAP_TERM 85
#define CHORDAL_HOLD
