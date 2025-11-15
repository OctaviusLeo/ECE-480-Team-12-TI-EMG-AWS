#ifndef MODE_SPLASH_H
#define MODE_SPLASH_H
#include <stdbool.h>
#include <stdint.h>

/* Begin a short “mode chosen” overlay. */
void mode_splash_begin(uint8_t selected_mode);

/* Is the overlay currently active? */
bool mode_splash_active(void);

/* Draw one frame. Returns true when the overlay has finished. */
bool mode_splash_tick(void);

#endif
