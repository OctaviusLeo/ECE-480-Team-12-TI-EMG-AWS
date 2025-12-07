/**
 * @file timer.h
 * @brief Millisecond timebase and blocking delay utilities.
 *
 * These functions provide a coarse-grained millisecond clock and a
 * simple busy-wait delay based on that timebase.
 */

#include <stdint.h>

/**
 * @brief Initialize the timer subsystem.
 *
 * Must be called once at startup before millis() or delay_ms() are used.
 */
void timer_init(void);

/**
 * @brief Get the number of milliseconds since system start.
 *
 * @return Milliseconds elapsed since the timer subsystem was initialized.
 *         Wraps naturally on 32-bit overflow.
 */
uint32_t millis(void);

/**
 * @brief Busy-wait for a specified number of milliseconds.
 *
 * @param ms Duration to block, in milliseconds.
 */
void delay_ms(uint32_t ms);
