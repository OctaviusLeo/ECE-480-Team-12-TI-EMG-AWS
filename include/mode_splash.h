/**
 * @file mode_splash.h
 * @brief Short "mode chosen" splash overlay.
 *
 * Displays a brief confirmation overlay after the user selects a mode
 * (e.g., Story, Tower, PVP).
 */

#ifndef MODE_SPLASH_H
#define MODE_SPLASH_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Begin a short "mode chosen" overlay.
 *
 * @param selected_mode Mode identifier that was chosen (e.g., MODE_STORY).
 */
void mode_splash_begin(uint8_t selected_mode);

/**
 * @brief Check whether the mode splash overlay is currently active.
 *
 * @return true if the overlay is active; false otherwise.
 */
bool mode_splash_active(void);

/**
 * @brief Draw one frame of the mode splash overlay.
 *
 * @return true when the overlay has finished and the caller may continue;
 *         false while the overlay is still running.
 */
bool mode_splash_tick(void);

#endif /* MODE_SPLASH_H */
