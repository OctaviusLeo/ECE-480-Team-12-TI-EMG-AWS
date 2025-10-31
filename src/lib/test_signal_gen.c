// test_signal_gen.c
#include "test_signal_gen.h"
#include <math.h>
#include <stdlib.h>

#define PI 3.14159265359f
#define SAMPLE_RATE 1000  // Match ADC sample rate

/**
 * Initialize signal generator
 */
void SigGen_Init(SignalGenerator *gen, SignalType type, int32_t amplitude) 
{
    gen->type = type;
    gen->amplitude = amplitude;
    gen->frequency_hz = 100;  // Default 100Hz (typical EMG)
    gen->dc_offset = 0;
    gen->noise_level = 10;  // 10% noise
    gen->sample_count = 0;
}

/**
 * Set signal frequency
 */
void SigGen_SetFrequency(SignalGenerator *gen, uint16_t freq_hz) 
{
    gen->frequency_hz = freq_hz;
}

/**
 * Generate next sample
 * Call at 1000 Hz to simulate real-time ADC
 */
int32_t SigGen_GetNext(SignalGenerator *gen) 
{
    int32_t signal = 0;
    float t = (float)gen->sample_count / SAMPLE_RATE;  // Time in seconds
    float phase = 2.0f * PI * gen->frequency_hz * t;
    
    // Generate base signal
    switch(gen->type) 
    {
        case SIGNAL_SINE:
            signal = (int32_t)(gen->amplitude * sinf(phase));
            break;
            
        case SIGNAL_SQUARE:
            signal = (sinf(phase) >= 0) ? gen->amplitude : -gen->amplitude;
            break;
            
        case SIGNAL_EMG_SIM:
            // Simulated EMG: High-freq oscillation with envelope
            // Carrier: 80-150 Hz, Envelope: 2-10 Hz
            {
                float carrier = sinf(phase);  // Fast oscillation
                float envelope_phase = 2.0f * PI * 5.0f * t;  // 5Hz envelope
                float envelope = 0.5f + 0.5f * sinf(envelope_phase);  // 0 to 1
                signal = (int32_t)(gen->amplitude * carrier * envelope);
            }
            break;
            
        case SIGNAL_RAMP:
            signal = (gen->sample_count % 1000) * gen->amplitude / 1000;
            break;
            
        case SIGNAL_NOISE:
            signal = 0;  // Pure noise, added below
            break;
    }
    
    // Add noise
    if(gen->noise_level > 0) 
    {
        int32_t noise_amp = (gen->amplitude * gen->noise_level) / 100;
        int32_t noise = (rand() % (2 * noise_amp)) - noise_amp;
        signal += noise;
    }
    
    // Add DC offset
    signal += gen->dc_offset;
    
    // Clamp to valid ADC range
    if(signal > 8388607) signal = 8388607;
    if(signal < -8388608) signal = -8388608;
    
    gen->sample_count++;
    return signal;
}