#ifndef WINNER2_H
#define WINNER2_H
#include <stdint.h>
#include <stdbool.h>

void winner2_start(float p1_avg_hz, float p2_avg_hz);
/* Draw one frame; returns true after the screen’s full duration elapses. */
bool winner2_tick(void);

#endif
