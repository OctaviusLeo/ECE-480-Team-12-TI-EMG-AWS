#include <string.h>
#include "intermission.h"
#include "timer.h"   // millis()
#include "gfx.h"     // gfx_clear, gfx_header, gfx_text2
#include "project.h"
#include "ti_logo.h"

static uint8_t  g_active = 0;
static uint32_t g_end_ms = 0;
static char     g_title[24];

void intermission_begin(uint32_t ms, const char* title){
  g_active = 1;
  g_end_ms = millis() + ms;
  if (!title) title = "GET READY";
  // safe copy
  unsigned i = 0; for (; title[i] && i < sizeof(g_title)-1; ++i) g_title[i] = title[i];
  g_title[i] = '\0';
}

bool intermission_active(void){
  return g_active != 0;
}

bool intermission_tick(void){
  if (!g_active) return false;

  // simple overlay screen
  gfx_clear(COL_BLACK);
  gfx_header(g_title, COL_WHITE);
  // seconds remaining (ceil to next)
  int32_t rem = (int32_t)(g_end_ms - millis());
  if (rem < 0) rem = 0;
  unsigned rem_s = (unsigned)((rem + 999) / 1000);

  char line[32];
  // “Starting in: Ns”
  // adjust y as you like; centered-ish
  snprintf(line, sizeof(line), "Starting in: %us", rem_s);
  gfx_text2(10, 30, line, COL_YELLOW, 1);

    uint8_t x = (uint8_t)((128 - TI_LOGO_W) / 2);
    uint8_t y = 30;  // adjust if you want it closer/further from the title
    gfx_blit565(x, y, TI_LOGO_W, TI_LOGO_H, TI_LOGO);

  if ((int32_t)(millis() - g_end_ms) >= 0){
    g_active = 0;
    return true;   // finished
  }
  return false;    // still counting
}
