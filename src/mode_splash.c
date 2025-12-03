#include <stdio.h>
#include <stdbool.h>
#include "mode_splash.h"
#include "gfx.h"
#include "project.h"
#include "timer.h"
#include "game.h"   // for game_mode_t

/* how long to show the splash */
#define SPLASH_MS 1200u

static uint8_t  g_active = 0;
static uint8_t  g_sel    = 0;
static uint32_t g_t0     = 0;
static bool     g_dirty  = false;   // true = need to draw the splash

static const char* mode_name(uint8_t m){
  switch ((game_mode_t)m){
    case MODE_PLAYGROUND: return "Playground";
    case MODE_PVP:        return "PVP";
    case MODE_STORY:      return "Story";
    case MODE_TOWER:      return "Tower";
    case MODE_CREDITS:    return "Credits + Trophy";
    default:              return "Unknown";
  }
}

void mode_splash_begin(uint8_t selected_mode){
  g_sel    = selected_mode;
  g_t0     = millis();
  g_active = 1;
  g_dirty  = true;      // new selection -> need to draw once
}

bool mode_splash_active(void){
  return g_active != 0;
}

bool mode_splash_tick(void){
  if (!g_active) return false;

  uint32_t dt = millis() - g_t0;

  if (g_dirty) {
    g_dirty = false;

    // Full splash drawn once when we enter
    gfx_clear(COL_BLACK);
    gfx_header("MODE SELECTED", COL_WHITE);
    gfx_bar(0, 18, 128, 1, COL_DKGRAY);     // under header

    const int base_y = 30, row_h = 18;
    for (int i = 0; i < 5; ++i){
      const char* name = mode_name((uint8_t)i);
      uint16_t fg = (i == g_sel) ? COL_BLACK : COL_WHITE;

      if (i == g_sel){
        // highlight bar behind the selected row
        gfx_bar(6, base_y + i*row_h - 2, 116, row_h, COL_GREEN);
      }
      gfx_text2(10, base_y + i*row_h, name, fg, 1);
    }
  }

  if (dt >= SPLASH_MS){
    g_active = 0;
    return true;
  }
  return false;
}
