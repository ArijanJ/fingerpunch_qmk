#include QMK_KEYBOARD_H

// Defines names for use in layer keycodes and the keymap
// enum layer_names {
//     _QWERTY,
//     _COLEMAK,
//     _LOWER,
//     _RAISE,
//     _ADJUST
// };

enum encoder_sides {
    ENC_LEFT = 0,
    ENC_RIGHT
};

enum keymap_layers {
    LAYER_BASE = 0,
    LAYER_QWERTY,
    LAYER_GAMING,
    LAYER_GAMING_ESDF,
    LAYER_GAMING_CANARY,
    LAYER_NAV,
    LAYER_NUMBER,
    LAYER_BROWSE,
    LAYER_MEDIA,
    LAYER_SYMBOL,
    LAYER_INTERNALS,
    LAYER_POINTER,
};

#define LOWER MO(_LOWER)
#define RAISE MO(_RAISE)

#define PT_Z LT(LAYER_POINTER, KC_Z)
#define PT_SLSH LT(LAYER_POINTER, KC_SLSH)
#define RSFT_GV MT(MOD_RSFT, KC_GRAVE)
#define NAV_ENT LT(LAYER_NAV, KC_ENTER)
#define NAV_SPC LT(LAYER_NAV, KC_SPC)
#define MEDIA LT(LAYER_MEDIA, KC_MPLY)
#define SYM_TAB LT(LAYER_SYMBOL, KC_TAB)
#define LC_TAB MT(MOD_LCTL, KC_TAB)
#define LC_CBSP MT(MOD_LCTL, KC_BSPC)
#define SFT_ESC MT(MOD_LSFT, KC_ESC)
#define SPC_NUM LT(LAYER_NUMBER, KC_SPC)
#define LGUI_CBS MT(MOD_LGUI, KC_BSPC)
#define LGUI_OSS MT(MOD_LGUI, KC_LSFT)
#define BROWSE_CBS LT(LAYER_BROWSE, KC_BSPC)
#define ESC_INT LT(LAYER_INTERNALS, KC_ESC)
#define ESC_SYM LT(LAYER_SYMBOL, KC_ESC)
#define MUTE_BROWSE LT(LAYER_BROWSE, KC_MUTE)
#define OSSPC_BROWSE LT(LAYER_BROWSE, KC_NO)

#define HRC(x) MT(MOD_LCTL, x)
#define HRS(x) MT(MOD_LSFT, x)
#define HRA(x) MT(MOD_LALT, x)
#define HRG(x) MT(MOD_LGUI, x)

enum custom_keycodes { FAKE_MOD = FP_SAFE_RANGE, ZOOM_MOD, GAMING_TOGGLE, GAMING_CANARY, HOLD_MOUSE_LAYER, OSS_THUMB, OSS_SPACE, SELWORD, MOUSE_LAYER_EXIT, DRGSCRL };
const uint16_t SELWD = SELWORD;

bool fake_mod_active  = false;
bool zoom_mod_active  = false;
bool is_gaming        = false;
bool hold_mouse_layer = false;
bool dragscrolling    = false;

