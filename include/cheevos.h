#ifndef CHEEVOS_H
#define CHEEVOS_H
#include <stdbool.h>
#include <stdint.h>
#include "save.h"

typedef enum {
  ACH_FIRST_WIN=0,
  ACH_STORY_START,
  ACH_CH1,
  ACH_SCARECROW,
  ACH_CH2,
  ACH_TRAINING_DUMMY,
  ACH_CH3,
  ACH_RAT_KING,
  ACH_CH4,
  ACH_BANDITS,
  ACH_CH5,
  ACH_KNIGHT,
  ACH_CH6,
  ACH_CHAMPION,
  ACH_CH7,
  ACH_SORCERER,
  ACH_CH8,
  ACH_DRAGON,
  ACH_CH9,
  ACH_ARCH_DEMON,
  ACH_CH10,
  ACH_DEMON_KING,
  ACH_STORY_CLEAR,
  ACH_TOWER_START,
  ACH_TOWER_5,
  ACH_TOWER_10,
  ACH_TOWER_15,
  ACH_TOWER_20,
  ACH_TOWER_24,
  ACH_TOWER_CLEAR
} cheevo_t;

void cheevos_bind_save(save_t *s);          // pass loaded save here
bool cheevos_unlock(cheevo_t id);           // returns true if newly unlocked
void cheevos_draw_panel(void);              // simple list renderer
void cheevos_draw_panel_page(uint8_t page);

// Draw a small "Achievement Unlocked!" overlay if one is active.
// Safe to call every frame; does nothing if no recent unlock.
void cheevos_draw_toast(void);

#endif
