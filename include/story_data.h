/**
 * @file story_data.h
 * @brief Story chapter metadata: names, enemies, and base target Hz.
 *
 * Each chapter has a display name, an enemy name, and a base target
 * frequency (in Hz) that the player must beat on average.
 */

// enemies + chapter names and base target Hz (0..150)
#ifndef STORY_DATA_H
#define STORY_DATA_H

#include <stdint.h>

#define STORY_CHAPTERS 10

/**
 * @brief Story chapter descriptor.
 *
 * name      - Display name of the chapter.
 * enemy     - Enemy name shown to the player.
 * enemy_hz  - Target average Hz required to clear this chapter.
 */
typedef struct {
  const char* name;
  const char* enemy;
  uint16_t    enemy_hz;   // target average Hz to beat this chapter
} story_chapter_t;

/**
 * @brief Global table of all story chapters.
 *
 * The table has STORY_CHAPTERS entries, one per chapter in story mode.
 */
extern const story_chapter_t g_story[STORY_CHAPTERS];

#endif /* STORY_DATA_H */
