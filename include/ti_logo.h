#ifndef TI_LOGO_H
#define TI_LOGO_H

#include <stdint.h>

#define TI_LOGO_W 128
#define TI_LOGO_H 72
#define TI_LOGO_PAL_SIZE 16

extern const uint16_t TI_LOGO_PAL[TI_LOGO_PAL_SIZE];
extern const uint8_t  TI_LOGO_IDX[(TI_LOGO_W * TI_LOGO_H) / 2];

#endif // TI_LOGO_H
