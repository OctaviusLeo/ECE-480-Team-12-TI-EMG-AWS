#include "tower_data.h"
/* Simple ramp ~8 Hz per floor, cap around 200 Hz; adjust as you balance */
const uint16_t g_tower_enemy_hz[TOWER_FLOORS] = {
   8, 16, 24, 32, 40,
  48, 56, 64, 72, 80,
  88, 96,104,112,120,
 128,136,144,152,160,
 168,176,184,192,200
};

//   8, 16, 24, 32, 40,
//  48, 56, 64, 72, 80,
//  88, 96,104,112,120,
// 128,136,144,152,160,
// 168,176,184,192,200