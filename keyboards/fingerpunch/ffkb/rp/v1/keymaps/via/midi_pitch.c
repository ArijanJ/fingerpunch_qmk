#include "midi_pitch.h"

/* Scientific octave convention: MIDI 60 = C4. Volatile until reboot. */
static uint8_t root_note = 60; /* playable roots C-1..B7 */
/* Transposable piano: slots 0..9 are white keys; 10..19 are the upper row.
 * Blank upper slots preserve the piano's two/three black-key grouping. */
static const uint8_t intervals[MIDI_PITCH_NOTE_COUNT] = {
    0, 2, 4, 5, 7, 9, 11, 12, 14, 16,
    MIDI_PITCH_NONE, 1, 3, MIDI_PITCH_NONE, 6, 8, 10, MIDI_PITCH_NONE, 13, 15,
};

uint8_t midi_pitch_root(void) {
    return root_note;
}

uint8_t midi_pitch_note(uint8_t index) {
    if (index >= MIDI_PITCH_NOTE_COUNT || intervals[index] == MIDI_PITCH_NONE) return MIDI_PITCH_NONE;
    return midi_pitch_root() + intervals[index];
}

bool midi_pitch_change_root(bool up) {
    /* Move chromatically across B/C, rather than jumping eleven semitones. */
    if ((up && root_note == 107) || (!up && root_note == 0)) return false;
    root_note += up ? 1 : -1;
    return true;
}

bool midi_pitch_change_octave(bool up) {
    /* Keep all ten scale notes within MIDI's 0..127 range for every root. */
    if ((up && root_note > 95) || (!up && root_note < 12)) return false;
    root_note += up ? 12 : -12;
    return true;
}
