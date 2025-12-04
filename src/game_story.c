#include <stdio.h>
#include <string.h>
#include "timer.h"
#include "gfx.h"
#include "project.h"
#include "game.h"          // game_get_metrics(...)
#include "choice_input.h"
#include "story_data.h"
#include "story_items.h"
#include "game_story_logo.h"
#include "story_opening_scene.h"
#include "story_final_scene.h"
#include "story_ch1.h"
#include "story_ch2.h"
#include "story_ch3.h"
#include "story_ch4.h"
#include "story_ch5.h"
#include "story_ch6.h"
#include "story_ch7.h"
#include "story_ch8.h"
#include "story_ch9.h"
#include "story_ch10.h"
#include "story_ch1_enemy.h"
#include "story_ch2_enemy.h"
#include "story_ch3_enemy.h"
#include "story_ch4_enemy.h"
#include "story_ch5_enemy.h"
#include "story_ch6_enemy.h"
#include "story_ch7_enemy.h"
#include "story_ch8_enemy.h"
#include "story_ch9_enemy.h"
#include "story_ch10_enemy.h"
#include "you_died.h"
#include "chest.h"
#include "cheevos.h"

#define STORY_FLEX_MENU_HZ 50.0f   // Hz needed to exit to menu after too many deaths
#define STORY_CHOICE_SPLIT_HZ 50.0f //A/B split threshold in Hz

typedef enum {
  STS_LOGO = 0,
  STS_BRAND,        // story “title card”
  STS_INTRO,        // show chapter title + enemy
  STS_CHOOSE,       // A/B choice via Hz
  STS_BATTLE,       // 10 s flex window; accumulate avg
  STS_RESULT,       // show pass/fail vs enemy
  STS_REWARD,       // echo chosen item
  STS_DEATH,          // you died screen (on defeat)
  STS_NEXT,         // advance chapter
  STS_ENDING,        // final scene
  STS_LORE_BRAND,     // global story intro lore text
  STS_LORE_CHAPTER,   // per-chapter lore text
  STS_LORE_ENDING,     // lore text before the final ending scene
  STS_FLEX_RETURN   // flex after too many deaths to go back to menu
} story_state_t;

static story_state_t g_s;
static uint32_t      g_t0;
static uint8_t       g_chapter;        // 0..9
static story_item_t  g_equipped;       // last choice (A/B)
static float         g_sum_hz;
static uint32_t      g_cnt_hz;

// Current chapter’s A/B item options
static const story_item_t *g_itemA;
static const story_item_t *g_itemB;

static bool          g_dirty;          // true = need to (re)draw this state's screen

// 3 deaths needed to exit
static uint8_t       g_story_deaths = 0u;

// Per-chapter enemy sprite descriptor for Story mode
typedef struct {
  uint8_t w;
  uint8_t h;
  const uint8_t  *idx;
  const uint16_t *pal;
} enemy_sprite_t;

// Per-chapter intro sprite descriptor (chapter opening art)
typedef struct {
  uint8_t w;
  uint8_t h;
  const uint8_t  *idx;
  const uint16_t *pal;
} intro_sprite_t;

// One entry per chapter (0..9)
static const intro_sprite_t g_intro_sprites[STORY_CHAPTERS] = {
  // chapter 0 -> story_ch1.png
  { STORY_CH1_W,  STORY_CH1_H,  STORY_CH1_IDX,  STORY_CH1_PAL  },
  // chapter 1 -> story_ch2.png
  { STORY_CH2_W,  STORY_CH2_H,  STORY_CH2_IDX,  STORY_CH2_PAL  },
  // chapter 2 -> story_ch3.png
  { STORY_CH3_W,  STORY_CH3_H,  STORY_CH3_IDX,  STORY_CH3_PAL  },
  // chapter 3 -> story_ch4.png
  { STORY_CH4_W,  STORY_CH4_H,  STORY_CH4_IDX,  STORY_CH4_PAL  },
  // chapter 4 -> story_ch5.png
  { STORY_CH5_W,  STORY_CH5_H,  STORY_CH5_IDX,  STORY_CH5_PAL  },
  // chapter 5 -> story_ch6.png
  { STORY_CH6_W,  STORY_CH6_H,  STORY_CH6_IDX,  STORY_CH6_PAL  },
  // chapter 6 -> story_ch7.png
  { STORY_CH7_W,  STORY_CH7_H,  STORY_CH7_IDX,  STORY_CH7_PAL  },
  // chapter 7 -> story_ch8.png
  { STORY_CH8_W,  STORY_CH8_H,  STORY_CH8_IDX,  STORY_CH8_PAL  },
  // chapter 8 -> story_ch9.png
  { STORY_CH9_W,  STORY_CH9_H,  STORY_CH9_IDX,  STORY_CH9_PAL  },
  // chapter 9 -> story_ch10.png
  { STORY_CH10_W, STORY_CH10_H, STORY_CH10_IDX, STORY_CH10_PAL },
};

