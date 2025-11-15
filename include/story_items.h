// simple A/B item templates per chapter
#ifndef STORY_ITEMS_H
#define STORY_ITEMS_H
#include <stdint.h>

typedef struct {
  const char* name;
  float       player_mult;   // multiply player avg Hz (1.10f)
  float       enemy_mult;    // multiply enemy target Hz (1.00f)
} story_item_t;

/* For skeleton: same A/B across chapters can specialize later */
extern const story_item_t STORY_ITEM_A;
extern const story_item_t STORY_ITEM_B;

#endif
