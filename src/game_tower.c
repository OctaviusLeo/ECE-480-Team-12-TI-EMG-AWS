#include <stdio.h>
#include "timer.h"
#include "gfx.h"
#include "project.h"
#include "game.h"
#include "choice_input.h"
#include "tower_data.h"
#include "game_tower_logo.h"
#include "tower_minotaur.h"   // kept for future enemy art integration

typedef enum {
  TWS_LOGO = 0,
  TWS_INTRO,
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
static uint8_t       g_floor;        // 0..24
static float         g_sum_hz;
static uint32_t      g_cnt_hz;
static float         g_player_mult;  // cumulative item effects on player
static uint8_t       g_last_choice_b; // 1 if last choice was B (enemy buff), else 0

static bool          g_dirty;        // true = need to (re)draw this state's screen

static void t_goto(tower_state_t ns){
  g_ts   = ns;
  g_t0   = millis();
  g_dirty = true;
}

void game_tower_init(void){
  g_floor        = 0u;
  g_player_mult  = 1.0f;
  g_sum_hz       = 0.0f;
  g_cnt_hz       = 0u;
  g_last_choice_b = 0u;
  t_goto(TWS_LOGO);
}

bool game_tower_tick(void){
  uint32_t dt = millis() - g_t0;

  float   hz   = 0.0f;
  float   base = 0.0f;
  uint8_t pct  = 0;
  game_get_metrics(&hz, &pct, &base);
  (void)base;
  (void)pct;

  switch(g_ts){

    case TWS_LOGO: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);

        gfx_blit565(
          (128 - GAME_TOWER_LOGO_W) / 2,
          (128 - GAME_TOWER_LOGO_H) / 2,
          GAME_TOWER_LOGO_W,
          GAME_TOWER_LOGO_H,
          GAME_TOWER_LOGO
        );
      }
      if (dt >= 3000u){
        t_goto(TWS_INTRO);
      }
    } break;

    case TWS_INTRO: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("TOWER MODE", COL_RED);
        gfx_text2(0, 120, "Challenge 25 floors.", COL_WHITE, 1);
      }
      if (dt >= 5000u){
        t_goto(TWS_FLOOR_INTRO);
      }
    } break;

    case TWS_FLOOR_INTRO: {
      if (g_dirty){
        g_dirty = false;
        uint16_t foe = g_tower_enemy_hz[g_floor];

        gfx_clear(COL_BLACK);
        gfx_header("FLOOR", COL_WHITE);
        char line[40];
        snprintf(line, sizeof(line),
                 "Floor %u  Target: %u Hz",
                 (unsigned)(g_floor + 1u),
                 (unsigned)foe);
        gfx_text2(6, 40, line, COL_YELLOW, 1);
        gfx_text2(0, 64, "Choose your item.", COL_DKGRAY, 1);
        choice_draw_hint(80);
      }
      if (dt >= 5000u){
        t_goto(TWS_CHOOSE);
      }
    } break;

    case TWS_CHOOSE: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("CHOOSE", COL_WHITE);
        gfx_text2(6, 32, "A) Tonic (+5% you)",  COL_CYAN,   1);
        gfx_text2(6, 46, "B) Hex (+10% enemy)", COL_YELLOW, 1);
        choice_draw_hint(80);

        // reset for this floor
        g_last_choice_b = 0u;
      }

      choice_t ch = choice_from_hz(hz, 50.0f);
      if (ch == CHOICE_A){
        g_player_mult *= 1.05f;      // stackable buff
        g_last_choice_b = 0u;
      } else if (ch == CHOICE_B){
        g_last_choice_b = 1u;        // enemy harder on this floor
      }

      if (dt >= 5000u){
        g_sum_hz = 0.0f;
        g_cnt_hz = 0u;
        t_goto(TWS_BATTLE);
      }
    } break;

    case TWS_BATTLE: {
      const uint32_t FLEX_MS = 10000u;

      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("BATTLE", COL_RED);
      }

      if (dt >= FLEX_MS){
        t_goto(TWS_RESULT);
        break;
      }

      // countdown text
      uint8_t remain_s = (uint8_t)((FLEX_MS - dt) / 1000u);
      char line[32];
      snprintf(line, sizeof(line), "Flex... %us left", (unsigned)remain_s);
      gfx_bar(0, 60, 128, 16, COL_BLACK);
      gfx_text2(6, 64, line, COL_WHITE, 1);

      if (hz >= 1.5f){
        g_sum_hz += hz;
        g_cnt_hz++;
      }
    } break;

    case TWS_RESULT: {
      if (g_dirty){
        g_dirty = false;

        uint16_t foe0 = g_tower_enemy_hz[g_floor];

        float enemy_mult = g_last_choice_b ? 1.10f : 1.00f;
        float enemy      = (float)foe0 * enemy_mult;
        float avg        = (g_cnt_hz ? (g_sum_hz / (float)g_cnt_hz) : 0.0f);
        float you        = avg * g_player_mult;

        gfx_clear(COL_BLACK);
        gfx_header("RESULT", COL_WHITE);

        char l1[40];
        char l2[40];
        snprintf(l1, sizeof(l1), "You: %.1f Hz",   you);
        snprintf(l2, sizeof(l2), "Enemy: %.1f Hz", enemy);
        gfx_text2(6, 36, l1, COL_CYAN,   1);
        gfx_text2(6, 50, l2, COL_YELLOW, 1);
        gfx_text2(6, 76,
                  (you >= enemy) ? "CLEAR" : "FAILED",
                  (you >= enemy) ? COL_GREEN : COL_RED,
                  2);
      }
      if (dt >= 5000u){
        t_goto(TWS_REWARD);
      }
    } break;

    case TWS_REWARD: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("REWARD", COL_WHITE);
        gfx_text2(6, 44, "Item granted.", COL_WHITE, 1);
        gfx_text2(6, 54, "Buff stacked.", COL_WHITE, 1);
      }
      if (dt >= 5000u){
        t_goto(TWS_NEXT);
      }
    } break;

    case TWS_NEXT: {
      uint8_t next = (uint8_t)(g_floor + 1u);
      if (next >= TOWER_FLOORS){
        t_goto(TWS_ENDING);
      } else {
        g_floor = next;
        t_goto(TWS_FLOOR_INTRO);
      }
    } break;

    case TWS_ENDING: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("TOWER CLEARED!", COL_WHITE);
      }
      if (dt >= 3000u){
        return true;  // tell game.c Tower mode is finished
      }
    } break;
  }

  return false;       // not finished yet
}
