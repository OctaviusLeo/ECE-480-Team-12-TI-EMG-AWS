#ifndef CHEST_H
#define CHEST_H

#include <stdint.h>

#define CHEST_W 128
#define CHEST_H 85
#define CHEST_PAL_SIZE 16

extern const uint16_t CHEST_PAL[CHEST_PAL_SIZE];
extern const uint8_t  CHEST_IDX[(CHEST_W * CHEST_H) / 2];

#endif // CHEST_H
