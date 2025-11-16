#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "timer.h"
#include "gfx.h"
#include "ssd1351.h"
#include "game.h"   // baseline_begin(), game_get_metrics()
#include "project.h"
#include "ti_logo.h"
#include "intermission.h"
#include "winner2.h"
#include "rankhist.h"
#include "MSU_logo.h"

/* helpers local to two-player */
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
  ST_PMODE,              // "Mode: TWO PLAYER"
  ST_COUNTDOWN_LABEL,    // READY P1? or READY P2?
  ST_COUNT_3, ST_COUNT_2, ST_COUNT_1,
  ST_FLEX_CUE, ST_FLEXING,
  ST_RESULTS2, ST_WINNER, ST_DECLARE,
  ST_RANKS2,             // immediate ranks (highlight only)
  ST_OVERALL_RANKS2,      // overall attempts per rank
  ST_ENDING_SCENE
} tp_state_t;

static tp_state_t g_state;
static uint32_t   g_t0_ms;
static bool       g_drawn_once = false;

static uint8_t  g_player = 1;
static float    g_p1_avg_hz = 0.0f, g_p2_avg_hz = 0.0f;
static unsigned g_p1_rank_pct = 0,  g_p2_rank_pct = 0;

static bool        g_dirty;   // true = need to (re)draw this state's screen

static void goto_state(tp_state_t s){
  g_state = s;
  g_t0_ms = millis();
  g_drawn_once = false;
  g_dirty = true;       // mark the new state as needing a redraw
  if (s == ST_BRAND || s == ST_COUNTDOWN_LABEL) baseline_begin(3000u);
}

// Live flex bar geometry (two-player)
#define FLEX_BX  6
#define FLEX_BY  64
#define FLEX_BW  116
#define FLEX_BH  18

static void draw_flex_static(float baseline_hz)
{
  // clear lower panel and draw static frame once
  gfx_bar(0, 18, 128, 110, COL_BLACK);
  ui_sep_h(18);    // under header
  ui_sep_h(60);    // separates text (Hz/baseline) from bar area
  ui_sep_v(0, 18, 110);
  ui_sep_v(127, 18, 110);

  // baseline text (baseline is stable during the flex window)
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
  // refresh only the Hz text + the bar region

  // Hz text line
  char line[32];
  snprintf(line, sizeof(line), "Hz: %.1f", hz);
  // clear a small rectangle behind the Hz text (not the whole screen)
  gfx_bar(6, 28, 120, 16, COL_BLACK);
  gfx_text2(6, 28, line, COL_WHITE, 2);

  // bar background and filled portion
  gfx_bar(FLEX_BX, FLEX_BY, FLEX_BW, FLEX_BH, COL_GRAY);
  uint8_t fw = (uint8_t)((clampf(hz, 0.0f, 250.0f) * (float)FLEX_BW) / 250.0f);
  gfx_bar(FLEX_BX, FLEX_BY, fw, FLEX_BH, COL_GREEN);
}

void game_two_init(void){
  g_player = 1;
  goto_state(ST_BRAND);
}

