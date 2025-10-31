// Generates synthetic EMG-like signals for testing without hardware

#ifndef TEST_SIGNAL_GEN_H_
#define TEST_SIGNAL_GEN_H_

#include <stdint.h>

/**
 * Signal generation modes
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
 * Signal generator configuration
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
void SigGen_Init(SignalGenerator *gen, SignalType type, int32_t amplitude);
int32_t SigGen_GetNext(SignalGenerator *gen);
void SigGen_SetFrequency(SignalGenerator *gen, uint16_t freq_hz);

#endif