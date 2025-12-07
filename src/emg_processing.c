/**
 * @file emg_processing.c
 * @brief Implementation of complete EMG processing pipeline.
 *
 * Pipeline:
 *  - DC offset removal
 *  - High-pass filtering
 *  - 60 Hz notch filtering
 *  - Rectification
 *  - Low-pass envelope detection
 *  - Baseline subtraction and tracking
 *  - Activation detection with hysteresis and debouncing
 */

/**
 * emg_processing.c
 * Implementation of complete EMG processing pipeline
 */

#include "emg_processing.h"
#include <string.h>
#include <math.h>

// FILTER COEFFICIENT CALCULATIONS

/**
 * Calculate alpha for high-pass filter
 * alpha = RC / (RC + dt)
 * where RC = 1 / (2*pi*fc)
 */
static float Calculate_HP_Alpha(float cutoff_freq, float sample_rate) {
    float RC = 1.0f / (2.0f * 3.14159f * cutoff_freq);
    float dt = 1.0f / sample_rate;
    return RC / (RC + dt);
}

/**
 * Calculate alpha for low-pass filter
 * alpha = dt / (RC + dt)
 * where RC = 1 / (2*pi*fc)
 */
static float Calculate_LP_Alpha(float cutoff_freq, float sample_rate) {
    float RC = 1.0f / (2.0f * 3.14159f * cutoff_freq);
    float dt = 1.0f / sample_rate;
    return dt / (RC + dt);
}


// INITIALIZATION
/**
 * Initialize EMG processor
 */
void EMG_Init(EMGProcessor *emg, int32_t dc_offset) {
    memset(emg, 0, sizeof(EMGProcessor));
    
    emg->dc_offset = dc_offset;
    
    EMG_InitFilters(emg);
    
    // Initialize baseline tracker
    emg->baseline.calibrated = false;
    emg->baseline.baseline_mean = 0.0f;
    emg->baseline.baseline_stddev = 0.0f;
    
    // Initialize detection
    emg->activation_threshold = 0.0f;
    emg->is_active = false;
    emg->activation_counter = 0;
}

/**
 * Initialize all filters
 */
void EMG_InitFilters(EMGProcessor *emg) {
    // High-pass filter (20 Hz cutoff)
    emg->hp_filter.alpha = Calculate_HP_Alpha(HP_FILTER_CUTOFF, EMG_SAMPLE_RATE);
    emg->hp_filter.prev_input = 0.0f;
    emg->hp_filter.prev_output = 0.0f;
    
    // Low-pass filter (10 Hz cutoff for envelope)
    emg->lp_filter.alpha = Calculate_LP_Alpha(LP_FILTER_CUTOFF, EMG_SAMPLE_RATE);
    emg->lp_filter.prev_output = 0.0f;
    
    // Notch filter (60 Hz power line)
    memset(&emg->notch_filter, 0, sizeof(NotchFilter));
}

// CALIBRATION
/**
 * Start calibration process
 */
void EMG_StartCalibration(EMGProcessor *emg) {
    emg->baseline.buffer_index = 0;
    emg->baseline.sample_count = 0;
    emg->baseline.calibrated = false;
}

/**
 * Process one sample during calibration
 * Returns true when calibration complete
 */
bool EMG_CalibrateStep(EMGProcessor *emg, int32_t raw_sample) {
    // Process through initial pipeline (DC removal, filters)
    float dc_removed = EMG_RemoveDC(emg, raw_sample);
    float hp_filtered = EMG_HighPassFilter(&emg->hp_filter, dc_removed);
    float notch_filtered = EMG_NotchFilter(&emg->notch_filter, hp_filtered);
    
    // Store for baseline calculation
    uint16_t idx = emg->baseline.sample_count % EMG_BASELINE_WINDOW;
    emg->baseline.sample_buffer[idx] = (int32_t)(notch_filtered * 1000.0f);
    emg->baseline.sample_count++;
    
    // Check if calibration complete
    if(emg->baseline.sample_count >= EMG_CALIBRATION_SAMPLES) {
        // Calculate baseline statistics
        int32_t sum = 0;
        for(int i = 0; i < EMG_BASELINE_WINDOW; i++) {
            sum += emg->baseline.sample_buffer[i];
        }
        emg->baseline.baseline_mean = (float)sum / (EMG_BASELINE_WINDOW * 1000.0f);
        
        // Calculate standard deviation
        emg->baseline.baseline_stddev = EMG_CalculateStdDev(
            emg->baseline.sample_buffer, 
            EMG_BASELINE_WINDOW, 
            emg->baseline.baseline_mean * 1000.0f
        ) / 1000.0f;
        
        // Set adaptive threshold
        emg->activation_threshold = emg->baseline.baseline_mean + 
                                   (ACTIVATION_THRESHOLD_MULTIPLIER * 
                                    emg->baseline.baseline_stddev);
        
        emg->baseline.calibrated = true;
        return true;
    }
    
    return false;
}

