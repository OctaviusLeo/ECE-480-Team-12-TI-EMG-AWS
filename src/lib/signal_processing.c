// signal_processing.c
#include "signal_processing.h"
#include <stdlib.h>
#include <math.h>

//============================================================================
// MOVING AVERAGE FILTER IMPLEMENTATION
//============================================================================

/**
 * Initialize filter structure
 * Call once at startup
 */
void Filter_Init(MovingAverageFilter *filter) 
{
    // Zero out buffer
    for(uint16_t i = 0; i < FILTER_SIZE; i++) 
    {
        filter->buffer[i] = 0;
    }
    
    filter->index = 0;
    filter->sum = 0;
    filter->filled = false;
}

/**
 * Add new sample and get filtered output
 * 
 * @param filter: Pointer to filter structure
 * @param new_sample: Raw input sample
 * @return: Filtered output (average of last N samples)
 */
int32_t Filter_Update(MovingAverageFilter *filter, int32_t new_sample) 
{
    // Subtract oldest sample from running sum
    filter->sum -= filter->buffer[filter->index];
    
    // Store new sample
    filter->buffer[filter->index] = new_sample;
    
    // Add new sample to running sum
    filter->sum += new_sample;
    
    // Advance index (circular)
    filter->index++;
    if(filter->index >= FILTER_SIZE) 
    {
        filter->index = 0;
        filter->filled = true;
    }
    
    // Calculate and return average
    if(filter->filled) 
    {
        // Buffer full, divide by actual size
        return (int32_t)(filter->sum / FILTER_SIZE);
    } 
    else 
    {
        // Buffer filling, divide by current count
        // Avoids bias from zero-initialized values
        return (int32_t)(filter->sum / (filter->index));
    }
}

/**
 * Reset filter to initial state
 * Use when starting new measurement
 */
void Filter_Reset(MovingAverageFilter *filter) 
{
    Filter_Init(filter);  // Reuse initialization
}

//============================================================================
// SIGNAL PROCESSING FUNCTIONS
//============================================================================

/**
 * Full-wave rectification
 * Converts bipolar signal to unipolar (all positive)
 * 
 * WHY: EMG is AC signal, we want amplitude envelope
 * 
 * @param signal: Raw signal (can be negative)
 * @return: Absolute value
 */
int32_t SignalProc_Rectify(int32_t signal) 
{
    return (signal < 0) ? -signal : signal;
}

/**
 * Clamp value to range
 * Prevents overflow, enforces valid ranges
 * 
 * @param value: Input value
 * @param min: Minimum allowed
 * @param max: Maximum allowed
 * @return: Clamped value
 */
int32_t SignalProc_Clamp(int32_t value, int32_t min, int32_t max) 
{
    if(value < min) return min;
    if(value > max) return max;
    return value;
}

/**
 * Convert 24-bit ADC value to voltage
 * 
 * @param adc_value: Raw ADC reading (-8388608 to +8388607)
 * @param vref: ADC reference voltage (typically 2.5V)
 * @return: Voltage in volts
 * 
 * FORMULA: V = (ADC / 2^23) × Vref
 * Divide by 2^23 not 2^24 because signed (23 bits magnitude)
 */
float SignalProc_ADCToVoltage(int32_t adc_value, float vref) 
{
    const float ADC_MAX = 8388607.0f;  // 2^23 - 1
    return ((float)adc_value / ADC_MAX) * vref;
}

//============================================================================
// STATISTICS FUNCTIONS
//============================================================================

/**
 * Initialize statistics structure
 */
void Stats_Init(SignalStats *stats) 
{
    stats->mean = 0;
    stats->std_dev = 0;
    stats->min = INT32_MAX;
    stats->max = INT32_MIN;
    stats->zero_crossings = 0;
}

/**
 * Update statistics with new sample (online algorithm)
 * Suitable for real-time streaming data
 */
void Stats_Update(SignalStats *stats, int32_t sample) 
{
    // Update min/max
    if(sample < stats->min) stats->min = sample;
    if(sample > stats->max) stats->max = sample;
    
    // Note: Mean/std_dev require full pass, use Stats_Calculate()
}

/**
 * Calculate statistics from data array
 * Use for batch analysis at end of trial
 * 
 * @param data: Array of samples
 * @param length: Number of samples
 * @return: Computed statistics
 */
SignalStats Stats_Calculate(const int32_t *data, uint16_t length) 
{
    SignalStats stats;
    Stats_Init(&stats);
    
    if(length == 0) return stats;
    
    // First pass: mean, min, max
    int64_t sum = 0;
    stats.min = data[0];
    stats.max = data[0];
    
    for(uint16_t i = 0; i < length; i++) 
    {
        sum += data[i];
        if(data[i] < stats.min) stats.min = data[i];
        if(data[i] > stats.max) stats.max = data[i];
    }
    
    stats.mean = (int32_t)(sum / length);
    
    // Second pass: standard deviation
    int64_t variance_sum = 0;
    for(uint16_t i = 0; i < length; i++) 
    {
        int32_t diff = data[i] - stats.mean;
        variance_sum += (int64_t)diff * diff;
    }
    
    float variance = (float)variance_sum / length;
    stats.std_dev = (int32_t)sqrtf(variance);
    
    // Count zero crossings
    stats.zero_crossings = 0;
    for(uint16_t i = 1; i < length; i++) 
    {
        if((data[i-1] < 0 && data[i] >= 0) || (data[i-1] >= 0 && data[i] < 0)) 
        {
            stats.zero_crossings++;
        }
    }
    
    return stats;
}