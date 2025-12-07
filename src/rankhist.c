/*==============================================================================
 * @file    rankhist.c
 * @brief   Rank histogram accumulation for single and two-player modes.
 *
 * Stores counts per rank band for single-player and P1/P2 in PVP.
 *============================================================================*/

#include "rankhist.h"

static uint16_t g_single[9] = {0};
static uint16_t g_p1[9]     = {0};
static uint16_t g_p2[9]     = {0};

static int clamp_idx(int idx){
  if (idx < 0) return 0;
  if (idx > 8) return 8;
  return idx;
}

void rankhist_add_single(int idx){
  g_single[clamp_idx(idx)]++;
}

void rankhist_get_single(uint16_t out[9]){
  for (int i=0;i<9;++i) out[i] = g_single[i];
}

void rankhist_add_two(uint8_t player, int idx){
  idx = clamp_idx(idx);
  if (player == 1) g_p1[idx]++; else if (player == 2) g_p2[idx]++;
}

void rankhist_get_two(uint16_t out1[9], uint16_t out2[9]){
  for (int i=0;i<9;++i){ out1[i] = g_p1[i]; out2[i] = g_p2[i]; }
}
