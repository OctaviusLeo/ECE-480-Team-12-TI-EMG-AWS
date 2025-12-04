#ifndef TOWER_DEMON_H
#define TOWER_DEMON_H

#include <stdint.h>

#define TOWER_DEMON_W 96
#define TOWER_DEMON_H 64
#define TOWER_DEMON_PAL_SIZE 16

extern const uint16_t TOWER_DEMON_PAL[TOWER_DEMON_PAL_SIZE];
extern const uint8_t  TOWER_DEMON_IDX[(TOWER_DEMON_W * TOWER_DEMON_H) / 2];

#endif // TOWER_DEMON_H
