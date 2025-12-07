/**
 * @file choice_input.h
 * @brief Map continuous Hz values into discrete A/B choices for UI.
 *
 * Converts the current EMG-derived Hz into a binary choice, and can
 * draw a simple on-screen hint for the mapping.
 */

#ifndef CHOICE_INPUT_H
#define CHOICE_INPUT_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Binary choice result.
 *
 * CHOICE_A usually corresponds to "lower Hz" and CHOICE_B to "higher Hz".
 */
typedef enum {
    CHOICE_A = 0,
    CHOICE_B = 1
} choice_t;

/**
 * @brief Map current Hz to an A/B choice using a threshold.
 *
 * Default concept: A for [0..threshold], B for (threshold..max].
 *
 * @param hz           Current measured Hz.
 * @param threshold_hz Threshold that separates A vs B.
 * @return CHOICE_A or CHOICE_B based on hz vs threshold_hz.
 */
choice_t choice_from_hz(float hz, float threshold_hz);

/**
 * @brief Draw a one-line UI hint for A/B mapping.
 *
 * Example text: "A (0–50)  |  B (51–max)".
 *
 * @param y Text row or pixel Y position to draw the hint at.
 */
void choice_draw_hint(uint8_t y);

#endif /* CHOICE_INPUT_H */
