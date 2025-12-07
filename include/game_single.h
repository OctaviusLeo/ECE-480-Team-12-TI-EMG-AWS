/**
 * @file game_single.h
 * @brief Playground (single-player) mode state machine.
 *
 * Formerly known as "single player"; provides a non-blocking
 * init/tick interface for the playground experience.
 */

#ifndef GAME_SINGLE_H
#define GAME_SINGLE_H

#include <stdint.h>

/**
 * @brief Initialize the single-player playground mode.
 *
 * Sets up any required assets and internal state.
 */
void game_single_init(void);

/**
 * @brief Advance the single-player mode by one frame.
 *
 * @return true when the mode is finished and control should return
 *         to the caller (e.g., main menu); false to keep running.
 */
bool game_single_tick(void);

#endif /* GAME_SINGLE_H */
