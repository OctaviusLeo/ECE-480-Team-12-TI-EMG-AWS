/*==============================================================================
 * @file    game.c
 * @brief   Top-level game state manager, dispatching between modes and menu.
 *
 * This file is part of the EMG flex-frequency game project and follows the
 * project coding standard for file-level documentation.
 *============================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "project.h"
#include "timer.h"
#include "gfx.h"
#include "ssd1351.h"
#include "game.h"

#include "game_single.h"
#include "game_two.h"
#include "game_story.h"
#include "game_tower.h"
#include "end_credits.h"
#include "mode_splash.h"
#include "menu.h"
#include "cheevos.h"

// Game mode identifiers
typedef enum {
  MODE_PLAYGROUND = 0,
  MODE_PVP,
  MODE_STORY,
  MODE_TOWER,
  MODE_CREDITS
} game_mode_t;

// Global game state
static bool     g_inited   = false;
static uint8_t  g_mode     = MODE_PLAYGROUND;
static uint8_t  g_in_menu  = 1;

// Forward declarations
static void app_boot(void);

// Initialize the entire game system.
void game_init(void){
  if (!g_inited){
    // Initialize the SSD1351 OLED display
    ssd1351_init();
    gfx_init();

    // Initialize achievements / save data, etc.
    app_boot();

    // Start in menu state
    g_in_menu = 1;
    menu_start();

    g_inited = true;
  }
}

// Set the current game mode (e.g., playground, PVP, story, tower, credits).
void game_set_mode(uint8_t mode){
  // Use the mode that was selected by the menu
  g_mode = mode;

  // Initialize the chosen mode
  switch (g_mode){
    case MODE_PLAYGROUND:
      game_single_init();
      break;
    case MODE_PVP:
      game_two_init();
      break;
    case MODE_STORY:
      game_story_init();
      break;
    case MODE_TOWER:
      game_tower_init();
      break;
    case MODE_CREDITS:
      end_credits_init();
      break;
    default:
      // Fallback to playground if an invalid mode is set
      g_mode = MODE_PLAYGROUND;
      game_single_init();
      break;
  }
}

// Get current EMG/Hz metrics from the active mode.
// The active mode fills out hz, pct (percentile), and base (baseline Hz).
void game_get_metrics(float *hz, uint8_t *pct, float *base){
  if (!hz || !pct || !base) return;

  switch (g_mode){
    case MODE_PLAYGROUND:
      game_single_metrics(hz, pct, base);
      break;
    case MODE_PVP:
      game_two_metrics(hz, pct, base);
      break;
    case MODE_STORY:
      game_story_metrics(hz, pct, base);
      break;
    case MODE_TOWER:
      game_tower_metrics(hz, pct, base);
      break;
    default:
      *hz   = 0.0f;
      *pct  = 0u;
      *base = 0.0f;
      break;
  }
}

// Main per-frame update for the entire game.
// Returns when the game loop has finished one tick.
void game_tick(void){
  if (!g_inited){
    // Ensure initialization is complete before ticking
    game_init();
    return;
  }

  // If we are currently in the menu, tick the menu.
  if (g_in_menu){
    uint8_t mode = 0xFFu;
    if (menu_tick(&mode)){
      // Menu has chosen a mode; switch out of the menu.
      g_in_menu = 0;
      game_set_mode(mode);
    }
    return;
  }

  // If we are here, we are in a gameplay/credits mode.
  // Tick the current mode and handle return-to-menu if complete.
  switch (g_mode){
    case MODE_PLAYGROUND:
      if (game_single_tick()){
        g_in_menu = 1;
        menu_start();
      }
      break;

    case MODE_PVP:
      if (game_two_tick()){
        g_in_menu = 1;
        menu_start();
      }
      break;

    case MODE_STORY:
      if (game_story_tick()){
        g_in_menu = 1;
        menu_start();
      }
      break;

    case MODE_TOWER:
      if (game_tower_tick()){
        g_in_menu = 1;
        menu_start();
      }
      break;

    case MODE_CREDITS:
    default:
      if (end_credits_tick()){
        g_in_menu = 1;
        menu_start();
      }
      break;
  }

  // Overlay any active "Achievement Unlocked!" toast
  cheevos_draw_toast();
}

// Application boot-time setup: load save data, bind achievements, etc.
static void app_boot(void){
  // Future: load persistent data, initialize cheevos, etc.
  cheevos_init();
}

