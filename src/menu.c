#include <stdio.h>
#include <stdbool.h>
#include "menu.h"
#include "gfx.h"
#include "project.h"
#include "timer.h"
#include "game.h"                 // game_get_metrics(), game_mode_t
#include "choice_input.h"         // choice_from_hz(), choice_draw_hint()
#include "game_opening_screen_logo.h" // GAME_OPENING_SCREEN_LOGO_* & bitmap

// Added MS_LOGO as first state
typedef enum { MS_LOGO = 0, MS_TITLE, MS_SELECT, MS_ATTRACT } mstate_t;

static mstate_t  g_ms;
static uint32_t  g_t0;
static uint8_t   g_cursor;          // 0..4
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

/* Helper: change menu state and mark screen dirty */
static void menu_goto(mstate_t ns)
{
    g_ms    = ns;
    g_t0    = millis();
    g_dirty = true;
}

void menu_start(void)
{
    g_cursor        = 0;
    g_last_input_ms = millis();
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

    /*  show game_opening_screen_logo for 5 seconds */
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

        // Stay on this screen for 5 seconds, then move on
        if ((now - g_t0) >= 5000u) {
            menu_goto(MS_TITLE);
        }
    } break;

    case MS_TITLE: {
        if (g_dirty) {
            g_dirty = false;
            gfx_clear(COL_BLACK);
            gfx_header("BOOTING", COL_RED);
            gfx_bar(0, 18, 128, 1, COL_DKGRAY);
            gfx_text2(25, 64, "downloading", COL_WHITE, 1);
            gfx_text2(25, 74, "packages...", COL_WHITE, 1);
        }

        if (hz > 3.0f || (now - g_t0) >= 2000u) {
            menu_goto(MS_SELECT);
        }
    } break;

    case MS_SELECT: {
        const int base_y = 32;
        const int row_h  = 18;

        /* INPUT / STATE UPDATE (every tick)*/

        /* Quick flex bursts to move cursor (debounced 250 ms) */
        if ((now - g_last_input_ms) >= 250u) {
            if (choice_from_hz(hz, 50.0f) == CHOICE_A && hz > 3.0f) {
                g_cursor = (uint8_t)((g_cursor + 1u) % 5u);
                g_last_input_ms = now;
                g_dirty = true;   // highlight changed -> redraw needed
            }
        }

        /* Hold > 1.0 s above threshold to confirm selection */
        static uint32_t hold_t0  = 0;
        static bool     holding  = false;
        const float     CONFIRM_THRESH = 0.0f;

        if (hz >= CONFIRM_THRESH) {
            if (!holding) {
                holding  = true;
                hold_t0  = now;
            } else if ((now - hold_t0) >= 1000u) {
                if (out_mode) {
                    *out_mode = g_cursor;   // MODE_*
                }
                return true;  // selection made
            }
        } else {
            holding = false;
        }

        /* DRAW ONLY WHEN DIRTY */
        if (g_dirty) {
            g_dirty = false;

            gfx_clear(COL_BLACK);
            gfx_header("FLEX to SELECT", COL_WHITE);
            gfx_bar(0, 18, 128, 1, COL_DKGRAY);

            for (int i = 0; i < 5; ++i) {
                uint16_t bg = (i == g_cursor) ? COL_GREEN : COL_BLACK;
                uint16_t fg = (i == g_cursor) ? COL_BLACK : COL_WHITE;
                if (i == g_cursor) {
                    gfx_bar(6, base_y + i * row_h - 2, 116, row_h, bg);
                }
                gfx_text2(10, base_y + i * row_h,
                          mode_name((uint8_t)i), fg, 1);
            }

            choice_draw_hint(base_y + 5 * row_h + 6);
        }
    } break;

    case MS_ATTRACT:
        /* Optional attract mode; can follow same g_dirty pattern if used */
        break;
    }

    return false;
}
