#include <stdio.h>
#include "timer.h"
#include "gfx.h"
#include "project.h"
#include "game.h"
#include "choice_input.h"
#include "tower_data.h"
#include "game_tower_logo.h"
#include "tower_minotaur.h"   // kept for future enemy art integration
#include "tower_black_knight.h"
#include "tower_dragon.h"
#include "tower_demon.h"
#include "tower_orc.h"
#include "tower_werewolf.h"
#include "you_died.h"
#include "chest.h"
#include "story_items.h"


typedef enum {
  TWS_LOGO = 0,
  TWS_INTRO,
  TWS_FLOOR_INTRO,
  TWS_CHOOSE,
  TWS_BATTLE,
  TWS_RESULT,
  TWS_REWARD,
  TWS_DEATH,
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
static float         g_last_you;
static float         g_last_enemy;

// Per-floor item state (shared item definitions)
static story_item_t        g_tower_equipped;
static const story_item_t *g_tower_itemA;
static const story_item_t *g_tower_itemB;
static float               g_enemy_mult_floor;

static bool          g_dirty;        // true = need to (re)draw this state's screen

// Draw the Tower battle Hz bar at the bottom of the screen.
// hz        = current player Hz
// target_hz = enemy's target Hz for this floor
static void tower_draw_hz_bar(float hz, float target_hz)
{
  const uint8_t bar_x = 4;
  const uint8_t bar_y = 112;     // bottom band
  const uint8_t bar_w = 120;
  const uint8_t bar_h = 12;

  if (target_hz <= 0.0f) target_hz = 1.0f;

  float max_hz = target_hz * 1.5f;
  if (max_hz < target_hz) max_hz = target_hz;   // safety

  if (hz < 0.0f) hz = 0.0f;
  if (hz > max_hz) hz = max_hz;

  // Clear band
  gfx_bar(bar_x, bar_y, bar_w, bar_h, COL_DKGRAY);

  // Current Hz bar (green)
  uint8_t cur_w = (uint8_t)((hz / max_hz) * (float)bar_w + 0.5f);
  if (cur_w > bar_w) cur_w = bar_w;
  if (cur_w > 0u){
    gfx_bar(bar_x, bar_y, cur_w, bar_h, COL_GREEN);
  }

  // Threshold line (red)
  uint8_t th_x = bar_x + (uint8_t)((target_hz / max_hz) * (float)bar_w + 0.5f);
  if (th_x < bar_x) th_x = bar_x;
  if (th_x >= (uint8_t)(bar_x + bar_w)) th_x = (uint8_t)(bar_x + bar_w - 1u);
  gfx_bar(th_x, bar_y, 1, bar_h, COL_RED);
}

static void t_goto(tower_state_t ns){
  g_ts   = ns;
  g_t0   = millis();
  g_dirty = true;
}

void game_tower_init(void){
  g_floor         = 0u;
  g_player_mult   = 1.0f;
  g_sum_hz        = 0.0f;
  g_cnt_hz        = 0u;
  g_last_choice_b = 0u;        // no longer critical, but fine to leave
  g_enemy_mult_floor = 1.0f;
  g_tower_itemA   = &STORY_ITEMS[0];
  g_tower_itemB   = &STORY_ITEMS[(STORY_ITEMS_COUNT > 1u) ? 1u : 0u];
  g_tower_equipped = STORY_ITEMS[0];
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

        uint8_t x = (uint8_t)((128 - GAME_TOWER_LOGO_W) / 2);
        uint8_t y = (uint8_t)((128 - GAME_TOWER_LOGO_H) / 2);

        gfx_blit_pal4(x, y,
                      GAME_TOWER_LOGO_W, GAME_TOWER_LOGO_H,
                      GAME_TOWER_LOGO_IDX,
                      GAME_TOWER_LOGO_PAL);
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

        // Chest center-screen
        uint8_t cx = (uint8_t)((128 - CHEST_W) / 2);
        uint8_t cy = (uint8_t)((128 - CHEST_H) / 2);

        gfx_blit_pal4(cx, cy,
                      CHEST_W, CHEST_H,
                      CHEST_IDX,
                      CHEST_PAL);

        // Random two items out of STORY_ITEMS[]
        uint8_t i0, i1;
        story_items_pick_two(&i0, &i1);
        g_tower_itemA = &STORY_ITEMS[i0];
        g_tower_itemB = &STORY_ITEMS[i1];

        gfx_header("CHOOSE", COL_WHITE);

        char lineA[40];
        char lineB[40];
        snprintf(lineA, sizeof(lineA), "A) %s", g_tower_itemA->name);
        snprintf(lineB, sizeof(lineB), "B) %s", g_tower_itemB->name);

        gfx_text2(0, 110, lineA, COL_CYAN,   1);
        gfx_text2(0, 120, lineB, COL_YELLOW, 1);
        choice_draw_hint(80);

        g_enemy_mult_floor = 1.0f;   // reset per floor
      }

      choice_t ch = choice_from_hz(hz, 50.0f);
      const story_item_t *cur = (ch == CHOICE_A) ? g_tower_itemA : g_tower_itemB;

      if (dt >= 5000u){
        if (cur) {
          g_tower_equipped    = *cur;
          g_player_mult      *= g_tower_equipped.player_mult;  // stacks across floors
          g_enemy_mult_floor *= g_tower_equipped.enemy_mult;   // only this floor
        }
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

        // Select enemy sprite based on floor:
        // 0-4: minotaur, 5-9: werewolf, 10-14: orc, 15-19: black knight,
        // 20-24: demon, 25: dragon.
        uint8_t ew = 0, eh = 0;
        const uint8_t  *eidx = NULL;
        const uint16_t *epal = NULL;

        if      (g_floor <= 4u) {
          ew   = TOWER_MINOTAUR_W;
          eh   = TOWER_MINOTAUR_H;
          eidx = TOWER_MINOTAUR_IDX;
          epal = TOWER_MINOTAUR_PAL;
        } else if (g_floor <= 9u) {
          ew   = TOWER_WEREWOLF_W;
          eh   = TOWER_WEREWOLF_H;
          eidx = TOWER_WEREWOLF_IDX;
          epal = TOWER_WEREWOLF_PAL;
        } else if (g_floor <= 14u) {
          ew   = TOWER_ORC_W;
          eh   = TOWER_ORC_H;
          eidx = TOWER_ORC_IDX;
          epal = TOWER_ORC_PAL;
        } else if (g_floor <= 19u) {
          ew   = TOWER_BLACK_KNIGHT_W;
          eh   = TOWER_BLACK_KNIGHT_H;
          eidx = TOWER_BLACK_KNIGHT_IDX;
          epal = TOWER_BLACK_KNIGHT_PAL;
        } else if (g_floor <= 24u) {
          ew   = TOWER_DEMON_W;
          eh   = TOWER_DEMON_H;
          eidx = TOWER_DEMON_IDX;
          epal = TOWER_DEMON_PAL;
        } else { // floor 25 and any above safety
          ew   = TOWER_DRAGON_W;
          eh   = TOWER_DRAGON_H;
          eidx = TOWER_DRAGON_IDX;
          epal = TOWER_DRAGON_PAL;
        }

        // Draw enemy centered, leaving bottom band for Hz bar & text
        if (eidx && epal){
          uint8_t ex = (uint8_t)((128 - ew) / 2);
          uint8_t ey = 16;   // under header, above bottom bar
          gfx_blit_pal4(ex, ey,
                        ew, eh,
                        eidx,
                        epal);
        }
      }

      if (dt >= FLEX_MS){
        t_goto(TWS_RESULT);
        break;
      }

      // countdown text near bottom, but above Hz bar
      uint8_t remain_s = (uint8_t)((FLEX_MS - dt) / 1000u);
      char line[32];
      snprintf(line, sizeof(line), "Flex... %us left", (unsigned)remain_s);
      gfx_bar(0, 96, 128, 12, COL_BLACK);
      gfx_text2(6, 96, line, COL_WHITE, 1);

      // Live Hz bar overlaid at the very bottom
      {
        uint16_t foe = g_tower_enemy_hz[g_floor];
        tower_draw_hz_bar(hz, (float)foe);
      }

      if (hz >= 1.5f){
        g_sum_hz += hz;
        g_cnt_hz++;
      }
    } break;

    case TWS_RESULT: {
      if (g_dirty) {
        g_dirty = false;

        uint16_t foe0 = g_tower_enemy_hz[g_floor];
        g_last_enemy  = (float)foe0 * g_enemy_mult_floor;

        float avg     = (g_cnt_hz ? (g_sum_hz / (float)g_cnt_hz) : 0.0f);
        g_last_you    = avg * g_player_mult;

        gfx_clear(COL_BLACK); gfx_header("RESULT", COL_WHITE);
        char l1[40]; snprintf(l1, sizeof(l1), "You: %.1f Hz", g_last_you);
        char l2[40]; snprintf(l2, sizeof(l2), "Enemy: %.1f Hz", g_last_enemy);
        gfx_text2(6, 36, l1, COL_CYAN,   1);
        gfx_text2(6, 50, l2, COL_YELLOW, 1);
        gfx_text2(6, 76,
                  (g_last_you >= g_last_enemy) ? "CLEAR" : "FAILED",
                  (g_last_you >= g_last_enemy) ? COL_GREEN : COL_RED,
                  2);
      }

      if (dt >= 5000u) {
        if (g_last_you >= g_last_enemy) {
          t_goto(TWS_REWARD);  // floor cleared
        } else {
          t_goto(TWS_DEATH);   // floor failed
        }
      }
    } break;

    case TWS_REWARD: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("REWARD", COL_WHITE);
        gfx_text2(6, 40, "Equipped:", COL_DKGRAY, 1);
        gfx_text2(6, 54, g_tower_equipped.name, COL_WHITE, 1);
      }
      if (dt >= 5000u){
        t_goto(TWS_NEXT);
      }
    } break;

    case TWS_DEATH: {
      if (g_dirty) {
        g_dirty = false;
        gfx_clear(COL_BLACK);

        // Center the YOU_DIED image
        uint8_t x = (uint8_t)((128 - YOU_DIED_W) / 2);
        uint8_t y = (uint8_t)((128 - YOU_DIED_H) / 2);

        gfx_blit_pal4(x, y,
                      YOU_DIED_W, YOU_DIED_H,
                      YOU_DIED_IDX,
                      YOU_DIED_PAL);

        gfx_text2(18, 110, "Retrying in 3...", COL_RED, 1);
      }

      if (dt >= 3000u) {
        // Reset counters and retry the same floor (same g_floor)
        g_sum_hz = 0.0f;
        g_cnt_hz = 0u;
        t_goto(TWS_FLOOR_INTRO);
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