// One entry per chapter (0..9)
static const enemy_sprite_t g_enemy_sprites[STORY_CHAPTERS] = {
  // chapter 0 -> story_ch1.png
  { STORY_CH1_ENEMY_W,  STORY_CH1_ENEMY_H,  STORY_CH1_ENEMY_IDX,  STORY_CH1_ENEMY_PAL  },
  // chapter 1 -> story_ch2.png
  { STORY_CH2_ENEMY_W,  STORY_CH2_ENEMY_H,  STORY_CH2_ENEMY_IDX,  STORY_CH2_ENEMY_PAL  },
  // chapter 2 -> story_ch3.png
  { STORY_CH3_ENEMY_W,  STORY_CH3_ENEMY_H,  STORY_CH3_ENEMY_IDX,  STORY_CH3_ENEMY_PAL  },
  // chapter 3 -> story_ch4.png
  { STORY_CH4_ENEMY_W,  STORY_CH4_ENEMY_H,  STORY_CH4_ENEMY_IDX,  STORY_CH4_ENEMY_PAL  },
  // chapter 4 -> story_ch5.png
  { STORY_CH5_ENEMY_W,  STORY_CH5_ENEMY_H,  STORY_CH5_ENEMY_IDX,  STORY_CH5_ENEMY_PAL  },
  // chapter 5 -> story_ch6.png
  { STORY_CH6_ENEMY_W,  STORY_CH6_ENEMY_H,  STORY_CH6_ENEMY_IDX,  STORY_CH6_ENEMY_PAL  },
  // chapter 6 -> story_ch8.png
  { STORY_CH7_ENEMY_W,  STORY_CH7_ENEMY_H,  STORY_CH7_ENEMY_IDX,  STORY_CH7_ENEMY_PAL  },
  // chapter 7 -> story_ch9.png
  { STORY_CH8_ENEMY_W,  STORY_CH8_ENEMY_H,  STORY_CH8_ENEMY_IDX,  STORY_CH8_ENEMY_PAL  },
  // chapter 8 -> story_ch10.png
  { STORY_CH9_ENEMY_W,  STORY_CH9_ENEMY_H,  STORY_CH9_ENEMY_IDX,  STORY_CH9_ENEMY_PAL  },
  // chapter 9 -> story_ch10.png
  { STORY_CH10_ENEMY_W,  STORY_CH1_ENEMY_H,  STORY_CH10_ENEMY_IDX,  STORY_CH10_ENEMY_PAL  },
};

// Lore text data
// Global intro lore lines
static const char* g_lore_brand_lines[] = {
  "You wake up from",
  "a dream. Eyes",
  "blinkng from the",
  "dry air. Casting",
  "your eyes outside,",
  "you enjoy the sun.",
  "Finally getting up,",
  "you start your day."
};
static const uint8_t g_lore_brand_count =
    sizeof(g_lore_brand_lines)/sizeof(g_lore_brand_lines[0]);

// Per-chapter lore: multi-line text per chapter

