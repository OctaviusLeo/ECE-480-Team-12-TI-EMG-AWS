#ifndef TOWER_DRAGON_H
#define TOWER_DRAGON_H

#include <stdint.h>

#define TOWER_DRAGON_W 100
#define TOWER_DRAGON_H 56
#define TOWER_DRAGON_PAL_SIZE 16

extern const uint16_t TOWER_DRAGON_PAL[TOWER_DRAGON_PAL_SIZE];
extern const uint8_t  TOWER_DRAGON_IDX[(TOWER_DRAGON_W * TOWER_DRAGON_H) / 2];

#endif // TOWER_DRAGON_H
