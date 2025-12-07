/**
 * @file save.h
 * @brief Versioned, CRC-protected save data and persistence API.
 *
 * Defines the on-flash save_t structure and helpers to initialize,
 * load, and write save data with a trailing CRC32 for integrity.
 */

#ifndef SAVE_H
#define SAVE_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Persistent save block.
 *
 * All fields before crc32 are covered by the CRC. version must be
 * bumped whenever the layout changes.
 */
typedef struct {
  uint32_t version;              // bump when layout changes
  uint8_t  story_chapter_cleared;// 0..10
  uint8_t  tower_best_floor;     // 0..25
  uint32_t best_avg_hz_milli;    // x1000
  uint8_t  options_flags;        // bit0=colorblind, bit1=bigtext
  uint32_t cheevos_bits;         // achievements bitmap
  uint32_t crc32;                // last field
} save_t;

/**
 * @brief Initialize a save block with default values.
 *
 * @param s Pointer to save structure to initialize.
 */
void     save_defaults(save_t *s);

/**
 * @brief Load the save block from persistent storage.
 *
 * @param[out] s Save structure to fill on success.
 * @return true if a valid save with matching CRC was loaded; false if
 *         no valid save exists or the CRC check failed.
 */
bool     save_load(save_t *s);                 // returns true on valid CRC

/**
 * @brief Write the save block to persistent storage.
 *
 * @param s Pointer to save structure to persist.
 * @return true on successful write; false on error.
 */
bool     save_write(const save_t *s);

#endif /* SAVE_H */