/**
 * Get calibration results
 */
CalibrationResult EMG_GetCalibrationResult(EMGProcessor *emg) {
    CalibrationResult result;
    result.dc_offset = emg->dc_offset;
    result.baseline_mean = emg->baseline.baseline_mean;
    result.baseline_stddev = emg->baseline.baseline_stddev;
    result.success = emg->baseline.calibrated;
    return result;
}

// SIGNAL PROCESSING PIPELINE
/**
 * Complete signal processing pipeline
 * Returns processed envelope value
 */
float EMG_ProcessSample(EMGProcessor *emg, int32_t raw_adc) {
    emg->total_samples++;
    
    // Step 1: Remove DC offset (2.5V bias)
    float dc_removed = EMG_RemoveDC(emg, raw_adc);
    
    // Step 2: High-pass filter (remove <20Hz: cardiac, respiratory)
    float hp_filtered = EMG_HighPassFilter(&emg->hp_filter, dc_removed);
    
    // Step 3: Notch filter (remove 60Hz power line)
    float notch_filtered = EMG_NotchFilter(&emg->notch_filter, hp_filtered);
    
    // Step 4: Rectification (full-wave)
    float rectified = fabsf(notch_filtered);
    
    // Step 5: Low-pass filter (envelope detection)
    float envelope = EMG_LowPassFilter(&emg->lp_filter, rectified);
    
    // Step 6: Baseline subtraction
    float baseline_corrected = EMG_SubtractBaseline(&emg->baseline, envelope);
    
    // Update envelope stats
    emg->current_envelope = baseline_corrected;
    if(baseline_corrected > emg->max_envelope) {
        emg->max_envelope = baseline_corrected;
    }
    
    // Step 7: Activation detection
    emg->is_active = EMG_DetectActivation(emg, baseline_corrected);
    
    return baseline_corrected;
}

/**
 * Remove DC offset
 */
float EMG_RemoveDC(EMGProcessor *emg, int32_t raw_adc) {
    return (float)(raw_adc - emg->dc_offset);
}

/**
 * High-pass filter implementation (1st order IIR)
 * Removes low-frequency artifacts (<20 Hz)
 */
float EMG_HighPassFilter(HighPassFilter *hpf, float input) {
    float output = hpf->alpha * (hpf->prev_output + input - hpf->prev_input);
    
    hpf->prev_input = input;
    hpf->prev_output = output;
    
    return output;
}

/**
 * Notch filter for 60 Hz power line interference
 * Uses moving average over one period (16.67ms = ~17 samples at 1kHz)
 */
float EMG_NotchFilter(NotchFilter *nf, float input) {
    // Remove oldest sample from sum
    nf->sum -= nf->buffer[nf->index];
    
    // Add new sample
    nf->buffer[nf->index] = input;
    nf->sum += input;
    
    // Advance index (circular buffer)
    nf->index = (nf->index + 1) % 17;
    
    // Return input minus moving average (removes 60Hz component)
    return input - (nf->sum / 17.0f);
}

/**
 * Low-pass filter for envelope detection (1st order IIR)
 */
float EMG_LowPassFilter(LowPassFilter *lpf, float input) {
    float output = lpf->alpha * input + (1.0f - lpf->alpha) * lpf->prev_output;
    lpf->prev_output = output;
    return output;
}

