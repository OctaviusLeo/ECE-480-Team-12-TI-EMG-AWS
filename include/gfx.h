#ifndef GFX_H
#define GFX_H

#include <stdint.h>
#include <stdbool.h>   

void gfx_clear(uint16_t color);
void gfx_bar(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t fill);

// text (legacy)
void gfx_text(uint8_t x, uint8_t y, const char* s, uint16_t color);

// scale=2 doubles size.
void gfx_text2(uint8_t x, uint8_t y, const char* s, uint16_t color, uint8_t scale);

// non-blocking countdown overlay ---
void gfx_countdown_begin(uint32_t now_ms, uint16_t bg);
bool gfx_countdown_tick(uint32_t now_ms);

void gfx_header(const char* s, uint16_t color);             // centered header draw (band-only clear)
void gfx_clear_header_band(uint16_t color);                  // explicit header-band clear

// Fullscreen, auto-fit text 
void gfx_fullscreen_text(const char* s, uint16_t color);
void gfx_fullscreen_text_nl(const char* s, uint16_t color);  // supports one '\n'

// Manual-size, centered text 
void gfx_center_text_scaled(const char* s, uint16_t color, uint8_t scale);
void gfx_center_text_scaled_xy(const char* s, uint16_t color, uint8_t sx, uint8_t sy);

// two-line variant using a single '\n' split with manual scales
void gfx_center_text_nl_scaled_xy(const char* s, uint16_t color, uint8_t sx, uint8_t sy);

#endif
