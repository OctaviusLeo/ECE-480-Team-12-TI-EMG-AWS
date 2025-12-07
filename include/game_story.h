/**
 * @file game_story.h
 * @brief Story mode state machine.
 *
 * Handles chapter progression, enemy images, and story-specific UI.
 */

#ifndef GAME_STORY_H
#define GAME_STORY_H

#include <stdint.h>

/**
 * @brief Initialize story mode state.
 *
 * Prepares the first chapter, assets, and progression data.
 */
void game_story_init(void);

/**
 * @brief Advance story mode by one frame.
 *
 * @return true when story mode wishes to exit (e.g., back to menu),
 *         false to keep running the current story flow.
 */
bool game_story_tick(void);

#endif /* GAME_STORY_H */
