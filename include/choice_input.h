#ifndef CHOICE_INPUT_H
#define CHOICE_INPUT_H
#include <stdint.h>
#include <stdbool.h>

typedef enum { CHOICE_A = 0, CHOICE_B = 1 } choice_t;

/* Map current Hz to A/B with a threshold. Default: A for 0..50, B for 51..max. */
choice_t choice_from_hz(float hz, float threshold_hz);

/* Optional: simple 1-line UI hint “A (0–50)  |  B (51–max)” */
void choice_draw_hint(uint8_t y);

#endif
