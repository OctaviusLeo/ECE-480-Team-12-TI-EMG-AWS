#ifndef MSU_LOGO_H
#define MSU_LOGO_H

#include <stdint.h>

#define MSU_LOGO_W 128
#define MSU_LOGO_H 126
#define MSU_LOGO_PAL_SIZE 16

extern const uint16_t MSU_LOGO_PAL[MSU_LOGO_PAL_SIZE];
extern const uint8_t  MSU_LOGO_IDX[(MSU_LOGO_W * MSU_LOGO_H) / 2];

#endif // MSU_LOGO_H
