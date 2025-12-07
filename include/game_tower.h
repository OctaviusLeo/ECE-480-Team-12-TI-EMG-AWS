/**
 * @file game_tower.h
 * @brief Tower challenge mode state machine.
 *
 * Manages floor progression, tower enemies, and tower-specific UI.
 */

#ifndef GAME_TOWER_H
#define GAME_TOWER_H

#include <stdint.h>

/**
 * @brief Initialize tower mode state.
 *
 * Resets floor counters and prepares the first tower floor.
 */
void game_tower_init(void);

/**
 * @brief Advance tower mode by one frame.
 *
 * @return true when tower mode wishes to exit (e.g., finished or quit),
 *         false to continue running.
 */
bool game_tower_tick(void);

#endif /* GAME_TOWER_H */
