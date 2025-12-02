#ifndef STORY_CH1_H
#define STORY_CH1_H

#include <stdint.h>

#define STORY_CH1_W 128
#define STORY_CH1_H 73
#define STORY_CH1_PAL_SIZE 16

extern const uint16_t STORY_CH1_PAL[STORY_CH1_PAL_SIZE];
extern const uint8_t  STORY_CH1_IDX[(STORY_CH1_W * STORY_CH1_H) / 2];

#endif // STORY_CH1_H
