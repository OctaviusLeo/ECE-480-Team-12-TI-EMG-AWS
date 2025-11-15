#include <stdio.h>
#include "menu.h"
#include "gfx.h"
#include "project.h"
#include "timer.h"
#include "game.h"        // game_get_metrics(), game_mode_t
#include "choice_input.h"// choice_from_hz(), choice_draw_hint()

typedef enum { MS_TITLE=0, MS_SELECT, MS_ATTRACT } mstate_t;
static mstate_t  g_ms;
static uint32_t  g_t0;
static uint8_t   g_cursor;      // 0..4
static uint32_t  g_last_input_ms;

static const char* mode_name(uint8_t m){
  switch((game_mode_t)m){
    case MODE_PLAYGROUND: return "Playground";
    case MODE_PVP:        return "PVP";
    case MODE_STORY:      return "Story";
    case MODE_TOWER:      return "Tower";
    case MODE_CREDITS:    return "End Credits";
    default:              return "Unknown";
  }
}

void menu_start(void){ g_ms = MS_TITLE; g_t0 = millis(); g_cursor = 0; g_last_input_ms = millis(); }

bool menu_tick(uint8_t *out_mode){
  float hz=0.0f, base=0.0f; uint8_t pct=0;
  game_get_metrics(&hz, &pct, &base);
  uint32_t now = millis();

  switch(g_ms){
    case MS_TITLE: {
      gfx_clear(COL_BLACK);
      gfx_header("BOOTING", COL_RED);
      gfx_bar(0,18,128,1,COL_DKGRAY);
      gfx_text2(14, 42, "downloading packages...", COL_WHITE, 1);
      if (hz > 3.0f || (now - g_t0) >= 1200u){ g_ms = MS_SELECT; g_t0 = now; }
    } break;

    case MS_SELECT: {
      gfx_clear(COL_BLACK);
      gfx_header("FLEX to SELECT", COL_WHITE);
      gfx_bar(0,18,128,1,COL_DKGRAY);

      const int base_y=32, row_h=18;
      for (int i=0;i<5;++i){
        uint16_t bg = (i==g_cursor)?COL_GREEN:COL_BLACK;
        uint16_t fg = (i==g_cursor)?COL_BLACK:COL_WHITE;
        if (i==g_cursor) gfx_bar(6, base_y+i*row_h-2, 116, row_h, bg);
        gfx_text2(10, base_y+i*row_h, mode_name((uint8_t)i), fg, 1);
      }
      choice_draw_hint(32 + 5*row_h + 6);

      /* Controls via Hz (no buttons):
         - Quick flex bursts to move cursor (debounced 250 ms)
         - Hold > 1.0 s above 50 Hz to confirm
      */
      if ((now - g_last_input_ms) >= 250u){
        if (choice_from_hz(hz, 50.0f) == CHOICE_A && hz > 3.0f){
          g_cursor = (uint8_t)((g_cursor + 1u) % 5u);
          g_last_input_ms = now;
        }
      }
      static uint32_t hold_t0=0; static uint8_t holding=0;
      if (hz >= 0.0f){
        if (!holding){ holding=1; hold_t0 = now; }
        if ((now - hold_t0) >= 1000u){
          if (out_mode) *out_mode = g_cursor;   // MODE_*
          return true;
        }
      } else { holding=0; }
    } break;
  }
  return false;
}
