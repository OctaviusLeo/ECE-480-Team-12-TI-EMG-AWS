/**
 * @file signal_processing.h
 * @brief Basic signal-processing utilities: moving-average filter, rectification,
 *        clamping, ADC-to-voltage conversion, and simple statistics.
 */

#ifndef SIGNAL_PROCESSING_H_
#define SIGNAL_PROCESSING_H_

#include <stdint.h>
#include <stdbool.h>

//============================================================================
// Configuration Constants
//============================================================================

/** ADC samples per second. */
#define SAMPLING_RATE_HZ        1000    // ADC samples per second

/** Moving-average window length in milliseconds. */
#define FILTER_WINDOW_MS        100     // Moving average window

/** Moving-average window size in samples. */
#define FILTER_SIZE             (FILTER_WINDOW_MS * SAMPLING_RATE_HZ / 1000)

/** Decimation factor for stored samples (store every Nth). */
#define DECIMATION_FACTOR       10      // Store every Nth sample

//============================================================================
// DATA STRUCTURES
//============================================================================

/**
 * @brief Moving Average Filter.
 *
 * Implements O(1) smoothing using a running sum over a fixed-size window.
 */
typedef struct 
{
    int32_t  buffer[FILTER_SIZE];   ///< Circular sample buffer.
    uint16_t index;                 ///< Current write position.
    int64_t  sum;                   ///< Running sum (prevents overflow).
    bool     filled;                ///< True once buffer wraps at least once.
} MovingAverageFilter;

/**
 * @brief Signal statistics for quality assessment.
 *
 * Tracks basic metrics over a stream of samples.
 */
typedef struct 
{
    int32_t  mean;            ///< Average amplitude.
    int32_t  std_dev;         ///< Standard deviation.
    int32_t  min;             ///< Minimum value seen.
    int32_t  max;             ///< Maximum value seen.
    uint16_t zero_crossings;  ///< Number of zero crossings.
} SignalStats;

//============================================================================
// FUNCTION PROTOTYPES
//============================================================================

// Filter operations

/**
 * @brief Initialize a moving-average filter to default state.
 *
 * @param filter Pointer to filter instance.
 */
void     Filter_Init(MovingAverageFilter *filter);

/**
 * @brief Push a new sample through the moving-average filter.
 *
 * @param filter     Pointer to filter instance.
 * @param new_sample New input sample.
 * @return Filtered (smoothed) sample.
 */
int32_t  Filter_Update(MovingAverageFilter *filter, int32_t new_sample);

/**
 * @brief Reset a moving-average filter (clears buffer and sum).
 *
 * @param filter Pointer to filter instance.
 */
void     Filter_Reset(MovingAverageFilter *filter);

// Signal processing

/**
 * @brief Full-wave rectify a signal sample.
 *
 * @param signal Input value.
 * @return Absolute value of signal.
 */
int32_t  SignalProc_Rectify(int32_t signal);

/**
 * @brief Clamp a value to the inclusive range [min, max].
 *
 * @param value Input value.
 * @param min   Lower bound.
 * @param max   Upper bound.
 * @return Clamped value.
 */
int32_t  SignalProc_Clamp(int32_t value, int32_t min, int32_t max);

/**
 * @brief Convert ADC code to voltage.
 *
 * @param adc_value Raw ADC reading.
 * @param vref      Reference/full-scale voltage in volts.
 * @return Voltage in volts.
 */
float    SignalProc_ADCToVoltage(int32_t adc_value, float vref);

// Statistics

/**
 * @brief Initialize a SignalStats structure.
 *
 * @param stats Pointer to stats instance.
 */
void      Stats_Init(SignalStats *stats);

/**
 * @brief Update statistics with a new sample.
 *
 * @param stats  Pointer to stats instance.
 * @param sample New sample to incorporate.
 */
void      Stats_Update(SignalStats *stats, int32_t sample);

/**
 * @brief Calculate statistics from a buffer of samples.
 *
 * @param data   Pointer to sample array.
 * @param length Number of samples.
 * @return Filled SignalStats structure.
 */
SignalStats Stats_Calculate(const int32_t *data, uint16_t length);

#endif /* SIGNAL_PROCESSING_H_ */
