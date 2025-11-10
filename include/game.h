#ifndef GAME_H
#define GAME_H

#include <stdint.h>

/* Initialize OLED + game state machine (non-blocking) */
void game_init(void);

/* Feed latest metrics measured in main loop */
void game_set_metrics(float hz, uint8_t intensity_pct);
void game_set_baseline(float baseline_hz);

/* Advance the OLED/game state; call frequently in main loop */
void baseline_begin(uint32_t window_ms);
void game_tick(void);

void game_set_mode(uint8_t two_player);  // 0 = single (default), 1 = two-player

#endif