uint16_t middle_click_scroll_buffer = 0;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
     [LAYER_BASE] = LAYOUT_ffkb(
     // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
          SYM_TAB,   KC_W,    KC_L,    KC_Y,    KC_P,    KC_B,      KC_Z,    KC_F,    KC_O,    KC_U, KC_QUOTE,  KC_MINUS,
     // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
     C(KC_BSPC), HRC(KC_C), HRS(KC_R), HRA(KC_S), HRG(KC_T), KC_G,  KC_M,   HRG(KC_N), HRA(KC_E),  HRS(KC_I),     HRC(KC_A),  KC_BSPC,
     // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
          SFT_ESC,   KC_Q,    KC_J,    KC_V,    KC_D,    KC_K,      KC_X,    KC_H,   KC_SLSH, KC_COMM,  KC_DOT, C(KC_BSPC),
     // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                     KC_MUTE,    OSS_THUMB,  NAV_SPC, OSSPC_BROWSE,      NAV_ENT,  SPC_NUM, C(KC_BSPC),   MEDIA
     //                            ╰───────────────────────────╯ ╰──────────────────╯
     ),

     [LAYER_QWERTY] = LAYOUT_ffkb(
     // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
          _______,    KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,       KC_Y,    KC_U,    KC_I,    KC_O,    KC_P, _______,
     // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
          _______, HRC(KC_A), HRS(KC_S), HRA(KC_D), HRG(KC_F),    KC_G,       KC_H,    HRG(KC_J),    HRA(KC_K),    HRS(KC_L), HRC(KC_SCLN), _______,
     // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
          _______,    KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,       KC_N,    KC_M, KC_COMM,  KC_DOT, KC_SLSH, _______,
     // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                          _______,    _______,  _______,   KC_LALT,      _______,  _______, XXXXXXX, _______
     //                            ╰───────────────────────────╯ ╰──────────────────╯
     ),

     [LAYER_GAMING] = LAYOUT_ffkb(
          // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
               ESC_SYM, KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,       KC_Y,    KC_U,    KC_I,    KC_O,    KC_P, KC_MINUS,
          // ├───────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────┤
               KC_LCTL, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,       KC_H,    KC_J,    KC_K,    KC_L, KC_SCLN, KC_BSPC,
          // ├───────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────┤
               KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,       KC_N,    KC_M, KC_COMM,  KC_DOT, KC_SLSH, DF(LAYER_GAMING_ESDF),
        // ╰─────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                   _______,    KC_LALT, KC_SPC, KC_TAB,        NAV_ENT, SPC_NUM, XXXXXXX, _______
          //                            ╰───────────────────────────╯ ╰──────────────────╯
          ),

     [LAYER_GAMING_ESDF] = LAYOUT_ffkb(
          // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
               ESC_SYM, KC_T,    KC_Q,    KC_W,    KC_E,    KC_R,       KC_Y,    KC_U,    KC_I,    KC_O,    KC_P, KC_MINUS,
          // ├───────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────┤
               KC_G, KC_LSFT,    KC_A,    KC_S,    KC_D,    KC_F,       KC_H,    KC_J,    KC_K,    KC_L, KC_SCLN, KC_BSPC,
          // ├───────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────┤
               KC_B, KC_LCTL,    KC_Z,    KC_X,    KC_C,    KC_V,       KC_N,    KC_M, KC_COMM,  KC_DOT, KC_SLSH, DF(LAYER_GAMING),
        // ╰─────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                   _______,    KC_LALT, KC_SPC, KC_TAB,        NAV_ENT, SPC_NUM, XXXXXXX, _______
          //                            ╰───────────────────────────╯ ╰──────────────────╯
          ),
     [LAYER_GAMING_CANARY] = LAYOUT_ffkb(
     // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
          SYM_TAB,   KC_W,    KC_L,    KC_Y,    KC_P,    KC_B,      KC_Z,    KC_F,    KC_O,    KC_U, KC_QUOTE,  KC_MINUS,
     // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
     C(KC_BSPC), KC_C, KC_R, KC_S, KC_T, KC_G,      KC_M,    KC_N,    KC_E,    KC_I,     KC_A,  KC_BSPC,
     // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
          SFT_ESC,   KC_Q,    KC_J,    KC_V,    KC_D,    KC_K,      KC_X,    KC_H,   KC_SLSH, KC_COMM,  KC_DOT, DF(LAYER_BASE),
     // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                   _______,    KC_LALT, KC_SPC, KC_TAB,        NAV_ENT, SPC_NUM, XXXXXXX, _______

     //                            ╰───────────────────────────╯ ╰──────────────────╯
     ),

     [LAYER_NAV] = LAYOUT_ffkb(
     // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
          _______, _______, SELWD, KC_END,  _______, _______,    _______,  _______,  _______, _______, KC_PGUP, MO(LAYER_INTERNALS),
     // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
          KC_LCTL, KC_HOME, C(KC_LEFT), KC_DEL, C(KC_RGHT),C(KC_HOME), KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, KC_BSPC, _______,
     // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
          KC_LSFT, _______, _______, _______,  EE_CLR, C(KC_LEFT), KC_PGDN,    _______,    _______,    _______, _______, _______,
     // ╰─────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                               _______,    _______, KC_LSFT, _______,    _______, KC_LSFT, XXXXXXX, _______
     //                            ╰───────────────────────────╯ ╰──────────────────╯
     ),

     [LAYER_NUMBER] = LAYOUT_ffkb(
     // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
          XXXXXXX, S(KC_1), S(KC_2), S(KC_3), S(KC_4), S(KC_5),      S(KC_6), S(KC_7),  S(KC_8), KC_MINUS, KC_F10, KC_F11,
     // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
          XXXXXXX, KC_1,    KC_2,    KC_3,    KC_4,    KC_5,         KC_6,     KC_7,    KC_8,    KC_9,    KC_0, _______,
     // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
          XXXXXXX, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5,                KC_F6, KC_F7, KC_F8, KC_F9, KC_DOT, KC_F12,
     // ╰─────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                               _______,    XXXXXXX, KC_SPC, _______,    KC_RSFT, _______, XXXXXXX, _______
     //                            ╰───────────────────────────╯ ╰──────────────────╯
     ),


     [LAYER_BROWSE] = LAYOUT_ffkb(
     // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
 MO(LAYER_INTERNALS), C(S(KC_1)), C(KC_W), XXXXXXX, XXXXXXX, XXXXXXX,       KC_F14, KC_F15, KC_F17, KC_F18, KC_F19, KC_F20,
     // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
          KC_LCTL, C(S(KC_2)), C(S(KC_3)), MEDIA, XXXXXXX, GAMING_TOGGLE, A(KC_LEFT), C(KC_TAB), C(S(KC_TAB)), A(KC_RGHT), XXXXXXX, XXXXXXX,
     // ├──────────────────────────────────────────────────────┤ ├───   ───────────────────────────────────────────────────┤
          FAKE_MOD, KC_LSFT, XXXXXXX, XXXXXXX, XXXXXXX, KC_MUTE,     KC_F13, KC_F21, XXXXXXX, XXXXXXX, KC_F22, KC_F16,
     // ╰─────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                               _______,    XXXXXXX, XXXXXXX, _______,    XXXXXXX, _______, XXXXXXX, _______
     //                            ╰───────────────────────────╯ ╰──────────────────╯
     ),

     [LAYER_MEDIA] = LAYOUT_ffkb(
          // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
               XXXXXXX, XXXXXXX, C(KC_MPRV), KC_VOLU, C(KC_MNXT), XXXXXXX,     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
          // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
               XXXXXXX, XXXXXXX, KC_MPRV, XXXXXXX, KC_MNXT, A(KC_MNXT),        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
          // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
               XXXXXXX, XXXXXXX, XXXXXXX, KC_VOLD, C(KC_MUTE), KC_MUTE,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        // ╰─────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                  _______,    XXXXXXX, XXXXXXX, _______,    XXXXXXX, _______, XXXXXXX, _______
          //                            ╰───────────────────────────╯ ╰──────────────────╯
          ),

     [LAYER_SYMBOL] = LAYOUT_ffkb(
          // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
               _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, GAMING_TOGGLE,  XXXXXXX, LSFT(KC_LBRC), KC_EQUAL, LSFT(KC_RBRC), KC_MINUS, KC_SCLN,
          // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
               _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, GAMING_TOGGLE,  KC_LBRC, LSFT(KC_9), KC_BSLS, LSFT(KC_0), KC_RBRC, S(KC_SCLN),
          // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
               _______, XXXXXXX, XXXXXXX, GAMING_CANARY, XXXXXXX, XXXXXXX,    XXXXXXX, XXXXXXX, XXXXXXX, KC_MINUS, S(KC_MINUS), KC_GRAVE,
          // ╰─────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                           _______,    XXXXXXX, KC_LSFT, _______,    KC_LSFT, _______, _______,      _______
          //                            ╰───────────────────────────╯ ╰──────────────────╯
          ),

     [LAYER_INTERNALS] = LAYOUT_ffkb(
          // ╭──────────────────────────────────────────────────────╮  ╭──────────────────────────────────────────────────────╮
               XXXXXXX,  XXXXXXX, XXXXXXX, XXXXXXX, FP_POINT_DPI_RESET, FP_POINT_DPI_UP,      XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
          // ├──────────────────────────────────────────────────────┤  ├──────────────────────────────────────────────────────┤
               XXXXXXX, FP_ACCEL_TOG, XXXXXXX, XXXXXXX, XXXXXXX, FP_POINT_DPI_DN, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
          // ├──────────────────────────────────────────────────────┤  ├──────────────────────────────────────────────────────┤
               XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, EE_CLR, QK_BOOT,        QK_BOOT, EE_CLR, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
          // ╰─────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                    _______,    XXXXXXX, XXXXXXX, _______,    XXXXXXX, _______, _______, _______
          //                            ╰───────────────────────────╯ ╰──────────────────╯
          ),


        [LAYER_POINTER] = LAYOUT_ffkb(
        // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
            _______,  _______, _______, _______, _______, _______,   KC_BTN1, DRGSCRL, KC_BTN2, _______,  _______, QK_BOOT,
        // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
            _______,  _______, _______, _______, _______, _______,    _______, _______, _______, _______, _______, _______,
            // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
            _______,  MOUSE_LAYER_EXIT, KC_BTN2, DRGSCRL, KC_BTN1, _______,    _______, _______, _______, _______, _______, _______,
            // _______,  _______, _______, _______, _______, _______,    _______, KC_BTN1, DRGSCRL, KC_BTN2, SNIPING, HOLD_MOUSE_LAYER,
        // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                 _______,    _______, _______, _______,    _______, _______, _______,     _______
        //                            ╰───────────────────────────╯ ╰──────────────────╯
        ),
};

