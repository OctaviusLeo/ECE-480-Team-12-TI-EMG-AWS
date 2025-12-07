/*==============================================================================
 * @file    game_single.c
 * @brief   Single-player playground mode flow and user interface.
 *
 * This file is part of the EMG flex-frequency game project and follows the
 * project coding standard for file-level documentation.
 *============================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "timer.h"
#include "gfx.h"
#include "ssd1351.h"
#include "game.h"         // baseline_begin(), game_get_metrics()
#include "project.h"
#include "result1.h"
#include "rankhist.h"
#include "team.h"
#include "game_single_logo.h"
#include "cheevos.h"

/* helpers local to single-player */
static float clampf(float v, float lo, float hi){
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static inline void ui_sep_h(uint8_t y){
  gfx_bar(0, y, 128, 1, COL_DKGRAY);
}
static inline void ui_sep_v(uint8_t x, uint8_t y, uint8_t h){
  gfx_bar(x, y, 1, h, COL_DKGRAY);
}

static inline int rank_index_from_percent(unsigned p){
  if (p >= 99) return 0;
  if (p >= 97) return 1;
  if (p >= 95) return 2;
  if (p >= 90) return 3;
  if (p >= 80) return 4;
  if (p >= 65) return 5;
  if (p >= 45) return 6;
  if (p >= 25) return 7;
  return 8;
}

typedef enum {
  ST_LOGO = 0,
  ST_PMODE,
  ST_TUTORIAL,
  ST_COUNTDOWN_LABEL,
  ST_COUNT_3, ST_COUNT_2, ST_COUNT_1,
  ST_FLEX_CUE, ST_FLEXING,
  ST_RESULT, ST_RANKS,
  ST_OVERALL_RANKS
} sp_state_t;

static sp_state_t g_state;
static uint32_t   g_t0_ms;           // state start time
static bool       g_drawn_once = false;
static unsigned   g_last_rank_pct = 0;

/* stats for the 10 s flex window */
static float    g_sum_hz = 0.0f;
static uint32_t g_cnt_hz = 0;

static bool        g_dirty;   // true = need to (re)draw this state's screen

static void goto_state(sp_state_t s){
  g_state = s;
  g_t0_ms = millis();
  g_drawn_once = false;
  g_dirty = true;       // mark the new state as needing a redraw
  // e.g., when you successfully enter an admin menu
  cheevos_unlock(ACH_TUTORIAL);
  if (s == ST_COUNTDOWN_LABEL) baseline_begin(3000u);
}

// Typewriter-style lore: slowly reveal text over time.
// NOTE: does NOT clear the screen or draw header; caller does that once via g_dirty.
static void playground_draw_lore_typewriter(const char* const* lines,
                                            uint8_t count,
                                            uint32_t dt,
                                            uint16_t ms_per_char)
{
  uint32_t chars = dt / ms_per_char;  // total characters across all lines
  uint8_t  y     = 24;                // first line Y

  for (uint8_t i = 0; i < count; ++i) {
    const char* s = lines[i];
    if (!s) {
      continue;
    }

    size_t len = strlen(s);
    if (chars == 0u) {
      break;  // nothing left to draw
    }

    uint32_t this_chars = chars;
    if (this_chars > len) {
      this_chars = len;
    }

    // Draw prefix of this line
    char buf[32];  // adjust if ever have >31-char lines
    if (this_chars > sizeof(buf) - 1u) {
      this_chars = sizeof(buf) - 1u;
    }
    memcpy(buf, s, this_chars);
    buf[this_chars] = '\0';

    // Draw only the visible prefix; earlier characters are redrawn in-place
    gfx_text2(4, y, buf, COL_WHITE, 1);

    y = (uint8_t)(y + 10u);
    if (y > 120u) {
      break;  // off-screen
    }

    // Consume characters for this line (+1 “newline” spacer)
    if (chars > (uint32_t)len + 1u) {
      chars -= (uint32_t)len + 1u;
    } else {
      chars = 0u;
    }
  }
}

// Live flex bar geometry (single-player)
#define FLEX_BX  6
#define FLEX_BY  64
#define FLEX_BW  116
#define FLEX_BH  18

static const char* g_playground_intro_lines[] = {
  "Hello newbie!",
  "Welcome to the world",
  "of PULSEBOUND!",
  "A place of muscle,",
  "but don't worry!",
  "I'll help you!",
  "Remember 2 rules:",
  "1. Flex.",
  "2. Win.",
  "Got it? Goodluck!"
};
static const uint8_t g_playground_intro_count =
    sizeof(g_playground_intro_lines)/sizeof(g_playground_intro_lines[0]);

static void draw_flex_static(float baseline_hz)
{
  gfx_bar(0, 18, 128, 110, COL_BLACK);

  ui_sep_h(18);   // under header
  ui_sep_h(58);   // separates text (Hz/baseline) from bar area
  ui_sep_v(0, 18, 110);
  ui_sep_v(127, 18, 110);

  // baseline line (baseline is stable during flex)
  char bline[32];
  snprintf(bline, sizeof(bline), "Baseline: %.1f", baseline_hz);
  gfx_text2(6, 44, bline, COL_YELLOW, 1);

  // axis labels (static)
  gfx_text2(FLEX_BX, FLEX_BY + FLEX_BH + 6, "0 Hz", COL_WHITE, 1);
  const char *rlabel = "250 Hz";
  uint8_t rx = FLEX_BX + FLEX_BW - (uint8_t)(6 * 1 * strlen(rlabel));
  gfx_text2(rx, FLEX_BY + FLEX_BH + 6, rlabel, COL_WHITE, 1);
}

static void draw_flex_dynamic(float hz)
{
  // update Hz text
  char line[32];
  snprintf(line, sizeof(line), "Hz: %.1f", hz);
  gfx_bar(6, 28, 120, 16, COL_BLACK);
  gfx_text2(6, 28, line, COL_WHITE, 2);

  // update red bar only
  gfx_bar(FLEX_BX, FLEX_BY, FLEX_BW, FLEX_BH, COL_GRAY);
  uint8_t fw = (uint8_t)((clampf(hz, 0.0f, 250.0f) * (float)FLEX_BW) / 250.0f);
  gfx_bar(FLEX_BX, FLEX_BY, fw, FLEX_BH, COL_RED);
}

void game_single_init(void){
  g_sum_hz = 0.0f;
  g_cnt_hz = 0;
  goto_state(ST_LOGO);
}

bool game_single_tick(void){
  uint32_t now = millis();
  uint32_t dt  = now - g_t0_ms;

  float   hz   = 0.0f;
  float   base = 0.0f;
  uint8_t pct  = 0;
  game_get_metrics(&hz, &pct, &base);

  switch (g_state){

    case ST_LOGO: {
      if (!g_drawn_once){
        g_drawn_once = true;
        gfx_clear(COL_BLACK);
        uint8_t x = (uint8_t)((128 - GAME_SINGLE_LOGO_W) / 2);
        uint8_t y = (uint8_t)((128 - GAME_SINGLE_LOGO_H) / 2);

        gfx_blit_pal4(x, y,
                      GAME_SINGLE_LOGO_W, GAME_SINGLE_LOGO_H,
                      GAME_SINGLE_LOGO_IDX,
                      GAME_SINGLE_LOGO_PAL);
      }
      if (dt >= 3000u){
        goto_state(ST_PMODE);
      }
    } break;

    case ST_PMODE: {
      if (g_dirty){
        g_dirty = false;
        gfx_header("Mode: Playground", COL_WHITE);
        ui_sep_h(18);
      }
      if (dt >= 3000u){
        goto_state(ST_TUTORIAL);
      }
    } break;

    case ST_TUTORIAL: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("Playground", COL_WHITE);
      }

      playground_draw_lore_typewriter(
          g_playground_intro_lines,
          g_playground_intro_count,
          dt,
          40u);

      if (dt >= 10000u){
        goto_state(ST_COUNTDOWN_LABEL);
      }
    } break;

    case ST_COUNTDOWN_LABEL: {
      if (g_dirty){
        g_dirty = false;
        gfx_header("READY?", COL_WHITE);
        ui_sep_h(18);
      }
      if (dt >= 2000u){
        goto_state(ST_COUNT_3);
      }
    } break;

    case ST_COUNT_3: {
      gfx_header("3", COL_WHITE);
      ui_sep_h(18);
      if (dt >= 1000u){
        goto_state(ST_COUNT_2);
      }
    } break;

    case ST_COUNT_2: {
      gfx_header("2", COL_YELLOW);
      ui_sep_h(18);
      if (dt >= 1000u){
        goto_state(ST_COUNT_1);
      }
    } break;

    case ST_COUNT_1: {
      gfx_header("1", COL_GREEN);
      ui_sep_h(18);
      if (dt >= 1000u){
        goto_state(ST_FLEX_CUE);
      }
    } break;

    case ST_FLEX_CUE: {
      gfx_header("FLEX!", COL_RED);
      ui_sep_h(18);
      if (dt >= 1000u){
        goto_state(ST_FLEXING);
      }
    } break;

    case ST_FLEXING: {
      if (g_dirty){
        g_dirty = false;
        gfx_header("FLEXING", COL_WHITE);
        draw_flex_static(base);
      }

      draw_flex_dynamic(hz);
      g_sum_hz += hz;
      g_cnt_hz++;

      if (dt >= 10000u){
        goto_state(ST_RESULT);
      }
    } break;

    case ST_RESULT: {
      if (g_dirty){
        g_dirty = false;

        float avg_hz = (g_cnt_hz ? (g_sum_hz / (float)g_cnt_hz) : 0.0f);

        gfx_clear(COL_BLACK);
        gfx_header("RESULT", COL_WHITE);
        ui_sep_h(18);

        char line[32];
        snprintf(line, sizeof(line), "Avg: %.1f Hz", avg_hz);
        gfx_text2(4, 28, line, COL_WHITE, 1);

        snprintf(line, sizeof(line), "Base: %.1f Hz", base);
        gfx_text2(4, 38, line, COL_WHITE, 1);

        unsigned pct_rank = rankhist_percentile(avg_hz);
        g_last_rank_pct = pct_rank;

        snprintf(line, sizeof(line), "You beat %u%%!", pct_rank);
        gfx_text2(4, 52, line, COL_CYAN, 1);

        // Unlock cheevo based on rank
        int idx = rank_index_from_percent(pct_rank);
        cheevos_unlock_for_rank(idx);
      }

      if (dt >= 6000u){
        goto_state(ST_RANKS);
      }
    } break;

    case ST_RANKS: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("RANK HISTORY", COL_WHITE);
        ui_sep_h(18);

        rankhist_draw(4, 24, 120, 90, g_last_rank_pct);
      }

      if (dt >= 8000u){
        goto_state(ST_OVERALL_RANKS);
      }
    } break;

    case ST_OVERALL_RANKS: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("TEAM RANKS", COL_WHITE);
        ui_sep_h(18);

        team_draw(4, 24, 120, 90);
      }

      if (dt >= 8000u){
        goto_state(ST_LOGO);
      }
    } break;

    default:
      goto_state(ST_LOGO);
      break;
  }

  return false;
}

// Single-player metrics: simple wrapper using global game metrics.
void game_single_metrics(float *hz, uint8_t *pct, float *base){
  game_get_metrics(hz, pct, base);
}

