#ifndef SIGNAL_PROCESSING_H_
#define SIGNAL_PROCESSING_H_

#include <stdint.h>
#include <stdbool.h>

// Configuration Constants
#define SAMPLING_RATE_HZ        1000    // ADC samples per second
#define FILTER_WINDOW_MS        100     // Moving average window
#define FILTER_SIZE             (FILTER_WINDOW_MS * SAMPLING_RATE_HZ / 1000)
#define DECIMATION_FACTOR       10      // Store every Nth sample

// DATA STRUCTURES
/**
 * Moving Average Filter
 * Implements O(1) smoothing using running sum
 */
typedef struct 
{
    int32_t buffer[FILTER_SIZE];    // Circular sample buffer
    uint16_t index;                  // Current write position
    int64_t sum;                     // Running sum (prevent overflow)
    bool filled;                     // Has buffer wrapped once?
} MovingAverageFilter;

/**
 * Signal Statistics
 * Computed metrics for quality assessment
 */
typedef struct 
{
    int32_t mean;           // Average amplitude
    int32_t std_dev;        // Standard deviation
    int32_t min;            // Minimum value seen
    int32_t max;            // Maximum value seen
    uint16_t zero_crossings;// How many times signal crosses zero
} SignalStats;

// FUNCTION PROTOTYPES
// Filter operations
void Filter_Init(MovingAverageFilter *filter);
int32_t Filter_Update(MovingAverageFilter *filter, int32_t new_sample);
void Filter_Reset(MovingAverageFilter *filter);

// Signal processing
int32_t SignalProc_Rectify(int32_t signal);
int32_t SignalProc_Clamp(int32_t value, int32_t min, int32_t max);
float SignalProc_ADCToVoltage(int32_t adc_value, float vref);

// Statistics
void Stats_Init(SignalStats *stats);
void Stats_Update(SignalStats *stats, int32_t sample);
SignalStats Stats_Calculate(const int32_t *data, uint16_t length);

#endif 