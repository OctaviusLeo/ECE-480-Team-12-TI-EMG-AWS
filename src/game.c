#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "project.h"
#include "timer.h"
#include "gfx.h"
#include "ssd1351.h"
#include "game.h"

// new headers for split modes
#include "game_single.h"
#include "game_two.h"
#include "game_story.h"
#include "game_tower.h"
#include "end_credits.h"
#include "mode_splash.h"

#include "save.h"
#include "cheevos.h"
#include "menu.h"

// private storage of latest metrics (read by modes via game_get_metrics)
static bool     g_inited = false;
static uint8_t  g_mode   = 1;        // 0 = playground, 1 = PVP, 2 = Story, 3 = Tower, 4 = ending credits
static float    g_latest_hz = 0.0f;
static uint8_t  g_latest_pct = 0;
static float    g_baseline_disp_hz = 0.0f;

static save_t G_SAVE;
static uint8_t g_in_menu = 1;   // 1 = menu active, 0 = running a mode

void app_boot(void){
  if (!save_load(&G_SAVE)) save_defaults(&G_SAVE);
  cheevos_bind_save(&G_SAVE);
}

void game_get_metrics(float *hz, uint8_t *pct, float *baseline_hz){
  if (hz)          *hz = g_latest_hz;
  if (pct)         *pct = g_latest_pct;
  if (baseline_hz) *baseline_hz = g_baseline_disp_hz;
}

void game_init(void){
  if (!g_inited){
    ssd1351_init();
    gfx_clear(COL_BLACK);
    g_inited = true;

    // Boot-time setup — run ONCE
    app_boot();        // load save + bind cheevos before entering menu
    g_in_menu = 1;
    menu_start();
  }
}


void game_set_mode(uint8_t mode){

  if (g_mode == MODE_PLAYGROUND)      game_single_init();
  else if (g_mode == MODE_PVP)        game_two_init();
  else if (g_mode == MODE_STORY)      game_story_init();
  else if (g_mode == MODE_TOWER)      game_tower_init();
  else /* MODE_CREDITS */             end_credits_start();

  mode_splash_begin(g_mode); // brief overlay; next frames will tick it, not the menu
}

void game_set_metrics(float hz, uint8_t intensity_pct){
  g_latest_hz  = hz;
  g_latest_pct = intensity_pct;
}

void game_set_baseline(float baseline_hz){
  g_baseline_disp_hz = baseline_hz;
}

void game_tick(void){
  // 1) If splash is running, draw it and return (prevents menu from reappearing)
  if (mode_splash_active()){
    (void)mode_splash_tick();
    return;
  }

  // 2) Menu loop until a mode is chosen
  if (g_in_menu){
    uint8_t chosen;
    if (menu_tick(&chosen)){
      g_in_menu = 0;           // leave menu
      game_set_mode(chosen);   // init selected mode (+starts splash)
    }
    return;                     // while in menu, never tick any mode
  }

  // 3) Dispatch to active mode (tick only)
  if (g_mode == MODE_PLAYGROUND)      game_single_tick();
  else if (g_mode == MODE_PVP)        game_two_tick();
  else if (g_mode == MODE_STORY)      game_story_tick();
  else if (g_mode == MODE_TOWER)      game_tower_tick();
  else { // MODE_CREDITS
    if (end_credits_tick()){
      // credits finished → return to menu explicitly
      g_in_menu = 1;
      menu_start();
    }
  }
}
