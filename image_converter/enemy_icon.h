#ifndef ENEMY_ICON_H
#define ENEMY_ICON_H

#include <stdint.h>

#define ENEMY_ICON_W 26
#define ENEMY_ICON_H 30
#define ENEMY_ICON_PAL_SIZE 16

extern const uint16_t ENEMY_ICON_PAL[ENEMY_ICON_PAL_SIZE];
extern const uint8_t  ENEMY_ICON_IDX[(ENEMY_ICON_W * ENEMY_ICON_H) / 2];

#endif // ENEMY_ICON_H
