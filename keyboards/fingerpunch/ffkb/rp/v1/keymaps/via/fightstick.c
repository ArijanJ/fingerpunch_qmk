// Copyright 2022 @waffle87
// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H
#include "wait.h"

#include "custom_keycodes.h"

// Joystick Config
joystick_config_t joystick_axes[JOYSTICK_AXIS_COUNT] = {
	JOYSTICK_AXIS_VIRTUAL,
	JOYSTICK_AXIS_VIRTUAL,
	JOYSTICK_AXIS_VIRTUAL,
	JOYSTICK_AXIS_VIRTUAL,
	JOYSTICK_AXIS_VIRTUAL,
	JOYSTICK_AXIS_VIRTUAL,
};

bool LSU_STATE = false;
bool LSD_STATE = false;
bool LSL_STATE = false;
bool LSR_STATE = false;
bool RSU_STATE = false;
bool RSD_STATE = false;
bool RSL_STATE = false;
bool RSR_STATE = false;
bool DPU_STATE = false;
bool DPD_STATE = false;
bool DPL_STATE = false;
bool DPR_STATE = false;

//#include "print.h"

bool process_fightstick_keycode(uint16_t keycode, keyrecord_t *record) {
    //uprintf("got keycode %d\n", keycode);
	switch (keycode) {
		case GC_LSU:
            //print("got GC_LSU\n");
			if (record->event.pressed) {
				LSU_STATE = true;
				joystick_set_axis(1, -127);
			} else {
				LSU_STATE = false;
				if (LSD_STATE) {
					joystick_set_axis(1, 127);
				} else {
					joystick_set_axis(1, 0);
				}
			}
			return false;
		case GC_LSD:
			if (record->event.pressed) {
				LSD_STATE = true;
				if (!LSU_STATE) {
					joystick_set_axis(1, 127);
				}
			} else {
				LSD_STATE = false;
				if (!LSU_STATE) {
					joystick_set_axis(1, 0);
				}
			}
			return false;
		case GC_LSL:
			if (record->event.pressed) {
				LSL_STATE = true;
				joystick_set_axis(0, -127);
			} else {
				LSL_STATE = false;
				if (LSR_STATE) {
					joystick_set_axis(0, 127);
				} else {
					joystick_set_axis(0, 0);
				}
			}
			return false;
		case GC_LSR:
			if (record->event.pressed) {
				LSR_STATE = true;
				joystick_set_axis(0, 127);
			} else {
				LSR_STATE = false;
				if (LSL_STATE) {
					joystick_set_axis(0, -127);
				} else {
					joystick_set_axis(0, 0);
				}
			}
			return false;
		case GC_DPU:
			if (record->event.pressed) {
				DPU_STATE = true;
				if (DPD_STATE) {
					unregister_joystick_button(16);
				}
				register_joystick_button(18);
			} else {
				DPU_STATE = false;
				unregister_joystick_button(18);
			}
			return false;
		case GC_DPD:
			if (record->event.pressed) {
				DPD_STATE = true;
				if (!DPU_STATE) {
					register_joystick_button(16);
				}
			} else {
				DPD_STATE = false;
				unregister_joystick_button(16);
			}
			return false;
		case GC_DPL:
			if (record->event.pressed) {
				DPL_STATE = true;
				if (DPR_STATE) {
					unregister_joystick_button(17);
				} else {
					register_joystick_button(15);
				}
			} else {
				DPL_STATE = false;
				unregister_joystick_button(15);
				if (DPR_STATE) {
					register_joystick_button(17);
				}
			}
			return false;
		case GC_DPR:
			if (record->event.pressed) {
				DPR_STATE = true;
				if (DPL_STATE) {
					unregister_joystick_button(15);
				} else {
					register_joystick_button(17);
				}
			} else {
				DPR_STATE = false;
				unregister_joystick_button(17);
				if (DPL_STATE) {
					register_joystick_button(15);
				}
			}
			return false;
		case GC_RSU:
			if (record->event.pressed) {
				RSU_STATE = true;
				joystick_set_axis(4, -127);
			} else {
				RSU_STATE = false;
				if (RSD_STATE) {
					joystick_set_axis(4, 127);
				} else {
					joystick_set_axis(4, 0);
				}
			}
			return false;
		case GC_RSD:
			if (record->event.pressed) {
				RSD_STATE = true;
				if (!RSU_STATE) {
					joystick_set_axis(4, 127);
				}
			} else {
				RSD_STATE = false;
				if (RSU_STATE) {
					joystick_set_axis(4, -127);
				} else {
					joystick_set_axis(4, 0);
				}
			}
			return false;
		case GC_RSL:
			if (record->event.pressed) {
				RSL_STATE = true;
				joystick_set_axis(3, -127);
			} else {
				RSL_STATE = false;
				if (RSR_STATE) {
					joystick_set_axis(3, 127);
				} else {
					joystick_set_axis(3, 0);
				}
			}
			return false;
		case GC_RSR:
			if (record->event.pressed) {
				RSR_STATE = true;
				joystick_set_axis(3, 127);
			} else {
				RSR_STATE = false;
				if (RSL_STATE) {
					joystick_set_axis(3, -127);
				} else {
					joystick_set_axis(3, 0);
				}
			}
			return false;
		case GC_CRO:
			if (record->event.pressed) {
				register_joystick_button(0);
			} else {
				unregister_joystick_button(0);
			}
			return false;
		case GC_CIR:
			if (record->event.pressed) {
				register_joystick_button(1);
			} else {
				unregister_joystick_button(1);
			}
			return false;
		case GC_SQU:
			if (record->event.pressed) {
				// for (uint8_t i = 1; i <= 16; i++) { // loop through first 16 keys for debugging/mapping
				// 	register_joystick_button(i);
				// 	wait_ms(1000);
				// 	unregister_joystick_button(i);
				// 	wait_ms(200);
				// }
				register_joystick_button(4);
			} else {
				unregister_joystick_button(4);
			}
			return false;
		case GC_TRI:
			if (record->event.pressed) {
				register_joystick_button(3);
			} else {
				unregister_joystick_button(3);
			}
			return false;
		case GC_L1:
			if (record->event.pressed) {
				register_joystick_button(6);
			} else {
				unregister_joystick_button(6);
			}
			return false;
		case GC_R1:
			if (record->event.pressed) {
				register_joystick_button(7);
			} else {
				unregister_joystick_button(7);
			}
			return false;
		case GC_L2: // this and lower are probably wrong
			if (record->event.pressed) {
				register_joystick_button(8);
			} else {
				unregister_joystick_button(8);
			}
			return false;
		case GC_R2:
			if (record->event.pressed) {
				register_joystick_button(9);
			} else {
				unregister_joystick_button(9);
			}
			return false;
		case GC_SEL:
			if (record->event.pressed) {
				register_joystick_button(10);
			} else {
				unregister_joystick_button(10);
			}
			return false;
		case GC_STA:
			if (record->event.pressed) {
				register_joystick_button(11);
			} else {
				unregister_joystick_button(11);
			}
			return false;
		case GC_L3:
			if (record->event.pressed) {
				register_joystick_button(13);
			} else {
				unregister_joystick_button(13);
			}
			return false;
		case GC_R3:
			if (record->event.pressed) {
				register_joystick_button(14);
			} else {
				unregister_joystick_button(14);
			}
			return false;
		case GC_HOM:
			if (record->event.pressed) {
				register_joystick_button(12);
			} else {
				unregister_joystick_button(12);
			}
			return false;
		default:
			return true;
	}
};