static const char* g_lore_ch1_lines[] = {
  "Long ago,",
  "EMG warriors trained",
  "to fight evil. Now,",
  "the realms grow into",
  "chaos. Peace is now",
  "gone.",
  "Hence, a new",
  "challenger rises..."
};
static const uint8_t g_lore_ch1_count =
    sizeof(g_lore_ch1_lines)/sizeof(g_lore_ch1_lines[0]);

static const char* g_lore_ch2_lines[] = {
  "After your daily",
  "morning session, you",
  "walk towards the tra",
  "-ining grounds. Here",
  "you start to get",
  "serious! An iron dum",
  "-my catches your eye",
  ". You stare at it and",
  "then strike!!!"
};
static const uint8_t g_lore_ch2_count =
    sizeof(g_lore_ch2_lines)/sizeof(g_lore_ch2_lines[0]);

static const char* g_lore_ch3_lines[] = {
  "A villager interrupts",
  "your training. He say",
  "the villager's kids",
  "explored the",
  "catacombs and haven't",
  "returned! Hence, you",
  "bravely travel to",
  "the catacombs to",
  "save the kids."
};
static const uint8_t g_lore_ch3_count =
    sizeof(g_lore_ch3_lines)/sizeof(g_lore_ch3_lines[0]);

static const char* g_lore_ch4_lines[] = {
  "The village thanks yo",
  "-our efforts. Giving",
  "a good sum of silver.",
  "With this money gained",
  "you head off to the",
  "kingdom to become",
  "a pulse-knight! However",
  "along the way you are",
  "stopped by a group of",
  "... bandits?!"
};
static const uint8_t g_lore_ch4_count =
    sizeof(g_lore_ch4_lines)/sizeof(g_lore_ch4_lines[0]);

static const char* g_lore_ch5_lines[] = {
  "After defeating the",
  "bandits, you continue",
  "onward. After several",
  "days you come across a",
  "knight camp near the",
  "kingdom. You are eager",
  "to test your skills.",
  "A training knight is",
  "angered by your ego.",
  "he challenges you!"
};
static const uint8_t g_lore_ch5_count =
    sizeof(g_lore_ch5_lines)/sizeof(g_lore_ch5_lines[0]);

static const char* g_lore_ch6_lines[] = {
  "Several days have past",
  "since your arrival at",
  "the kingdom. Hearing",
  "news about a tourna",
  "-ment, you enter,",
  "hoping to make a name",
  "for yourself. After",
  "many duels, you enter",
  "the final. The knight",
  "prepares his weapon."
};
static const uint8_t g_lore_ch6_count =
    sizeof(g_lore_ch6_lines)/sizeof(g_lore_ch6_lines[0]);

static const char* g_lore_ch7_lines[] = {
  "Winning the tourney",
  "gave you the pass",
  "to Arcane Tower. Hap",
  "-ppily accepting the",
  "reward, you head to",
  "the floating tower in",
  "the sky. Entering in",
  "-side, Arcane Tower's",
  "founder wants your",
  "help. But you must",
  "defeat him first."
};
static const uint8_t g_lore_ch7_count =
    sizeof(g_lore_ch7_lines)/sizeof(g_lore_ch7_lines[0]);

static const char* g_lore_ch8_lines[] = {
  "The sorcerer needs",
  "help. To locate the D",
  "emon Castle, he needs",
  "a dragon's heart. So,",
  "after traveling,",
  "the dragon's lair is",
  "infront of you. Step",
  "by step, you traverse",
  "inside. Inside you",
  "the dragon staring.",
  "It roars and charges!"
};
static const uint8_t g_lore_ch8_count =
    sizeof(g_lore_ch8_lines)/sizeof(g_lore_ch8_lines[0]);

static const char* g_lore_ch9_lines[] = {
  "You return to Arcane To",
  "-wer. Givin the Sorc",
  "-erer the dragon's he",
  "-art, he gives you",
  "the location of the",
  "Demon Castle. Many",
  "years past in your",
  "jourey. The hellish",
  "enviroment homes the",
  "Demon King. Entering,",
  "he sits on his throne."
};
static const uint8_t g_lore_ch9_count =
    sizeof(g_lore_ch9_lines)/sizeof(g_lore_ch9_lines[0]);

