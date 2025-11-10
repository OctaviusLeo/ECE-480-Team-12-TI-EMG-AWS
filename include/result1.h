#ifndef RESULT1_H
#define RESULT1_H
#include <stdbool.h>
#include <stdint.h>

void result1_start(float avg_hz, float baseline_hz);   // starts the animated RESULT screen
bool result1_tick(void);                               // draw a frame; returns true when done (then advance)

#endif