// BASELINE TRACKING
/**
 * Update rolling baseline estimate
 */
void EMG_UpdateBaseline(BaselineTracker *bt, float sample) {
    if(!bt->calibrated) return;
    
    // Update rolling buffer
    bt->sample_buffer[bt->buffer_index] = (int32_t)(sample * 1000.0f);
    bt->buffer_index = (bt->buffer_index + 1) % EMG_BASELINE_WINDOW;
    
    // Recalculate mean periodically (every 100 samples)
    if(bt->buffer_index % 100 == 0) {
        int32_t sum = 0;
        for(int i = 0; i < EMG_BASELINE_WINDOW; i++) {
            sum += bt->sample_buffer[i];
        }
        bt->baseline_mean = (float)sum / (EMG_BASELINE_WINDOW * 1000.0f);
    }
}

/**
 * Subtract baseline from signal
 */
float EMG_SubtractBaseline(BaselineTracker *bt, float sample) {
    if(!bt->calibrated) return sample;
    
    float corrected = sample - bt->baseline_mean;
    
    // Update baseline tracker
    EMG_UpdateBaseline(bt, sample);
    
    return (corrected > 0.0f) ? corrected : 0.0f;
}

// ACTIVATION DETECTION
/**
 * Detect muscle activation with hysteresis and debouncing
 */
bool EMG_DetectActivation(EMGProcessor *emg, float envelope) {
    if(!emg->baseline.calibrated) return false;
    
    // Calculate thresholds with hysteresis
    float activate_threshold = emg->activation_threshold;
    float deactivate_threshold = emg->activation_threshold * HYSTERESIS_FACTOR;
    
    // State machine with debouncing
    if(!emg->is_active) {
        // Currently inactive - check for activation
        if(envelope > activate_threshold) {
            emg->activation_counter++;
            if(emg->activation_counter >= (MIN_ACTIVATION_DURATION * EMG_SAMPLE_RATE / 1000)) {
                emg->activation_counter = 0;
                return true;  // Activate!
            }
        } else {
            emg->activation_counter = 0;
        }
    } else {
        // Currently active - check for deactivation
        if(envelope < deactivate_threshold) {
            emg->activation_counter++;
            if(emg->activation_counter >= (MIN_ACTIVATION_DURATION * EMG_SAMPLE_RATE / 1000)) {
                emg->activation_counter = 0;
                return false;  // Deactivate
            }
        } else {
            emg->activation_counter = 0;
        }
    }
    
    return emg->is_active;  // Maintain current state
}

/**
 * Update adaptive threshold based on recent data
 */
void EMG_UpdateThreshold(EMGProcessor *emg) {
    if(!emg->baseline.calibrated) return;
    
    // Recalculate threshold based on current baseline stddev
    float stddev = EMG_CalculateStdDev(
        emg->baseline.sample_buffer,
        EMG_BASELINE_WINDOW,
        emg->baseline.baseline_mean * 1000.0f
    ) / 1000.0f;
    
    emg->activation_threshold = emg->baseline.baseline_mean + 
                               (ACTIVATION_THRESHOLD_MULTIPLIER * stddev);
}

// UTILITY FUNCTIONS
/**
 * Calculate RMS of signal
 */
float EMG_CalculateRMS(int32_t *samples, uint16_t count) {
    int64_t sum_squares = 0;
    for(uint16_t i = 0; i < count; i++) {
        int64_t val = samples[i];
        sum_squares += (val * val);
    }
    return sqrtf((float)sum_squares / count);
}

/**
 * Calculate standard deviation
 */
float EMG_CalculateStdDev(int32_t *samples, uint16_t count, float mean) {
    float sum_sq_diff = 0.0f;
    for(uint16_t i = 0; i < count; i++) {
        float diff = samples[i] - mean;
        sum_sq_diff += (diff * diff);
    }
    return sqrtf(sum_sq_diff / count);
}

/**
 * Print processor status for debugging
 */
void EMG_PrintStatus(EMGProcessor *emg) {
    // This will be implemented in main.c using UARTprintf
    // Just a placeholder here
    (void)emg;
}
