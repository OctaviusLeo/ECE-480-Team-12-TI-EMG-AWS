/**
 * @file menu.h
 * @brief Title/menu screen state machine.
 *
 * Provides a non-blocking start/tick API for the main menu, which
 * allows the user to select a game mode.
 */

#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Start the title/menu loop.
 *
 * Initializes menu state and prepares the first frame.
 */
void menu_start(void);

/**
 * @brief Draw one frame of the menu and handle input.
 *
 * @param[out] out_mode On success (return true), receives the chosen MODE_*.
 *
 * @return true when a mode was chosen and out_mode has been written;
 *         false while the menu is still active.
 */
bool menu_tick(uint8_t *out_mode);

#endif /* MENU_H */
