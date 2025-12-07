/**
 * @file tower_data.h
 * @brief Base target Hz per floor for Tower mode.
 *
 * The tower consists of TOWER_FLOORS floors. Each floor has a base
 * target frequency that determines its difficulty.
 */

#ifndef TOWER_DATA_H
#define TOWER_DATA_H

#include <stdint.h>

#define TOWER_FLOORS 25

/* Base target Hz per floor (progressive) */
extern const uint16_t g_tower_enemy_hz[TOWER_FLOORS];

#endif /* TOWER_DATA_H */