static const char* g_lore_ch10_lines[] = {
"[Wonderful. WONDERFUL!]",
"A unkown voice enters",
"your head. [You did it",
"but, this wasn't supp",
"-osed to happen... well",
"i'll just remove you].",
"Opening your eyes, a",
"figure smiles at you.",
"They raise their hand",
"but you rush in to",
"stop him!"
};
static const uint8_t g_lore_ch10_count =
    sizeof(g_lore_ch10_lines)/sizeof(g_lore_ch10_lines[0]);

// Table of chapter-lore blocks
typedef struct {
  const char* const* lines;
  uint8_t count;
} lore_block_t;

static const lore_block_t g_lore_chapters[STORY_CHAPTERS] = {
  { g_lore_ch1_lines,  g_lore_ch1_count  }, // chapter 0
  { g_lore_ch2_lines,  g_lore_ch2_count  }, // chapter 1
  { g_lore_ch3_lines,  g_lore_ch3_count  }, // chapter 2
  { g_lore_ch4_lines,  g_lore_ch4_count  }, // chapter 3
  { g_lore_ch5_lines,  g_lore_ch5_count  }, // chapter 4
  { g_lore_ch6_lines,  g_lore_ch6_count  }, // chapter 5
  { g_lore_ch7_lines,  g_lore_ch7_count  }, // chapter 6
  { g_lore_ch8_lines,  g_lore_ch8_count  }, // chapter 7
  { g_lore_ch9_lines,  g_lore_ch9_count  }, // chapter 8
  { g_lore_ch10_lines, g_lore_ch10_count }  // chapter 9
};

// Ending lore lines
static const char* g_lore_ending_lines[] = {
"You slowly open your",
"eyes. Mind is cloudy.",
"The dead Demon King",
"lays at your feet. Yea",
"... you saved the world",
"you raise your hand in",
"-to the sky. YOU SAVED",
"the world! You travel",
"back and spend your ye",
"-ars as a hero! Chaos",
"is no more, or is it?"
};
static const uint8_t g_lore_ending_count =
    sizeof(g_lore_ending_lines)/sizeof(g_lore_ending_lines[0]);

// Draw a simple lore screen: header + multiple lines of text.
static void draw_lore_screen(const char* title,
                             const char* const* lines,
                             uint8_t count)
{
  gfx_clear(COL_BLACK);
  gfx_header(title, COL_WHITE);

  uint8_t y = 24;  // start below header
  for (uint8_t i = 0; i < count; ++i) {
    if (lines[i] && lines[i][0] != '\0') {
      gfx_text2(4, y, lines[i], COL_WHITE, 1);
    }
    y = (uint8_t)(y + 10);
    if (y > 120u) break;
  }
}

