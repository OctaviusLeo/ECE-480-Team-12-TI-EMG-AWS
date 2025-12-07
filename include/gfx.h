/**
 * @file gfx.h
 * @brief Graphics helper API for the SSD1351 OLED.
 *
 * Provides basic drawing primitives, text rendering (5x7 font with scaling),
 * centered/fullscreen text helpers, logo blits, and a simple countdown header
 * overlay. All coordinates are in pixel units on a 128x128 display.
 */

#ifndef GFX_H
#define GFX_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Clear the entire screen to a solid color.
 *
 * @param color RGB565 color value.
 */
void gfx_clear(uint16_t color);

/**
 * @brief Draw a filled rectangle ("bar").
 *
 * @param x     Left X coordinate in pixels.
 * @param y     Top Y coordinate in pixels.
 * @param w     Width in pixels.
 * @param h     Height in pixels.
 * @param fill  RGB565 fill color.
 */
void gfx_bar(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t fill);

/**
 * @brief Draw legacy text using a fixed-size font.
 *
 * @param x     Left X coordinate in pixels.
 * @param y     Top Y coordinate in pixels.
 * @param s     Null-terminated string to draw.
 * @param color RGB565 text color.
 */
void gfx_text(uint8_t x, uint8_t y, const char* s, uint16_t color);

/**
 * @brief Draw text using a 5x7 font scaled by an integer factor.
 *
 * @param x     Left X coordinate in pixels.
 * @param y     Top Y coordinate in pixels.
 * @param s     Null-terminated string to draw.
 * @param color RGB565 text color.
 * @param scale Integer scale factor (1 = 5x7, 2 = 10x14, etc.).
 */
void gfx_text2(uint8_t x, uint8_t y, const char* s, uint16_t color, uint8_t scale);

/* Optional countdown overlay API */

/**
 * @brief Begin a countdown overlay.
 *
 * @param now_ms Current time in milliseconds.
 * @param bg     Background color for the overlay.
 */
void gfx_countdown_begin(uint32_t now_ms, uint16_t bg);

/**
 * @brief Advance/draw the countdown overlay.
 *
 * @param now_ms Current time in milliseconds.
 * @return true when the countdown has completed; false otherwise.
 */
bool gfx_countdown_tick(uint32_t now_ms);

/* Header-band helpers */

/**
 * @brief Draw a header band with text.
 *
 * @param s     Null-terminated string to display in the header.
 * @param color RGB565 text color.
 */
void gfx_header(const char* s, uint16_t color);

/**
 * @brief Clear the header band to a solid color.
 *
 * @param color RGB565 fill color.
 */
void gfx_clear_header_band(uint16_t color);

/**
 * @brief Blit a raw RGB565 image to the screen.
 *
 * @param x      Left X coordinate.
 * @param y      Top Y coordinate.
 * @param w      Image width in pixels.
 * @param h      Image height in pixels.
 * @param pixels Pointer to RGB565 pixel data (w*h entries).
 */
void gfx_blit565(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint16_t *pixels);

/**
 * @brief Draw a 4-bit (16-color) paletted image.
 *
 * idx is packed indices, 2 pixels per byte (hi nibble = left, lo nibble = right).
 *
 * @param x    Left X coordinate.
 * @param y    Top Y coordinate.
 * @param w    Image width in pixels.
 * @param h    Image height in pixels.
 * @param idx  Packed palette indices (2 pixels per byte).
 * @param pal  Palette as 16 RGB565 entries.
 */
void gfx_blit_pal4(uint8_t x, uint8_t y,
                   uint8_t w, uint8_t h,
                   const uint8_t  *idx,
                   const uint16_t *pal);

/* Internal tiny helpers for text layout, matching gfx_text2 spacing */

/**
 * @brief Compute text width in pixels for a given scale, for internal layout.
 *
 * @param s     Null-terminated string.
 * @param scale Scale factor as used by gfx_text2().
 * @return Width in pixels, clamped to 255.
 */
