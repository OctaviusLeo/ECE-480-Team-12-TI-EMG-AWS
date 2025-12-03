#ifndef PVP_TIE_PIC_H
#define PVP_TIE_PIC_H

#include <stdint.h>

#define PVP_TIE_PIC_W 64
#define PVP_TIE_PIC_H 45
#define PVP_TIE_PIC_PAL_SIZE 16

extern const uint16_t PVP_TIE_PIC_PAL[PVP_TIE_PIC_PAL_SIZE];
extern const uint8_t  PVP_TIE_PIC_IDX[(PVP_TIE_PIC_W * PVP_TIE_PIC_H) / 2];

#endif // PVP_TIE_PIC_H
