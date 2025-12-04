#include <stdio.h>
#include <string.h>
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
#include "cheevos.h"

#define TOWER_FLEX_MENU_HZ 50.0f   // Hz needed to exit to menu after too many deaths
#define TOWER_CHOICE_SPLIT_HZ 50.0f   // NEW: A/B split threshold in Hz
#define TOWER_BAR_MAX_HZ 200.0f   // or 220/250 if highest floors are huge
#define CHOICE_BAR_MAX_HZ 50.0f   // adjust based on what “normal” player Hz looks like

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
  TWS_ENDING,
  TWS_FLEX_RETURN
} tower_state_t;

// Tower lore text blocks

static const char* g_tower_intro_lines[] = {
  "You gaze at the sky-",
  "high tower with anti",
  "-cipation. Some say",
  "it's filled with mys",
  "-teries and unknown.",
  "Other say it's unbea",
  "-table. However, you",
  "beg to differ. Takin",
  "-g steps, you enter."
};
static const uint8_t g_tower_intro_count =
    sizeof(g_tower_intro_lines)/sizeof(g_tower_intro_lines[0]);

static const char* g_tower_end_lines[] = {
  "The dragon lays dead",
  "at your feet. You've",
  "conquered the tower.",
  "However you weren't",
  "unscathed. Dragging",
  "your feet, you reach",
  "the edge and look do",
  "-wn. Closing your eye",
  "you let yourself go."
};
static const uint8_t g_tower_end_count =
    sizeof(g_tower_end_lines)/sizeof(g_tower_end_lines[0]);

