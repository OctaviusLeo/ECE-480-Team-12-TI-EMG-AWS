#include <stdio.h>
#include "timer.h"
#include "gfx.h"
#include "project.h"
#include "game.h"
#include "choice_input.h"
#include "tower_data.h"

typedef enum {
  TWS_BRAND = 0,
  TWS_FLOOR_INTRO,
  TWS_CHOOSE,
  TWS_BATTLE,
  TWS_RESULT,
  TWS_REWARD,
  TWS_NEXT,
  TWS_ENDING
} tower_state_t;

static tower_state_t g_ts;
static uint32_t      g_t0;
static uint8_t       g_floor;      // 0..24
static float         g_sum_hz;
static uint32_t      g_cnt_hz;
static float         g_player_mult; // cumulative item effects (skeleton)

static void t_goto(tower_state_t ns){ g_ts = ns; g_t0 = millis(); }

void game_tower_init(void){
  g_floor = 0;
  g_player_mult = 1.0f;
  t_goto(TWS_BRAND);
}

void game_tower_tick(void){
  uint32_t dt = millis() - g_t0;
  float hz=0.0f, base=0.0f; uint8_t pct=0;
  game_get_metrics(&hz, &pct, &base);

  switch(g_ts){
    case TWS_BRAND:
      gfx_clear(COL_BLACK); gfx_header("TOWER MODE", COL_RED);
      gfx_text2(6, 42, "25 floors. Get stronger.", COL_WHITE, 1);
      if (dt >= 2000u) t_goto(TWS_FLOOR_INTRO);
      break;

    case TWS_FLOOR_INTRO: {
      uint16_t foe = g_tower_enemy_hz[g_floor];
      gfx_clear(COL_BLACK); gfx_header("FLOOR", COL_WHITE);
      char line[40]; snprintf(line, sizeof(line), "Floor %u  Target: %u Hz", (unsigned)(g_floor+1), (unsigned)foe);
      gfx_text2(6, 40, line, COL_YELLOW, 1);
      gfx_text2(6, 64, "Choose an item (A/B) with Hz", COL_DKGRAY, 1);
      choice_draw_hint(80);
      if (dt >= 2000u) t_goto(TWS_CHOOSE);
    } break;

    case TWS_CHOOSE: {
      gfx_clear(COL_BLACK); gfx_header("CHOOSE", COL_WHITE);
      gfx_text2(6, 32, "A) Tonic (+5% you)", COL_CYAN, 1);
      gfx_text2(6, 46, "B) Hex (+10% enemy)", COL_YELLOW, 1);
      choice_draw_hint(80);

      choice_t ch = choice_from_hz(hz, 50.0f);
      if (ch == CHOICE_A) g_player_mult *= 1.05f; // stackable buff
      else                ; // enemy harder (we’ll apply in RESULT)

      if (dt >= 1000u){ g_sum_hz=0.0f; g_cnt_hz=0; t_goto(TWS_BATTLE); }
    } break;

    case TWS_BATTLE: {
      const uint32_t FLEX_MS = 10000u;
      gfx_clear(COL_BLACK); gfx_header("BATTLE", COL_RED);
      char line[32]; snprintf(line, sizeof(line), "Flex... %us left", (unsigned)((FLEX_MS - dt)/1000u));
      gfx_text2(6, 64, line, COL_WHITE, 1);

      if (hz >= 1.5f){ g_sum_hz += hz; g_cnt_hz++; }
      if (dt >= FLEX_MS) t_goto(TWS_RESULT);
    } break;

    case TWS_RESULT: {
      uint16_t foe0 = g_tower_enemy_hz[g_floor];
      // Apply last choice effect B (+10% enemy) heuristically: check dt from CHOOSE
      // (Skeleton: just always apply +10% enemy for B when chosen. Keep a static.)
      static uint8_t last_choice_b = 0;
      // Hack: infer from buff change (if no A buff added, assume B). In real code, track explicitly in CHOOSE.
      last_choice_b = 0; // set this properly when you wire choice state

      float enemy = (float)foe0 * (last_choice_b ? 1.10f : 1.00f);
      float avg   = (g_cnt_hz ? (g_sum_hz / (float)g_cnt_hz) : 0.0f);
      float you   = avg * g_player_mult;

      gfx_clear(COL_BLACK); gfx_header("RESULT", COL_WHITE);
      char l1[40]; snprintf(l1, sizeof(l1), "You: %.1f Hz", you);
      char l2[40]; snprintf(l2, sizeof(l2), "Enemy: %.1f Hz", enemy);
      gfx_text2(6, 36, l1, COL_CYAN, 1);
      gfx_text2(6, 50, l2, COL_YELLOW, 1);
      gfx_text2(6, 76, (you >= enemy) ? "CLEAR" : "FAILED", (you>=enemy)?COL_GREEN:COL_RED, 2);

      if (dt >= 2000u) t_goto(TWS_REWARD);
    } break;

    case TWS_REWARD:
      gfx_clear(COL_BLACK); gfx_header("REWARD", COL_WHITE);
      gfx_text2(6, 44, "Item granted. Buff stacked.", COL_WHITE, 1);
      if (dt >= 1500u) t_goto(TWS_NEXT);
      break;

    case TWS_NEXT: {
      uint8_t next = (uint8_t)(g_floor + 1u);
      if (next >= TOWER_FLOORS) t_goto(TWS_ENDING);
      else { g_floor = next; t_goto(TWS_FLOOR_INTRO); }
    } break;

    case TWS_ENDING:
      gfx_clear(COL_BLACK); gfx_header("TOWER CLEARED!", COL_WHITE);
      if (dt >= 3000u) t_goto(TWS_BRAND);
      break;
  }
}