void game_two_tick(void){
  uint32_t now = millis();
  uint32_t dt  = now - g_t0_ms;

  float hz=0.0f, base=0.0f; uint8_t pct=0;
  game_get_metrics(&hz, &pct, &base);

    if (intermission_active()){
    if (intermission_tick()){
        goto_state(ST_COUNTDOWN_LABEL);   // proceed to P2 countdown when done
    }
    return;  // overlay handled this frame
    }

  switch(g_state){
    case ST_BRAND: {
      if (g_dirty) {
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("Texas Instruments", COL_RED);
        ui_sep_h(18);
        
        uint8_t x = (uint8_t)((128 - TI_LOGO_W) / 2);
        uint8_t y = 30;  // adjust if you want it closer/further from the title
        gfx_blit565(x, y, TI_LOGO_W, TI_LOGO_H, TI_LOGO);
      }
      if (dt >= 3000u) goto_state(ST_PMODE);
    } break;

    case ST_PMODE:
      gfx_header("Mode: PVP", COL_WHITE);
        ui_sep_h(18);

      if (dt >= 3000u) goto_state(ST_COUNTDOWN_LABEL);
      break;

    case ST_COUNTDOWN_LABEL: {
      char hdr[20]; snprintf(hdr, sizeof(hdr), "READY P%u?", (unsigned)g_player);
        ui_sep_h(18);

      gfx_header(hdr, COL_WHITE);
      if (dt >= 2000u) goto_state(ST_COUNT_3);
    } break;

    case ST_COUNT_3: gfx_header("3", COL_WHITE);   if (dt >= 1000u) goto_state(ST_COUNT_2); break;
    ui_sep_h(18);
    case ST_COUNT_2: gfx_header("2", COL_YELLOW);  if (dt >= 1000u) goto_state(ST_COUNT_1); break;
    ui_sep_h(18);
    case ST_COUNT_1: gfx_header("1", COL_GREEN);   if (dt >= 1000u) goto_state(ST_FLEX_CUE); break;
    ui_sep_h(18);

    case ST_FLEX_CUE:
      gfx_header("FLEX!", COL_RED);
      ui_sep_h(18);

      if (dt >= 1000u) goto_state(ST_FLEXING);
      break;

    case ST_FLEXING: {
      static float    sum_hz = 0.0f;
      static uint32_t cnt_hz = 0;

      if (!g_drawn_once){
        sum_hz     = 0.0f;
        cnt_hz     = 0;
        g_drawn_once = true;

        // draw static panel once using current baseline
        draw_flex_static(base);
      }

      const uint32_t FLEX_MS = 10000u;
      if (dt >= FLEX_MS){
        float avg     = (cnt_hz > 0) ? (sum_hz / (float)cnt_hz) : 0.0f;
        float pct250  = clampf(avg, 0.0f, 250.0f);
        unsigned rank_pct = (unsigned)((pct250 * 100.0f / 250.0f) + 0.5f);

        if (g_player == 1u){
          g_p1_avg_hz   = avg;
          g_p1_rank_pct = rank_pct;
          g_player      = 2u;
          g_drawn_once  = false;
          intermission_begin(13000u, "GET READY P2");
          return;   // let the overlay run until it finishes
        } else {
          g_p2_avg_hz   = avg;
          g_p2_rank_pct = rank_pct;
          g_drawn_once  = false;
          goto_state(ST_RESULTS2);
        }
        break;
      }

      // update header countdown (top strip, still cheap)
      uint8_t remain_s = (uint8_t)((FLEX_MS - dt) / 1000u);
      char hdr[24];
      snprintf(hdr, sizeof(hdr), "FLEX: %u", (unsigned)remain_s);
      gfx_header(hdr, COL_RED);

      if (hz >= 1.5f){
        sum_hz += hz;
        cnt_hz++;
      }

      // dynamic part: only Hz text + green bar are refreshed
      draw_flex_dynamic(hz);
    } break;

    case ST_RESULTS2: {
      if (g_dirty) {
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("SCORES", COL_WHITE);
          ui_sep_h(18);    // under header
          ui_sep_h(60);    // divides P1 (top) and P2 (bottom)

        char l1[32]; snprintf(l1, sizeof(l1), "P1 Avg Hz: %.1f", g_p1_avg_hz);
        gfx_text2(6, 28, l1, COL_RED, 1);
        const char* r1 = rank_from_percent(g_p1_rank_pct);
        char l1r[32]; snprintf(l1r, sizeof(l1r), "Rank: %s", r1);
        gfx_text2(6, 38, l1r, COL_RED, 1);

        char l2[32]; snprintf(l2, sizeof(l2), "P2 Avg Hz: %.1f", g_p2_avg_hz);
        gfx_text2(6, 70, l2, COL_WHITE, 1);
        const char* r2 = rank_from_percent(g_p2_rank_pct);
        char l2r[32]; snprintf(l2r, sizeof(l2r), "Rank: %s", r2);
        gfx_text2(6, 80, l2r, COL_WHITE, 1);
      }
      if (dt >= 3000u) goto_state(ST_WINNER);
    } break;

    case ST_WINNER: {
    if (!g_drawn_once){
        g_drawn_once = true;
        winner2_start(g_p1_avg_hz, g_p2_avg_hz);
    }
    if (winner2_tick()){
        goto_state(ST_DECLARE);
    }
    } break;

    case ST_DECLARE: {
      if (g_dirty) {
        g_dirty = false;

        gfx_clear(COL_BLACK);
        gfx_header("WINNER", COL_WHITE);
        ui_sep_h(18);    // under header
        ui_sep_h(80);    // (optional) extra separator

        const char *msg;
        uint16_t    color = COL_GOLD;
        uint8_t     x;

        if (g_p1_avg_hz > g_p2_avg_hz) {
          msg = "WINNER: P1!";
          x   = 0;       // left-aligned
        } else if (g_p2_avg_hz > g_p1_avg_hz) {
          msg = "WINNER: P2!";
          x   = 0;       // left-aligned
        } else {
          msg = "TIE";
          x   = 50;      // roughly centered
        }

        gfx_text2(x, 62, msg, color, 2);
      }

      if (dt >= 2000u) {
        goto_state(ST_RANKS2);
      }
    } break;

    case ST_RANKS2: {
      if (!g_drawn_once){
        g_drawn_once = true;
        gfx_clear(COL_BLACK);
        gfx_header("RANKINGS", COL_RED);
        ui_sep_h(18);

        static const char* ranks[] = {
          "Challenger 1%","Grandmaster 3%","Master 5%","Diamond 10%",
          "Platinum 20%","Gold 35%","Silver 55%","Bronze 75%","Iron 100%"
        };
        const int base_y=24, row_h=10;
        for (int i=0;i<9;++i) gfx_text2(0, base_y+i*row_h, ranks[i], COL_WHITE, 1);

        int i1 = rank_index_from_percent(g_p1_rank_pct);
        int i2 = rank_index_from_percent(g_p2_rank_pct);
        if (i1<0) i1=0; if (i1>8) i1=8;
        if (i2<0) i2=0; if (i2>8) i2=8;
        for (int i=0; i<9; ++i){
        uint16_t col = (i==i1 && i==i2) ? COL_MAGENTA      // both got same rank
                        : (i==i1)        ? COL_CYAN        // P1’s rank row
                        : (i==i2)        ? COL_YELLOW      // P2’s rank row
                                        : COL_WHITE;      // others
        gfx_text2(0, base_y + i*row_h, ranks[i], col, 1);
        }
        // optional arrows (kept)
        gfx_text2(70, base_y + i1*row_h, "<-P1", COL_RED,   1);
        gfx_text2(98, base_y + i2*row_h, "<-P2", COL_YELLOW, 1);

      }
      if (dt >= 6000u) goto_state(ST_OVERALL_RANKS2);
    } break;

    case ST_OVERALL_RANKS2: {
      if (!g_drawn_once){
        g_drawn_once = true;

        /* compute this attempt’s rank indices for both players */
        int i1 = rank_index_from_percent(g_p1_rank_pct);
        int i2 = rank_index_from_percent(g_p2_rank_pct);
        if (i1 < 0) i1 = 0; if (i1 > 8) i1 = 8;
        if (i2 < 0) i2 = 0; if (i2 > 8) i2 = 8;

        /* record this attempt exactly once per player */
        rankhist_add_two(1, i1);
        rankhist_add_two(2, i2);

        /* fetch history counts */
        uint16_t h1[9], h2[9];
        rankhist_get_two(h1, h2);

        gfx_clear(COL_BLACK);
        gfx_header("OVERALL RANKS", COL_WHITE);
        ui_sep_h(18);

        static const char* ranks[] = {
          "Challenger","Grandmaster","Master","Diamond",
          "Platinum","Gold","Silver","Bronze","Iron"
        };

        const int base_y = 24, row_h = 10;
        for (int i = 0; i < 9; ++i){
          char line[32];
          /* e.g., "Gold 35%   P1: 3 P2: 5" */
          snprintf(line, sizeof(line), "%-11s P1:%1u P2:%1u",
                   ranks[i], (unsigned)h1[i], (unsigned)h2[i]);

          /* highlight rows for this round’s ranks */
          uint16_t col = (i == i1 && i == i2) ? COL_MAGENTA  // both same rank
                           : (i == i1)        ? COL_CYAN     // P1’s rank row
                           : (i == i2)        ? COL_YELLOW   // P2’s rank row
                                              : COL_WHITE;   // others

          gfx_text2(0, base_y + i*row_h, line, col, 1);
        }
      }

      if (dt >= 6000u){
        goto_state(ST_BRAND);
      }
    } break;

    case ST_ENDING_SCENE: {
      if (g_dirty) {
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("MSU", COL_WHITE);

        uint8_t x = (uint8_t)((128 - MSU_LOGO_W) / 2);
        uint8_t y = 24;                
        gfx_blit565(x, y, MSU_LOGO_W, MSU_LOGO_H, MSU_LOGO);
      }
      if (dt >= 3000u){
        goto_state(ST_BRAND);     
      }
    } break;
  }
}
