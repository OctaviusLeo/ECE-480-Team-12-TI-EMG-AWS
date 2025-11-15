#include "cheevos.h"
#include "gfx.h"
#include "project.h"

static save_t *g_save = 0;
static const char* NAMES[ACH__COUNT] = {
  "First Win", "Story Clear", "Tower 25", "100 Hz+", "150 Hz+"
};

void cheevos_bind_save(save_t *s){ g_save = s; }

bool cheevos_unlock(cheevo_t id){
  if (!g_save || id >= ACH__COUNT) return false;
  uint32_t mask = (1u << id);
  if (g_save->cheevos_bits & mask) return false;
  g_save->cheevos_bits |= mask;
  (void)save_write(g_save);
  return true;
}

void cheevos_draw_panel(void){
  gfx_clear(COL_BLACK);
  gfx_header("ACHIEVEMENTS", COL_WHITE);
  gfx_bar(0,18,128,1,COL_DKGRAY);

  const int base_y=30, row_h=12;
  for (int i=0;i<ACH__COUNT;i++){
    uint8_t got = g_save && (g_save->cheevos_bits & (1u<<i));
    uint16_t col = got ? COL_GREEN : COL_DKGRAY;
    gfx_text2(8, base_y + i*row_h, NAMES[i], col, 1);
  }
}
