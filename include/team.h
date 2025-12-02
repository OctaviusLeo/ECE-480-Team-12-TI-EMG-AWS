#ifndef TEAM_H
#define TEAM_H

#include <stdint.h>

#define TEAM_W 128
#define TEAM_H 96
#define TEAM_PAL_SIZE 16

extern const uint16_t TEAM_PAL[TEAM_PAL_SIZE];
extern const uint8_t  TEAM_IDX[(TEAM_W * TEAM_H) / 2];

#endif // TEAM_H
