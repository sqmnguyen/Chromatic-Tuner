#include "ui.h"
#include "lcd.h"
#include <string.h>     // memcpy

// =======================================
// INTERNAL BUFFERS
// =======================================

static HomeState front_home;
static HomeState back_home;

static UIScreen currentScreen = UI_HOME;

// 20 ms update limiter
static uint32_t lastUpdateMs = 0;

// Forward declarations
static void draw_home(void);


// =======================================
// UI INIT — runs ONCE at boot
// =======================================

void UI_init(void)
{
    clrScr();               // clear whole display

    // Draw static elements that never change
    setColor(255,255,255);
    lcdPrint("Tuner", 10, 5);

    // Tick marks background (not dynamic)
    setColor(80,80,80);
    // static tick marks: -50 to +50 every 5 cents
    for (int c = -50; c <= 50; c += 5) {
        int x = 20 + (c + 50) * (200.0f / 100.0f);
        int h = (c % 25 == 0) ? 10 : 5;
        fillRect(x, 40, x+1, 40+h);
    }

    // Bottom menu (static)
    setColor(150,150,150);
    lcdPrint("[DEBUG]  [HOME]", 10, 220);

    // Default front/back state
    memset(&front_home, 0, sizeof(HomeState));
    memset(&back_home, 0, sizeof(HomeState));

    back_home.valid = 0;  // start as "no sound"
}


// =======================================
// SET SCREEN MODE
// =======================================

void UI_setScreen(UIScreen s)
{
    currentScreen = s;
    clrScr();

    if (s == UI_HOME) {
        UI_init();  // redraw static home elements
    }
}


// =======================================
// UPDATE HOME STATE (from DSP)
// Called at any time by your FFT/DSP code
// =======================================

void UI_updateHomeState(const HomeState *src)
{
    back_home = *src;    
}

int UI_centsToPixel(float cents)
{
    if (cents >  50) cents = 50;
    if (cents < -50) cents = -50;
    return 20 + (int)((cents + 50) * (200.0f / 100.0f));
}


// this will draw all the dynamic stuff (called every set time interval )
static void draw_home(void)
{
    // Copy stable snapshot
    front_home = back_home;

    // Dynamic area clear
    fillRect(0, 60, 240, 200);

    if (!front_home.valid) {
        setColor(255,255,255);
        lcdPrint("--", 105, 100);
        return;
    }

    // 1. Print note name
    setColor(255,255,255);
    lcdPrint(front_home.note, 105, 80);

    // 2. Print cents value
    char buf[16];
    if (front_home.cents >= 0)
        sprintf(buf, "+%.1f", front_home.cents);
    else
        sprintf(buf, "%.1f", front_home.cents);

    lcdPrint(buf, 100, 110);

    // 3. Draw color band
    setColor(front_home.r, front_home.g, front_home.b);
    fillRect(20, 140, 220, 150);

    // 4. Draw needle
    int px = front_home.cents_px;
    setColor(front_home.r, front_home.g, front_home.b);
    fillRect(px-2, 120, px+2, 160);
}


// =======================================
// PUBLIC DRAW FUNCTION
// called every 20ms by main loop or FSM
// =======================================

void UI_draw(void)
{
    uint32_t now = millis();       
    if (now - lastUpdateMs < 20)   // 20 ms limiter
        return;

    lastUpdateMs = now;

    if (currentScreen == UI_HOME) {
        draw_home();
    }
}
