#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "driverlib/pin_map.h"
#include "timer.h"   

void     ads_init(void);
int16_t  ads_read_sample_ch1_blocking(void);   // returns one signed sample (scaled to 16b) from CH1

int16_t ads_read_sample_ch1_blocking(void);
int     ads_read_sample_ch1_timeout(uint32_t timeout_ms, int16_t* out);  // 0=ok, -1=timeout

void ads_debug_dump_once(void);

int ads_drdy_edge_count_ms(uint32_t ms);

int  ads_drdy_edge_count_ms(uint32_t window_ms);

int  ads_configure_start(void);   // returns 0 on success
