/**
 * @file ads131m02.h
 * @brief Minimal interface for the ADS131M02 EMG front-end ADC.
 *
 * Provides init, blocking/timeout sample read, DRDY edge counting,
 * and one-shot debug/diagnostic helpers for the EMG front-end.
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "driverlib/pin_map.h"
#include "timer.h"   

/**
 * @brief Initialize the ADS131M02 interface and hardware.
 *
 * Configures GPIO, SSI, and any ADC-specific startup sequence needed
 * before samples can be read.
 */
void     ads_init(void);

/**
 * @brief Read one signed sample from channel 1 (blocking).
 *
 * Returns once a fresh sample has been acquired and converted to a
 * 16-bit signed value.
 *
 * @return Signed 16-bit sample (scaled to 16b) from CH1.
 */
int16_t  ads_read_sample_ch1_blocking(void);   // returns one signed sample (scaled to 16b) from CH1

/* NOTE: ads_read_sample_ch1_blocking() is declared twice for historical
 * reasons; both declarations are identical and compile to the same symbol.
 */
int16_t ads_read_sample_ch1_blocking(void);

/**
 * @brief Read one sample from CH1, with timeout.
 *
 * @param timeout_ms Maximum time to wait for a sample, in milliseconds.
 * @param[out] out   Pointer to receive the signed 16-bit sample.
 *
 * @return 0 on success, -1 on timeout.
 */
int     ads_read_sample_ch1_timeout(uint32_t timeout_ms, int16_t* out);  // 0=ok, -1=timeout

/**
 * @brief Perform a one-shot debug dump of ADS-related state.
 *
 * Typically prints diagnostic information to a console or debug UART.
 */
void ads_debug_dump_once(void);

/**
 * @brief Count DRDY edges over a specified measurement window.
 *
 * @param ms Measurement window length in milliseconds.
 *
 * @return Number of DRDY edges observed in the given window.
 */
int ads_drdy_edge_count_ms(uint32_t ms);

/* Alternate declaration of ads_drdy_edge_count_ms() with a window_ms parameter.
 * Both declarations refer to the same underlying function symbol.
 */
int  ads_drdy_edge_count_ms(uint32_t window_ms);

/**
 * @brief Configure and start continuous sampling on the ADS131M02.
 *
 * @return 0 on success, non-zero on configuration error.
 */
int  ads_configure_start(void);   // returns 0 on success
