#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

void process_init(uint32_t fs_hz, uint32_t window_ms);
void process_push(int16_t a, int16_t b);
float process_envA(void);
float process_envB(void);
void  process_begin_baseline(void);
void  process_get_baseline(float* meanA, float* sigA, float* meanB, float* sigB);
void  process_reset_round(void);
void  process_accumulate_round(float envA, float envB);
void  process_get_round_avg(float* avgA, float* avgB);

// used during baseline capture
void  process_push_baseline_sample(void);

#endif
