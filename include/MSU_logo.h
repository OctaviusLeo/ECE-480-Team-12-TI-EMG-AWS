#include <stdint.h>

/* Match the size you used in the converter (you said: same as TEAM = 120x90) */
#define MSU_LOGO_W  120
#define MSU_LOGO_H  90

/* Simple RGB565 frame buffer, row-major: MSU_LOGO[y * MSU_LOGO_W + x] */
extern const uint16_t MSU_LOGO[MSU_LOGO_W * MSU_LOGO_H];
