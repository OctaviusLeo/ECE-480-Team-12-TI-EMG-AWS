/**
 * @file intermission.h
 * @brief Short overlay between rounds (intermission) controller.
 *
 * Manages a timed overlay screen with a title, suitable for brief pauses
 * between game rounds or phases.
 */

#ifndef INTERMISSION_H
#define INTERMISSION_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Begin an intermission overlay.
 *
 * @param ms    Duration in milliseconds for the intermission.
 * @param title Title text to display on the overlay (null-terminated).
 */
void intermission_begin(uint32_t ms, const char* title);

/**
 * @brief Check whether an intermission overlay is active.
 *
 * @return true if intermission is in progress; false otherwise.
 */
bool intermission_active(void);

/**
 * @brief Draw one frame of the intermission overlay.
 *
 * Should be called regularly (e.g., every frame). Automatically clears
 * the overlay when finished.
 *
 * @return true when the intermission has finished and auto-cleared;
 *         false while still active.
 */
bool intermission_tick(void);

#endif /* INTERMISSION_H */
