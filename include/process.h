/**
 * @file process.h
 * @brief High-level EMG processing wrapper for the game.
 *
 * Wraps the lower-level EMG pipeline into a simpler API that:
 *  - initializes processing given a sample rate and window size,
 *  - ingests raw channel samples,
 *  - exposes per-channel envelopes,
 *  - manages baseline capture and round statistics.
 */

#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

/**
 * @brief Initialize EMG processing.
 *
 * @param fs_hz      Sample rate in Hz (e.g., 1000).
 * @param window_ms  Window length in milliseconds for envelope averaging.
 */
void process_init(uint32_t fs_hz, uint32_t window_ms);

/**
 * @brief Push a new pair of raw samples into the processing pipeline.
 *
 * @param a Raw sample for channel A.
 * @param b Raw sample for channel B.
 */
void process_push(int16_t a, int16_t b);

/**
 * @brief Get the current envelope value for channel A.
 *
 * @return Envelope for channel A (implementation-defined units).
 */
float process_envA(void);

/**
 * @brief Get the current envelope value for channel B.
 *
 * @return Envelope for channel B (implementation-defined units).
 */
float process_envB(void);

/**
 * @brief Begin a baseline-capture phase.
 *
 * Used to compute resting statistics for channels A/B.
 */
void  process_begin_baseline(void);

/**
 * @brief Retrieve baseline statistics after capture.
 *
 * @param[out] meanA Mean of channel A baseline.
 * @param[out] sigA  Standard deviation of channel A baseline.
 * @param[out] meanB Mean of channel B baseline.
 * @param[out] sigB  Standard deviation of channel B baseline.
 */
void  process_get_baseline(float* meanA, float* sigA, float* meanB, float* sigB);

/**
 * @brief Reset per-round accumulators.
 *
 * Call at the start of each competitive round.
 */
void  process_reset_round(void);

/**
 * @brief Accumulate per-round envelope statistics.
 *
 * @param envA Envelope value for channel A.
 * @param envB Envelope value for channel B.
 */
void  process_accumulate_round(float envA, float envB);

/**
 * @brief Get round-average envelope values.
 *
 * @param[out] avgA Average envelope for channel A over the round.
 * @param[out] avgB Average envelope for channel B over the round.
 */
void  process_get_round_avg(float* avgA, float* avgB);

/**
 * @brief Push one sample into the baseline capture buffer.
 *
 * Used during baseline capture routines.
 */
void  process_push_baseline_sample(void);

#endif /* PROCESS_H */
