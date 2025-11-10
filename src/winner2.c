#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "timer.h"     // millis()
#include "gfx.h"       // gfx_clear, gfx_header, gfx_text2, gfx_bar
#include "project.h"   // colors

// Geometry matches your existing layout
#define BX   6
#define BW   116
#define BH   12
#define BY1  46
#define BY2  86

#define ANIM_MS         3000u  // time to reach full bar if target == max
#define SCREEN_HOLD_MS  6000u  // total time to keep the screen visible

static float    g_p1_target_hz = 0.0f, g_p2_target_hz = 0.0f;
static uint8_t  g_p1_px = 0, g_p2_px = 0;
static uint32_t g_t0_ms = 0;

static float clampf(float v, float lo, float hi){
  if (v < lo) return lo; if (v > hi) return hi; return v;
}

void winner2_start(float p1_avg_hz, float p2_avg_hz){
  g_p1_target_hz = clampf(p1_avg_hz, 0.0f, 250.0f);
  g_p2_target_hz = clampf(p2_avg_hz, 0.0f, 250.0f);
  g_p1_px = (uint8_t)((g_p1_target_hz * (float)BW) / 250.0f);
  g_p2_px = (uint8_t)((g_p2_target_hz * (float)BW) / 250.0f);
  g_t0_ms = millis();
}

bool winner2_tick(void){
  uint32_t t = millis();
  uint32_t dt = t - g_t0_ms;

  // Compute the shared ramp (both rise together); each clamps at its own target
  uint8_t ramp_px = (dt >= ANIM_MS) ? BW : (uint8_t)((dt * BW) / ANIM_MS);
  uint8_t w1 = (ramp_px < g_p1_px) ? ramp_px : g_p1_px;
  uint8_t w2 = (ramp_px < g_p2_px) ? ramp_px : g_p2_px;

  // Draw
  gfx_clear(COL_BLACK);
  gfx_header("RESULTS", COL_RED);
  gfx_bar(0, 18, 128, 1, COL_DKGRAY);   // separator under header

  // Bars + backgrounds
  gfx_bar(BX, BY1, BW, BH, COL_GRAY);  gfx_bar(BX, BY1, w1, BH, COL_GREEN);
  gfx_bar(BX, BY2, BW, BH, COL_GRAY);  gfx_bar(BX, BY2, w2, BH, COL_GREEN);

  // Rank segment separators
  static const uint8_t cuts[] = {25,45,65,80,90,95,97,99};
  for (unsigned i=0; i<sizeof(cuts)/sizeof(cuts[0]); ++i){
    uint8_t x = (uint8_t)(BX + (cuts[i] * BW) / 100u);
    gfx_bar(x, BY1, 1, BH, COL_WHITE);
    gfx_bar(x, BY2, 1, BH, COL_WHITE);
  }

  // Labels
  char p1txt[24], p2txt[24];
  snprintf(p1txt, sizeof(p1txt), "P1: %.1f Hz", g_p1_target_hz);
  snprintf(p2txt, sizeof(p2txt), "P2: %.1f Hz", g_p2_target_hz);
  gfx_text2(6, 34, p1txt, COL_RED,   1);
  gfx_text2(6, 74, p2txt, COL_YELLOW, 1);

  // Finish after full screen hold duration
  return (dt >= SCREEN_HOLD_MS);
}
