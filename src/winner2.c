#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "timer.h"     // millis()
#include "gfx.h"       // gfx_clear, gfx_header, gfx_text2, gfx_bar
#include "project.h"   // colors

// Geometry for the two winner bars
#define BX   6
#define BW   116
#define BH   12
#define BY1  46
#define BY2  86

#define ANIM_MS         3000u  // time to ramp 0 -> target
#define SCREEN_HOLD_MS  6000u  // total time to keep winner screen

static float    g_p1_target_hz = 0.0f;
static float    g_p2_target_hz = 0.0f;
static uint8_t  g_p1_px        = 0;
static uint8_t  g_p2_px        = 0;
static uint32_t g_t0_ms        = 0;

static float clampf(float v, float lo, float hi){
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void winner2_start(float p1_avg_hz, float p2_avg_hz)
{
    g_p1_target_hz = clampf(p1_avg_hz, 0.0f, 250.0f);
    g_p2_target_hz = clampf(p2_avg_hz, 0.0f, 250.0f);

    g_p1_px = (uint8_t)((g_p1_target_hz * (float)BW) / 250.0f);
    g_p2_px = (uint8_t)((g_p2_target_hz * (float)BW) / 250.0f);

    g_t0_ms = millis();

    /* ---- STATIC FRAME: drawn once ---- */
    gfx_clear(COL_BLACK);
    gfx_header("RESULTS", COL_RED);
    gfx_bar(0, 18, 128, 1, COL_DKGRAY);   // separator under header

    // initial gray backgrounds for both bars
    gfx_bar(BX, BY1, BW, BH, COL_GRAY);
    gfx_bar(BX, BY2, BW, BH, COL_GRAY);

    // numeric labels above bars
    char p1txt[24], p2txt[24];
    snprintf(p1txt, sizeof(p1txt), "P1: %.1f Hz", g_p1_target_hz);
    snprintf(p2txt, sizeof(p2txt), "P2: %.1f Hz", g_p2_target_hz);
    gfx_text2(6, 34, p1txt, COL_BLUE, 1);
    gfx_text2(6, 74, p2txt, COL_RED,  1);

    // axis labels (static)
    gfx_text2(BX,           BY2 + BH + 4, "0 Hz",   COL_WHITE, 1);
    gfx_text2(BX + BW - 36, BY2 + BH + 4, "250 Hz", COL_WHITE, 1);
}

bool winner2_tick(void)
{
    uint32_t now = millis();
    uint32_t dt  = now - g_t0_ms;

    // shared ramp: 0 -> BW over ANIM_MS, each bar clamps at its target
    uint8_t ramp_px = (dt >= ANIM_MS)
                      ? BW
                      : (uint8_t)((dt * BW) / ANIM_MS);

    uint8_t w1 = (ramp_px < g_p1_px) ? ramp_px : g_p1_px;
    uint8_t w2 = (ramp_px < g_p2_px) ? ramp_px : g_p2_px;

    /* ---- DYNAMIC PART: ONLY BAR ROWS ARE TOUCHED ---- */

    // clear bar rows back to gray
    gfx_bar(BX, BY1, BW, BH, COL_GRAY);
    gfx_bar(BX, BY2, BW, BH, COL_GRAY);

    // foreground bars: P1 = BLUE, P2 = RED
    if (w1 > 0u) {
        gfx_bar(BX, BY1, w1, BH, COL_BLUE);
    }
    if (w2 > 0u) {
        gfx_bar(BX, BY2, w2, BH, COL_RED);
    }

    // rank segment separators drawn on top of bars (still only in bar region)
    static const uint8_t cuts[] = {25,45,65,80,90,95,97,99};
    for (unsigned i = 0; i < sizeof(cuts)/sizeof(cuts[0]); ++i){
        uint8_t x = (uint8_t)(BX + (cuts[i] * BW) / 100u);
        gfx_bar(x, BY1, 1, BH, COL_WHITE);
        gfx_bar(x, BY2, 1, BH, COL_WHITE);
    }

    // game_two.c decides when to leave this screen
    return (dt >= SCREEN_HOLD_MS);
}