static inline uint8_t __gfx_text_width_px(const char* s, uint8_t scale){
  if (!s) return 0;
  if (scale == 0) scale = 1;
  uint32_t len = 0; for (const char* p = s; *p; ++p) len++;
  if (len == 0) return 0;
  /* 5px glyph + 1px gap, last glyph has no trailing gap */
  uint32_t w = len * (5u*scale + 1u) - 1u;
  return (w > 255u) ? 255u : (uint8_t)w;
}

/**
 * @brief Draw a centered single line of text at a specific Y.
 *
 * @param y     Y coordinate for the text baseline.
 * @param s     Null-terminated string.
 * @param color RGB565 text color.
 * @param scale Scale factor as used by gfx_text2().
 */
static inline void __gfx_center_line_xy(uint8_t y, const char* s, uint16_t color, uint8_t scale){
  uint8_t w = __gfx_text_width_px(s, scale);
  uint8_t x = (w < 128u) ? (uint8_t)((128u - w)/2u) : 0u;
  gfx_text2(x, y, s, color, scale);
}

/**
 * @brief Fullscreen, auto-fit single-line text.
 *
 * Picks the largest integer scale that still fits within 128 px width and
 * vertically centers the line.
 *
 * @param s     Null-terminated string.
 * @param color RGB565 text color.
 */
static inline void gfx_fullscreen_text(const char* s, uint16_t color){
  if (!s) return;
  uint8_t scale = 1;
  /* Try scales 4..1 (tweak if you have room) */
  for (uint8_t try_s = 4; try_s >= 1; --try_s){
    if (__gfx_text_width_px(s, try_s) <= 128u){ scale = try_s; break; }
    if (try_s == 1) break;
  }
  /* Roughly center vertically; header band (if used) is usually ~18px tall */
  uint8_t y =  (uint8_t)((128u - (7u*scale)) / 2u);
  __gfx_center_line_xy(y, s, color, scale);
}

/**
 * @brief Fullscreen, auto-fit text with one optional newline.
 *
 * First line slightly above center, second slightly below.
 *
 * @param s     String (may contain at most one '\n').
 * @param color RGB565 text color.
 */
static inline void gfx_fullscreen_text_nl(const char* s, uint16_t color){
  if (!s) return;
  /* Find newline if present */
  const char* nl = s;
  while (*nl && *nl != '\n') ++nl;

  if (*nl == '\n'){
    /* Split into two stack strings (safe because OLED text is short) */
    char top[64] = {0}, bot[64] = {0};
    uint32_t i = 0;
    for (const char* p = s; *p && *p != '\n' && i < sizeof(top)-1; ++p, ++i) top[i] = *p;
    i = 0;
    for (const char* p = nl+1; *p && i < sizeof(bot)-1; ++p, ++i) bot[i] = *p;

    /* Choose a scale that makes the wider line fit */
    uint8_t scale = 1;
    for (uint8_t try_s = 4; try_s >= 1; --try_s){
      uint8_t w_top = __gfx_text_width_px(top, try_s);
      uint8_t w_bot = __gfx_text_width_px(bot, try_s);
      if (w_top <= 128u && w_bot <= 128u){ scale = try_s; break; }
      if (try_s == 1) break;
    }

    /* Draw the two lines around center */
    uint8_t line_h = (uint8_t)(7u*scale + 4u);
    uint8_t y0 = (uint8_t)(64u - line_h);
    uint8_t y1 = (uint8_t)(64u + 4u);
    __gfx_center_line_xy(y0, top, color, scale);
    __gfx_center_line_xy(y1, bot, color, scale);
  } else {
    gfx_fullscreen_text(s, color);
  }
}

/**
 * @brief Centered single-line text with a manual scale factor.
 *
 * @param s     Null-terminated string.
 * @param color RGB565 text color.
 * @param scale Scale factor as used by gfx_text2().
 */
