#include "cheevos.h"
#include "gfx.h"
#include "project.h"

static save_t *g_save = 0;

/* enum in cheevos.h runs from ACH_FIRST_WIN (=0) to ACH_TOWER_CLEAR (=29),
 * so total count is last-id + 1. */
enum { CHEEVO_COUNT = ACH_TOWER_CLEAR + 1 };

static const char *NAMES[CHEEVO_COUNT] = {
    "First Win",        // ACH_FIRST_WIN
    "Story Start",      // ACH_STORY_START
    "Chapter 1",        // ACH_CH1
    "Scarecrow",        // ACH_SCARECROW
    "Chapter 2",        // ACH_CH2
    "Training Dummy",   // ACH_TRAINING_DUMMY
    "Chapter 3",        // ACH_CH3
    "Rat King",         // ACH_RAT_KING
    "Chapter 4",        // ACH_CH4
    "Bandits",          // ACH_BANDITS
    "Chapter 5",        // ACH_CH5
    "Knight",           // ACH_KNIGHT
    "Chapter 6",        // ACH_CH6
    "Champion",         // ACH_CHAMPION
    "Chapter 7",        // ACH_CH7
    "Sorcerer",         // ACH_SORCERER
    "Chapter 8",        // ACH_CH8
    "Dragon",           // ACH_DRAGON
    "Chapter 9",        // ACH_CH9
    "Arch Demon",       // ACH_ARCH_DEMON
    "Chapter 10",       // ACH_CH10
    "Demon King",       // ACH_DEMON_KING
    "Story Clear",      // ACH_STORY_CLEAR
    "Tower Start",      // ACH_TOWER_START
    "Tower 5",          // ACH_TOWER_5
    "Tower 10",         // ACH_TOWER_10
    "Tower 15",         // ACH_TOWER_15
    "Tower 20",         // ACH_TOWER_20
    "Tower 24",         // ACH_TOWER_24
    "Tower Clear"       // ACH_TOWER_CLEAR
};

void cheevos_bind_save(save_t *s){
    g_save = s;
}

bool cheevos_unlock(cheevo_t id){
    if (!g_save) return false;
    if (id < 0 || id >= CHEEVO_COUNT) return false;

    uint32_t mask = (1u << id);
    if (g_save->cheevos_bits & mask) {
        // already unlocked
        return false;
    }

    g_save->cheevos_bits |= mask;
    (void)save_write(g_save);
    return true;    // newly unlocked
}

void cheevos_draw_panel(void){
    gfx_clear(COL_BLACK);
    gfx_header("ACHIEVEMENTS", COL_WHITE);
    gfx_bar(0, 18, 128, 1, COL_DKGRAY);

    const int base_y = 30;
    const int row_h  = 12;

    for (int i = 0; i < CHEEVO_COUNT; i++) {
        const char *label = NAMES[i] ? NAMES[i] : "???";
        uint8_t got = g_save && (g_save->cheevos_bits & (1u << i));
        uint16_t col = got ? COL_GREEN : COL_DKGRAY;
        gfx_text2(8, base_y + i * row_h, label, col, 1);
    }
}
