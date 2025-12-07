/**
 * @file result1.h
 * @brief Single-player result screen state machine.
 *
 * Displays an animated RESULT screen using the player's average Hz and
 * baseline Hz, advancing frame-by-frame.
 */

#ifndef RESULT1_H
#define RESULT1_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Start the animated RESULT screen for single-player.
 *
 * @param avg_hz       Player's average Hz over the round.
 * @param baseline_hz  Baseline Hz for reference.
 */
void result1_start(float avg_hz, float baseline_hz);   // starts the animated RESULT screen

/**
 * @brief Draw one frame of the RESULT screen.
 *
 * @return true when the animation is complete and the caller may advance;
 *         false while the RESULT screen is still active.
 */
bool result1_tick(void);                               // draw a frame; returns true when done (then advance)

#endif /* RESULT1_H */
