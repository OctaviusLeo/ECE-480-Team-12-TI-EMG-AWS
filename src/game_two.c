#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "timer.h"
#include "gfx.h"
#include "ssd1351.h"
#include "game.h"   // baseline_begin(), game_get_metrics()
#include "project.h"

/* helpers local to two-player */
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
  ST_PMODE,              // "Mode: TWO PLAYER"
  ST_COUNTDOWN_LABEL,    // READY P1? or READY P2?
  ST_COUNT_3, ST_COUNT_2, ST_COUNT_1,
  ST_FLEX_CUE, ST_FLEXING,
  ST_RESULTS2, ST_WINNER, ST_DECLARE, ST_RANKS2
} tp_state_t;

static tp_state_t g_state;
static uint32_t   g_t0_ms;
static bool       g_drawn_once = false;

static uint8_t  g_player = 1;
static float    g_p1_avg_hz = 0.0f, g_p2_avg_hz = 0.0f;
static unsigned g_p1_rank_pct = 0,  g_p2_rank_pct = 0;

static void goto_state(tp_state_t s){
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
  gfx_bar(bx, by, fw, bh, COL_GREEN);
  gfx_text2(bx, by + bh + 6, "0 Hz", COL_WHITE, 1);
  const char* rlabel="250 Hz";
  uint8_t rx = bx + bw - (uint8_t)(6 * 1 * strlen(rlabel));
  gfx_text2(rx, by + bh + 6, rlabel, COL_WHITE, 1);
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

  switch(g_state){
    case ST_BRAND: {
      gfx_clear(COL_BLACK);
      gfx_header("Texas Instruments", COL_RED);
      if (dt >= 3000u) goto_state(ST_PMODE);
    } break;

    case ST_PMODE:
      gfx_clear(COL_BLACK);
      gfx_header("Mode: TWO PLAYER", COL_WHITE);
      if (dt >= 3000u) goto_state(ST_COUNTDOWN_LABEL);
      break;

    case ST_COUNTDOWN_LABEL: {
      char hdr[20]; snprintf(hdr, sizeof(hdr), "READY P%u?", (unsigned)g_player);
      gfx_header(hdr, COL_WHITE);
      if (dt >= 2000u) goto_state(ST_COUNT_3);
    } break;

    case ST_COUNT_3: gfx_header("3", COL_WHITE);   if (dt >= 1000u) goto_state(ST_COUNT_2); break;
    case ST_COUNT_2: gfx_header("2", COL_YELLOW);  if (dt >= 1000u) goto_state(ST_COUNT_1); break;
    case ST_COUNT_1: gfx_header("1", COL_GREEN);   if (dt >= 1000u) goto_state(ST_FLEX_CUE); break;

    case ST_FLEX_CUE:
      gfx_header("FLEX!", COL_RED);
      if (dt >= 1000u) goto_state(ST_FLEXING);
      break;

    case ST_FLEXING: {
      static float    sum_hz = 0.0f;
      static uint32_t cnt_hz = 0;
      if (!g_drawn_once){ sum_hz = 0.0f; cnt_hz = 0; g_drawn_once = true; }

      const uint32_t FLEX_MS = 10000u;
      if (dt >= FLEX_MS){
        float avg = (cnt_hz > 0) ? (sum_hz / (float)cnt_hz) : 0.0f;
        float pct250 = clampf(avg, 0.0f, 250.0f);
        unsigned rank_pct = (unsigned)((pct250 * 100.0f / 250.0f) + 0.5f);

        if (g_player == 1u){
          g_p1_avg_hz = avg; g_p1_rank_pct = rank_pct;
          g_player = 2u; g_drawn_once = false;
          goto_state(ST_COUNTDOWN_LABEL);
        } else {
          g_p2_avg_hz = avg; g_p2_rank_pct = rank_pct;
          g_drawn_once = false;
          goto_state(ST_RESULTS2);
        }
        break;
      }

      uint8_t remain_s = (uint8_t)((FLEX_MS - dt)/1000u);
      char hdr[24]; snprintf(hdr, sizeof(hdr), "FLEX: %u", (unsigned)remain_s);
      gfx_header(hdr, COL_RED);

      if (hz >= 1.5f){ sum_hz += hz; cnt_hz++; }
      draw_flex_body(hz, base);
    } break;

    case ST_RESULTS2: {
      gfx_clear(COL_BLACK);
      gfx_header("RESULTS", COL_WHITE);

      char l1[32]; snprintf(l1, sizeof(l1), "P1 Avg Hz: %.1f", g_p1_avg_hz);
      gfx_text2(6, 28, l1, COL_WHITE, 1);
      const char* r1 = rank_from_percent(g_p1_rank_pct);
      char l1r[32]; snprintf(l1r, sizeof(l1r), "Rank: %s", r1);
      gfx_text2(6, 38, l1r, COL_WHITE, 1);

      char l2[32]; snprintf(l2, sizeof(l2), "P2 Avg Hz: %.1f", g_p2_avg_hz);
      gfx_text2(6, 70, l2, COL_WHITE, 1);
      const char* r2 = rank_from_percent(g_p2_rank_pct);
      char l2r[32]; snprintf(l2r, sizeof(l2r), "Rank: %s", r2);
      gfx_text2(6, 80, l2r, COL_WHITE, 1);

      if (dt >= 3000u) goto_state(ST_WINNER);
    } break;

    case ST_WINNER: {
      if (!g_drawn_once){
        g_drawn_once = true;
        gfx_clear(COL_BLACK);
        gfx_header("YOUR RESULTS", COL_RED);

        const uint8_t bx=6, bw=116, bh=12;
        const uint8_t by1=46, by2=86;
        uint8_t w1 = (uint8_t)((clampf(g_p1_avg_hz,0,250)*bw)/250.0f);
        uint8_t w2 = (uint8_t)((clampf(g_p2_avg_hz,0,250)*bw)/250.0f);

        gfx_bar(bx, by1, bw, bh, COL_GRAY);  gfx_bar(bx, by1, w1, bh, COL_GREEN);
        gfx_bar(bx, by2, bw, bh, COL_GRAY);  gfx_bar(bx, by2, w2, bh, COL_GREEN);

        const uint8_t cuts[] = {25,45,65,80,90,95,97,99};
        for (unsigned i=0;i<sizeof(cuts)/sizeof(cuts[0]);++i){
          uint8_t x = (uint8_t)(bx + (cuts[i] * bw) / 100u);
          gfx_bar(x, by1, 1, bh, COL_WHITE);
          gfx_bar(x, by2, 1, bh, COL_WHITE);
        }

        char p1txt[24], p2txt[24];
        snprintf(p1txt, sizeof(p1txt), "P1: %.1f Hz", g_p1_avg_hz);
        snprintf(p2txt, sizeof(p2txt), "P2: %.1f Hz", g_p2_avg_hz);
        gfx_text2(6, 34, p1txt, COL_WHITE, 1);
        gfx_text2(6, 74, p2txt, COL_WHITE, 1);

      }
      if (dt >= 6000u) goto_state(ST_DECLARE);
    } break;

    case ST_DECLARE: {
      gfx_clear(COL_BLACK);
      gfx_header("WHO WINS?", COL_WHITE);
      const char* msg = (g_p1_avg_hz > g_p2_avg_hz) ? "WINNER: P1!" :
                        (g_p2_avg_hz > g_p1_avg_hz) ? "WINNER: P2!" : "TIE";
      gfx_text2(0, 62, msg, COL_GOLD, 2);
      if (dt >= 2000u) goto_state(ST_RANKS2);
    } break;

    case ST_RANKS2: {
      if (!g_drawn_once){
        g_drawn_once = true;
        gfx_clear(COL_BLACK);
        gfx_header("RANKINGS", COL_RED);
        static const char* ranks[] = {
          "Challenger","Grandmaster","Master","Diamond",
          "Platinum","Gold","Silver","Bronze","Iron"
        };
        const int base_y=24, row_h=10;
        for (int i=0;i<9;++i) gfx_text2(0, base_y+i*row_h, ranks[i], COL_WHITE, 1);
        int i1 = rank_index_from_percent(g_p1_rank_pct);
        int i2 = rank_index_from_percent(g_p2_rank_pct);
        if (i1<0) i1=0; if (i1>8) i1=8;
        if (i2<0) i2=0; if (i2>8) i2=8;
        gfx_text2(88, base_y + i1*row_h, "<- P1", COL_CYAN,   1);
        gfx_text2(98, base_y + i2*row_h, "<- P2", COL_YELLOW, 1);
      }
      if (dt >= 6000u) goto_state(ST_BRAND);
    } break;
  }
}
