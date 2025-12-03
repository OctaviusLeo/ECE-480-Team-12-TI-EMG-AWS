#ifndef TOWER_ORC_H
#define TOWER_ORC_H

#include <stdint.h>

#define TOWER_ORC_W 64
#define TOWER_ORC_H 51
#define TOWER_ORC_PAL_SIZE 16

extern const uint16_t TOWER_ORC_PAL[TOWER_ORC_PAL_SIZE];
extern const uint8_t  TOWER_ORC_IDX[(TOWER_ORC_W * TOWER_ORC_H) / 2];

#endif // TOWER_ORC_H
