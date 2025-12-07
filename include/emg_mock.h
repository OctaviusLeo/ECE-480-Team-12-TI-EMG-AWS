/**
 * @file emg_mock.h
 * @brief Software EMG sample generator for testing without hardware.
 *
 * Provides a simple two-channel mock EMG source that can be stepped
 * frame-by-frame from the main loop or a test harness.
 */

#ifndef EMG_MOCK_H
#define EMG_MOCK_H

#include <stdint.h>

/**
 * @brief Two-channel EMG sample.
 *
 * a and b represent two independent EMG channels (e.g., two electrodes).
 */
typedef struct {
    int16_t a;  ///< Channel A sample.
    int16_t b;  ///< Channel B sample.
} emg_sample_t;

/**
 * @brief Initialize the mock EMG generator.
 *
 * @param fs_hz Sampling rate in Hz to emulate (e.g., 1000).
 */
void emg_mock_init(uint32_t fs_hz);

/**
 * @brief Set the current mock EMG levels for both channels.
 *
 * @param levelA Desired level for channel A (arbitrary units).
 * @param levelB Desired level for channel B (arbitrary units).
 */
void emg_mock_set_level(float levelA, float levelB); 

/**
 * @brief Produce one mock sample.
 *
 * @param[out] out Pointer to emg_sample_t to fill with next sample.
 * @return 0 on success, non-zero on error or end-of-stream conditions.
 */
int emg_mock_read(emg_sample_t* out); 

#endif /* EMG_MOCK_H */
