/**
 * @file emg_processing.h
 * @brief Complete EMG signal processing pipeline.
 *
 * Features:
 * - DC offset removal
 * - High-pass filtering (remove cardiac/respiratory artifacts)
 * - Rectification
 * - Low-pass filtering (envelope detection)
 * - Baseline calibration and tracking
 * - Adaptive thresholding
 * - Muscle activation detection
 */

#ifndef EMG_PROCESSING_H_
#define EMG_PROCESSING_H_

#include <stdint.h>
#include <stdbool.h>
#include <math.h>


// CONFIGURATION PARAMETERS
/** EMG ADC sampling rate in Hz (from ADS131M04). */
#define EMG_SAMPLE_RATE         1000    // Hz (from ADS131M04)
/** Number of samples used for initial calibration (rest). */
#define EMG_CALIBRATION_SAMPLES 3000    // 3 seconds of rest data
/** Window size for rolling baseline statistics. */
#define EMG_BASELINE_WINDOW     500     // Samples for rolling baseline

// Filter parameters

/** High-pass cutoff in Hz (removes cardiac, respiration, drift). */
#define HP_FILTER_CUTOFF        20.0f   // Hz (remove <20Hz: cardiac, resp, drift)
/** Low-pass cutoff in Hz (envelope detection). */
#define LP_FILTER_CUTOFF        10.0f   // Hz (envelope detection)
/** Notch filter center frequency (e.g., power line). */
#define NOTCH_FILTER_FREQ       60.0f   // Hz (power line interference)

// Detection thresholds

/** Multiplier for activation threshold above baseline standard deviation. */
#define ACTIVATION_THRESHOLD_MULTIPLIER  3.0f  // Stddev above baseline
/** Minimum activation duration in ms to avoid chattering. */
#define MIN_ACTIVATION_DURATION          50    // ms (debounce)
/** Fraction of activation threshold used for deactivation hysteresis. */
#define HYSTERESIS_FACTOR                0.7f  // For deactivation threshold


// DATA STRUCTURES
/**
 * @brief Simple IIR high-pass filter (1st order).
 *
 * Transfer function: H(z) = (1 - z^-1) / (1 - alpha*z^-1)
 */
typedef struct {
    float alpha;           ///< Filter coefficient.
    float prev_input;      ///< Previous input sample.
    float prev_output;     ///< Previous output sample.
} HighPassFilter;

/**
 * @brief Simple IIR low-pass filter (1st order).
 *
 * Transfer function: H(z) = (1-alpha) / (1 - alpha*z^-1)
 */
typedef struct {
    float alpha;           ///< Filter coefficient.
    float prev_output;     ///< Previous output sample.
} LowPassFilter;

/**
 * @brief Notch filter for power line interference (e.g., 60 Hz).
 *
 * Simple moving-average-based implementation tuned for 1 kHz sample rate.
 */
typedef struct {
    float   buffer[17];    ///< Ring buffer for averaging.
    uint8_t index;         ///< Current index into buffer.
    float   sum;           ///< Running sum of buffer contents.
} NotchFilter;

/**
 * @brief Baseline tracker for adaptive baseline subtraction.
 *
 * Tracks mean and standard deviation over a rolling window.
 */
typedef struct {
    float    baseline_mean;               ///< Current baseline estimate (mean).
    float    baseline_stddev;             ///< Baseline standard deviation.
    int32_t  sample_buffer[EMG_BASELINE_WINDOW];
    uint16_t buffer_index;
    uint16_t sample_count;
    bool     calibrated;                  ///< true once initial calibration is done.
} BaselineTracker;

/**
 * @brief Complete EMG processor state.
 *
 * Owns filters, baseline tracker, thresholds, and summary statistics.
 */
typedef struct {
    // DC offset
    int32_t       dc_offset;           ///< Measured DC offset (ADC units).
    
    // Filters
    HighPassFilter hp_filter;
    NotchFilter    notch_filter;
    LowPassFilter  lp_filter;
    
    // Baseline tracking
    BaselineTracker baseline;
    
    // Detection
    float    activation_threshold;     ///< Adaptive activation threshold.
    bool     is_active;                ///< Current activation state.
    uint16_t activation_counter;       ///< Debounce counter in samples.
    
    // Statistics
    float    current_envelope;         ///< Current signal envelope.
    float    max_envelope;             ///< Maximum envelope seen.
    uint32_t total_samples;            ///< Total processed samples.
    
} EMGProcessor;

/**
 * @brief Calibration result snapshot.
 */
typedef struct {
    int32_t dc_offset;
    float   baseline_mean;
    float   baseline_stddev;
    bool    success;
} CalibrationResult;


// FUNCTION PROTOTYPES
// Initialization

