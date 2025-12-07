/**
 * @file choice_input.c
 * @brief Map continuous Hz values into discrete A/B choices and draw hints.
 */

#include "choice_input.h"
#include "gfx.h"
#include "project.h"

/**
 * @brief Convert Hz into CHOICE_A or CHOICE_B using a threshold.
 *
 * @param hz           Current measured Hz.
 * @param threshold_hz Threshold between A and B.
 * @return CHOICE_B if hz > threshold_hz, else CHOICE_A.
 */
choice_t choice_from_hz(float hz, float threshold_hz){
  return (hz > threshold_hz) ? CHOICE_B : CHOICE_A;
}

/**
 * @brief Draw a simple hint for the A/B choice mapping.
 *
 * Currently uses a placeholder text at a given Y position.
 *
 * @param y Y coordinate in pixels for the hint text.
 */
void choice_draw_hint(uint8_t y){
  gfx_text2(6, y, "", COL_RED, 1);
}
