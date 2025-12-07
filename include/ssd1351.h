/**
 * @file ssd1351.h
 * @brief Low-level driver interface for SSD1351 OLED display.
 *
 * Provides basic initialization and drawing primitives for a 128x128
 * RGB565 OLED panel.
 */

#ifndef SSD1351_H
#define SSD1351_H

#include <stdint.h>

/**
 * @brief Initialize the SSD1351 display controller.
 *
 * Configures SPI, display registers, and readies the panel for drawing.
 */
void ssd1351_init(void);

/**
 * @brief Fill the entire display with a solid color.
 *
 * @param color RGB565 color value.
 */
void ssd1351_fill(uint16_t color);

/**
 * @brief Set an active drawing window region.
 *
 * Subsequent pixel pushes will populate this region.
 *
 * @param x Left X coordinate in pixels.
 * @param y Top Y coordinate in pixels.
 * @param w Width in pixels.
 * @param h Height in pixels.
 */
void ssd1351_set_window(uint8_t x, uint8_t y, uint8_t w, uint8_t h);

/**
 * @brief Push a contiguous span of pixels to the display.
 *
 * @param src   Pointer to RGB565 pixel buffer.
 * @param count Number of pixels to send.
 */
void ssd1351_push_pixels(const uint16_t *src, uint32_t count);

/**
 * @brief Draw a filled rectangle.
 *
 * Convenience helper that sets a window and fills it.
 *
 * @param x     Left X coordinate.
 * @param y     Top Y coordinate.
 * @param w     Width in pixels.
 * @param h     Height in pixels.
 * @param color RGB565 fill color.
 */
void ssd1351_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);

#endif /* SSD1351_H */
