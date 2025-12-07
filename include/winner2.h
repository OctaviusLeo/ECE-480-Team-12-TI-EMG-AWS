/**
 * @file winner2.h
 * @brief PVP winner screen logic.
 *
 * Renders a winner screen based on average Hz for player 1 and player 2,
 * then advances per-frame until the screen's duration is complete.
 */

#ifndef WINNER2_H
#define WINNER2_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Start the winner screen sequence.
 *
 * @param p1_avg_hz Average Hz achieved by player 1.
 * @param p2_avg_hz Average Hz achieved by player 2.
 */
void winner2_start(float p1_avg_hz, float p2_avg_hz);

/**
 * @brief Draw one frame of the winner screen.
 *
 * Should be called periodically from the main loop.
 *
 * @return true once the screen's full duration has elapsed and it is
 *         safe to transition back to the caller's state machine.
 */
/* Draw one frame; returns true after the screen’s full duration elapses. */
bool winner2_tick(void);

#endif /* WINNER2_H */
