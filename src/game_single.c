#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "timer.h"
#include "gfx.h"
#include "ssd1351.h"
#include "game.h"         // baseline_begin(), game_get_metrics()
#include "project.h"

/* helpers local to single-player */
static float clampf(float v, float lo, float hi){
  if (v < lo) return lo; if (v > hi) return hi; return v;
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
      gfx_clear(COL_BLACK);
      gfx_header("RESULT", COL_WHITE);

      float avg_hz = (g_cnt_hz > 0) ? (g_sum_hz / (float)g_cnt_hz) : 0.0f;
      float pct250 = clampf(avg_hz, 0.0f, 250.0f);
      unsigned rank_pct = (unsigned)((pct250 * 100.0f / 250.0f) + 0.5f);
      g_last_rank_pct = rank_pct;

      char res1[48]; snprintf(res1, sizeof(res1), "Avg Hz: %.1f", avg_hz);
      gfx_text2(12, 28, res1, COL_WHITE, 1);
      char res0[48]; snprintf(res0, sizeof(res0), "Baseline: %.1f", base);
      gfx_text2(12, 40, res0, COL_YELLOW, 1);

      char res2[48]; snprintf(res2, sizeof(res2), "Stronger Than: %u%%", rank_pct);
      gfx_text2(12, 70, res2, COL_RED, 1);
      gfx_text2(12, 80, "of People!!!", COL_WHITE, 1);

      const char* rank = rank_from_percent(rank_pct);
      char res4[48]; snprintf(res4, sizeof(res4), "Ranking: %s", rank);
      gfx_text2(10, 100, res4, COL_WHITE, 1);

      const uint8_t bx=6, by=110, bw=116, bh=18;
      uint8_t bwfill = (uint8_t)((pct250 * 116.0f) / 250.0f);
      gfx_bar(bx, by, bw, bh, COL_GRAY);
      gfx_bar(bx, by, bwfill, bh, COL_GREEN);
      const uint8_t cuts[] = {25,45,65,80,90,95,97,99};
      for (unsigned i=0;i<sizeof(cuts)/sizeof(cuts[0]);++i){
        uint8_t x = (uint8_t)(bx + (cuts[i] * bw) / 100);
        gfx_bar(x, by, 1, bh, COL_WHITE);
      }

      if (dt >= 6000u) goto_state(ST_RANKS);
    } break;

    case ST_RANKS: {
      if (!g_drawn_once){
        gfx_clear(COL_BLACK);
        gfx_header("RANKINGS", COL_RED);
        static const char* ranks[] = {
          "Challenger  1%","Grandmaster 3%","Master      5%","Diamond     10%",
          "Platinum    20%","Gold        35%","Silver      55%","Bronze      75%","Iron        100%"
        };
        const int base_y=24, row_h=10;
        for (int i=0;i<9;++i) gfx_text2(0, base_y+i*row_h, ranks[i], COL_WHITE, 1);
        int idx = rank_index_from_percent(g_last_rank_pct);
        if (idx<0) idx=0; if (idx>8) idx=8;
        gfx_text2(110, base_y+idx*row_h, "<-", COL_YELLOW, 1);
        g_drawn_once = true;
      }
      if (dt >= 6000u) goto_state(ST_BRAND);
    } break;
  }
}
