/**
 * @file cheevos.h
 * @brief Achievement (trophy) system for story, tower, and PVP modes.
 *
 * Binds to the save system, tracks unlocks, and renders both the full
 * achievements panel and a transient "toast" overlay on unlock.
 */

#ifndef CHEEVOS_H
#define CHEEVOS_H

#include <stdbool.h>
#include <stdint.h>
#include "save.h"

/**
 * @brief All achievement IDs used in the game.
 *
 * Enum values map directly to indices in the save's achievement bitfield.
 */
typedef enum {
    ACH_TUTORIAL = 0,
    ACH_FIRST_WIN_PVP,     ///< "You're Better"
    ACH_STORY_START,       ///< "Story Start"
    ACH_CH1,               ///< "Chapter 1"
    ACH_SCARECROW,         ///< "Scarecrow"
    ACH_CH2,               ///< "Chapter 2"
    ACH_TRAINING_DUMMY,    ///< "Training Dummy"
    ACH_CH3,               ///< "Chapter 3"
    ACH_RAT_KING,          ///< "Rat King"
    ACH_CH4,               ///< "Chapter 4"
    ACH_BANDITS,           ///< "Bandits"
    ACH_CH5,               ///< "Chapter 5"
    ACH_KNIGHT,            ///< "Knight"
    ACH_CH6,               ///< "Chapter 6"
    ACH_CHAMPION,          ///< "Champion"
    ACH_CH7,               ///< "Chapter 7"
    ACH_SORCERER,          ///< "Sorcerer"
    ACH_CH8,               ///< "Chapter 8"
    ACH_DRAGON,            ///< "Dragon"
    ACH_CH9,               ///< "Chapter 9"
    ACH_DEMON_KING,        ///< "Demon King"
    ACH_CH10,              ///< "Chapter 10"
    ACH_GAME_ADMIN,        ///< "Game Admin"
    ACH_STORY_CLEAR,       ///< "Story Cleared?"
    ACH_TOWER_START,       ///< "Tower Start"
    ACH_TOWER_5,           ///< "Tower 5"
    ACH_TOWER_10,          ///< "Tower 10"
    ACH_TOWER_15,          ///< "Tower 15"
    ACH_TOWER_20,          ///< "Tower 20"
    ACH_TOWER_24,          ///< "Tower 24"
    ACH_TOWER_CLEAR        ///< "Tower Clear"
} cheevo_t;

/**
 * @brief Bind the achievements system to a loaded save slot.
 *
 * Must be called once after a save file has been loaded.
 *
 * @param s Pointer to the active save object.
 */
void cheevos_bind_save(save_t *s);          // pass loaded save here

/**
 * @brief Unlock an achievement by ID.
 *
 * @param id Achievement to unlock.
 * @return true if this call caused a *new* unlock, false if it was
 *         already unlocked before.
 */
bool cheevos_unlock(cheevo_t id);           // returns true if newly unlocked

/**
 * @brief Draw the full achievements panel.
 *
 * Typically used in a dedicated "Achievements" screen.
 */
void cheevos_draw_panel(void);              // simple list renderer

/**
 * @brief Draw a single page of the achievements panel.
 *
 * @param page Zero-based page index for paginated panels.
 */
void cheevos_draw_panel_page(uint8_t page);

/**
 * @brief Draw a transient "Achievement Unlocked!" toast overlay.
 *
 * Safe to call every frame from the main render loop. Does nothing if
 * no recent unlock is pending display.
 */
void cheevos_draw_toast(void);

#endif /* CHEEVOS_H */
