/**
 * @file rankhist.h
 * @brief Rank histogram tracking for single-player and two-player modes.
 *
 * Keeps running counts of how often each rank (0..8) is achieved in
 * different modes, for basic lifetime statistics or analytics.
 */

#ifndef RANKHIST_H
#define RANKHIST_H

#include <stdint.h>

/**
 * @brief Record a rank index for single-player mode.
 *
 * @param idx Rank index in [0..8], where 0 = top tier.
 */
void rankhist_add_single(int idx);                 // idx: 0..8 (0=top tier)

/**
 * @brief Retrieve accumulated single-player rank histogram.
 *
 * @param[out] out Array of 9 entries to be filled with rank counts.
 */
void rankhist_get_single(uint16_t out[9]);         // totals for single-player

/**
 * @brief Record a rank index for two-player mode.
 *
 * @param player Player index (1 or 2).
 * @param idx    Rank index in [0..8], where 0 = top tier.
 */
void rankhist_add_two(uint8_t player, int idx);    // player: 1 or 2

/**
 * @brief Retrieve accumulated two-player rank histograms.
 *
 * @param[out] out1 Array of 9 entries for player 1.
 * @param[out] out2 Array of 9 entries for player 2.
 */
void rankhist_get_two(uint16_t out1[9], uint16_t out2[9]);

#endif /* RANKHIST_H */
