/*==============================================================================
 * @file    gfx.c
 * @brief   Graphics helper routines for the SSD1351 OLED display.
 *
 * This file is part of the EMG flex-frequency game project and follows the
 * project coding standard for file-level documentation.
 *============================================================================*/

#include <stdint.h>
#include <stdbool.h>   
#include <string.h>     
#include "ssd1351.h"
#include "project.h"
#include "ssd1351.h"
#include "gfx.h"

// 5x7 font
// Each byte: bit0=row0 (top), bit6=row6 (bottom).
static const uint8_t F[96][5] = {
/* 0x20 ' ' */ {0,0,0,0,0},
/* 0x21 '!' */ {0x00,0x00,0x5F,0x00,0x00},
/* 0x22 '"'*/ {0x00,0x07,0x00,0x07,0x00},
/* 0x23 '#'*/ {0x14,0x7F,0x14,0x7F,0x14},
/* 0x24 '$'*/ {0x24,0x2A,0x7F,0x2A,0x12},
/* 0x25 '%'*/ {0x23,0x13,0x08,0x64,0x62},
/* 0x26 '&'*/ {0x36,0x49,0x55,0x22,0x50},
/* 0x27 '''*/ {0x00,0x05,0x03,0x00,0x00},
/* 0x28 '(' */ {0x00,0x1C,0x22,0x41,0x00},
/* 0x29 ')' */ {0x00,0x41,0x22,0x1C,0x00},
/* 0x2A '*' */ {0x14,0x08,0x3E,0x08,0x14},
/* 0x2B '+' */ {0x08,0x08,0x3E,0x08,0x08},
/* 0x2C ',' */ {0x00,0x50,0x30,0x00,0x00},
/* 0x2D '-' */ {0x08,0x08,0x08,0x08,0x08},
/* 0x2E '.' */ {0x00,0x40,0x40,0x00,0x00},
/* 0x2F '/' */ {0x02,0x04,0x08,0x10,0x20},

/* 0x30 '0' */ {0x3E,0x51,0x49,0x45,0x3E},
/* 0x31 '1' */ {0x00,0x42,0x7F,0x40,0x00},
/* 0x32 '2' */ {0x42,0x61,0x51,0x49,0x46},
/* 0x33 '3' */ {0x21,0x41,0x45,0x4B,0x31},
/* 0x34 '4' */ {0x18,0x14,0x12,0x7F,0x10},
/* 0x35 '5' */ {0x27,0x45,0x45,0x45,0x39},
/* 0x36 '6' */ {0x3C,0x4A,0x49,0x49,0x30},
/* 0x37 '7' */ {0x01,0x71,0x09,0x05,0x03},
/* 0x38 '8' */ {0x36,0x49,0x49,0x49,0x36},
/* 0x39 '9' */ {0x06,0x49,0x49,0x29,0x1E},
/* 0x3A ':' */ {0x00,0x14,0x00,0x14,0x00},
/* 0x3B ';' */ {0x00,0x56,0x36,0x00,0x00},
/* 0x3C '<' */ {0x08,0x14,0x22,0x41,0x00},
/* 0x3D '=' */ {0x14,0x14,0x14,0x14,0x14},
/* 0x3E '>' */ {0x00,0x41,0x22,0x14,0x08},
/* 0x3F '?' */ {0x02,0x01,0x51,0x09,0x06},

/* 0x40 '@' */ {0x32,0x49,0x79,0x41,0x3E},
/* 0x41 'A' */ {0x7E,0x11,0x11,0x11,0x7E},
/* 0x42 'B' */ {0x7F,0x49,0x49,0x49,0x36},
/* 0x43 'C' */ {0x3E,0x41,0x41,0x41,0x22},
/* 0x44 'D' */ {0x7F,0x41,0x41,0x22,0x1C},
/* 0x45 'E' */ {0x7F,0x49,0x49,0x49,0x41},
/* 0x46 'F' */ {0x7F,0x09,0x09,0x09,0x01},
/* 0x47 'G' */ {0x3E,0x41,0x49,0x49,0x7A},
/* 0x48 'H' */ {0x7F,0x08,0x08,0x08,0x7F},
/* 0x49 'I' */ {0x00,0x41,0x7F,0x41,0x00},
/* 0x4A 'J' */ {0x20,0x40,0x41,0x3F,0x01},
/* 0x4B 'K' */ {0x7F,0x08,0x14,0x22,0x41},
/* 0x4C 'L' */ {0x7F,0x40,0x40,0x40,0x40},
/* 0x4D 'M' */ {0x7F,0x02,0x0C,0x02,0x7F},
/* 0x4E 'N' */ {0x7F,0x04,0x08,0x10,0x7F},
/* 0x4F 'O' */ {0x3E,0x41,0x41,0x41,0x3E},

/* 0x50 'P' */ {0x7F,0x09,0x09,0x09,0x06},
/* 0x51 'Q' */ {0x3E,0x41,0x51,0x21,0x5E},
/* 0x52 'R' */ {0x7F,0x09,0x19,0x29,0x46},
/* 0x53 'S' */ {0x46,0x49,0x49,0x49,0x31},
/* 0x54 'T' */ {0x01,0x01,0x7F,0x01,0x01},
/* 0x55 'U' */ {0x3F,0x40,0x40,0x40,0x3F},
/* 0x56 'V' */ {0x1F,0x20,0x40,0x20,0x1F},
/* 0x57 'W' */ {0x7F,0x10,0x08,0x10,0x7F},
/* 0x58 'X' */ {0x63,0x14,0x08,0x14,0x63},
/* 0x59 'Y' */ {0x07,0x08,0x70,0x08,0x07},
/* 0x5A 'Z' */ {0x61,0x51,0x49,0x45,0x43},
/* 0x5B '[' */ {0x00,0x7F,0x41,0x41,0x00},
/* 0x5C '\' */ {0x20,0x10,0x08,0x04,0x02},
/* 0x5D ']' */ {0x00,0x41,0x41,0x7F,0x00},
/* 0x5E '^' */ {0x04,0x02,0x01,0x02,0x04},
/* 0x5F '_' */ {0x40,0x40,0x40,0x40,0x40},

/* 0x60 '`' */ {0x00,0x01,0x02,0x00,0x00},
/* 0x61 'a' */ {0x20,0x54,0x54,0x54,0x78},
/* 0x62 'b' */ {0x7F,0x48,0x44,0x44,0x38},
/* 0x63 'c' */ {0x38,0x44,0x44,0x44,0x20},
/* 0x64 'd' */ {0x38,0x44,0x44,0x48,0x7F},
/* 0x65 'e' */ {0x38,0x54,0x54,0x54,0x18},
/* 0x66 'f' */ {0x08,0x7E,0x09,0x01,0x02},
/* 0x67 'g' */ {0x0C,0x52,0x52,0x52,0x3E},
/* 0x68 'h' */ {0x7F,0x08,0x04,0x04,0x78},
/* 0x69 'i' */ {0x00,0x44,0x7D,0x40,0x00},
/* 0x6A 'j' */ {0x20,0x40,0x44,0x3D,0x00},
/* 0x6B 'k' */ {0x7F,0x10,0x28,0x44,0x00},
/* 0x6C 'l' */ {0x00,0x41,0x7F,0x40,0x00},
/* 0x6D 'm' */ {0x7C,0x04,0x18,0x04,0x78},
/* 0x6E 'n' */ {0x7C,0x08,0x04,0x04,0x78},
/* 0x6F 'o' */ {0x38,0x44,0x44,0x44,0x38},

/* 0x70 'p' */ {0x7C,0x14,0x14,0x14,0x08},
/* 0x71 'q' */ {0x08,0x14,0x14,0x18,0x7C},
/* 0x72 'r' */ {0x7C,0x08,0x04,0x04,0x08},
/* 0x73 's' */ {0x48,0x54,0x54,0x54,0x24},
/* 0x74 't' */ {0x04,0x3F,0x44,0x40,0x20},
/* 0x75 'u' */ {0x3C,0x40,0x40,0x20,0x7C},
/* 0x76 'v' */ {0x1C,0x20,0x40,0x20,0x1C},
/* 0x77 'w' */ {0x3C,0x40,0x30,0x40,0x3C},
/* 0x78 'x' */ {0x44,0x28,0x10,0x28,0x44},
/* 0x79 'y' */ {0x0C,0x50,0x50,0x50,0x3C},
/* 0x7A 'z' */ {0x44,0x64,0x54,0x4C,0x44},
/* 0x7B '{' */ {0x00,0x08,0x36,0x41,0x00},
/* 0x7C '|' */ {0x00,0x00,0x7F,0x00,0x00},
/* 0x7D '}' */ {0x00,0x41,0x36,0x08,0x00},
/* 0x7E '~' */ {0x08,0x04,0x08,0x10,0x08},
/* 0x7F     */ {0,0,0,0,0}
};

