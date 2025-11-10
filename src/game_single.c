#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "timer.h"
#include "gfx.h"
#include "ssd1351.h"
#include "game.h"         // baseline_begin(), game_get_metrics()
#include "project.h"
#include "ti_logo.h"
#include "result1.h"

/* helpers local to single-player */
static float clampf(float v, float lo, float hi){
  if (v < lo) return lo; if (v > hi) return hi; return v;
}

static inline void ui_sep_h(uint8_t y){ gfx_bar(0, y, 128, 1, COL_DKGRAY); }
static inline void ui_sep_v(uint8_t x, uint8_t y, uint8_t h){ gfx_bar(x, y, 1, h, COL_DKGRAY); }

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
static inline int rank_index_from_percent(unsigned p){
  if (p >= 99) return 0; if (p >= 97) return 1; if (p >= 95) return 2;
  if (p >= 90) return 3; if (p >= 80) return 4; if (p >= 65) return 5;
  if (p >= 45) return 6; if (p >= 25) return 7; return 8;
}

typedef enum {
  ST_BRAND = 0,
  ST_COUNTDOWN_LABEL,
  ST_COUNT_3, ST_COUNT_2, ST_COUNT_1,
  ST_FLEX_CUE, ST_FLEXING,
  ST_RESULT, ST_RANKS
} sp_state_t;

static sp_state_t g_state;
static uint32_t   g_t0_ms;           // state start time
static bool       g_drawn_once = false;
static unsigned   g_last_rank_pct = 0;

/* stats for the 10 s flex window */
static float    g_sum_hz = 0.0f;
static uint32_t g_cnt_hz = 0;

static void goto_state(sp_state_t s){
  g_state = s;
  g_t0_ms = millis();
  g_drawn_once = false;
  if (s == ST_BRAND || s == ST_COUNTDOWN_LABEL) baseline_begin(3000u);
}

static void draw_flex_body(float hz, float baseline_hz){
  gfx_bar(0, 18, 128, 110, COL_BLACK);

    ui_sep_h(18);   // under header
    ui_sep_h(58);   // separates text (Hz/baseline) from bar area
    ui_sep_v(0, 18, 110); ui_sep_v(127, 18, 110);

  char line[32]; snprintf(line, sizeof(line), "Hz: %.1f", hz);
  gfx_text2(6, 28, line, COL_WHITE, 2);
  char bline[32]; snprintf(bline, sizeof(bline), "Baseline: %.1f", baseline_hz);
  gfx_text2(6, 44, bline, COL_YELLOW, 1);
  const uint8_t bx=6, by=64, bw=116, bh=18;
  gfx_bar(bx, by, bw, bh, COL_GRAY);
  uint8_t fw = (uint8_t)((clampf(hz, 0.0f, 250.0f) * (float)bw) / 250.0f);
  gfx_bar(bx, by, fw, bh, COL_RED);
  gfx_text2(bx, by + bh + 6, "0 Hz", COL_WHITE, 1);
  const char* rlabel = "250 Hz";
  uint8_t rx = bx + bw - (uint8_t)(6 * 1 * strlen(rlabel));
  gfx_text2(rx, by + bh + 6, rlabel, COL_WHITE, 1);
}

void game_single_init(void){
  g_sum_hz = 0.0f; g_cnt_hz = 0;
  goto_state(ST_BRAND);
}

void game_single_tick(void){
  uint32_t now = millis();
  uint32_t dt  = now - g_t0_ms;

  float hz=0.0f, base=0.0f; uint8_t pct=0;
  game_get_metrics(&hz, &pct, &base);

  switch(g_state){
    case ST_BRAND: {
      gfx_clear(COL_BLACK);
      gfx_header("Texas Instruments", COL_RED);
      ui_sep_h(18);  

      uint8_t x = (uint8_t)((128 - TI_LOGO_W) / 2);
      uint8_t y = 30;  // adjust if you want it closer/further from the title
      gfx_blit565(x, y, TI_LOGO_W, TI_LOGO_H, TI_LOGO);

      if (dt >= 3000u) goto_state(ST_COUNTDOWN_LABEL);
    } break;

    case ST_COUNTDOWN_LABEL:
      gfx_header("COUNTDOWN", COL_WHITE);
      if (dt >= 2000u) goto_state(ST_COUNT_3);
      break;

    case ST_COUNT_3: gfx_header("3", COL_WHITE);   if (dt >= 1000u) goto_state(ST_COUNT_2); break;
    case ST_COUNT_2: gfx_header("2", COL_YELLOW);  if (dt >= 1000u) goto_state(ST_COUNT_1); break;
    case ST_COUNT_1: gfx_header("1", COL_GREEN);   if (dt >= 1000u) goto_state(ST_FLEX_CUE); break;

    case ST_FLEX_CUE:
      gfx_header("FLEX!", COL_RED);
        ui_sep_h(18);

      if (dt >= 1000u){ g_sum_hz = 0.0f; g_cnt_hz = 0; goto_state(ST_FLEXING); }
      break;

    case ST_FLEXING: {
      const uint32_t FLEX_MS = 10000u;
      uint32_t elapsed = dt;
      if (elapsed >= FLEX_MS){
        goto_state(ST_RESULT);
        break;
      }
      uint8_t remain_s = (uint8_t)((FLEX_MS - elapsed)/1000u);
      char hdr[24]; snprintf(hdr, sizeof(hdr), "FLEX: %u", (unsigned)remain_s);
      gfx_header(hdr, COL_RED);

      if (hz >= 1.5f){ g_sum_hz += hz; g_cnt_hz++; }
      draw_flex_body(hz, base);
    } break;

    case ST_RESULT: {
    // compute once, then delegate the screen to the animator module
    float avg_hz = (g_cnt_hz > 0) ? (g_sum_hz / (float)g_cnt_hz) : 0.0f;
    float pct250 = (avg_hz < 0.0f ? 0.0f : (avg_hz > 250.0f ? 250.0f : avg_hz));
    unsigned rank_pct = (unsigned)((pct250 * 100.0f / 250.0f) + 0.5f);
    g_last_rank_pct = rank_pct;

    if (!g_drawn_once){
        result1_start(avg_hz, base);
        g_drawn_once = true;
    }
    if (result1_tick()){
        goto_state(ST_RANKS);
    }
    } break;

    case ST_RANKS: {
      if (!g_drawn_once){
        gfx_clear(COL_BLACK);
        gfx_header("RANKINGS", COL_RED);
        ui_sep_h(18);

        static const char* ranks[] = {
          "Principal Fellow 1%","TI Senior Fellow 3%","TI Fellow 5%","DMTS 10%",
          "SMTS 20%","MGTS 35%","Lead 55%","Senior Engineer 75%","Engineer 100%"
        };
        const int base_y=24, row_h=10;

        int idx = rank_index_from_percent(g_last_rank_pct);
        if (idx<0) idx=0; if (idx>8) idx=8;
        for (int i=0; i<9; ++i){
        uint16_t col = (i == idx) ? COL_CYAN : COL_WHITE;   // highlight your rank row
        gfx_text2(0, base_y + i*row_h, ranks[i], col, 1);
        }
        gfx_text2(110, base_y + idx*row_h, "<-", COL_RED, 1);

        g_drawn_once = true;
      }
      if (dt >= 6000u) goto_state(ST_BRAND);
    } break;
  }
}