static inline void gfx_center_text_scaled(const char* s, uint16_t color, uint8_t scale){
  if (!s) return;
  uint8_t y = (uint8_t)((128u - (7u*scale)) / 2u);
  __gfx_center_line_xy(y, s, color, scale);
}

/**
 * @brief Centered single-line text at custom Y, with manual X scale.
 *
 * sy is currently ignored (5x7 font is scaled uniformly).
 *
 * @param s     Null-terminated string.
 * @param color RGB565 text color.
 * @param sx    X/Y scale factor (for 5x7).
 * @param sy    Unused (kept for API symmetry).
 */
static inline void gfx_center_text_scaled_xy(const char* s, uint16_t color, uint8_t sx, uint8_t sy){
  (void)sy; // suppress unused-parameter warning (sy intentionally ignored for 5x7 font)
  if (!s) return;
  uint8_t y = (uint8_t)((128u - (7u*sx)) / 2u);
  __gfx_center_line_xy(y, s, color, sx);
}

/**
 * @brief Two-line centered text with manual scales; expects exactly one '\n'.
 *
 * @param s     String with exactly one newline separator.
 * @param color RGB565 text color.
 * @param sx    Scale factor (used for both lines).
 * @param sy    Unused (kept for API symmetry).
 */
static inline void gfx_center_text_nl_scaled_xy(const char* s, uint16_t color, uint8_t sx, uint8_t sy){
  (void)sy; // suppress unused-parameter warning (sy intentionally ignored for 5x7 font)
  if (!s) return;

  // Re-use the NL fitter: place two lines near center with same sx
  const char* nl = s;
  while (*nl && *nl != '\n') ++nl;
  if (*nl != '\n'){ __gfx_center_line_xy((uint8_t)((128u - (7u*sx))/2u), s, color, sx); return; }

  char top[64] = {0}, bot[64] = {0};
  uint32_t i = 0;
  for (const char* p = s; *p && *p != '\n' && i < sizeof(top)-1; ++p, ++i) top[i] = *p;
  i = 0;
  for (const char* p = nl+1; *p && i < sizeof(bot)-1; ++p, ++i) bot[i] = *p;

  uint8_t line_h = (uint8_t)(7u*sx + 4u);
  uint8_t y0 = (uint8_t)(64u - line_h);
  uint8_t y1 = (uint8_t)(64u + 4u);
  __gfx_center_line_xy(y0, top, color, sx);
  __gfx_center_line_xy(y1, bot, color, sx);
}

/**
 * @brief Draw a single pixel.
 *
 * @param x     X coordinate.
 * @param y     Y coordinate.
 * @param color RGB565 color.
 */
void gfx_pixel(uint8_t x, uint8_t y, uint16_t color);

/**
 * @brief Draw an unfilled rectangle.
 *
 * @param x     Left X coordinate.
 * @param y     Top Y coordinate.
 * @param w     Width in pixels.
 * @param h     Height in pixels.
 * @param color RGB565 border color.
 */
void gfx_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);

/**
 * @brief Draw an "X" shape inside a rectangle.
 *
 * @param x     Left X coordinate.
 * @param y     Top Y coordinate.
 * @param w     Width in pixels.
 * @param h     Height in pixels.
 * @param color RGB565 color.
 */
void gfx_xshape(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);

/**
 * @brief Draw a filled triangle with vertices (x0,y0), (x1,y1), (x2,y2).
 *
 * @param x0    First vertex X.
 * @param y0    First vertex Y.
 * @param x1    Second vertex X.
 * @param y1    Second vertex Y.
 * @param x2    Third vertex X.
 * @param y2    Third vertex Y.
 * @param color RGB565 fill color.
 */
void gfx_triangle(uint8_t x0, uint8_t y0,
                  uint8_t x1, uint8_t y1,
                  uint8_t x2, uint8_t y2,
                  uint16_t color);

#endif /* GFX_H */