static void draw_pixel_run(uint8_t x, uint8_t y, uint8_t w, uint16_t color){
  ssd1351_draw_rect(x, y, w, 1, color);
}

static void put_char_scaled(uint8_t x, uint8_t y, char c, uint16_t color, uint8_t scale){
  uint8_t uc = (uint8_t)c;
  const uint8_t* p = (uc >= 32 && uc < 128) ? F[uc - 32] : F[0]; // fallback to space
  for(int col=0; col<5; col++){
    for(int row=0; row<7; row++){
      if(p[col] & (1<<row)){
        uint8_t px = x + col*scale;
        uint8_t py = y + row*scale;
        // draw a scale×scale block
        for(uint8_t dy=0; dy<scale; dy++){
          draw_pixel_run(px, py+dy, scale, color);
        }
      }
    }
  }
}

void gfx_text2(uint8_t x, uint8_t y, const char* s, uint16_t color, uint8_t scale){
  if(scale==0) scale=1;
  const uint8_t spacing = 1;                    
  while(*s){
    if (x + 5*scale > 128) break;               // don't draw past right edge
    put_char_scaled(x, y, *s, color, scale);
    x += (5*scale) + spacing;                  
    s++;
  }
}

// Legacy 1 wrapper
void gfx_text(uint8_t x, uint8_t y, const char* s, uint16_t color){
  gfx_text2(x,y,s,color,1);
}

