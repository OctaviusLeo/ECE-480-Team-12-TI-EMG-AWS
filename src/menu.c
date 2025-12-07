/*==============================================================================
 * @file    menu.c
 * @brief   Main menu state machine and title/attract screens.
 *
 * This file is part of the EMG flex-frequency game project and follows the
 * project coding standard for file-level documentation.
 *============================================================================*/
#include <stdio.h>
#include <stdbool.h>
#include "menu.h"
#include "gfx.h"
#include "project.h"
#include "timer.h"
#include "game.h"                 // game_get_metrics(), game_mode_t
#include "choice_input.h"         // choice_from_hz(), choice_draw_hint()
#include "game_opening_screen_logo.h" // GAME_OPENING_SCREEN_LOGO_* & bitmap

#define TITLE_LOAD_MS 2000u   // total time for 0–100% bar (2s)

// Added MS_LOGO as first state
typedef enum { MS_LOGO = 0, MS_TITLE, MS_SELECT, MS_ATTRACT } mstate_t;

static mstate_t  g_ms;
static uint32_t  g_t0;
static uint8_t   g_cursor;          // 0..4
// Used as last scroll time while in MS_SELECT
static uint32_t  g_last_input_ms;
static bool      g_dirty;           // true = need to (re)draw current state

static const char* mode_name(uint8_t m)
{
    switch ((game_mode_t)m) {
    case MODE_PLAYGROUND: return "Playground";
    case MODE_PVP:        return "PVP";
    case MODE_STORY:      return "Story";
    case MODE_TOWER:      return "Tower";
    case MODE_CREDITS:    return "Credits + Trophy";
    default:              return "Unknown";
    }
}

/* change menu state and mark screen dirty */
static void menu_goto(mstate_t ns)
{
    g_ms    = ns;
    g_t0    = millis();
    g_dirty = true;
}

void menu_start(void)
{
    g_cursor        = 0;
    g_last_input_ms = millis();   // also seed scroll timer
    // Start with the logo state instead of MS_TITLE
    menu_goto(MS_LOGO);
}

bool menu_tick(uint8_t *out_mode)
{
    float   hz   = 0.0f;
    float   base = 0.0f;
    uint8_t pct  = 0;
    game_get_metrics(&hz, &pct, &base);

    uint32_t now = millis();

    switch (g_ms) {

    /*  show game_opening_screen_logo for 3 seconds */
    case MS_LOGO: {
        if (g_dirty) {
            g_dirty = false;

            gfx_clear(COL_BLACK);

            uint8_t x = (uint8_t)((128 - GAME_OPENING_SCREEN_LOGO_W) / 2);
            uint8_t y = (uint8_t)((128 - GAME_OPENING_SCREEN_LOGO_H) / 2);

            gfx_blit_pal4(x, y,
                        GAME_OPENING_SCREEN_LOGO_W, GAME_OPENING_SCREEN_LOGO_H,
                        GAME_OPENING_SCREEN_LOGO_IDX,
                        GAME_OPENING_SCREEN_LOGO_PAL);
        }

        // Stay on this screen for 3 seconds, then move on
        if ((now - g_t0) >= 3000u) {
            menu_goto(MS_TITLE);
        }
    } break;

    case MS_TITLE: {
        uint32_t dt = now - g_t0;

        if (g_dirty) {
            g_dirty = false;
            gfx_clear(COL_BLACK);
            gfx_header("BOOTING", COL_RED);
            gfx_bar(0, 18, 128, 1, COL_DKGRAY);
            gfx_text2(0, 30, "downloading:", COL_WHITE, 1);
            gfx_text2(50, 40, "packages...", COL_WHITE, 1);
            gfx_text2(50, 50, "patching...", COL_WHITE, 1);
            gfx_text2(50, 60, "finalizing...", COL_WHITE, 1);
        }

        // Progress computation (0–100%)
        uint8_t load_pct;
        if (dt >= TITLE_LOAD_MS) {
            load_pct = 100u;
        } else {
            // 100 steps over TITLE_LOAD_MS
            load_pct = (uint8_t)((dt * 100u) / TITLE_LOAD_MS);
        }

        // Draw progress bar + % text

        const uint8_t bar_x = 10;
        const uint8_t bar_y = 92;    // just under the text
        const uint8_t bar_w = 108;
        const uint8_t bar_h = 8;

        // Gray baseline bar
        gfx_bar(bar_x, bar_y, bar_w, bar_h, COL_DKGRAY);

        // Green fill according to % complete
        uint8_t fill_w = (uint8_t)((load_pct * bar_w) / 100u);
        if (fill_w > 0u) {
            gfx_bar(bar_x, bar_y, fill_w, bar_h, COL_RED);
        }

        // Percentage text on top of the bar
        char buf[8];
        snprintf(buf, sizeof(buf), "%3u%%", load_pct);

        // Rough center horizontally over the bar
        uint8_t text_x = (uint8_t)(bar_x + (bar_w / 2) - 12);
        uint8_t text_y = (uint8_t)(bar_y - 14);
        gfx_text2(text_x, text_y, buf, COL_WHITE, 1);

        // When loading hits 100%, go to select
        if (load_pct >= 100u) {
            menu_goto(MS_SELECT);
        }
    } break;

    case MS_SELECT: {
        if (g_dirty) {
            g_dirty = false;
            gfx_clear(COL_BLACK);
            gfx_header("PULSEBOUND", COL_WHITE);
            gfx_bar(0, 18, 128, 1, COL_DKGRAY);
        }

        // Simple text menu
        const char* items[] = {
            "Playground",
            "PVP",
            "Story",
            "Tower",
            "Credits + Trophy"
        };
        const uint8_t n_items = (uint8_t)(sizeof(items)/sizeof(items[0]));

        // Up/Down selection from Hz input
        int choice = choice_from_hz(hz, 5);
        if (choice >= 0) {
            g_cursor = (uint8_t)choice;
            g_last_input_ms = now;
        }

        // Draw items
        uint8_t y = 28;
        for (uint8_t i = 0; i < n_items; ++i) {
            uint16_t col = (i == g_cursor) ? COL_CYAN : COL_WHITE;
            gfx_text2(8, y, items[i], col, 1);
            y = (uint8_t)(y + 10);
        }

        // Hints and bar
        choice_draw_hint(8, 90);
        choice_draw_bar(8, 100, 112, 8, hz);

        // A “select” threshold: if you maintain a high Hz for some time,
        // confirm the current cursor as the mode to enter.
        if ((now - g_last_input_ms) > 1500u && hz > 25.0f) {
            if (out_mode) {
                *out_mode = g_cursor;
            }
            return true;
        }
    } break;

    case MS_ATTRACT: {
        // Not used in this build; placeholder for idle animation.
        // Could run an attract mode / demo loop here.
        if (g_dirty) {
            g_dirty = false;
            gfx_clear(COL_BLACK);
            gfx_header("ATTRACT", COL_WHITE);
        }
    } break;
    }

    return false;
}

