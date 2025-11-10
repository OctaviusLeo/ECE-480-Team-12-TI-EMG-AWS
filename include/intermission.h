#ifndef INTERMISSION_H
#define INTERMISSION_H
#include <stdint.h>
#include <stdbool.h>

void intermission_begin(uint32_t ms, const char* title);
bool intermission_active(void);
/* Draw overlay; returns true when finished (and auto-clears). */
bool intermission_tick(void);

#endif