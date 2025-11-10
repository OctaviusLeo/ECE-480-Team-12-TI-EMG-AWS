#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "project.h"
#include "timer.h"
#include "gfx.h"
#include "ssd1351.h"
#include "game.h"

// new headers for split modes
#include "game_single.h"
#include "game_two.h"

// private storage of latest metrics (read by modes via game_get_metrics)
static bool     g_inited = false;
static uint8_t  g_mode   = 0;        // 0 = single, 1 = two-player
static float    g_latest_hz = 0.0f;
static uint8_t  g_latest_pct = 0;
static float    g_baseline_disp_hz = 0.0f;

void game_get_metrics(float *hz, uint8_t *pct, float *baseline_hz){
  if (hz)          *hz = g_latest_hz;
  if (pct)         *pct = g_latest_pct;
  if (baseline_hz) *baseline_hz = g_baseline_disp_hz;
}

void game_init(void){
  if (!g_inited){
    ssd1351_init();
    gfx_clear(COL_BLACK);
    g_inited = true;
  }
  if (g_mode == 0) game_single_init();
  else             game_two_init();
}

void game_set_mode(uint8_t two_player){
  g_mode = two_player ? 1u : 0u;
  // re-init the chosen mode immediately
  if (g_mode == 0) game_single_init();
  else             game_two_init();
}

void game_set_metrics(float hz, uint8_t intensity_pct){
  g_latest_hz  = hz;
  g_latest_pct = intensity_pct;
}

void game_set_baseline(float baseline_hz){
  g_baseline_disp_hz = baseline_hz;
}

void game_tick(void){
  if (g_mode == 0) game_single_tick();
  else             game_two_tick();
}
