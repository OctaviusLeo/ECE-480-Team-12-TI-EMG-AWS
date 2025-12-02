#include <string.h>
#include <stdbool.h>
#include "intermission.h"
#include "timer.h"   // millis()
#include "gfx.h"     // gfx_clear, gfx_header, gfx_text2, gfx_bar
#include "project.h"
#include "game_two_logo.h"
static uint8_t  g_active   = 0;
static uint32_t g_end_ms   = 0;
static char     g_title[24];

static bool     g_dirty    = false;
static unsigned g_last_rem_s = (unsigned)-1;  // sentinel
static uint8_t  g_digits_x = 0;
static uint8_t  g_digits_y = 30;

void intermission_begin(uint32_t ms, const char* title){
  g_active = 1;
  g_end_ms = millis() + ms;
  if (!title) title = "GET READY";

  // safe copy of title
  unsigned i = 0;
  for (; title[i] && i < sizeof(g_title)-1; ++i) g_title[i] = title[i];
  g_title[i] = '\0';

  // one-time draw needed
  g_dirty      = true;
  g_last_rem_s = (unsigned)-1;

  // compute where the digits will start, assuming 6px per char at scale=1
  const char *label = "Starting in: ";
  size_t len = strlen(label);
  g_digits_x = (uint8_t)(10 + 6u * (unsigned)len);
  g_digits_y = 30;
}

bool intermission_active(void){
  return g_active != 0;
}

bool intermission_tick(void){
  if (!g_active) return false;

  if (g_dirty) {
    g_dirty = false;

    gfx_clear(COL_BLACK);
    gfx_header(g_title, COL_WHITE);

    gfx_text2(20, g_digits_y, "Starting in: ", COL_YELLOW, 1);

    // PVP logo
    uint8_t x = (uint8_t)((128 - GAME_TWO_LOGO_W) / 2);
    uint8_t y = (uint8_t)((128 - GAME_TWO_LOGO_H) / 2);

    gfx_blit_pal4(x, y,
                  GAME_TWO_LOGO_W, GAME_TWO_LOGO_H,
                  GAME_TWO_LOGO_IDX,
                  GAME_TWO_LOGO_PAL);
  }

  // seconds remaining (ceil to next)
  int32_t rem = (int32_t)(g_end_ms - millis());
  if (rem < 0) rem = 0;
  unsigned rem_s = (unsigned)((rem + 999) / 1000);

  // Only refresh when the number actually changes
  if (rem_s != g_last_rem_s) {
    g_last_rem_s = rem_s;

    // clear just the digits area (small rectangle to the right of the label)
    gfx_bar(g_digits_x, g_digits_y, 40, 12, COL_BLACK);

    char num[8];
    snprintf(num, sizeof(num), "%us", rem_s);
    gfx_text2(g_digits_x, g_digits_y, num, COL_YELLOW, 1);
  }

  if ((int32_t)(millis() - g_end_ms) >= 0){
    g_active = 0;
    return true;   // finished
  }
  return false;    // still counting
}
