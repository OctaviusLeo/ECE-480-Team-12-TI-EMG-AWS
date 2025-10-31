#include "emg_mock.h"
#include "timer.h"
#include <math.h>

// Simple LCG for noise
static uint32_t rng=1;
static inline float frand(){ rng = 1664525u*rng + 1013904223u; return ((rng>>8)&0xFFFF)/65535.0f; }

static uint32_t g_interval_ms=1, g_last_ms=0;
static float g_levelA=0.05f, g_levelB=0.05f;
static float phaseA=0, phaseB=0;

void emg_mock_init(uint32_t fs_hz){
  g_interval_ms = (1000u/fs_hz);
  g_last_ms = millis();
}

void emg_mock_set_level(float la, float lb){
  if(la<0) la=0; if(la>1) la=1; if(lb<0) lb=0; if(lb>1) lb=1;
  g_levelA = la; g_levelB = lb;
}

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
