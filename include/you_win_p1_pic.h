#ifndef YOU_WIN_P1_PIC_H
#define YOU_WIN_P1_PIC_H

#include <stdint.h>

#define YOU_WIN_P1_PIC_W 116
#define YOU_WIN_P1_PIC_H 128
#define YOU_WIN_P1_PIC_PAL_SIZE 16

extern const uint16_t YOU_WIN_P1_PIC_PAL[YOU_WIN_P1_PIC_PAL_SIZE];
extern const uint8_t  YOU_WIN_P1_PIC_IDX[(YOU_WIN_P1_PIC_W * YOU_WIN_P1_PIC_H) / 2];

#endif // YOU_WIN_P1_PIC_H
