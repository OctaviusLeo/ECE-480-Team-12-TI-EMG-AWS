#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "project.h"
#include "timer.h"      // millis()
#include "gfx.h"        // gfx_clear, gfx_header, gfx_text2, gfx_bar
#include "ssd1351.h"
#include "game.h"

void baseline_begin(uint32_t window_ms);

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

static inline int rank_index_from_percent(unsigned p){
  if (p >= 99) return 0;   // Challenger
  if (p >= 97) return 1;   // Grandmaster
  if (p >= 95) return 2;   // Master
  if (p >= 90) return 3;   // Diamond
  if (p >= 80) return 4;   // Platinum
  if (p >= 65) return 5;   // Gold
  if (p >= 45) return 6;   // Silver
  if (p >= 25) return 7;   // Bronze
  return 8;                // Iron
}

/* state machine */
typedef enum {
  ST_BRAND = 0,
  ST_SINGLE_PLAYER,
  ST_TWO_PLAYER,
  ST_COUNTDOWN_LABEL,
  ST_COUNT_3,
  ST_COUNT_2,
  ST_COUNT_1,
  ST_FLEX_CUE,
  ST_FLEXING,
  ST_RESULT,
  ST_RANKS,
  ST_RESULTS2,     // two-player: combined results
  ST_WINNER,       // two-player: animated bars to AVG Hz
  ST_DECLARE,      // two-player: declare winner
  ST_RANKS2        // two-player: rankings for both players
} game_state_t;

static bool         g_inited = false;
static game_state_t g_state;
static uint32_t     g_t0_ms;           // state start time
static float        g_latest_hz = 0.0f;
static uint8_t      g_latest_pct = 0;
static unsigned     g_last_rank_pct = 0;

static bool g_drawn_once = false;

static uint8_t  g_mode   = 0;   // 0 single, 1 two-player
static uint8_t  g_player = 1;   // current player (1 or 2)
static float    g_p1_avg_hz = 0.0f, g_p2_avg_hz = 0.0f;
static unsigned g_p1_rank_pct = 0,  g_p2_rank_pct = 0;

static uint8_t  g_w1 = 0, g_w2 = 0;      // precomputed bar widths (px)
static uint8_t  g_y1 = 24, g_y2 = 24;    // precomputed Y rows for rank arrows
static char     g_p1txt[24], g_p2txt[24];
static char     g_wmsg[16];

/* stats for the 10 s flex window */
static float        g_sum_hz = 0.0f;
static uint32_t     g_cnt_hz = 0;
static float        g_baseline_disp_hz = 0.0f;

/* draw the live flex body (below header) */
static void draw_flex_body(float hz, uint8_t percent){
  // clear body area y=18..127 to black
  gfx_bar(0, 18, 128, 110, COL_BLACK);

  // Hz line
  char line[32];
  snprintf(line, sizeof(line), "Hz: %.1f", hz);
  gfx_text2(6, 28, line, COL_WHITE, 2);

  char bline[32]; snprintf(bline, sizeof(bline), "Baseline: %.1f", g_baseline_disp_hz);
  gfx_text2(6, 44, bline, COL_YELLOW, 1);

  // percent bar
  const uint8_t bx = 6, by = 64, bw = 116, bh = 18;
  gfx_bar(bx, by, bw, bh, COL_GRAY);
  uint8_t fw = (uint8_t)((clampf(hz, 0.0f, 250.0f) * (float)bw) / 250.0f); // change bar fill to use Hz on a 0Ã¢â‚¬â€œ250 Hz scale
  if (fw > bw) fw = bw;
  gfx_bar(bx, by, fw, bh, COL_RED);

  // replace the % text under the bar with 0 Hz (left) and 250 Hz (right) labels
  gfx_text2(bx, by + bh + 6, "0 Hz", COL_WHITE, 1);
  { const char* rlabel = "250 Hz"; uint8_t rx = bx + bw - (uint8_t)(6 * 1 * strlen(rlabel)); gfx_text2(rx, by + bh + 6, rlabel, COL_WHITE, 1); }
}

static void goto_state(game_state_t s){
  g_state = s;
  g_t0_ms = millis();
  g_drawn_once = false;
  if (s == ST_BRAND) { g_player = 1u; baseline_begin(3000u); }
  if (s == ST_COUNTDOWN_LABEL) { baseline_begin(3000u); }
}

/* Public API */
void game_init(void){
  if (!g_inited){
    ssd1351_init();
    gfx_clear(COL_BLACK);
    g_inited = true;
  }
  g_sum_hz = 0.0f; g_cnt_hz = 0;
  goto_state(ST_BRAND);
}

void game_set_mode(uint8_t two_player){
  g_mode   = two_player ? 1u : 0u;
  g_player = 1u;
  goto_state(ST_BRAND);
}

void game_set_metrics(float hz, uint8_t intensity_pct){
  g_latest_hz  = hz;
  g_latest_pct = intensity_pct;
}

