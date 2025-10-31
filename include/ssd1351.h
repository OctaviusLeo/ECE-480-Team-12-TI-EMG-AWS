#ifndef SSD1351_H
#define SSD1351_H

#include <stdint.h>

void ssd1351_init(void);
void ssd1351_fill(uint16_t color);
void ssd1351_set_window(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void ssd1351_push_pixels(const uint16_t *src, uint32_t count);
void ssd1351_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);

#endif
