#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "timer.h"
#include "gfx.h"
#include "project.h"

// geometry matches single-player result bar area
#define BX   6
#define BY   110
#define BW   116
#define BH   18

#define ANIM_MS         3000u   // time to reach target width
#define SCREEN_HOLD_MS  6000u   // total time to keep the screen visible

static float    g_avg_hz       = 0.0f;
static float    g_baseline_hz  = 0.0f;
static uint8_t  g_target_px    = 0;
static uint8_t  g_baseline_px  = 0;   // baseline position in pixels on bar
static uint32_t g_t0_ms        = 0;

static float clampf(float v, float lo, float hi){
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static const char* rank_from_percent(unsigned p){
    if (p >= 99) return "Challenger";
    if (p >= 97) return "Grandmaster";
    if (p >= 95) return "Master";
    if (p >= 90) return "Diamond";
    if (p >= 80) return "Platinum";
    if (p >= 65) return "Gold";
    if (p >= 45) return "Silver";
    if (p >= 25) return "Bronze";
    return "Iron";
}

void result1_start(float avg_hz, float baseline_hz){
    g_avg_hz      = clampf(avg_hz,      0.0f, 250.0f);
    g_baseline_hz = clampf(baseline_hz, 0.0f, 250.0f);
    g_target_px   = (uint8_t)((g_avg_hz * (float)BW) / 250.0f);
    g_t0_ms       = millis();

    /* derived values for static text */
    unsigned      rank_pct = (unsigned)((g_avg_hz * 100.0f / 250.0f) + 0.5f);
    const char   *rank     = rank_from_percent(rank_pct);

    /* baseline position in pixels on the bar */
    if (g_baseline_hz > 0.0f) {
        g_baseline_px = (uint8_t)((g_baseline_hz * (float)BW) / 250.0f);
    } else {
        g_baseline_px = 0;
    }

    /* STATIC SCREEN: draw once */
    gfx_clear(COL_BLACK);
    gfx_header("RESULT", COL_WHITE);
    gfx_bar(0, 18, 128, 1, COL_DKGRAY);  // separator under header
    gfx_bar(0, 60, 128, 1, COL_DKGRAY);  // separates text block from bar

    char res1[48]; snprintf(res1, sizeof(res1), "Avg Hz: %.1f", g_avg_hz);
    gfx_text2(12, 28, res1, COL_WHITE, 1);

    char res0[48]; snprintf(res0, sizeof(res0), "Baseline: %.1f", g_baseline_hz);
    gfx_text2(12, 40, res0, COL_YELLOW, 1);

    char res2[48]; snprintf(res2, sizeof(res2), "Stronger Than: %u%%", rank_pct);
    gfx_text2(12, 70, res2, COL_RED, 1);
    gfx_text2(12, 80, "of People!!!", COL_WHITE, 1);

    char res4[48]; snprintf(res4, sizeof(res4), "Ranking: %s", rank);
    gfx_text2(10, 100, res4, COL_RED, 1);

    /* bar background and axis labels (no green bar yet) */
    gfx_bar(BX, BY, BW, BH, COL_GRAY);

    gfx_text2(BX,           BY + BH + 6, "0 Hz",   COL_WHITE, 1);
    gfx_text2(BX + BW - 36, BY + BH + 6, "250 Hz", COL_WHITE, 1);
}

bool result1_tick(void){
    uint32_t dt = millis() - g_t0_ms;

    /* animated bar width: ramp from 0 -> g_target_px over ANIM_MS */
    uint8_t w;
    if (dt >= ANIM_MS) {
        w = g_target_px;
    } else {
        w = (uint8_t)((dt * g_target_px) / ANIM_MS);
    }

    /* DYNAMIC PART: ONLY BAR REGION REFRESHES */

    /* grow the green bar (0), background to the right
       stays gray from result1_start() */
    if (w > 0) {
        gfx_bar(BX, BY, w, BH, COL_GREEN);
    }

    /* vertical cut lines for each rank band (drawn on top) */
    static const uint8_t cuts[] = {25,45,65,80,90,95,97,99};
    for (unsigned i = 0; i < sizeof(cuts)/sizeof(cuts[0]); ++i){
        uint8_t x = (uint8_t)(BX + (cuts[i] * BW) / 100u);
        gfx_bar(x, BY, 1, BH, COL_WHITE);
    }

    /* vertical red line at the baseline position (if inside bar) */
    if (g_baseline_px > 0 && g_baseline_px < BW){
        gfx_bar((uint8_t)(BX + g_baseline_px), BY, 1, BH, COL_RED);
    }

    return (dt >= SCREEN_HOLD_MS);
}
