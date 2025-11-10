/**
 * emg_processing.h
 * Complete EMG signal processing pipeline
 * 
 * Features:
 * - DC offset removal
 * - High-pass filtering (remove cardiac/respiratory artifacts)
 * - Rectification
 * - Low-pass filtering (envelope detection)
 * - Baseline calibration
 * - Adaptive thresholding
 * - Muscle activation detection
 */

#ifndef EMG_PROCESSING_H_
#define EMG_PROCESSING_H_

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

//============================================================================
// CONFIGURATION PARAMETERS
//============================================================================

#define EMG_SAMPLE_RATE         1000    // Hz (from ADS131M04)
#define EMG_CALIBRATION_SAMPLES 3000    // 3 seconds of rest data
#define EMG_BASELINE_WINDOW     500     // Samples for rolling baseline

// Filter parameters
#define HP_FILTER_CUTOFF        20.0f   // Hz (remove <20Hz: cardiac, resp, drift)
#define LP_FILTER_CUTOFF        10.0f   // Hz (envelope detection)
#define NOTCH_FILTER_FREQ       60.0f   // Hz (power line interference)

// Detection thresholds
#define ACTIVATION_THRESHOLD_MULTIPLIER  3.0f  // Stddev above baseline
#define MIN_ACTIVATION_DURATION          50    // ms (debounce)
#define HYSTERESIS_FACTOR                0.7f  // For deactivation threshold

//============================================================================
// DATA STRUCTURES
//============================================================================

/**
 * Simple IIR high-pass filter (1st order)
 * Transfer function: H(z) = (1 - z^-1) / (1 - alpha*z^-1)
 */
typedef struct {
    float alpha;           // Filter coefficient
    float prev_input;      // Previous input sample
    float prev_output;     // Previous output sample
} HighPassFilter;

/**
 * Simple IIR low-pass filter (1st order)
 * Transfer function: H(z) = (1-alpha) / (1 - alpha*z^-1)
 */
typedef struct {
    float alpha;           // Filter coefficient
    float prev_output;     // Previous output sample
} LowPassFilter;

/**
 * Notch filter for power line interference (60 Hz)
 * Simple implementation using moving average
 */
typedef struct {
    float buffer[17];      // For 60Hz notch at 1kHz sample rate
    uint8_t index;
    float sum;
} NotchFilter;

/**
 * Baseline tracker for adaptive baseline subtraction
 */
typedef struct {
    float baseline_mean;         // Current baseline estimate
    float baseline_stddev;       // Standard deviation
    int32_t sample_buffer[EMG_BASELINE_WINDOW];
    uint16_t buffer_index;
    uint16_t sample_count;
    bool calibrated;
} BaselineTracker;

/**
 * Complete EMG processor state
 */
typedef struct {
    // DC offset
    int32_t dc_offset;           // Measured DC offset (ADC units)
    
    // Filters
    HighPassFilter hp_filter;
    NotchFilter notch_filter;
    LowPassFilter lp_filter;
    
    // Baseline tracking
    BaselineTracker baseline;
    
    // Detection
    float activation_threshold;   // Adaptive threshold
    bool is_active;               // Current activation state
    uint16_t activation_counter;  // Debounce counter
    
    // Statistics
    float current_envelope;       // Current signal envelope
    float max_envelope;           // Maximum seen
    uint32_t total_samples;       // Total processed
    
} EMGProcessor;

/**
 * Calibration result
 */
typedef struct {
    int32_t dc_offset;
    float baseline_mean;
    float baseline_stddev;
    bool success;
} CalibrationResult;

//============================================================================
// FUNCTION PROTOTYPES
//============================================================================

// Initialization
void EMG_Init(EMGProcessor *emg, int32_t dc_offset);
void EMG_InitFilters(EMGProcessor *emg);

// Calibration
void EMG_StartCalibration(EMGProcessor *emg);
bool EMG_CalibrateStep(EMGProcessor *emg, int32_t raw_sample);
CalibrationResult EMG_GetCalibrationResult(EMGProcessor *emg);

// Signal processing pipeline
float EMG_ProcessSample(EMGProcessor *emg, int32_t raw_adc);
float EMG_RemoveDC(EMGProcessor *emg, int32_t raw_adc);
float EMG_HighPassFilter(HighPassFilter *hpf, float input);
float EMG_NotchFilter(NotchFilter *nf, float input);
float EMG_LowPassFilter(LowPassFilter *lpf, float input);

// Baseline tracking
void EMG_UpdateBaseline(BaselineTracker *bt, float sample);
float EMG_SubtractBaseline(BaselineTracker *bt, float sample);

// Detection
bool EMG_DetectActivation(EMGProcessor *emg, float envelope);
void EMG_UpdateThreshold(EMGProcessor *emg);

// Utilities
float EMG_CalculateRMS(int32_t *samples, uint16_t count);
float EMG_CalculateStdDev(int32_t *samples, uint16_t count, float mean);

// Debug/info
void EMG_PrintStatus(EMGProcessor *emg);

#endif // EMG_PROCESSING_H_