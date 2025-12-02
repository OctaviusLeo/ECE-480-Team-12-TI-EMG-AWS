#ifndef YOU_DIED_H
#define YOU_DIED_H

#include <stdint.h>

#define YOU_DIED_W 128
#define YOU_DIED_H 84
#define YOU_DIED_PAL_SIZE 16

extern const uint16_t YOU_DIED_PAL[YOU_DIED_PAL_SIZE];
extern const uint8_t  YOU_DIED_IDX[(YOU_DIED_W * YOU_DIED_H) / 2];

#endif // YOU_DIED_H
