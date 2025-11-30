#include "note.h"
#include "ui.h"
// array to store note names for findNote
static char notes[12][3] = {
    "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
};

// finds and prints note of frequency and deviation in cents
HomeState findNote(float f) {

    HomeState s;
    memset(&s, 0, sizeof(HomeState));   // ensure all fields start zero
    s.valid = 1;

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

    // Convert Hz cents 
    float cents = 1200.0f * (log(f / f_left) / log(2.0f));

    s.cents = cents;

    // store note string 
    sprintf(s.note, "%s%d", notes[chosen_note], oct);

    // convert cents to pixel
    s.cents_px = UI_centsToPixel(cents);

    // choose color from tuning accuracy 
    float a = fabsf(cents);
    if (a < 5) {
        s.r = 0;   s.g = 255; s.b = 0;     // green = in tune
    }
    else if (a < 20) {
        s.r = 255; s.g = 255; s.b = 0;     // yellow = close
    }
    else {
        s.r = 255; s.g = 0;   s.b = 0;     // red = off
    }

    return s; 


    // === Print === -->keep for testing
    xil_printf("N:%s%d ", notes[chosen_note], oct);

    if (cents >= 0)
        xil_printf("D:+%d cents\r\n", (int)(cents + 0.5f));
    else
        xil_printf("D:%d cents\r\n", (int)(cents - 0.5f));  // round negative correctly
}