/**
 * @brief Initialize EMGProcessor state and set initial DC offset.
 *
 * @param emg       Processor state to initialize.
 * @param dc_offset Initial DC offset estimate in ADC units.
 */
void EMG_Init(EMGProcessor *emg, int32_t dc_offset);

/**
 * @brief Initialize filter coefficients inside an EMGProcessor.
 *
 * Must be called after sample-rate-dependent constants are configured.
 *
 * @param emg Processor state whose filters will be configured.
 */
void EMG_InitFilters(EMGProcessor *emg);

// Calibration

/**
 * @brief Begin baseline calibration sequence.
 *
 * Resets counters and prepares to accumulate EMG_CALIBRATION_SAMPLES.
 *
 * @param emg Processor state.
 */
void EMG_StartCalibration(EMGProcessor *emg);

/**
 * @brief Feed one raw sample into the calibration procedure.
 *
 * @param emg        Processor state.
 * @param raw_sample Raw ADC sample (pre-processing).
 * @return true when calibration has completed, false otherwise.
 */
bool EMG_CalibrateStep(EMGProcessor *emg, int32_t raw_sample);

/**
 * @brief Get current calibration results.
 *
 * @param emg Processor state.
 * @return Snapshot of calibration parameters.
 */
CalibrationResult EMG_GetCalibrationResult(EMGProcessor *emg);

// Signal processing pipeline

/**
 * @brief Process a single raw ADC sample through the full pipeline.
 *
 * Steps:
 *  - DC offset removal
 *  - High-pass + notch + rectification
 *  - Low-pass envelope detection
 *  - Baseline update and activation detection
 *
 * @param emg     Processor state.
 * @param raw_adc Raw ADC sample.
 * @return Current envelope or processed value (implementation-defined).
 */
float EMG_ProcessSample(EMGProcessor *emg, int32_t raw_adc);

/**
 * @brief Remove DC offset from a raw sample.
 *
 * @param emg     Processor state (holds dc_offset).
 * @param raw_adc Raw ADC sample.
 * @return DC-removed value as float.
 */
float EMG_RemoveDC(EMGProcessor *emg, int32_t raw_adc);

/**
 * @brief Apply high-pass filter to a sample.
 *
 * @param hpf   High-pass filter state.
 * @param input Input sample.
 * @return High-pass filtered sample.
 */
float EMG_HighPassFilter(HighPassFilter *hpf, float input);

/**
 * @brief Apply notch filter to a sample (e.g., 60 Hz).
 *
 * @param nf    Notch filter state.
 * @param input Input sample.
 * @return Notch-filtered sample.
 */
float EMG_NotchFilter(NotchFilter *nf, float input);

/**
 * @brief Apply low-pass filter to obtain the envelope.
 *
 * @param lpf   Low-pass filter state.
 * @param input Rectified input sample.
 * @return Smoothed envelope sample.
 */
float EMG_LowPassFilter(LowPassFilter *lpf, float input);

// Baseline tracking

/**
 * @brief Update baseline statistics with a new sample.
 *
 * @param bt     Baseline tracker state.
 * @param sample New baseline sample.
 */
void EMG_UpdateBaseline(BaselineTracker *bt, float sample);

/**
 * @brief Subtract the tracked baseline from a sample.
 *
 * @param bt     Baseline tracker state.
 * @param sample Input sample.
 * @return Baseline-subtracted sample.
 */
float EMG_SubtractBaseline(BaselineTracker *bt, float sample);

// Detection

/**
 * @brief Update and check activation state based on envelope.
 *
 * @param emg      Processor state.
 * @param envelope Current envelope value.
 * @return true if muscle is considered "active", else false.
 */
bool EMG_DetectActivation(EMGProcessor *emg, float envelope);

/**
 * @brief Recompute the activation threshold based on baseline stats.
 *
 * @param emg Processor state.
 */
void EMG_UpdateThreshold(EMGProcessor *emg);

// Utilities

/**
 * @brief Compute RMS (root mean square) of integer samples.
 *
 * @param samples Pointer to array of int32 samples.
 * @param count   Number of samples.
 * @return RMS value as float.
 */
float EMG_CalculateRMS(int32_t *samples, uint16_t count);

/**
 * @brief Compute standard deviation given samples and mean.
 *
 * @param samples Pointer to array of int32 samples.
 * @param count   Number of samples.
 * @param mean    Precomputed mean of the samples.
 * @return Sample standard deviation.
 */
float EMG_CalculateStdDev(int32_t *samples, uint16_t count, float mean);

// Debug/info

/**
 * @brief Print or log status for debugging (implementation-defined).
 *
 * @param emg Processor state.
 */
void EMG_PrintStatus(EMGProcessor *emg);

#endif // EMG_PROCESSING_H_
