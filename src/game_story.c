#include <stdio.h>
#include "timer.h"
#include "gfx.h"
#include "project.h"
#include "game.h"          // game_get_metrics(...)
#include "choice_input.h"
#include "story_data.h"
#include "story_items.h"

typedef enum {
  STS_LOGO = 0,
  STS_BRAND,        // story “title card”
  STS_INTRO,        // show chapter title + enemy
  STS_CHOOSE,       // A/B choice via Hz
  STS_BATTLE,       // 10 s flex window; accumulate avg
  STS_RESULT,       // show pass/fail vs enemy
  STS_REWARD,       // echo chosen item
  STS_NEXT,         // advance chapter
  STS_ENDING        // final scene
} story_state_t;

static story_state_t g_s;
static uint32_t      g_t0;
static uint8_t       g_chapter;        // 0..9
static story_item_t  g_equipped;       // last choice (A/B)
static float         g_sum_hz;
static uint32_t      g_cnt_hz;

static bool          g_dirty;          // true = need to (re)draw this state's screen

static void s_goto(story_state_t ns){
  g_s    = ns;
  g_t0   = millis();
  g_dirty = true;
}

void game_story_init(void){
  g_chapter  = 0;
  g_equipped = STORY_ITEM_A;
  g_sum_hz   = 0.0f;
  g_cnt_hz   = 0u;
  s_goto(STS_LOGO);
}

bool game_story_tick(void){
  uint32_t dt = millis() - g_t0;

  float   hz   = 0.0f;
  float   base = 0.0f;
  uint8_t pct  = 0;
  game_get_metrics(&hz, &pct, &base);
  (void)base;
  (void)pct;

  switch(g_s){

    case STS_LOGO: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        // optional: story logo splash here
      }
      if (dt >= 3000u) {
        s_goto(STS_BRAND);
      }
    } break;

    case STS_BRAND: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("STORY MODE", COL_RED);
        gfx_text2(6, 40, "Welcome challenger!", COL_WHITE, 1);
      }
      if (dt >= 2000u){
        s_goto(STS_INTRO);
      }
    } break;

    case STS_INTRO: {
      const story_chapter_t* c = &g_story[g_chapter];
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header(c->name, COL_WHITE);

        char line[40];
        snprintf(line, sizeof(line), "%s  Target: %u Hz",
                 c->enemy, (unsigned)c->enemy_hz);
        gfx_text2(6, 40, line, COL_YELLOW, 1);
        gfx_text2(6, 64, "Choose an item (A/B) with Hz", COL_DKGRAY, 1);
        choice_draw_hint(80);
      }
      if (dt >= 2000u){
        s_goto(STS_CHOOSE);
      }
    } break;

    case STS_CHOOSE: {
      const story_chapter_t* c = &g_story[g_chapter];
      (void)c;   // header already handled above; kept for future use

      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("CHOOSE", COL_WHITE);
        gfx_text2(6, 32, "A) Bandage (+10% you)",   COL_CYAN,   1);
        gfx_text2(6, 46, "B) Cursed Ring (+20% enemy)", COL_YELLOW, 1);
        choice_draw_hint(80);
      }

      // live choice via Hz threshold
      choice_t ch = choice_from_hz(hz, 50.0f);
      g_equipped  = (ch == CHOICE_A) ? STORY_ITEM_A : STORY_ITEM_B;

      // simple 1s settle before locking choice and moving on
      if (dt >= 1000u) {
        g_sum_hz = 0.0f;
        g_cnt_hz = 0u;
        s_goto(STS_BATTLE);
      }
    } break;

    case STS_BATTLE: {
      const uint32_t FLEX_MS = 10000u;

      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("BATTLE", COL_RED);
      }

      if (dt >= FLEX_MS){
        s_goto(STS_RESULT);
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

    case STS_RESULT: {
      if (g_dirty){
        g_dirty = false;

        const story_chapter_t* c = &g_story[g_chapter];
        float avg = (g_cnt_hz ? (g_sum_hz / (float)g_cnt_hz) : 0.0f);
        float you = avg * g_equipped.player_mult;
        float foe = (float)c->enemy_hz * g_equipped.enemy_mult;

        gfx_clear(COL_BLACK);
        gfx_header("RESULT", COL_WHITE);

        char l1[40];
        char l2[40];
        snprintf(l1, sizeof(l1), "You: %.1f Hz", you);
        snprintf(l2, sizeof(l2), "Enemy: %.1f Hz", foe);
        gfx_text2(6, 36, l1, COL_CYAN,   1);
        gfx_text2(6, 50, l2, COL_YELLOW, 1);

        gfx_text2(6, 76,
                  (you >= foe) ? "VICTORY" : "DEFEAT",
                  (you >= foe) ? COL_GREEN : COL_RED,
                  2);
      }

      if (dt >= 2000u){
        s_goto(STS_REWARD);
      }
    } break;

    case STS_REWARD: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("LOOT", COL_WHITE);
        gfx_text2(6, 40, "Equipped:", COL_DKGRAY, 1);
        gfx_text2(6, 54, g_equipped.name, COL_WHITE, 1);
      }
      if (dt >= 1500u){
        s_goto(STS_NEXT);
      }
    } break;

    case STS_NEXT: {
      uint8_t next = (uint8_t)(g_chapter + 1u);
      if (next >= STORY_CHAPTERS){
        s_goto(STS_ENDING);
      } else {
        g_chapter = next;
        s_goto(STS_INTRO);
      }
    } break;

    case STS_ENDING: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("THE END", COL_WHITE);
        gfx_text2(6, 30, "You cleared Story Mode!",             COL_GREEN, 1);
        gfx_text2(6, 50, "You beat the Demon King and",         COL_WHITE, 1);
        gfx_text2(6, 70, "became the strongest in the lands",   COL_WHITE, 1);
        gfx_text2(6, 90, "and saved the world!",                COL_WHITE, 1);
        gfx_text2(6,110, "or did you...",                       COL_GREEN, 1);
      }
      if (dt >= 10000u){
        return true;    // tell game.c that Story mode is finished
      }
    } break;
  }

  // not finished yet
  return false;
}