void gfx_bar(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t fill){
  ssd1351_draw_rect(x,y,w,h,fill);
}

void gfx_clear(uint16_t color){
  ssd1351_draw_rect(0,0,128,128,color);
}

// Non-blocking countdown overlay 
typedef struct { const char* label; uint16_t color; } cd_item_t;
static const cd_item_t _cd_seq[] = {
  {"COUNTDOWN", COL_WHITE},
  {"READY?",    COL_WHITE},
  {"3",         COL_WHITE},
  {"2",         COL_WHITE},
  {"1",         COL_WHITE},
  {"FLEX!",     COL_RED}
};

static struct {
  bool active;
  uint8_t idx;
  uint32_t next_ms;
  uint8_t band_y, band_h;
  uint16_t bg;
} _cd = {0};

// Text width helper: width = len*(5*scale+1)-1  (matches gfx_text2 spacing)
static uint8_t _text_width_px(const char* s, uint8_t scale){
  if(scale==0) scale=1;
  size_t len = 0; for(const char* p=s; *p; ++p) len++;
  if(len==0) return 0;
  uint32_t w = (uint32_t)len * (5u*scale + 1u) - 1u;
  return (w > 255u) ? 255u : (uint8_t)w;
}

// Clear a horizontal band using ≤8-px stripes (avoids 128x128 full transfer)
static void _clear_band_tiled(uint8_t y, uint8_t h, uint16_t color){
  uint8_t end = (uint8_t)((y + h) > 128 ? 128 : (y + h));
  for(uint8_t yy = y; yy < end; ){
    uint8_t hh = (uint8_t)((end - yy) > 8 ? 8 : (end - yy));
    ssd1351_draw_rect(0, yy, 128, hh, color);
    yy = (uint8_t)(yy + hh);
  }
}

// Draw one centered line at y with largest scale that fits safely
static void _draw_centered_line(uint8_t y, const char* s, uint16_t color){
  uint8_t scale = 1;
  for(uint8_t try=1; try<=8; ++try){
    uint8_t w = _text_width_px(s, try);
    uint16_t h = (uint16_t)(7u*try);
    if(w <= 124 && (y + h) <= 124) scale = try; else break;
  }
  uint8_t w = _text_width_px(s, scale);
  uint8_t x = (w < 128) ? (uint8_t)((128 - w)/2) : 0;
  gfx_text2(x, y, s, color, scale);
}

void gfx_countdown_begin(uint32_t now_ms, uint16_t bg){
  _cd.active = true;
  _cd.idx = 0;
  _cd.band_y = 32;
  _cd.band_h = 56;
  _cd.bg = bg;
  _clear_band_tiled(_cd.band_y, _cd.band_h, _cd.bg);
  _draw_centered_line(40, _cd_seq[_cd.idx].label, _cd_seq[_cd.idx].color);
  _cd.next_ms = now_ms + 1000u;
}

bool gfx_countdown_tick(uint32_t now_ms){
  if(!_cd.active) return true;
  if((int32_t)(now_ms - _cd.next_ms) >= 0){
    _cd.idx++;
    if(_cd.idx >= (sizeof(_cd_seq)/sizeof(_cd_seq[0]))){
      _cd.active = false;
      return true;
    }
    _clear_band_tiled(_cd.band_y, _cd.band_h, _cd.bg);
    _draw_centered_line(40, _cd_seq[_cd.idx].label, _cd_seq[_cd.idx].color);
    _cd.next_ms = now_ms + 1000u;
  }
  return false;
}

// Header helpers (band-only clear + centered draw)
static uint8_t _hdr_text_width_px(const char* s, uint8_t scale){
  if(scale==0) scale=1;
  size_t len=0; for(const char* p=s; *p; ++p) ++len;
  if(len==0) return 0;
  uint32_t w = (uint32_t)len * (5u*scale + 1u) - 1u;   // matches gfx_text2 spacing
  return (w>255u)?255u:(uint8_t)w;
}

