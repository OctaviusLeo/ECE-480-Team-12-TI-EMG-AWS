#include "cheevos.h"
#include "gfx.h"
#include "project.h"
#include "timer.h"

#define CHEEVOS_PER_PAGE 8

static save_t *g_save = 0;

/* enum in cheevos.h runs from ACH_FIRST_WIN_PVP (=0) to ACH_TOWER_CLEAR (=29),
 * so total count is last-id + 1. */
enum { CHEEVO_COUNT = ACH_TOWER_CLEAR + 1 };

static const char *NAMES[CHEEVO_COUNT] = {
    "NOT a newbie",     //ACH_TUTORIAL
    "You're Better",    // ACH_FIRST_WIN_PVP
    "PULSEBOUND",      // ACH_STORY_START
    "Ch1:The Awakening",        // ACH_CH1
    "Scarecrow",        // ACH_SCARECROW
    "Ch2:Training Grounds",        // ACH_CH2
    "Training Dummy",   // ACH_TRAINING_DUMMY
    "Ch3:Catacombs",        // ACH_CH3
    "Rat King",         // ACH_RAT_KING
    "Ch4:Ambush Road",        // ACH_CH4
    "Bandits",          // ACH_BANDITS
    "Ch5:Tourney Field",        // ACH_CH5
    "Knight",           // ACH_KNIGHT
    "Ch6:Grand Arena",        // ACH_CH6
    "Champion",         // ACH_CHAMPION
    "Ch7:Arcane Tower",        // ACH_CH7
    "Sorcerer",         // ACH_SORCERER
    "Ch8:Dragon's Lair",        // ACH_CH8
    "Dragon",           // ACH_DRAGON
    "Ch9:Demon Castle",        // ACH_CH9
    "Demon King",       // ACH_DEMON_KING
    "Ch10:ERROR",       // ACH_CH10
    "Game Admin",       // ACH_GAME_ADMIN
    "Story Cleared?",   // ACH_STORY_CLEAR
    "The Forgotten Tower",      // ACH_TOWER_START
    "F5:Skeletal Field",          // ACH_TOWER_5
    "F10:Fire's Hell",         // ACH_TOWER_10
    "F15:Magical Crystals",         // ACH_TOWER_15
    "F20:The Void",         // ACH_TOWER_20
    "F25:Tower's Secret",         // ACH_TOWER_24
    "Tower Clear"       // ACH_TOWER_CLEAR
};

static cheevo_t  g_toast_id    = (cheevo_t)(-1);
static uint32_t  g_toast_until = 0;

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

    g_toast_id    = id;
    g_toast_until = millis() + 2000u;   // show for ~2 seconds

    return true;    // newly unlocked
}

// Draw one page of achievements (0-based page index)
// Each page shows at most 9 achievements.
void cheevos_draw_panel_page(uint8_t page){
    // Total pages, rounded up
    uint8_t pages = (uint8_t)((CHEEVO_COUNT + CHEEVOS_PER_PAGE - 1) / CHEEVOS_PER_PAGE);
    if (pages == 0u) {
        pages = 1u;
    }
    if (page >= pages) {
        page = (uint8_t)(pages - 1u);
    }

    gfx_clear(COL_BLACK);
    gfx_header("ACHIEVEMENTS", COL_WHITE);
    gfx_bar(0, 18, 128, 1, COL_DKGRAY);

    const int base_y = 30;
    const int row_h  = 12;

    int start = (int)page * CHEEVOS_PER_PAGE;
    int end   = start + CHEEVOS_PER_PAGE;
    if (end > CHEEVO_COUNT) {
        end = CHEEVO_COUNT;
    }

    // Only draw rows 0..(end-start-1), so max 9 per screen
    int row = 0;
    for (int i = start; i < end; ++i, ++row) {
        const char *label = NAMES[i] ? NAMES[i] : "???";
        uint8_t got       = g_save && (g_save->cheevos_bits & (1u << i));
        uint16_t col      = got ? COL_GREEN : COL_DKGRAY;
        gfx_text2(8, base_y + row * row_h, label, col, 1);
    }

    // Optional: tiny page indicator in the bottom-right
    char buf[12];
    snprintf(buf, sizeof(buf), "%u/%u",
             (unsigned)(page + 1u), (unsigned)pages);
    gfx_text2(96, 120, buf, COL_DKGRAY, 1);
}

// Keep old API as "page 0"
void cheevos_draw_panel(void){
    cheevos_draw_panel_page(0);
}

void cheevos_draw_toast(void){
    // No save bound or no active toast -> nothing to draw
    if (!g_save) return;
    if (g_toast_until == 0u) return;
    if (g_toast_id < 0 || g_toast_id >= CHEEVO_COUNT) return;

    uint32_t now = millis();
    if (now >= g_toast_until){
        // Toast expired; clear it and stop drawing
        g_toast_until = 0u;
        return;
    }

    const char *label = NAMES[g_toast_id] ? NAMES[g_toast_id] : "???";

    // Simple banner at bottom of screen
    const uint8_t y = 110;  // last ~18 px of 128x128

    gfx_bar(0, y,   128, 18, COL_DKGRAY);
    gfx_text2(2, y + 1,  "Achievement Unlocked!", COL_YELLOW, 1);
    gfx_text2(2, y + 9,  label,                   COL_WHITE,  1);
}
