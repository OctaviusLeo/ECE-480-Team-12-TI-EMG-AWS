/**
 * @file end_credits.h
 * @brief End-credits sequence controller.
 *
 * Drives a non-blocking end-credits sequence that can be advanced
 * frame-by-frame from the main loop.
 */

#ifndef END_CREDITS_H
#define END_CREDITS_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Start the end-credits sequence.
 *
 * Resets internal timers and prepares the credits to play.
 */
void end_credits_start(void);

/**
 * @brief Draw one frame of the end-credits sequence.
 *
 * Should be called from the main loop at the display refresh rate.
 *
 * @return true once the sequence is finished; false while still running.
 */
bool end_credits_tick(void);

#endif /* END_CREDITS_H */
