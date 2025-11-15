// enemies + chapter names and base target Hz (0..150)
#ifndef STORY_DATA_H
#define STORY_DATA_H
#include <stdint.h>

#define STORY_CHAPTERS 10

typedef struct {
  const char* name;
  const char* enemy;
  uint16_t    enemy_hz;   // target average Hz to beat this chapter
} story_chapter_t;

extern const story_chapter_t g_story[STORY_CHAPTERS];

#endif
