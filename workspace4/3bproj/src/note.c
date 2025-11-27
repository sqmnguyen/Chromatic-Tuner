#include "note.h"

// array to store note names for findNote
static char notes[12][3] = {
    "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
};

// finds and prints note of frequency and deviation in cents
void findNote(float f) {

    float c = 261.63f;     // C4 reference
    float r;
    int oct = 4;
    int note = 0;

    // === Find octave ===
    if (f >= c) {
        while (f > c * 2) {
            c *= 2;
            oct++;
        }
    } else {
        while (f < c) {
            c /= 2;
            oct--;
        }
    }

    // === Find nearest note ===
    // c is now C<oct>
    r = c * root2;    // next semitone

    while (f > r) {
        c = r;
        r = r * root2;
        note++;
    }

    // --- Determine closest note ---
    float f_left  = c;
    float f_right = r;
    int chosen_note;

    float dist_left  = f - f_left;
    float dist_right = f_right - f;

    if (dist_left <= dist_right) {
        chosen_note = note;
    } else {
        chosen_note = note + 1;
        if (chosen_note >= 12) chosen_note = 0;
        f_left = f_right;  // reference frequency becomes the right note
    }

    // === Convert Hz cents ===
    float cents = 1200.0f * (log(f / f_left) / log(2.0f));

    // === Print ===
    xil_printf("N:%s%d ", notes[chosen_note], oct);

    if (cents >= 0)
        xil_printf("D:+%d cents\r\n", (int)(cents + 0.5f));
    else
        xil_printf("D:%d cents\r\n", (int)(cents - 0.5f));  // round negative correctly
}
