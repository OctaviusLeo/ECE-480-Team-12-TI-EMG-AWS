#include "choice_input.h"
#include "gfx.h"
#include "project.h"

choice_t choice_from_hz(float hz, float threshold_hz){
  return (hz > threshold_hz) ? CHOICE_B : CHOICE_A;
}

void choice_draw_hint(uint8_t y){
  gfx_text2(6, y, "A:0–50|B: 51–max", COL_RED, 1);
}