// Typewriter-style lore: slowly reveal text over time.
// NOTE: does NOT clear the screen or draw header; caller does that once via g_dirty.
static void draw_lore_typewriter(const char* const* lines,
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

// Draw the enemy battle Hz bar at the bottom of the screen.
// hz        = current player Hz
// target_hz = enemy's target Hz for this chapter
static void draw_enemy_hz_bar(float hz, float target_hz)
{
  const uint8_t bar_x = 4;
  const uint8_t bar_y = 112;     // bottom band
  const uint8_t bar_w = 120;
  const uint8_t bar_h = 12;

  if (target_hz <= 0.0f) target_hz = 1.0f;

  // Define a max scale so the bar doesn't overflow; tweak factor as you like.
  float max_hz = target_hz * 1.5f;
  if (max_hz < target_hz) max_hz = target_hz;   // safety

  if (hz < 0.0f) hz = 0.0f;
  if (hz > max_hz) hz = max_hz;

  // Clear the band once per frame
  gfx_bar(bar_x, bar_y, bar_w, bar_h, COL_DKGRAY);

  // Current Hz bar (green-ish)
  uint8_t cur_w = (uint8_t)((hz / max_hz) * (float)bar_w + 0.5f);
  if (cur_w > bar_w) cur_w = bar_w;
  if (cur_w > 0u){
    gfx_bar(bar_x, bar_y, cur_w, bar_h, COL_GREEN);
  }

  // Threshold line (enemy required Hz) as a 1-px wide red bar
  uint8_t th_x = bar_x + (uint8_t)((target_hz / max_hz) * (float)bar_w + 0.5f);
  if (th_x < bar_x) th_x = bar_x;
  if (th_x >= (uint8_t)(bar_x + bar_w)) th_x = (uint8_t)(bar_x + bar_w - 1u);
  gfx_bar(th_x, bar_y, 1, bar_h, COL_RED);
}

// Chest choice Hz bar (Story):
// A on the left, B on the right, bar sits over the chest image.
static void story_choice_bar_static(void)
{
  const uint8_t bar_x = 4;
  const uint8_t bar_y = 60;    // roughly across the chest
  const uint8_t bar_w = 120;
  const uint8_t bar_h = 8;

  // baseline band
  gfx_bar(bar_x, bar_y, bar_w, bar_h, COL_DKGRAY);

  // A / B labels just above the bar
  gfx_text2(bar_x,           bar_y - 10, "A", COL_CYAN,   1);
  gfx_text2(bar_x + bar_w - 6, bar_y - 10, "B", COL_YELLOW, 1);

  // draw static threshold marker once (we'll redraw it in the dynamic pass anyway)
  float max_hz = 80.0f;
  float th_hz  = STORY_CHOICE_SPLIT_HZ;
  if (th_hz < 0.0f)     th_hz = 0.0f;
  if (th_hz > max_hz)   th_hz = max_hz;
  uint8_t th_x = bar_x + (uint8_t)((th_hz / max_hz) * (float)bar_w + 0.5f);
  if (th_x >= (uint8_t)(bar_x + bar_w)) th_x = (uint8_t)(bar_x + bar_w - 1u);
  gfx_bar(th_x, bar_y, 1, bar_h, COL_RED);
}

// Dynamic fill for choice bar (Story)
// Call every tick with current hz.
static void story_choice_bar_update(float hz)
{
  const uint8_t bar_x = 4;
  const uint8_t bar_y = 60;
  const uint8_t bar_w = 120;
  const uint8_t bar_h = 8;

  float max_hz = 80.0f;
  if (hz < 0.0f)     hz = 0.0f;
  if (hz > max_hz)   hz = max_hz;

  // Clear band back to baseline
  gfx_bar(bar_x, bar_y, bar_w, bar_h, COL_DKGRAY);

  // Fill according to current Hz
  uint8_t cur_w = (uint8_t)((hz / max_hz) * (float)bar_w + 0.5f);
  if (cur_w > bar_w) cur_w = bar_w;
  if (cur_w > 0u){
    gfx_bar(bar_x, bar_y, cur_w, bar_h, COL_GREEN);
  }

  // Re-draw threshold marker on top
  float th_hz = STORY_CHOICE_SPLIT_HZ;
  if (th_hz < 0.0f)     th_hz = 0.0f;
  if (th_hz > max_hz)   th_hz = max_hz;
  uint8_t th_x = bar_x + (uint8_t)((th_hz / max_hz) * (float)bar_w + 0.5f);
  if (th_x >= (uint8_t)(bar_x + bar_w)) th_x = (uint8_t)(bar_x + bar_w - 1u);
  gfx_bar(th_x, bar_y, 1, bar_h, COL_RED);
}

static void story_unlock_for_chapter(uint8_t ch){
  switch(ch){
    case 0:  // Chapter 1 – Scarecrow
      cheevos_unlock(ACH_CH1);
      cheevos_unlock(ACH_SCARECROW);
      break;
    case 1:  // Chapter 2 – Training Dummy
      cheevos_unlock(ACH_CH2);
      cheevos_unlock(ACH_TRAINING_DUMMY);
      break;
    case 2:  // Chapter 3 – Rat King
      cheevos_unlock(ACH_CH3);
      cheevos_unlock(ACH_RAT_KING);
      break;
    case 3:  // Chapter 4 – Bandits
      cheevos_unlock(ACH_CH4);
      cheevos_unlock(ACH_BANDITS);
      break;
    case 4:  // Chapter 5 – Knight
      cheevos_unlock(ACH_CH5);
      cheevos_unlock(ACH_KNIGHT);
      break;
    case 5:  // Chapter 6 – Champion
      cheevos_unlock(ACH_CH6);
      cheevos_unlock(ACH_CHAMPION);
      break;
    case 6:  // Chapter 7 – Sorcerer
      cheevos_unlock(ACH_CH7);
      cheevos_unlock(ACH_SORCERER);
      break;
    case 7:  // Chapter 8 – Dragon
      cheevos_unlock(ACH_CH8);
      cheevos_unlock(ACH_DRAGON);
      break;
    case 8:  // Chapter 9 – Demon King
      cheevos_unlock(ACH_CH9);
      cheevos_unlock(ACH_DEMON_KING);
      break;
    case 9:  // Chapter 10 – Game Admin
      cheevos_unlock(ACH_CH10);
      cheevos_unlock(ACH_GAME_ADMIN);
      break;
    default:
      break;
  }
}

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
  g_itemA    = &STORY_ITEM_A;
  g_itemB    = &STORY_ITEM_B;
  g_story_deaths = 0u;
  cheevos_unlock(ACH_STORY_START);
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
        
        uint8_t x = (uint8_t)((128 - GAME_STORY_LOGO_W) / 2);
        uint8_t y = (uint8_t)((128 - GAME_STORY_LOGO_H) / 2);

        gfx_blit_pal4(x, y,
                      GAME_STORY_LOGO_W, GAME_STORY_LOGO_H,
                      GAME_STORY_LOGO_IDX,
                      GAME_STORY_LOGO_PAL);


      }
      if (dt >= 5000u) {
        s_goto(STS_LORE_BRAND);
      }
    } break;

    case STS_LORE_BRAND: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("Prologue", COL_WHITE);
        // any static art/lines go here ONCE
      }

      // Only text updates each tick; no clears
      draw_lore_typewriter(g_lore_brand_lines,
                          g_lore_brand_count,
                          dt,
                          50u);   // ms per char

      if (dt >= 10000u){
        s_goto(STS_BRAND);
      }
    } break;

    case STS_BRAND: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);

        // Draw global story opening scene image, centered
        uint8_t x = (uint8_t)((128 - STORY_OPENING_SCENE_W) / 2);
        uint8_t y = (uint8_t)((128 - STORY_OPENING_SCENE_H) / 2);

        gfx_blit_pal4(x, y,
                      STORY_OPENING_SCENE_W, STORY_OPENING_SCENE_H,
                      STORY_OPENING_SCENE_IDX,
                      STORY_OPENING_SCENE_PAL);
      }
      if (dt >= 5000u){
        s_goto(STS_LORE_CHAPTER);
      }
    } break;

    case STS_LORE_CHAPTER: {
      const story_chapter_t* c = &g_story[g_chapter];
      const lore_block_t*    lb = &g_lore_chapters[g_chapter];

      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header(c->name, COL_WHITE);
        // any static decorations per chapter go here once
      }

      draw_lore_typewriter(lb->lines,
                          lb->count,
                          dt,
                          50u);

      if (dt >= 10000u){
        s_goto(STS_INTRO);
      }
    } break;

    case STS_INTRO: {
      const story_chapter_t* c = &g_story[g_chapter];
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);

        // Chapter name in header
        gfx_header(c->name, COL_WHITE);

        // Draw chapter-specific intro art for this chapter
        const intro_sprite_t *is = &g_intro_sprites[g_chapter];

        uint8_t ix = (uint8_t)((128 - is->w) / 2);
        uint8_t iy = 20;   // leave header band at top

        gfx_blit_pal4(ix, iy,
                      is->w, is->h,
                      is->idx,
                      is->pal);

        // Overlay target info near the bottom over a small band
        char line[40];
        snprintf(line, sizeof(line), "%s STR: %u Hz",
                 c->enemy, (unsigned)c->enemy_hz);
        gfx_bar(0, 104, 128, 24, COL_BLACK);
        gfx_text2(0, 106, line, COL_YELLOW, 1);
        gfx_text2(0, 118, "Choose an item (A/B) with Hz", COL_DKGRAY, 1);
      }
      if (dt >= 5000u){
        s_goto(STS_CHOOSE);
      }
    } break;

    case STS_CHOOSE: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);

        // Chest in center
        uint8_t cx = (uint8_t)((128 - CHEST_W) / 2);
        uint8_t cy = (uint8_t)((128 - CHEST_H) / 2);

        gfx_blit_pal4(cx, cy,
                      CHEST_W, CHEST_H,
                      CHEST_IDX,
                      CHEST_PAL);

        // Randomly pick two distinct items from STORY_ITEMS[]
        uint8_t i0, i1;
        story_items_pick_two(&i0, &i1);
        g_itemA = &STORY_ITEMS[i0];
        g_itemB = &STORY_ITEMS[i1];

        gfx_header("CHOOSE", COL_WHITE);

        char lineA[40];
        char lineB[40];
        snprintf(lineA, sizeof(lineA), "A) %s", g_itemA->name);
        snprintf(lineB, sizeof(lineB), "B) %s", g_itemB->name);

        gfx_text2(0, 110, lineA, COL_CYAN,   1);
        gfx_text2(0, 120, lineB, COL_YELLOW, 1);
        choice_draw_hint(80);

        // draw static A/B bar over the chest
        story_choice_bar_static();
      }

      // live bar update using current Hz
      story_choice_bar_update(hz);

      // Live Hz-based choice: last one wins before timeout
      choice_t ch = choice_from_hz(hz, STORY_CHOICE_SPLIT_HZ);
      const story_item_t *cur = (ch == CHOICE_A) ? g_itemA : g_itemB;
      if (cur) {
        g_equipped = *cur;   // copy struct RESULT/REWARD use g_equipped
      }

      if (dt >= 5000u) {
        g_sum_hz = 0.0f;
        g_cnt_hz = 0u;
        s_goto(STS_BATTLE);
      }
    } break;

    case STS_BATTLE: {
      const uint32_t FLEX_MS = 10000u;
      const story_chapter_t* c = &g_story[g_chapter];

      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("BATTLE", COL_RED);

        // Draw this chapter's enemy image once
        const enemy_sprite_t *es = &g_enemy_sprites[g_chapter];

        uint8_t ex = (uint8_t)((128 - es->w) / 2);
        uint8_t ey = 16;  // top area; leaves bottom for Hz bar + text

        gfx_blit_pal4(ex, ey,
                      es->w, es->h,
                      es->idx,
                      es->pal);
      }

      if (dt >= FLEX_MS){
        s_goto(STS_RESULT);
        break;
      }

      // Countdown + bar logic
      uint8_t remain_s = (uint8_t)((FLEX_MS - dt) / 1000u);
      char line[32];
      snprintf(line, sizeof(line), "Flex... %us left", (unsigned)remain_s);
      gfx_bar(0, 96, 128, 12, COL_BLACK);
      gfx_text2(8, 96, line, COL_WHITE, 1);

      draw_enemy_hz_bar(hz, (float)c->enemy_hz);

      if (hz >= 1.5f){
        g_sum_hz += hz;
        g_cnt_hz++;
      }
    } break;


    case STS_RESULT: {
      const story_chapter_t* c = &g_story[g_chapter];
      float avg = (g_cnt_hz ? (g_sum_hz / (float)g_cnt_hz) : 0.0f);
      float you  = avg * g_equipped.player_mult;
      float foe  = (float)c->enemy_hz * g_equipped.enemy_mult;

      if (g_dirty) {
        g_dirty = false; 
        gfx_clear(COL_BLACK);
        gfx_header("RESULT", COL_WHITE);
        char l1[40]; snprintf(l1, sizeof(l1), "You: %.1f Hz", you);
        char l2[40]; snprintf(l2, sizeof(l2), "Enemy: %.1f Hz", foe);
        gfx_text2(6, 36, l1, COL_CYAN, 1);
        gfx_text2(6, 50, l2, COL_YELLOW, 1);
        gfx_text2(6, 76,
                  (you >= foe) ? "VICTORY" : "DEFEAT",
                  (you >= foe) ? COL_GREEN  : COL_RED,
                  2);
      }
      if (dt >= 2000u) {
        if (you >= foe) {
          // Win → continue normal flow
          story_unlock_for_chapter(g_chapter);
          s_goto(STS_REWARD);
        } else {
          // Lose → show you died + retry this chapter
          s_goto(STS_DEATH);
        }
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
      if (dt >= 5000u){
        s_goto(STS_NEXT);
      }
    } break;

    case STS_DEATH: {
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

        if (g_story_deaths + 1u < 3u) {
          gfx_text2(30, 110, "Retrying...", COL_RED, 1);
        } else {
          gfx_text2(10, 110, "Too many deaths...", COL_RED, 1);
        }
      }

      if (dt >= 3000u) {
        // Count this death
        g_story_deaths++;

        // Reset counters regardless
        g_sum_hz = 0.0f;
        g_cnt_hz = 0u;

        if (g_story_deaths >= 3u) {
          // go to flex-to-menu screen instead of retry
          s_goto(STS_FLEX_RETURN);
        } else {
          // retry same chapter from intro
          s_goto(STS_INTRO);
        }
      }
    } break;

    case STS_FLEX_RETURN: {
        // hz already computed at top of game_story_tick

        if (g_dirty) {
            g_dirty = false;
            gfx_clear(COL_BLACK);
            gfx_header("REST & RESET", COL_WHITE);
            gfx_bar(0, 18, 128, 1, COL_DKGRAY);

            gfx_text2(4, 36, "You have fallen...", COL_RED,   1);
            gfx_text2(4, 48, "Flex to go back",    COL_WHITE, 1);
            gfx_text2(4, 60, "home. Failure...",            COL_WHITE, 1);

            char line[40];
            snprintf(line, sizeof(line),
                    "Need: %.1f Hz", STORY_FLEX_MENU_HZ);
            gfx_text2(4, 82, line, COL_CYAN, 1);
        }

        // strong flex -> back to main menu
        if (hz >= STORY_FLEX_MENU_HZ) {
            g_story_deaths = 0u;   // optional reset
            return true;           // tell game.c Story is done -> menu
        }

        // wait 5 seconds -> retry same chapter
        if (dt >= 5000u) {
            g_sum_hz = 0.0f;
            g_cnt_hz = 0u;
            s_goto(STS_INTRO);     // restart chapter intro
        }
    } break;

    case STS_NEXT: {
      uint8_t next = (uint8_t)(g_chapter + 1u);
      if (next >= STORY_CHAPTERS){
        s_goto(STS_LORE_ENDING);
      } else {
        g_chapter = next;
        s_goto(STS_LORE_CHAPTER);
      }
    } break;

    case STS_LORE_ENDING: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);
        gfx_header("Epilogue", COL_WHITE);
      }

      draw_lore_typewriter(g_lore_ending_lines,
                          g_lore_ending_count,
                          dt,
                          50u);

      if (dt >= 5000u){
        s_goto(STS_ENDING);
      }
    } break;

    case STS_ENDING: {
      if (g_dirty){
        g_dirty = false;
        gfx_clear(COL_BLACK);

        // Draw final story scene image, centered
        uint8_t x = (uint8_t)((128 - STORY_FINAL_SCENE_W) / 2);
        uint8_t y = (uint8_t)((128 - STORY_FINAL_SCENE_H) / 2);

        gfx_blit_pal4(x, y,
                      STORY_FINAL_SCENE_W, STORY_FINAL_SCENE_H,
                      STORY_FINAL_SCENE_IDX,
                      STORY_FINAL_SCENE_PAL);
        cheevos_unlock(ACH_STORY_CLEAR);
      }
      if (dt >= 10000u){
        return true;    // tell game.c that Story mode is finished
      }
    } break;
  }

  // not finished yet
  return false;
}
