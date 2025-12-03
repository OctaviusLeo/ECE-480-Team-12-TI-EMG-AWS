#ifndef STORY_ITEMS_H
#define STORY_ITEMS_H

#include <stdint.h>

typedef struct {
    const char* name;
    float       player_mult;   // multiply player avg Hz (e.g. 1.10f)
    float       enemy_mult;    // multiply enemy target Hz (e.g. 0.90f)
} story_item_t;

/*
 * Backwards-compatible A/B items.
 */
extern const story_item_t STORY_ITEM_A;  // Bandage (+10% you)
extern const story_item_t STORY_ITEM_B;  // Cursed Ring (+0% you, +20% enemy)

/*
 * Shared item pool for both Story and Tower.
 *
 * STORY_ITEMS[] holds all offensive/defensive/utility-style items.
 * STORY_ITEMS_COUNT gives the number of entries in STORY_ITEMS[].
 *
 * Typical pattern in story/tower code:
 *   extern const story_item_t STORY_ITEMS[];
 *   extern const uint8_t      STORY_ITEMS_COUNT;
 *
 *   const story_item_t* left  = &STORY_ITEMS[some_index];
 *   const story_item_t* right = &STORY_ITEMS[other_index];
 *   // feed these into the chest + two-box UI
 */
extern const story_item_t STORY_ITEMS[];
extern const uint8_t      STORY_ITEMS_COUNT;

void story_items_pick_two(uint8_t *out_i0, uint8_t *out_i1);

#endif /* STORY_ITEMS_H */