#
const char chordal_hold_layout[MATRIX_ROWS][MATRIX_COLS] PROGMEM = LAYOUT_ffkb(
    'L', 'L', 'L', 'L', 'L', 'L', 'R', 'R', 'R', 'R', 'R', 'R',
    'L', 'L', 'L', 'L', 'L', 'L', 'R', 'R', 'R', 'R', 'R', 'R',
    'L', 'L', 'L', 'L', 'L', 'L', 'R', 'R', 'R', 'R', 'R', 'R',
            'L',   'L', 'L', 'L', 'R', 'R', 'R',   'R');


// layer_state_t layer_state_set_user(layer_state_t state) {
//   return update_tri_layer_state(state, _LOWER, _RAISE, _ADJUST);
// }

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (layer_state_is(LAYER_INTERNALS)) {
        if (index == ENC_RIGHT) {
            tap_code16(clockwise ? KC_BRIU : KC_BRID);
        }
    } else if (layer_state_is(LAYER_BROWSE)) {
        if (index == ENC_RIGHT) { // Shift can be pressed with Z on browse layer
            tap_code16(clockwise ? C(KC_EQUAL) : C(KC_MINUS));
            // tap_code(clockwise ? KC_MNXT : KC_MPRV);
        }
    } else if (layer_state_is(LAYER_NAV)) {
        if (index == ENC_RIGHT) {
            tap_code16(clockwise ? C(KC_RIGHT): C(KC_LEFT));
        }
    } else if (layer_state_is(LAYER_MEDIA)) {
        if (index == ENC_RIGHT) {
            tap_code16(clockwise ? KC_MNXT : KC_MPRV);
        } else {
            tap_code16(clockwise ? KC_BRIU : KC_BRID);
        }
    } else /* Any other layer */ {
        if (index == ENC_LEFT)
            tap_code(clockwise ? KC_VOLU : KC_VOLD);
            // tap_code(clockwise ? KC_MS_WH_DOWN : KC_MS_WH_UP);
        else if (index == ENC_RIGHT)
            tap_code(clockwise ? KC_VOLU : KC_VOLD);
    }

    return false;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // if (!process_sentence_case(keycode, record)) { return false; }
    // if (!process_select_word(keycode, record)) { return false; }

    // if (keycode == FAKE_MOD) {
    //     if (zoom_mod_active) {
    //         if (record->event.pressed) { // act as shift instead
    //             register_mods(MOD_BIT(KC_LSFT));
    //         } else {
    //             unregister_mods(MOD_BIT(KC_LSFT));
    //         }
    //         return false;
    //     }
    //     fake_mod_active = record->event.pressed;
    //     return false;
    // } else
    // if (keycode == ZOOM_MOD) {
    //     zoom_mod_active = record->event.pressed;
    //     if (!record->event.pressed) {
    //         unregister_mods(MOD_BIT(KC_LSFT));
    //     }
    //     return false;
    // } else
    // if (keycode == HOLD_MOUSE_LAYER) {
    //     hold_mouse_layer = record->event.pressed;
    //     return false;
    // }

    if (keycode == OSS_THUMB) {
        if (record->event.pressed) {                      // hijack tap only
            if (get_oneshot_mods() == MOD_BIT(KC_LSFT)) { // second press
                caps_word_on();
            } else {
                set_oneshot_mods(MOD_BIT(KC_LSFT));
            }
            return false;
        }
    }

    static uint32_t dragscroll_timer = 0;
    if (keycode == DRGSCRL) {
        // charybdis_set_pointer_sniping_enabled(record->event.pressed);
        dragscrolling = record->event.pressed;
        if (record->event.pressed) {
            dragscroll_timer           = timer_read32(); // start timer on dragscroll down
            middle_click_scroll_buffer = 0;
            register_code16(KC_BTN8);
        } else { // on release
            if ((timer_elapsed32(dragscroll_timer) < 120) && middle_click_scroll_buffer < 10) {
                tap_code16(KC_BTN3);
                middle_click_scroll_buffer = 0;
            }
            dragscroll_timer = 0; // nullify timer on dragscroll release
            unregister_code16(KC_BTN8);
        }
        // charybdis_set_pointer_dragscroll_enabled(record->event.pressed);
        return false;
    }


    if (keycode == LT(LAYER_BROWSE, KC_NO)) {
        if (record->tap.count && record->event.pressed) {
            tap_code16(KC_SPC);
            set_oneshot_mods(MOD_BIT(KC_LSFT));
            return false;
        }
        return true;
    }

    if (keycode == NAV_ENT) {
        if (is_gaming && record->tap.count && record->event.pressed) {
            tap_code_delay(KC_ENTER, 16);
            return false;
        }
    }

    if (keycode == GAMING_TOGGLE || keycode == GAMING_CANARY) {
        if (record->event.pressed) {
            layer_off(LAYER_NAV);
            layer_off(LAYER_INTERNALS);
            layer_off(LAYER_SYMBOL);
            if (is_gaming) {
                // uprintf("process_record_user: deactivating layer LAYER_GAMING\n");
                set_single_default_layer(LAYER_BASE);
            } else {
                // uprintf("process_record_user: activating layer LAYER_GAMING\n");
                if(keycode == GAMING_CANARY)
                    set_single_default_layer(LAYER_GAMING_CANARY);
                else
                    set_single_default_layer(LAYER_GAMING);
            }
            is_gaming = !is_gaming;
            combo_toggle();
        }
        return false;
    }

    if ((keycode == LGUI_CBS || keycode == BROWSE_CBS || keycode == LC_CBSP) && record->event.pressed) {
        if (record->tap.count && record->event.pressed) {
            tap_code16(C(KC_BSPC));
            return false;
        }
    }
    if ((keycode == LGUI_OSS) && record->event.pressed) {
        if (record->tap.count && record->event.pressed) {
            set_oneshot_mods(MOD_BIT(KC_LSFT));
            return false;
        }
    }

    if (layer_state_is(LAYER_NUMBER) && record->event.pressed) {
        clear_weak_mods(); // Clear any residual weak shift mod from the previous key (work around qmk#17569).
        send_keyboard_report();
    }

    if (layer_state_is(LAYER_POINTER)) {
        if (keycode != KC_BTN1 && keycode != KC_BTN2 && keycode != KC_BTN3 && keycode != DRGSCRL) {
            if (/*keycode < QK_KB_0 && */ !hold_mouse_layer) {
                uprintf("process_record_user: keycode %04X on pointer layer\n", keycode);
                // if (!IS_MODIFIER_KEYCODE(keycode)) { // mod+mouse is also acceptable // or is it? mod+workspace e.g.
                // auto_pointer_layer_timer = 0;
                layer_off(LAYER_POINTER); // disable mouse layer if non-mouse key pressed
                                          // }
                if (keycode == MOUSE_LAYER_EXIT) return false;
            }
        } else {
            // auto_pointer_layer_timer = timer_read(); // reset timer on mouse press
        }
    }

    if (record->event.pressed) {
        // uprintf("process_record_user: keycode %04X pressed, mods: %04X\n", keycode, get_mods());
        if ((get_mods() != 0) && (!(get_mods() & MOD_MASK_SHIFT))) {
            //uprintf("process_record_user: activating layer LAYER_QWERTY\n");
            layer_on(LAYER_QWERTY);
        } else { // regular other key
            if (layer_state_is(LAYER_QWERTY) && (get_mods() == 0 || get_mods() == MOD_MASK_SHIFT)) {
                //uprintf("process_record_user: deactivating layer LAYER_QWERTY\n");
                layer_off(LAYER_QWERTY);
            } else if (get_mods() != 0 && get_mods() != 2) {
                //uprintf("process_record_user: [2] activating layer LAYER_QWERTY\n");
                layer_on(LAYER_QWERTY);
            }
        }
    } else {
        if (get_mods() == 0 || get_mods() == MOD_MASK_SHIFT) {
            layer_off(LAYER_QWERTY);
        }
    }

    return true;
}