// Clear
void gfx_clear_header_band(uint16_t color){
  for(uint8_t y=0; y<18; ){
    uint8_t h = (uint8_t)((18 - y) > 8 ? 8 : (18 - y));
    ssd1351_draw_rect(0, y, 128, h, color);
    y = (uint8_t)(y + h);
  }
}

// Draw centered header line, auto scale (1 or 2), band-only clear
void gfx_header(const char* s, uint16_t color){
  // choose largest scale that fits header band width/height safely
  uint8_t scale = (_hdr_text_width_px(s,2) <= 124 && (2*7) <= 16) ? 2 : 1;
  uint8_t w = _hdr_text_width_px(s, scale);
  uint8_t x = (w < 128) ? (uint8_t)((128 - w)/2) : 0;

  gfx_clear_header_band(COL_BLACK);      // erase header band only
  gfx_text2(x, 2, s, color, scale);      // draw text at y=2 inside band
}

void gfx_blit565(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint16_t *pixels){
  for (uint8_t j = 0; j < h; ++j){
    const uint16_t *row = pixels + (uint16_t)j * w;
    for (uint8_t i = 0; i < w; ++i){
      ssd1351_draw_rect((uint8_t)(x + i), (uint8_t)(y + j), 1, 1, row[i]);
    }
  }
}

void gfx_clear_rect(uint8_t x, uint8_t y,
                    uint8_t w, uint8_t h,
                    uint16_t color)
{
    ssd1351_set_window(x, y, x + w - 1, y + h - 1);

    uint32_t n = (uint32_t)w * (uint32_t)h;
    while (n--) {
        ssd1351_push_pixel(color);
    }
}

// Draw a 4-bit (16-color) paletted image.
// idx: packed indices, 2 pixels per byte (hi nibble = left, lo nibble = right)
void gfx_blit_pal4(uint8_t x, uint8_t y,
                   uint8_t w, uint8_t h,
                   const uint8_t  *idx,
                   const uint16_t *pal)
{
    uint16_t xi, yi;
    uint32_t p = 0;  // index into idx array

    for (yi = 0; yi < h; ++yi) {
        for (xi = 0; xi < w; xi += 2) {
            uint8_t b  = idx[p++];          // two pixels
            uint8_t i0 = (b >> 4) & 0x0F;   // left pixel index
            uint8_t i1 =  b       & 0x0F;   // right pixel index

            uint16_t c0 = pal[i0];
            uint16_t c1 = pal[i1];

            ssd1351_draw_rect((uint8_t)(x + xi),     (uint8_t)(y + yi), 1, 1, c0);
            ssd1351_draw_rect((uint8_t)(x + xi + 1), (uint8_t)(y + yi), 1, 1, c1);
        }
    }
}

void gfx_pixel(uint8_t x, uint8_t y, uint16_t color){
    // 1x1 rect = 1 pixel
    ssd1351_draw_rect(x, y, 1, 1, color);
}

static void gfx_line(int x0, int y0, int x1, int y1, uint16_t color){
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = (y1 > y0) ? (y0 - y1) : (y1 - y0);  // note: y inverted
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (1) {
        if (x0 >= 0 && x0 < 128 && y0 >= 0 && y0 < 128) {
            gfx_pixel((uint8_t)x0, (uint8_t)y0, color);
        }
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void gfx_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color){
    if (w == 0u || h == 0u) return;

    int x0 = x;
    int y0 = y;
    int x1 = x + (int)w - 1;
    int y1 = y + (int)h - 1;

    // Top, bottom, left, right
    gfx_line(x0, y0, x1, y0, color); // top
    gfx_line(x0, y1, x1, y1, color); // bottom
    gfx_line(x0, y0, x0, y1, color); // left
    gfx_line(x1, y0, x1, y1, color); // right
}

void gfx_xshape(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color){
    if (w == 0u || h == 0u) return;

    int x0 = x;
    int y0 = y;
    int x1 = x + (int)w - 1;
    int y1 = y + (int)h - 1;

    // Diagonal: top-left to bottom-right
    gfx_line(x0, y0, x1, y1, color);
    // Diagonal: top-right to bottom-left
    gfx_line(x1, y0, x0, y1, color);
}

void gfx_triangle(uint8_t x0, uint8_t y0,
                  uint8_t x1, uint8_t y1,
                  uint8_t x2, uint8_t y2,
                  uint16_t color)
{
    gfx_line(x0, y0, x1, y1, color);
    gfx_line(x1, y1, x2, y2, color);
    gfx_line(x2, y2, x0, y0, color);
}
