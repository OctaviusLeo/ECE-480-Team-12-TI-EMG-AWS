#include <stdint.h>
#include <stdbool.h>
#include "timer.h"      // millis()
#include "gfx.h"        // gfx_bar, gfx_blit565
#include "project.h"    // COL_BLACK
#include "ti_logo.h"    // TI_LOGO_W, TI_LOGO_H, TI_LOGO[]
#include "MSU_logo.h"   // MSU_LOGO_W, MSU_LOGO_H, MSU_LOGO[]

static bool     g_active         = false;
static uint32_t g_t0_ms          = 0;
static uint32_t g_duration_ms    = 0;
static uint32_t g_last_frame_idx = (uint32_t)-1;

void logo_anim_start(uint32_t duration_ms)
{
    g_active      = true;
    g_t0_ms       = millis();
    g_duration_ms = duration_ms;
    g_last_frame_idx = (uint32_t)-1;   // force redraw on first tick
}

bool logo_anim_tick(void)
{
    if (!g_active) return false;

    uint32_t now     = millis();
    uint32_t elapsed = now - g_t0_ms;

    if (elapsed >= g_duration_ms) {
        g_active = false;
        return true;  // animation done
    }

    /* Switch frame every 200 ms (so over 2s you get ~10 flips) */
    uint32_t frame = (elapsed / 200u) & 1u;  // 0 or 1

    if (frame != g_last_frame_idx) {
        g_last_frame_idx = frame;

        /* Compute a bounding box that fits BOTH logos,
           so we can clear once and draw either centered inside. */
        uint8_t box_w = (TI_LOGO_W  > MSU_LOGO_W)  ? TI_LOGO_W  : MSU_LOGO_W;
        uint8_t box_h = (TI_LOGO_H  > MSU_LOGO_H)  ? TI_LOGO_H  : MSU_LOGO_H;

        uint8_t box_x = (uint8_t)((128 - box_w) / 2);
        uint8_t box_y = (uint8_t)((128 - box_h) / 2);

        /* Clear just the logo region before drawing the new one. */
        gfx_bar(box_x, box_y, box_w, box_h, COL_BLACK);

        if (frame == 0u) {
            /* TI logo centered in box */
            uint8_t x = (uint8_t)(box_x + (box_w - TI_LOGO_W) / 2);
            uint8_t y = (uint8_t)(box_y + (box_h - TI_LOGO_H) / 2);
            gfx_blit565(x, y, TI_LOGO_W, TI_LOGO_H, TI_LOGO);
        } else {
            /* MSU logo centered in box */
            uint8_t x = (uint8_t)(box_x + (box_w - MSU_LOGO_W) / 2);
            uint8_t y = (uint8_t)(box_y + (box_h - MSU_LOGO_H) / 2);
            gfx_blit565(x, y, MSU_LOGO_W, MSU_LOGO_H, MSU_LOGO);
        }
    }

    return false;  // still animating
}
