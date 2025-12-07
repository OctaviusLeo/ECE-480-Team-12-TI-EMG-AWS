/**
 * @file story_items.h
 * @brief Item definitions and helpers shared by Story and Tower modes.
 *
 * Items provide multipliers to player/enemy target Hz and are drawn
 * from a shared item pool for both game modes.
 */

#ifndef STORY_ITEMS_H
#define STORY_ITEMS_H

#include <stdint.h>

/**
 * @brief Story/tower item descriptor.
 *
 * name        - Display name of the item.
 * player_mult - Multiplier applied to player average Hz (e.g. 1.10f).
 * enemy_mult  - Multiplier applied to enemy target Hz (e.g. 0.90f).
 */
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

/**
 * @brief Randomly pick two distinct indices into STORY_ITEMS[].
 *
 * This helper encapsulates the shared item-picking policy used by
 * story and tower modes.
 *
 * @param[out] out_i0  Index of the first chosen item.
 * @param[out] out_i1  Index of the second chosen item.
 */
void story_items_pick_two(uint8_t *out_i0, uint8_t *out_i1);

#endif /* STORY_ITEMS_H */
