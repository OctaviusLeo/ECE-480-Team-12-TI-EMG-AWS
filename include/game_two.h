/**
 * @file game_two.h
 * @brief Two-player (PVP) game mode state machine.
 *
 * Provides a non-blocking init/tick interface for the two-player mode.
 */

#ifndef GAME_TWO_H
#define GAME_TWO_H

#include <stdint.h>

/**
 * @brief Initialize the two-player (PVP) game mode.
 *
 * Sets up any state required before the mode starts running.
 */
void game_two_init(void);

/**
 * @brief Advance the two-player game mode by one frame.
 *
 * Intended to be called from the main loop.
 *
 * @return true when the mode has finished and the caller may transition
 *         back to a higher-level menu; false while the mode is still active.
 */
bool game_two_tick(void);

#endif /* GAME_TWO_H */