// Typewriter-style lore: slowly reveal text over time.
// NOTE: does NOT clear the screen or draw header; caller does that once via g_dirty.
static void tower_draw_lore_typewriter(const char* const* lines,
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
    char buf[32];  // adjust if you ever have >31-char lines
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
static story_item_t        g_tower_prev_equipped; // previous item
static const story_item_t *g_tower_itemA;
static const story_item_t *g_tower_itemB;
static float               g_enemy_mult_floor;

static bool          g_dirty;        // true = need to (re)draw this state's screen

static uint8_t       g_tower_deaths = 0u;

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

  float max_hz = TOWER_BAR_MAX_HZ;
  if (max_hz < 1.0f) max_hz = 1.0f;

  if (target_hz > max_hz) target_hz = max_hz;

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

// Chest choice Hz bar (Tower): same visual as Story.
static void tower_choice_bar_static(void)
{
  const uint8_t bar_x = 4;
  const uint8_t bar_y = 60;
  const uint8_t bar_w = 120;
  const uint8_t bar_h = 8;

  gfx_bar(bar_x, bar_y, bar_w, bar_h, COL_DKGRAY);

  gfx_text2(bar_x,             bar_y - 10, "A", COL_CYAN,   1);
  gfx_text2(bar_x + bar_w - 6, bar_y - 10, "B", COL_YELLOW, 1);

  float max_hz = 80.0f;
  float th_hz  = TOWER_CHOICE_SPLIT_HZ;
  if (th_hz < 0.0f)     th_hz = 0.0f;
  if (th_hz > max_hz)   th_hz = max_hz;
  uint8_t th_x = bar_x + (uint8_t)((th_hz / max_hz) * (float)bar_w + 0.5f);
  if (th_x >= (uint8_t)(bar_x + bar_w)) th_x = (uint8_t)(bar_x + bar_w - 1u);
  gfx_bar(th_x, bar_y, 1, bar_h, COL_RED);
}

static void draw_choice_bar(float hz, float need_hz)
{
  const uint8_t bar_x = 4;
  const uint8_t bar_y = 60;
  const uint8_t bar_w = 120;
  const uint8_t bar_h = 10;

  if (need_hz <= 0.0f) need_hz = 1.0f;
  float max_hz = CHOICE_BAR_MAX_HZ;
  if (max_hz < 1.0f) max_hz = 1.0f;

  if (hz < 0.0f) hz = 0.0f;
  if (hz > max_hz) hz = max_hz;

  if (need_hz > max_hz) need_hz = max_hz;

  // Background
  gfx_bar(bar_x, bar_y, bar_w, bar_h, COL_DKGRAY);

  // Current Hz (green)
  uint8_t cur_w = (uint8_t)((hz / max_hz) * (float)bar_w + 0.5f);
  if (cur_w > bar_w) cur_w = bar_w;
  if (cur_w > 0u){
    gfx_bar(bar_x, bar_y, cur_w, bar_h, COL_GREEN);
  }

  // Threshold red line at need_hz
  uint8_t th_x = bar_x + (uint8_t)((need_hz / max_hz) * (float)bar_w + 0.5f);
  if (th_x < bar_x) th_x = bar_x;
  if (th_x >= (uint8_t)(bar_x + bar_w)) th_x = (uint8_t)(bar_x + bar_w - 1u);
  gfx_bar(th_x, bar_y, 1, bar_h, COL_RED);

  // Labels A / B (if this bar is for item choice)
  gfx_text2(bar_x + 2,           bar_y - 8, "A", COL_WHITE, 1);
  gfx_text2(bar_x + bar_w - 8u,  bar_y - 8, "B", COL_WHITE, 1);
}

static void tower_choice_bar_update(float hz)
{
  const uint8_t bar_x = 4;
  const uint8_t bar_y = 60;
  const uint8_t bar_w = 120;
  const uint8_t bar_h = 8;

  float max_hz = 80.0f;
  if (hz < 0.0f)     hz = 0.0f;
  if (hz > max_hz)   hz = max_hz;

  gfx_bar(bar_x, bar_y, bar_w, bar_h, COL_DKGRAY);

  uint8_t cur_w = (uint8_t)((hz / max_hz) * (float)bar_w + 0.5f);
  if (cur_w > bar_w) cur_w = bar_w;
  if (cur_w > 0u){
    gfx_bar(bar_x, bar_y, cur_w, bar_h, COL_GREEN);
  }

  float th_hz = TOWER_CHOICE_SPLIT_HZ;
  if (th_hz < 0.0f)     th_hz = 0.0f;
  if (th_hz > max_hz)   th_hz = max_hz;
  uint8_t th_x = bar_x + (uint8_t)((th_hz / max_hz) * (float)bar_w + 0.5f);
  if (th_x >= (uint8_t)(bar_x + bar_w)) th_x = (uint8_t)(bar_x + bar_w - 1u);
  gfx_bar(th_x, bar_y, 1, bar_h, COL_RED);
}

static void tower_maybe_unlock_floor_cheevo(uint8_t floor){
  // floor is 0-based (0..24)
  switch(floor){
    case 4:   // just cleared floor 5
      cheevos_unlock(ACH_TOWER_5);
      break;
    case 9:   // just cleared floor 10
      cheevos_unlock(ACH_TOWER_10);
      break;
    case 14:  // just cleared floor 15
      cheevos_unlock(ACH_TOWER_15);
      break;
    case 19:  // just cleared floor 20
      cheevos_unlock(ACH_TOWER_20);
      break;
    case 23:  // just cleared floor 24
      cheevos_unlock(ACH_TOWER_24);
      break;
    case 24:  // just cleared floor 25 (final boss)
      cheevos_unlock(ACH_TOWER_CLEAR);
      break;
    default:
      break;
  }
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
  g_tower_prev_equipped = STORY_ITEMS[0];
  g_tower_deaths  = 0u;
  cheevos_unlock(ACH_TOWER_START);
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
      if (dt >= 5000u){
        t_goto(TWS_INTRO);
      }
    } break;

    case TWS_INTRO: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("TOWER MODE", COL_WHITE);
      }

      tower_draw_lore_typewriter(g_tower_intro_lines,
                                g_tower_intro_count,
                                dt,
                                50u);

      if (dt >= 13000u){
        t_goto(TWS_FLOOR_INTRO);
      }
    } break;

    case TWS_FLOOR_INTRO: {
      if (g_dirty){
        g_dirty = false;
        uint16_t foe = g_tower_enemy_hz[g_floor];

        gfx_clear(COL_BLACK);
        gfx_header("FLOOR", COL_WHITE);

        // Decide enemy type string based on floor range
        const char *enemy = "Unknown";
        if      (g_floor <= 4u)  enemy = "Minotaur";
        else if (g_floor <= 9u)  enemy = "Werewolf";
        else if (g_floor <= 14u) enemy = "Orc";
        else if (g_floor <= 19u) enemy = "Black Knight";
        else if (g_floor <= 23u) enemy = "Demon";
        else                     enemy = "Dragon";

        char line[40];

        // Enemy type
        snprintf(line, sizeof(line), "Enemy: %s", enemy);
        gfx_text2(4, 40, line, COL_YELLOW, 1);

        // Hz needed to defeat (per-floor target)
        snprintf(line, sizeof(line), "Need: %u Hz", (unsigned)foe);
        gfx_text2(4, 52, line, COL_WHITE, 1);

        // Previously equipped item
        snprintf(line, sizeof(line), "Prev: %s", g_tower_prev_equipped.name);
        gfx_text2(4, 84, line, COL_RED, 1);

        // Currently equipped item
        snprintf(line, sizeof(line), "Now:  %s", g_tower_equipped.name);
        gfx_text2(4, 96, line, COL_GREEN, 1);
      }

      // After a short delay, move on to the chest (item choice) screen
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

        // static A/B bar over chest
        //tower_choice_bar_static();
        draw_choice_bar(hz, TOWER_CHOICE_SPLIT_HZ);
      }

      // live update of Hz bar
      tower_choice_bar_update(hz);

      choice_t ch = choice_from_hz(hz, TOWER_CHOICE_SPLIT_HZ);
      const story_item_t *cur = (ch == CHOICE_A) ? g_tower_itemA : g_tower_itemB;

      if (dt >= 5000u){
        if (cur) {
          g_tower_prev_equipped = g_tower_equipped;            // remember old
          g_tower_equipped      = *cur;                        // set new
          g_player_mult        *= g_tower_equipped.player_mult;  // stacks across floors
          g_enemy_mult_floor   *= g_tower_equipped.enemy_mult;   // only this floor
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

        // Final floor: always Dragon
        // (Assumes last floor index is TOWER_FLOORS-1)
        bool final_floor = ((uint8_t)(g_floor + 1u) >= TOWER_FLOORS);

        if (final_floor) {
          ew   = TOWER_DRAGON_W;
          eh   = TOWER_DRAGON_H;
          eidx = TOWER_DRAGON_IDX;
          epal = TOWER_DRAGON_PAL;
        } else {
          // Non-final floors: random among 5 enemies
          uint32_t r = millis();
          r ^= (uint32_t)g_floor * 17u;     // cheap decorrelation
          uint8_t pick = (uint8_t)(r % 5u); // 0..4

          switch (pick) {
            case 0: // Minotaur
              ew   = TOWER_MINOTAUR_W;
              eh   = TOWER_MINOTAUR_H;
              eidx = TOWER_MINOTAUR_IDX;
              epal = TOWER_MINOTAUR_PAL;
              break;
            case 1: // Werewolf
              ew   = TOWER_WEREWOLF_W;
              eh   = TOWER_WEREWOLF_H;
              eidx = TOWER_WEREWOLF_IDX;
              epal = TOWER_WEREWOLF_PAL;
              break;
            case 2: // Orc
              ew   = TOWER_ORC_W;
              eh   = TOWER_ORC_H;
              eidx = TOWER_ORC_IDX;
              epal = TOWER_ORC_PAL;
              break;
            case 3: // Black Knight
              ew   = TOWER_BLACK_KNIGHT_W;
              eh   = TOWER_BLACK_KNIGHT_H;
              eidx = TOWER_BLACK_KNIGHT_IDX;
              epal = TOWER_BLACK_KNIGHT_PAL;
              break;
            default: // 4: Demon
              ew   = TOWER_DEMON_W;
              eh   = TOWER_DEMON_H;
              eidx = TOWER_DEMON_IDX;
              epal = TOWER_DEMON_PAL;
              break;
          }
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
        float foe_target = (float)g_tower_enemy_hz[g_floor] * g_enemy_mult_floor;
        tower_draw_hz_bar(hz, foe_target);
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
                  (g_last_you >= g_last_enemy) ? "VICTORY" : "DEFEAT",
                  (g_last_you >= g_last_enemy) ? COL_GREEN : COL_RED,
                  3);
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

        gfx_text2(6, 32, "Prev:", COL_WHITE, 1);
        gfx_text2(6, 44, g_tower_prev_equipped.name, COL_RED, 1);

        gfx_text2(6, 64, "Now:",  COL_WHITE, 1);
        gfx_text2(6, 76, g_tower_equipped.name,     COL_GREEN, 1);
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

        if (g_tower_deaths + 1u < 3u) {
          gfx_text2(30, 110, "Retrying...", COL_RED, 1);
        } else {
          gfx_text2(4, 110, "Too many deaths...", COL_RED, 1);
        }
      }

      if (dt >= 3000u) {
        // Count this death
        g_tower_deaths++;

        // Reset counters and retry logic
        g_sum_hz = 0.0f;
        g_cnt_hz = 0u;

        if (g_tower_deaths >= 3u) {
          // go to flex-to-menu instead of floor intro
          t_goto(TWS_FLEX_RETURN);
        } else {
          t_goto(TWS_FLOOR_INTRO);
        }
      }
    } break;

    case TWS_FLEX_RETURN: {
      if (g_dirty) {
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("GO BACK", COL_WHITE);
        gfx_bar(0, 18, 128, 1, COL_DKGRAY);

        gfx_text2(4, 36, "You have fallen 3+", COL_RED,   1);
        gfx_text2(4, 48, "in the Tower.",            COL_RED,   1);
        gfx_text2(4, 60, "Flex to return...",   COL_WHITE, 1);
        gfx_text2(4, 72, "you're weak.",           COL_WHITE, 1);

        char line[40];
        snprintf(line, sizeof(line),
                 "Need: %.1f Hz", TOWER_FLEX_MENU_HZ);
        gfx_text2(4, 92, line, COL_CYAN, 1);
        }
        // strong flex -> back to main menu
        if (hz >= TOWER_FLEX_MENU_HZ) {
            g_tower_deaths = 0u;   // optional reset
            return true;           // tell is done -> menu
        }

        // wait 5 seconds -> retry same chapter
        if (dt >= 5000u) {
            g_sum_hz = 0.0f;
            g_cnt_hz = 0u;
            t_goto(TWS_FLOOR_INTRO);     // restart chapter intro
        }

    } break;

    case TWS_NEXT: {
      tower_maybe_unlock_floor_cheevo(g_floor);
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

      tower_draw_lore_typewriter(g_tower_end_lines,
                                g_tower_end_count,
                                dt,
                                50u);

      if (dt >= 10000u){
        return true;
      }
    } break;
  }
  return false;       // not finished yet
}
