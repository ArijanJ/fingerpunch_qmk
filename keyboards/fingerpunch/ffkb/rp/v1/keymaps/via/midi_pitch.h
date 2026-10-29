#pragma once

#include <stdbool.h>
#include <stdint.h>

#define MIDI_SCALE_NOTE_COUNT 10
#define MIDI_PITCH_NOTE_COUNT (2 * MIDI_SCALE_NOTE_COUNT)
#define MIDI_PITCH_NONE UINT8_MAX
#define MIDI_MAX_CHORD_NOTES 4
#define MIDI_PLAY_KEY_COUNT (MIDI_PITCH_NOTE_COUNT + MIDI_SCALE_NOTE_COUNT)

typedef enum { MIDI_CHORD_TRIAD, MIDI_CHORD_SEVENTH, MIDI_CHORD_DOMINANT, MIDI_CHORD_TYPE_COUNT } midi_chord_type_t;

uint8_t midi_pitch_root(void);
uint8_t midi_pitch_note(uint8_t index);
bool midi_pitch_change_root(bool up);
bool midi_pitch_change_octave(bool up);
bool midi_chord_is_minor(void);
bool midi_chord_toggle_minor(void);
midi_chord_type_t midi_chord_type(void);
midi_chord_type_t midi_chord_cycle_type(void);
uint8_t midi_chord_notes(uint8_t degree, uint8_t notes[MIDI_MAX_CHORD_NOTES]);
