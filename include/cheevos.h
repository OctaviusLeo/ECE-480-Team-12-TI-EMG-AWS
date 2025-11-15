#ifndef CHEEVOS_H
#define CHEEVOS_H
#include <stdbool.h>
#include <stdint.h>
#include "save.h"

typedef enum {
  ACH_FIRST_WIN=0,
  ACH_STORY_CLEAR,
  ACH_TOWER_25,
  ACH_100HZ,
  ACH_150HZ,
  ACH__COUNT
} cheevo_t;

void cheevos_bind_save(save_t *s);          // pass your loaded save here
bool cheevos_unlock(cheevo_t id);           // returns true if newly unlocked
void cheevos_draw_panel(void);              // simple list renderer

#endif
