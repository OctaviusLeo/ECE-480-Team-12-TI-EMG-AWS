/**
 * @file game.h
 * @brief Top-level game state machine and mode selection.
 *
 * Owns the OLED/game loop, routes ticks to the active mode
 * (Playground, PVP, Story, Tower, Credits), and exposes shared
 * metrics (Hz, intensity, baseline).
 */

#ifndef GAME_H
#define GAME_H

#include <stdint.h>

/**
 * @brief Game mode identifiers.
 */
typedef enum {
  MODE_PLAYGROUND = 0,   ///< Former single-player sandbox.
  MODE_PVP       = 1,    ///< Former two-player competitive mode.
  MODE_STORY     = 2,    ///< Story campaign.
  MODE_TOWER     = 3,    ///< Tower challenge mode.
  MODE_CREDITS   = 4     ///< End credits / roll.
} game_mode_t;

/**
 * @brief Initialize OLED and game state machine (non-blocking).
 *
 * Sets up rendering, mode state, and any shared game resources.
 */
void game_init(void);

/**
 * @brief Feed latest EMG-derived metrics from the main loop.
 *
 * @param hz            Current Hz measurement.
 * @param intensity_pct Current intensity (0–100%).
 */
void game_set_metrics(float hz, uint8_t intensity_pct);

/**
 * @brief Set the current baseline Hz.
 *
 * @param baseline_hz Baseline frequency estimate in Hz.
 */
void game_set_baseline(float baseline_hz);

/**
 * @brief Begin baseline measurement over a fixed-time window.
 *
 * @param window_ms Duration in milliseconds over which to average.
 */
void baseline_begin(uint32_t window_ms);

/**
 * @brief Advance the OLED/game state; call frequently in main loop.
 *
 * Routes control to the active mode's tick function and updates UI.
 */
void game_tick(void);

/**
 * @brief Legacy API: set mode by two-player flag.
 *
 * @param two_player 0 = single (default), 1 = two-player/PVP.
 *
 * Note: coexists with the more general game_set_mode(uint8_t mode)
 * below; both declarations are preserved for compatibility.
 */
void game_set_mode(uint8_t two_player);  // 0 = single (default), 1 = two-player

/**
 * @brief Retrieve current metrics (Hz, intensity, baseline).
 *
 * All output pointers are optional; pass NULL to skip a value.
 *
 * @param[out] hz            Pointer to receive current Hz.
 * @param[out] intensity_pct Pointer to receive current intensity.
 * @param[out] baseline_hz   Pointer to receive baseline Hz.
 */
void game_get_metrics(float *hz, uint8_t *intensity_pct, float *baseline_hz);

/**
 * @brief Set game mode by numeric mode ID.
 *
 * @param mode One of the values of game_mode_t.
 */
void game_set_mode(uint8_t mode);

#endif /* GAME_H */