bool get_hold_on_other_key_press(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case LC_TAB:
        case SYM_TAB:
        case LGUI_OSS:
        case LGUI_CBS:
        case BROWSE_CBS:
        case SFT_ESC:
        case MEDIA:
        case OSSPC_BROWSE:
            return true; // Immediately select the hold action when another key is pressed.
        case NAV_SPC:
        case SPC_NUM:
            return false;
        default:
            return false;
    }
}

uint16_t get_quick_tap_term(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case LC_CBSP: // allow repeat on double-tap
            return QUICK_TAP_TERM;
        case ESC_SYM: // allow escape hold?
            return QUICK_TAP_TERM;
        case NAV_SPC: // don't repeat spaces
        case SPC_NUM:
            return 0;
        default:
            return QUICK_TAP_TERM;
    }
}



bool get_chordal_hold(uint16_t tap_hold_keycode, keyrecord_t *tap_hold_record, uint16_t other_keycode, keyrecord_t *other_record) {
    // Exceptionally allow some one-handed chords for hotkeys.
    switch (tap_hold_keycode) {
        case NAV_SPC:
        case OSSPC_BROWSE:
        case SFT_ESC:
        case MEDIA:
        case NAV_ENT:
        case SPC_NUM:
            return true;
            break;
    }
    // Otherwise defer to the opposite hands rule.
    return get_chordal_hold_default(tap_hold_record, other_record);
}

