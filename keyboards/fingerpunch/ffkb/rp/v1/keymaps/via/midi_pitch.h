#pragma once

#include <stdbool.h>
#include <stdint.h>

#define MIDI_SCALE_NOTE_COUNT 10
#define MIDI_PITCH_NOTE_COUNT (2 * MIDI_SCALE_NOTE_COUNT)
#define MIDI_PITCH_NONE UINT8_MAX

uint8_t midi_pitch_root(void);
uint8_t midi_pitch_note(uint8_t index);
bool midi_pitch_change_root(bool up);
bool midi_pitch_change_octave(bool up);
