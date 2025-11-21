#ifndef LOGO_ANIM_H
#define LOGO_ANIM_H

#include <stdint.h>
#include <stdbool.h>

/* Start a TI/MSU logo toggle animation that lasts duration_ms */
void logo_anim_start(uint32_t duration_ms);

/* Tick the animation.
   Returns true when animation is finished (duration elapsed). */
bool logo_anim_tick(void);

#endif
