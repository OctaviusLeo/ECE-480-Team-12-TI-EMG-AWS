#ifndef END_CREDITS_H
#define END_CREDITS_H
#include <stdbool.h>
#include <stdint.h>

/* Start the end-credits sequence. */
void end_credits_start(void);

/* Draw a frame. Returns true when the sequence is finished. */
bool end_credits_tick(void);

#endif
