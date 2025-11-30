#ifndef UI_H
#define UI_H

#include <stdint.h>

// page type
typedef enum {
    UI_HOME = 0,
    UI_DEBUG = 1,
    UI_SETTINGS = 2
} UIScreen;

//HOME SCREEN
typedef struct {
    int valid;          // optional? 0 if no note, 1 if note detected
    char note[4];       // string for displayed note (max would be C#3 and null terminator)
    float cents;        // -50 .. +50
    int cents_px;       // converted to pixel position
    uint8_t r, g, b;    // color of the bar (green if in tune, yellow if slightly off, red if very off)
} HomeState;

//DEBUG SCREEN - WIP

// #define UI_MAX_BARS   64

// typedef struct {
//     int n_bars;                     // how many bins shown
//     int bar_height[UI_MAX_BARS];   // bar graph heights
//     float max_freq;
//     float bin_spacing;
// } DebugState;

// Initialize static UI elements (fonts, background, labels) -> just the default display
void UI_init(void);

// Change to a different screen
void UI_setScreen(UIScreen screen);

// Update "back buffer" states (called from FFT / tuner logic)
void UI_updateHomeState(const HomeState *src);
// void UI_updateDebugState(const DebugState *src);

// Draw current screen (called every 20ms w/ interrupt?)
void UI_draw(void);

// some way to find the pixel that corresponds to which cent on the bar
int UI_centsToPixel(float cents);

#endif
