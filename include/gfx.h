#ifndef GFX_H
#define GFX_H

#include <stdint.h>
#include <stdbool.h>

/* Basic primitives provided by gfx/SSD1351 stack */
void gfx_clear(uint16_t color);
void gfx_bar(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t fill);

/* text (legacy) */
void gfx_text(uint8_t x, uint8_t y, const char* s, uint16_t color);

/* 5x7 font with scalable draw; scale=2 doubles size */
void gfx_text2(uint8_t x, uint8_t y, const char* s, uint16_t color, uint8_t scale);

/* Optional countdown overlay API (unchanged) */
void gfx_countdown_begin(uint32_t now_ms, uint16_t bg);
bool gfx_countdown_tick(uint32_t now_ms);

/* Header-band helpers (unchanged) */
void gfx_header(const char* s, uint16_t color);
void gfx_clear_header_band(uint16_t color);

// for TI logo
void gfx_blit565(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint16_t *pixels);

/* Internal tiny helpers for text layout, matching gfx_text2 spacing */
static inline uint8_t __gfx_text_width_px(const char* s, uint8_t scale){
  if (!s) return 0;
  if (scale == 0) scale = 1;
  uint32_t len = 0; for (const char* p = s; *p; ++p) len++;
  if (len == 0) return 0;
  /* 5px glyph + 1px gap, last glyph has no trailing gap */
  uint32_t w = len * (5u*scale + 1u) - 1u;
  return (w > 255u) ? 255u : (uint8_t)w;
}

static inline void __gfx_center_line_xy(uint8_t y, const char* s, uint16_t color, uint8_t scale){
  uint8_t w = __gfx_text_width_px(s, scale);
  uint8_t x = (w < 128u) ? (uint8_t)((128u - w)/2u) : 0u;
  gfx_text2(x, y, s, color, scale);
}

/* Fullscreen, auto-fit text (single line).
   Picks the largest integer scale that still fits within 128 px width. */
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

/* Fullscreen, auto-fit text with one optional newline.
   First line slightly above center, second slightly below. */
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

/* Manual-size, centered single line */
static inline void gfx_center_text_scaled(const char* s, uint16_t color, uint8_t scale){
  if (!s) return;
  uint8_t y = (uint8_t)((128u - (7u*scale)) / 2u);
  __gfx_center_line_xy(y, s, color, scale);
}

/* Manual-size, centered at custom Y (sx=scale X, sy ignored for 5x7) */
static inline void gfx_center_text_scaled_xy(const char* s, uint16_t color, uint8_t sx, uint8_t sy){
  (void)sy; // suppress unused-parameter warning (sy intentionally ignored for 5x7 font)
  if (!s) return;
  uint8_t y = (uint8_t)((128u - (7u*sx)) / 2u);
  __gfx_center_line_xy(y, s, color, sx);
}

/* Two-line centered with manual scales; expects exactly one '\n' */
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

#endif /* GFX_H */
