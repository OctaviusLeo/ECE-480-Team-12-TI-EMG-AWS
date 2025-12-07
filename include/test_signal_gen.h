/**
 * @file test_signal_gen.h
 * @brief Synthetic EMG-like signal generator for testing without hardware.
 *
 * Provides an abstract "SignalGenerator" object that can produce samples
 * of different test waveforms (sine, square, EMG-like noise, ramp, noise).
 */

// Generates synthetic EMG-like signals for testing without hardware

#ifndef TEST_SIGNAL_GEN_H_
#define TEST_SIGNAL_GEN_H_

#include <stdint.h>

/**
 * @brief Signal generation modes.
 */
typedef enum 
{
    SIGNAL_SINE,        // Pure sine wave
    SIGNAL_SQUARE,      // Square wave
    SIGNAL_EMG_SIM,     // Simulated EMG with noise
    SIGNAL_RAMP,        // Linear ramp
    SIGNAL_NOISE        // Random noise only
} SignalType;

/**
 * @brief Signal generator configuration/state.
 *
 * type         - Waveform type.
 * amplitude    - Peak amplitude in ADC units.
 * frequency_hz - Waveform frequency in Hz.
 * dc_offset    - DC bias in ADC units.
 * noise_level  - Noise amplitude as a percentage (0–100).
 * sample_count - Internal sample counter.
 */
typedef struct 
{
    SignalType type;
    int32_t amplitude;      // Peak amplitude (ADC units)
    uint16_t frequency_hz;  // Signal frequency
    int32_t dc_offset;      // DC bias
    uint16_t noise_level;   // Noise amplitude (0-100%)
    uint32_t sample_count;  // Internal counter
} SignalGenerator;

// Function prototypes

/**
 * @brief Initialize a signal generator instance.
 *
 * @param gen      Pointer to generator state object.
 * @param type     Initial waveform type.
 * @param amplitude Peak amplitude in ADC units.
 */
void SigGen_Init(SignalGenerator *gen, SignalType type, int32_t amplitude);

/**
 * @brief Get the next sample from the generator.
 *
 * Advances the internal sample counter and returns the next waveform value.
 *
 * @param gen Pointer to initialized generator.
 * @return Next sample value in ADC units.
 */
int32_t SigGen_GetNext(SignalGenerator *gen);

/**
 * @brief Set the generator's output frequency.
 *
 * @param gen     Pointer to initialized generator.
 * @param freq_hz New frequency in Hz.
 */
void SigGen_SetFrequency(SignalGenerator *gen, uint16_t freq_hz);

#endif
