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
  if (v < lo) return lo; if (v > hi) return hi; return v;
}

static inline void ui_sep_h(uint8_t y){ gfx_bar(0, y, 128, 1, COL_DKGRAY); }
static inline void ui_sep_v(uint8_t x, uint8_t y, uint8_t h){ gfx_bar(x, y, 1, h, COL_DKGRAY); }

static inline int rank_index_from_percent(unsigned p){
  if (p >= 99) return 0; if (p >= 97) return 1; if (p >= 95) return 2;
  if (p >= 90) return 3; if (p >= 80) return 4; if (p >= 65) return 5;
  if (p >= 45) return 6; if (p >= 25) return 7; return 8;
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
  g_sum_hz = 0.0f; g_cnt_hz = 0;
  goto_state(ST_LOGO);
}

bool game_single_tick(void){
  uint32_t now = millis();
  uint32_t dt  = now - g_t0_ms;

  float hz=0.0f, base=0.0f; uint8_t pct=0;
  game_get_metrics(&hz, &pct, &base);  

  switch(g_state){

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
        if (dt >= 3000) goto_state(ST_PMODE);
    } break;

    case ST_PMODE:
      gfx_header("Mode: PLAYGROUND", COL_WHITE);
        ui_sep_h(18);

      if (dt >= 3000u) goto_state(ST_TUTORIAL);
      break;

    case ST_TUTORIAL:
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("TUTORIAL", COL_WHITE);
        // optional static elements here
      }
        
      playground_draw_lore_typewriter(g_playground_intro_lines,
                                g_playground_intro_count,
                                dt,
                                50u);

      if (dt >= 10000u) goto_state(ST_COUNTDOWN_LABEL);
      break;

    case ST_COUNTDOWN_LABEL:
      if (g_dirty) {
        g_dirty = false;
        gfx_header("COUNTDOWN", COL_WHITE);
      }
      if (dt >= 2000u) goto_state(ST_COUNT_3);
      break;

    case ST_COUNT_3: gfx_header("3", COL_WHITE);   if (dt >= 1000u) goto_state(ST_COUNT_2); break;
    case ST_COUNT_2: gfx_header("2", COL_YELLOW);  if (dt >= 1000u) goto_state(ST_COUNT_1); break;
    case ST_COUNT_1: gfx_header("1", COL_GREEN);   if (dt >= 1000u) goto_state(ST_FLEX_CUE); break;

    case ST_FLEX_CUE:
      if (g_dirty) {
        g_dirty = false;
        gfx_header("FLEX!", COL_RED);
        ui_sep_h(18);
      }
      if (dt >= 1000u){ g_sum_hz = 0.0f; g_cnt_hz = 0; goto_state(ST_FLEXING); }
      break;

    case ST_FLEXING: {
      const uint32_t FLEX_MS = 10000u;
      uint32_t elapsed = dt;

      if (!g_drawn_once){
        g_drawn_once = true;
        g_sum_hz     = 0.0f;
        g_cnt_hz     = 0;

        // draw static frame once based on current baseline
        draw_flex_static(base);
      }

      if (elapsed >= FLEX_MS){
        goto_state(ST_RESULT);
        break;
      }

      uint8_t remain_s = (uint8_t)((FLEX_MS - elapsed) / 1000u);
      char hdr[24];
      snprintf(hdr, sizeof(hdr), "FLEX: %u", (unsigned)remain_s);
      gfx_header(hdr, COL_RED);

      if (hz >= 1.5f){
        g_sum_hz += hz;
        g_cnt_hz++;
      }

      // dynamic part: Hz text + red bar only
      draw_flex_dynamic(hz);
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
        // compute index for this attempt and store it exactly once
        int idx = rank_index_from_percent(g_last_rank_pct);
        if (idx < 0) idx = 0;
        if (idx > 8) idx = 8;
        rankhist_add_single(idx);

        gfx_clear(COL_BLACK);
        gfx_header("RANKINGS", COL_RED);
        ui_sep_h(18);

        // TI-style tiers
        static const char* ranks[] = {
          "Challenger  1%","Grandmaster 3%","Master      5%","Diamond    10%",
          "Platinum   20%","Gold       35%","Silver     55%","Bronze     75%","Iron      100%"
        };

        const int base_y = 24, row_h = 10;

        for (int i=0; i<9; ++i){
          uint16_t col = (i==idx) ? COL_CYAN : COL_WHITE;   // highlight only
          gfx_text2(0, base_y + i*row_h, ranks[i], col, 1);
        }
        gfx_text2(110, base_y + idx*row_h, "<-", COL_CYAN, 1);

        g_drawn_once = true;   // everything for this state done once
      }

      if (dt >= 6000u){
        goto_state(ST_OVERALL_RANKS);      // NEXT: totals screen
      }
    } break;

    case ST_OVERALL_RANKS: {
      if (!g_drawn_once){
        gfx_clear(COL_BLACK);
        gfx_header("OVERALL RANKS", COL_WHITE);
        ui_sep_h(18);

        static const char* ranks[] = {
          "Challenger  1%","Grandmaster 3%","Master      5%","Diamond    10%",
          "Platinum   20%","Gold       35%","Silver     55%","Bronze     75%","Iron      100%"
        };
        uint16_t hc[9]; 
        rankhist_get_single(hc);

        /* last attempt’s rank index for highlighting */
        int idx = rank_index_from_percent(g_last_rank_pct);
        if (idx < 0) idx = 0;
        if (idx > 8) idx = 8;

        const int base_y = 24, row_h = 10;
        for (int i = 0; i < 9; ++i){
          char line[28];
          snprintf(line, sizeof(line), "%-16s %u",
                   ranks[i], (unsigned)hc[i]);

          /* highlight the row that matches this attempt’s rank */
          uint16_t col = (i == idx) ? COL_CYAN : COL_WHITE;
          gfx_text2(0, base_y + i*row_h, line, col, 1);
        }
        g_drawn_once = true;
      }

      if (dt >= 6000u){
        return true;
      }
    } break;
  }

  // default: not finished yet
  return false;
}
