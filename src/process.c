/*==============================================================================
 * @file    process.c
 * @brief   EMG envelope tracking, baseline stats, and round averaging.
 *
 * Implements exponential envelope followers for two channels plus helpers
 * to compute baselines and per-round averages.
 *============================================================================*/

#include "process.h"
#include <math.h>

static float alpha=0.2f;
static float envA=0, envB=0;
static uint32_t fs=1000;

static float b_cnt=0, b_sumA=0, b_sumB=0, b_sumsqA=0, b_sumsqB=0;
static float r_cnt=0, r_sumA=0, r_sumB=0;

void process_init(uint32_t fs_hz, uint32_t window_ms){
  fs = fs_hz;
  float dt = 1.0f/fs_hz;
  float tau = window_ms/1000.0f;
  alpha = 1.0f - expf(-dt/tau);
  envA=envB=0;
}

void process_push(int16_t a, int16_t b){
  float ra = (float)(a<0?-a:a);
  float rb = (float)(b<0?-b:b);
  envA += alpha*(ra - envA);
  envB += alpha*(rb - envB);
}

float process_envA(void){ return envA; }
float process_envB(void){ return envB; }

void process_begin_baseline(void){
  b_cnt=0; b_sumA=b_sumB=b_sumsqA=b_sumsqB=0;
}

void process_get_baseline(float* meanA, float* sigA, float* meanB, float* sigB){
  float mA = (b_cnt>0)? b_sumA/b_cnt : 0;
  float mB = (b_cnt>0)? b_sumB/b_cnt : 0;
  float vA = (b_cnt>1)? (b_sumsqA/b_cnt - mA*mA):0;
  float vB = (b_cnt>1)? (b_sumsqB/b_cnt - mB*mB):0;
  *meanA=mA; *sigA=sqrtf(vA);
  *meanB=mB; *sigB=sqrtf(vB);
}

void process_reset_round(void){ r_cnt=r_sumA=r_sumB=0; }

void process_accumulate_round(float eA, float eB){ r_cnt++; r_sumA+=eA; r_sumB+=eB; }

void process_get_round_avg(float* avgA, float* avgB){
  *avgA = (r_cnt>0)? r_sumA/r_cnt : 0;
  *avgB = (r_cnt>0)? r_sumB/r_cnt : 0;
}

// Called each sample during baseline capture
void process_push_baseline_sample(void){
  b_cnt++;
  b_sumA += envA; b_sumB += envB;
  b_sumsqA += envA*envA; b_sumsqB += envB*envB;
}
