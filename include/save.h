#ifndef SAVE_H
#define SAVE_H
#include <stdint.h>
#include <stdbool.h>

/* Versioned, CRC-protected save block. */
typedef struct {
  uint32_t version;              // bump when layout changes
  uint8_t  story_chapter_cleared;// 0..10
  uint8_t  tower_best_floor;     // 0..25
  uint32_t best_avg_hz_milli;    // x1000
  uint8_t  options_flags;        // bit0=colorblind, bit1=bigtext
  uint32_t cheevos_bits;         // achievements bitmap
  uint32_t crc32;                // last field
} save_t;

void     save_defaults(save_t *s);
bool     save_load(save_t *s);                 // returns true on valid CRC
bool     save_write(const save_t *s);

#endif
