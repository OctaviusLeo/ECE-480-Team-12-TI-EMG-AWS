#ifndef RANKHIST_H
#define RANKHIST_H
#include <stdint.h>

void rankhist_add_single(int idx);                 // idx: 0..8 (0=top tier)
void rankhist_get_single(uint16_t out[9]);         // totals for single-player

void rankhist_add_two(uint8_t player, int idx);    // player: 1 or 2
void rankhist_get_two(uint16_t out1[9], uint16_t out2[9]);

#endif