void game_set_baseline(float baseline_hz){
  g_baseline_disp_hz = baseline_hz;
}

/* Non-blocking state machine; call this often */
void game_tick(void){
  if (!g_inited) return;
  uint32_t now = millis();
  uint32_t dt  = now - g_t0_ms;

  switch(g_state){
    case ST_BRAND: {
      gfx_clear(COL_BLACK);
      gfx_header("Texas Instruments", COL_RED);

      // choose flow based on current mode, then stay in this case until 3s elapse
      if (g_mode == 0) {
        if (dt >= 3000u) { goto_state(ST_SINGLE_PLAYER); }
      } else {
        if (dt >= 3000u) { goto_state(ST_TWO_PLAYER); }
      }
      break;
    }

    case ST_SINGLE_PLAYER:
      gfx_clear(COL_BLACK);
      gfx_header("Mode: SINGLE PLAYER", COL_WHITE);
      if (dt >= 3000u){ goto_state(ST_COUNTDOWN_LABEL); }
      break;

    case ST_TWO_PLAYER:
      gfx_clear(COL_BLACK);
      gfx_header("Mode: TWO PLAYER", COL_WHITE);
      if (dt >= 3000u){ goto_state(ST_COUNTDOWN_LABEL); }
      break;

    case ST_COUNTDOWN_LABEL:
      if (g_mode == 1){
        char hdr[20]; snprintf(hdr, sizeof(hdr), "READY P%u?", (unsigned)g_player);
        gfx_header(hdr, COL_WHITE);
      } else {
        gfx_header("COUNTDOWN", COL_WHITE);
      }
      if (dt >= 2000u){ goto_state(ST_COUNT_3); }
      break;

    case ST_COUNT_3:
      gfx_header("3", COL_WHITE);
      if (dt >= 1000u){ goto_state(ST_COUNT_2); }
      break;

    case ST_COUNT_2:
      gfx_header("2", COL_YELLOW);
      if (dt >= 1000u){ goto_state(ST_COUNT_1); }
      break;

    case ST_COUNT_1:
      gfx_header("1", COL_GREEN);
      if (dt >= 1000u){ goto_state(ST_FLEX_CUE); }
      break;

    case ST_FLEX_CUE:
      gfx_header("FLEX!", COL_RED);
      if (dt >= 1000u){
        // reset stats for 10 s window
        g_sum_hz = 0.0f; g_cnt_hz = 0;
        goto_state(ST_FLEXING);
      }
      break;

    case ST_FLEXING: {
      // header with remaining seconds
      uint32_t elapsed = dt;
      const uint32_t FLEX_MS = 10000u;
      if (elapsed >= FLEX_MS){
        float avg = (g_cnt_hz > 0) ? (g_sum_hz / (float)g_cnt_hz) : 0.0f;
        float pct = clampf(avg, 0.0f, 250.0f);
        unsigned rank_pct = (unsigned)((pct * 100.0f / 250.0f) + 0.5f);

        if (g_mode == 0){
          goto_state(ST_RESULT);
        } else {
          if (g_player == 1u){
            g_p1_avg_hz = avg; g_p1_rank_pct = rank_pct;
            g_sum_hz = 0.0f; g_cnt_hz = 0;
            g_player = 2u;
            goto_state(ST_COUNTDOWN_LABEL);
          } else {
            g_p2_avg_hz = avg; g_p2_rank_pct = rank_pct;
            g_sum_hz = 0.0f; g_cnt_hz = 0;
            
            goto_state(ST_RESULTS2);
          }
        }
        break;
      }

      uint8_t remain_s = (uint8_t)((FLEX_MS - elapsed)/1000u);
      char hdr[24];
      snprintf(hdr, sizeof(hdr), "FLEX: %u", (unsigned)remain_s);
      gfx_header(hdr, COL_RED);

      // accumulate stats ignoring tiny values (< ~1.5 Hz)
      if (g_latest_hz >= 1.5f){
        g_sum_hz += g_latest_hz;
        g_cnt_hz++;
      }
    
      // live body (Hz text + red bar 0..100%)
      draw_flex_body(g_latest_hz, g_latest_pct);
    } break;

    case ST_RESULT: {
      gfx_clear(COL_BLACK);
      gfx_header("RESULT", COL_WHITE);

      float avg_hz = (g_cnt_hz > 0) ? (g_sum_hz / (float)g_cnt_hz) : 0.0f;
      float pct    = clampf(avg_hz, 0.0f, 250.0f);
      unsigned rank_pct = (unsigned)((pct * 100.0f / 250.0f) + 0.5f);
      g_last_rank_pct = rank_pct;

      char res1[48]; snprintf(res1, sizeof(res1), "Avg Hz: %.1f", avg_hz);
      gfx_text2(12, 28, res1, COL_WHITE, 1);

      char res0[48]; snprintf(res0, sizeof(res0), "Baseline: %.1f", g_baseline_disp_hz);
      gfx_text2(12, 40, res0, COL_YELLOW, 1);

      char res2[48]; snprintf(res2, sizeof(res2), "Stronger Than: %u%%", (unsigned)((pct * 100.0f / 250.0f) + 0.5f));
      gfx_text2(12, 70, res2, COL_RED, 1);

      char res3[48]; snprintf(res3, sizeof(res3), "of People!!!");
      gfx_text2(12, 80, res3, COL_WHITE, 1);

      // result bar
      const char* rank = rank_from_percent(rank_pct);
      char res4[48]; snprintf(res4, sizeof(res4), "Ranking: %s", rank);
      gfx_text2(10, 100, res4, COL_WHITE, 1);
      uint8_t bw = (uint8_t)((pct * 116.0f) / 250.0f);
      if (bw > 116) bw = 116;
      gfx_bar(6, 110, 116, 18, COL_GRAY);
      gfx_bar(6, 110, bw, 18, COL_GREEN);
      const uint8_t bx = 6, by = 110, bwpx = 116, bh = 18;
      const uint8_t cuts[] = {25,45,65,80,90,95,97,99};
      for (unsigned i = 0; i < sizeof(cuts)/sizeof(cuts[0]); ++i){
        uint8_t x = (uint8_t)(bx + (cuts[i] * bwpx) / 100);
        gfx_bar(x, by, 1, bh, COL_WHITE); // vertical separator; use a gray constant if available
      }

      // hold ~6 s then restart flow
      if (dt >= 6000u){
        goto_state(ST_RANKS);
      }
    } break;

    case ST_RESULTS2: {
      gfx_clear(COL_BLACK);
      gfx_header("RESULTS", COL_WHITE);

      // top: Player 1
      char l1[32]; snprintf(l1, sizeof(l1), "P1 Avg Hz: %.1f", g_p1_avg_hz);
      gfx_text2(6, 28, l1, COL_WHITE, 1);
      const char* r1 = rank_from_percent(g_p1_rank_pct);
      char l1r[32]; snprintf(l1r, sizeof(l1r), "Rank: %s", r1);
      gfx_text2(6, 38, l1r, COL_WHITE, 1);

      // bottom: Player 2
      char l2[32]; snprintf(l2, sizeof(l2), "P2 Avg Hz: %.1f", g_p2_avg_hz);
      gfx_text2(6, 70, l2, COL_RED, 1);
      const char* r2 = rank_from_percent(g_p2_rank_pct);
      char l2r[32]; snprintf(l2r, sizeof(l2r), "Rank: %s", r2);
      gfx_text2(6, 80, l2r, COL_RED, 1);

      // hold ~3 s, then animate bars
      if (dt >= 3000u){ goto_state(ST_WINNER); }
    } break;

    // ST_WINNER case
    case ST_WINNER: {
    gfx_clear(COL_BLACK);
    gfx_header("WINNER?", COL_RED);

    if (dt >= 3000u){ goto_state(ST_DECLARE); }
  } break;

    case ST_DECLARE: {
      gfx_clear(COL_BLACK);
      gfx_header("RESULT", COL_WHITE);
      const char* msg =
        (g_p1_avg_hz > g_p2_avg_hz) ? "WINNER: P1!" :
        (g_p2_avg_hz > g_p1_avg_hz) ? "WINNER: P2!" : "TIE";
      gfx_text2(0, 62, msg, COL_GOLD, 2);

      if (dt >= 2000u){ goto_state(ST_RANKS2); }
    } break;

    // ST_RANKS2
    case ST_RANKS2: {
      gfx_clear(COL_RED);
      
      if (dt >= 2000u){ goto_state(ST_BRAND); }
    } break;

    case ST_RANKS: {
      if (!g_drawn_once){
        gfx_clear(COL_BLACK);
        gfx_header("RANKINGS", COL_RED);

          static const char* ranks[] = {
          "Challenger  1%"
          ,"Grandmaster 3%"
          ,"Master      5%"
          ,"Diamond     10%"
          ,"Platinum    20%"
          ,"Gold        35%"
          ,"Silver      55%"
          ,"Bronze      75%"
          ,"Iron        100%"
        };
        const int base_y = 24, row_h = 10;
        for (int i = 0; i < 9; ++i){
          gfx_text2(0, base_y + i*row_h, ranks[i], COL_WHITE, 1);
        }

        // mark "You" at your rank
        int idx = rank_index_from_percent(g_last_rank_pct);
        if (idx < 0) idx = 0; if (idx > 8) idx = 8;
        gfx_text2(110, base_y + idx*row_h, "<-", COL_YELLOW, 1);

        g_drawn_once = true;   // draw only once for the 6 s hold
      }

      // hold 6 s, then recycle
      if (dt >= 6000u){ goto_state(ST_BRAND); }
    } break;
  }
}