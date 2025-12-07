/**
 * @file emg_mock.c
 * @brief Software EMG signal generator for testing without hardware.
 *
 * Produces two-channel pseudo-EMG using noisy sinusoids at variable
 * levels driven by a simple time-based update.
 */

#include "emg_mock.h"
#include "timer.h"
#include <math.h>

// Simple LCG for noise
static uint32_t rng=1;

/**
 * @brief Generate a random float in [0,1] using a simple LCG.
 *
 * @return Pseudo-random float in [0,1].
 */
static inline float frand(){ rng = 1664525u*rng + 1013904223u; return ((rng>>8)&0xFFFF)/65535.0f; }

/** Desired sample interval in ms (derived from fs). */
static uint32_t g_interval_ms=1, g_last_ms=0;
/** Current normalized level for channel A (0..1). */
static float g_levelA=0.05f, g_levelB=0.05f;
/** Internal phase accumulators for A/B sinusoids. */
static float phaseA=0, phaseB=0;

/**
 * @brief Initialize mock EMG generator for a given sampling rate.
 *
 * @param fs_hz Target sample rate in Hz.
 */
void emg_mock_init(uint32_t fs_hz){
  g_interval_ms = (1000u/fs_hz);
  g_last_ms = millis();
}

/**
 * @brief Set normalized output levels for channels A and B.
 *
 * Values are automatically clamped to [0,1].
 *
 * @param la Level for channel A.
 * @param lb Level for channel B.
 */
void emg_mock_set_level(float la, float lb){
  if(la<0) la=0; if(la>1) la=1; if(lb<0) lb=0; if(lb>1) lb=1;
  g_levelA = la; g_levelB = lb;
}

/**
 * @brief Read one mock EMG sample (non-blocking).
 *
 * @param[out] out Pointer to emg_sample_t to fill.
 * @return 1 if a new sample was produced, 0 if called before the next interval.
 */
int emg_mock_read(emg_sample_t* out){
  uint32_t now = millis();
  if ((now - g_last_ms) < g_interval_ms) return 0;
  g_last_ms = now;

  // base frequencies ~ 60–120 Hz + random jitter
  float fA = 60.0f + 40.0f*frand();
  float fB = 70.0f + 50.0f*frand();
  phaseA += 2*3.1415926f * fA * (g_interval_ms/1000.0f);
  phaseB += 2*3.1415926f * fB * (g_interval_ms/1000.0f);

  float nA = (frand()*2-1) * 0.5f;
  float nB = (frand()*2-1) * 0.5f;
  float sA = (sinf(phaseA) + nA) * g_levelA;
  float sB = (sinf(phaseB) + nB) * g_levelB;

  // scale to ~12-bit signed counts
  out->a = (int16_t)(sA * 1500);
  out->b = (int16_t)(sB * 1500);
  return 1;
}