bool get_speculative_hold(uint16_t keycode, keyrecord_t* record) {
    switch (keycode) { // These keys may be speculatively held.
        case LCTL_T(KC_ESC):
        case LSFT_T(KC_Z):
        case RSFT_T(KC_SLSH):
            return true;
    }
    return false; // Disable otherwise.
}

const key_override_t brightness_up_override   = ko_make_basic(MOD_MASK_SHIFT, KC_VOLU, KC_BRIU);
const key_override_t brightness_down_override = ko_make_basic(MOD_MASK_SHIFT, KC_VOLD, KC_BRID);

// This globally defines all key overrides to be used
const key_override_t *key_overrides[] = {&brightness_up_override, &brightness_down_override};

bool is_flow_tap_key(uint16_t keycode) {
    if ((get_mods() & (MOD_MASK_CG | MOD_BIT_LALT)) != 0) {
        return false; // Disable Flow Tap on hotkeys.
    }
    switch (keycode) {
        case NAV_SPC:
        case SPC_NUM:
            return false;
    }
    switch (get_tap_keycode(keycode)) {
        case KC_SPC:
        case KC_A ... KC_Z:
        case KC_DOT:
        case KC_COMM:
        case KC_SCLN:
        case KC_SLSH:
            return true;
    }
    return false;
}

