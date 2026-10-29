#include "midi_pitch.h"

/* Scientific octave convention: MIDI 60 = C4. Volatile until reboot. */
static uint8_t root_note = 60; /* playable roots C-1..B7 */
static bool chord_minor;
static midi_chord_type_t chord_type;
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

bool midi_chord_is_minor(void) { return chord_minor; }
bool midi_chord_toggle_minor(void) { return chord_minor = !chord_minor; }
midi_chord_type_t midi_chord_type(void) { return chord_type; }
midi_chord_type_t midi_chord_cycle_type(void) {
    chord_type = (chord_type + 1) % MIDI_CHORD_TYPE_COUNT;
    return chord_type;
}

static uint8_t scale_offset(uint8_t degree) {
    static const uint8_t major[7] = {0, 2, 4, 5, 7, 9, 11};
    static const uint8_t minor[7] = {0, 2, 3, 5, 7, 8, 10};
    return (degree / 7) * 12 + (chord_minor ? minor : major)[degree % 7];
}

uint8_t midi_chord_notes(uint8_t degree, uint8_t notes[MIDI_MAX_CHORD_NOTES]) {
    if (degree >= MIDI_SCALE_NOTE_COUNT) return 0;
    uint8_t count = chord_type == MIDI_CHORD_TRIAD ? 3 : 4;
    uint16_t pitches[MIDI_MAX_CHORD_NOTES];
    for (uint8_t voice = 0; voice < count; ++voice) {
        if (chord_type == MIDI_CHORD_DOMINANT) {
            const uint8_t shape[4] = {0, chord_minor ? 3 : 4, 7, 10};
            pitches[voice] = root_note + scale_offset(degree) + shape[voice];
        } else {
            /* Diatonic stacked thirds in major or natural minor. */
            pitches[voice] = root_note + scale_offset(degree + voice * 2);
        }
    }
    /* Move the whole voicing down an octave near MIDI's upper limit. */
    uint8_t shift = pitches[count - 1] > 127 ? 12 : 0;
    for (uint8_t voice = 0; voice < count; ++voice) notes[voice] = pitches[voice] - shift;
    return count;
}
