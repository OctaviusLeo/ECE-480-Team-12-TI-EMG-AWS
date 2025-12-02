#include "story_items.h"
#include "timer.h"

/*
 * Core idea:
 *  - player_mult > 1.0f  => fight is easier for player (their effective Hz goes up)
 *  - enemy_mult  < 1.0f  => fight is easier (enemy threshold goes down)
 *  - enemy_mult  > 1.0f  => fight is harder (enemy threshold goes up)
 *
 * These items are shared between Story and Tower.
 */

// ---------------------------------------------------------------------------
// Simple RNG for item selection
// ---------------------------------------------------------------------------

static uint32_t s_items_rng_state = 0u;

static uint32_t story_items_rand_internal(void)
{
    if (s_items_rng_state == 0u) {
        s_items_rng_state = millis() ^ 0xA5A5A5A5u;
    }
    s_items_rng_state = s_items_rng_state * 1664525u + 1013904223u;
    return s_items_rng_state;
}

void story_items_pick_two(uint8_t *out_i0, uint8_t *out_i1)
{
    if (!out_i0 || !out_i1) return;

    if (STORY_ITEMS_COUNT < 2u) {
        *out_i0 = 0u;
        *out_i1 = 0u;
        return;
    }

    uint8_t i0 = (uint8_t)(story_items_rand_internal() % STORY_ITEMS_COUNT);
    uint8_t i1;
    do {
        i1 = (uint8_t)(story_items_rand_internal() % STORY_ITEMS_COUNT);
    } while (i1 == i0);

    *out_i0 = i0;
    *out_i1 = i1;
}

/* ------------------------------------------------------------------------- */
/*  Backwards-compatible A/B items                                           */
/* ------------------------------------------------------------------------- */

/* Small, safe buff to the player */
const story_item_t STORY_ITEM_A = {
    "Bandage (+10% you)",
    1.10f,   // +10% player
    1.00f    // enemy unchanged
};

/* Pure risk: makes the enemy significantly stronger */
const story_item_t STORY_ITEM_B = {
    "Cursed Ring (+0% you, +20% enemy)",
    1.00f,   // player unchanged
    1.20f    // +20% enemy
};

/* ------------------------------------------------------------------------- */
/*  Shared item pool (Story + Tower)                                         */
/* ------------------------------------------------------------------------- */
/*
 * You can think of these roughly as:
 *   - Offense: buff player (player_mult > 1.0f)
 *   - Defense: weaken enemy (enemy_mult < 1.0f)
 *   - Risky: buff both or nerf both
 *   - Utility: mild tweaks in both directions
 */

const story_item_t STORY_ITEMS[] = {
    /* 0: Safe offensive buff */
    {
        "Bandage (+10% you)",
        1.10f,  // player
        1.00f   // enemy
    },

    /* 1: Defensive charm, weakens the enemy */
    {
        "Shield Charm (-15% enemy)",
        1.00f,  // player unchanged
        0.85f   // -15% enemy threshold
    },

    /* 2: Glass Cannon – strong player buff, small enemy buff */
    {
        "Glass Cannon (+25% you, +10% enemy)",
        1.25f,  // big player boost
        1.10f   // enemy a bit tougher
    },

    /* 3: Stamina Tonic – modest buff, slight enemy debuff */
    {
        "Stamina Tonic (+8% you, -5% enemy)",
        1.08f,  // +8% player
        0.95f   // -5% enemy
    },

    /* 4: Cursed Ring – pure risk, enemy much stronger */
    {
        "Cursed Ring (+0% you, +20% enemy)",
        1.00f,  // player unchanged
        1.20f   // +20% enemy
    },

    /* 5: Rally Banner – good offensive buff, slight enemy debuff */
    {
        "Rally Banner (+15% you, -5% enemy)",
        1.15f,  // +15% player
        0.95f   // -5% enemy
    },

    /* 6: Training Weights – both sides get a bit stronger */
    {
        "Training Weights (+5% you, +5% enemy)",
        1.05f,  // +5% player
        1.05f   // +5% enemy
    },

    /* 7: Mirror Talisman – slight player nerf, big enemy nerf */
    {
        "Mirror Talisman (-5% you, -15% enemy)",
        0.95f,  // -5% player
        0.85f   // -15% enemy
    }
};

/* Number of entries in STORY_ITEMS[] */
const uint8_t STORY_ITEMS_COUNT =
    (uint8_t)(sizeof(STORY_ITEMS) / sizeof(STORY_ITEMS[0]));