const uint16_t PROGMEM question_mark_combo[] = {KC_SLSH, KC_COMMA, COMBO_END};
const uint16_t PROGMEM backspace_combo[]     = {HRA(KC_E), HRS(KC_I), COMBO_END};
const uint16_t PROGMEM c_backspace_combo[]   = {HRG(KC_N), HRA(KC_E), HRS(KC_I), COMBO_END};
combo_t                key_combos[]          = {
    COMBO(question_mark_combo, S(KC_SLSH)),
    // COMBO(backspace_combo, KC_BSPC),
    // COMBO(c_backspace_combo, C(KC_BSPC)),
    // COMBO(test_combo2, LCTL(KC_Z)), // keycodes with modifiers are possible too!
};






static int16_t custom_trackball_counter = 0;
#define CUSTOM_TRACKBALL_TRIGGER_DELTA 40

#define SCROLLING_DIRECTION_TRIGGER_DELTA 20
typedef enum scrolling_state {
    SCROLLING_UNDECIDED,
    SCROLLING_HORIZONTAL,
    SCROLLING_VERTICAL,
} scrolling_state_t;

scrolling_state_t scroll_state        = SCROLLING_UNDECIDED;
report_mouse_t    scroll_state_buffer = {};

int16_t horizontal_buffer = 0;
int16_t vertical_buffer   = 0;

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    // Lower sensitivity while scrolling, I guess - doesn't work, reports are often 1 or -1
    // if (dragscrolling) {
    //     mouse_report.x /= 2;
    //     mouse_report.y /= 2;
    // }

    report_mouse_t accelerated_report = mouse_report;

    // if (accelerated_report.x != 0 || accelerated_report.y != 0) {
    //     uprintf("Accelerated mouse report: x=%4d, y=%4d, buttons=%04X\n", accelerated_report.x, accelerated_report.y, accelerated_report.buttons);
    //     uprintf("Custom trackball counter: %d\n", custom_trackball_counter);
    // }

    if (layer_state_is(LAYER_BROWSE) || layer_state_is(LAYER_NAV)) {
        // Decide direction
        horizontal_buffer += accelerated_report.x;
        vertical_buffer += accelerated_report.y;

        if (abs(horizontal_buffer) > SCROLLING_DIRECTION_TRIGGER_DELTA) {
            if (scroll_state != SCROLLING_HORIZONTAL) {
                custom_trackball_counter = 0;
            }
            scroll_state      = SCROLLING_HORIZONTAL;
            vertical_buffer   = 0;
            horizontal_buffer = SCROLLING_DIRECTION_TRIGGER_DELTA / 2; // prioritize same dir
        } else if (abs(vertical_buffer) > SCROLLING_DIRECTION_TRIGGER_DELTA) {
            if (scroll_state != SCROLLING_VERTICAL) {
                custom_trackball_counter = 0;
            }
            scroll_state      = SCROLLING_VERTICAL;
            horizontal_buffer = 0;
            vertical_buffer   = SCROLLING_DIRECTION_TRIGGER_DELTA / 2; // prioritize same dir
        }

        if (scroll_state == SCROLLING_UNDECIDED) {
            accelerated_report.y = 0;
            accelerated_report.x = 0;
            return accelerated_report;
        }

        // if (scroll_state == SCROLLING_HORIZONTAL) {
        //     uprintf("Scrolling direction: horizontal\n");
        // } else if (scroll_state == SCROLLING_VERTICAL) {
        //     uprintf("Scrolling direction: vertical\n");
        // } else {
        //     uprintf("Scrolling direction: undecided\n");
        // }

        if (scroll_state == SCROLLING_HORIZONTAL) {
            custom_trackball_counter += accelerated_report.x;
        } else if (scroll_state == SCROLLING_VERTICAL) {
            custom_trackball_counter += accelerated_report.y;
        }

        if (abs(custom_trackball_counter) >= CUSTOM_TRACKBALL_TRIGGER_DELTA) {
            // Trigger the action
            if (scroll_state == SCROLLING_HORIZONTAL) {
                if (accelerated_report.x > 0) {
                    if (layer_state_is(LAYER_NAV)) {
                        tap_code(KC_RIGHT);
                    } else {
                        tap_code(KC_VOLU);
                    }
                } else if (accelerated_report.x < 0) {
                    if (layer_state_is(LAYER_NAV)) {
                        tap_code(KC_LEFT);
                    } else {
                        tap_code(KC_VOLD);
                    }
                }
            } else if (scroll_state == SCROLLING_VERTICAL) {
                if (accelerated_report.y > 0) {
                    if (fake_mod_active) {
                        tap_code(KC_BRID);
                    } else if (zoom_mod_active) {
                        tap_code16(C(KC_EQUAL));
                    } else if (layer_state_is(LAYER_NAV)) {
                        tap_code(KC_DOWN);
                    } else {
                        tap_code(KC_VOLD);
                    }
                } else if (accelerated_report.y < 0) {
                    if (fake_mod_active) {
                        tap_code(KC_BRIU);
                    } else if (zoom_mod_active) {
                        tap_code16(C(KC_MINUS));
                    } else if (layer_state_is(LAYER_NAV)) {
                        tap_code(KC_UP);
                    } else {
                        tap_code(KC_VOLU);
                    }
                }
            }

            if (custom_trackball_counter < 0)
                custom_trackball_counter += CUSTOM_TRACKBALL_TRIGGER_DELTA;
            else
                custom_trackball_counter -= CUSTOM_TRACKBALL_TRIGGER_DELTA;

            // reset the buffer but divide the curr dir by two to prioritize same direction
            vertical_buffer   = 0;
            horizontal_buffer = 0;
            if (scroll_state == SCROLLING_HORIZONTAL) {
                horizontal_buffer = SCROLLING_DIRECTION_TRIGGER_DELTA / 2;
            } else if (scroll_state == SCROLLING_VERTICAL) {
                vertical_buffer = SCROLLING_DIRECTION_TRIGGER_DELTA / 2;
            }
        }

        accelerated_report.y = 0;
        accelerated_report.x = 0;
        return accelerated_report;
    }

    return mouse_report;
}
