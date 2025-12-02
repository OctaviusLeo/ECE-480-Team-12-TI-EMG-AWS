#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"

#include "timer.h"
#include "ads131m02.h"
#include "game.h"

static bool     g_baseline_done = false;
static uint32_t g_baseline_until_ms = 0;
static float    g_baseline_acc = 0.0f;
static uint32_t g_baseline_n = 0;
static float    g_baseline_hz = 0.0f;

void baseline_begin(uint32_t window_ms){
  g_baseline_done = false;
  g_baseline_acc = 0.0f;
  g_baseline_n = 0;
  g_baseline_until_ms = millis() + window_ms;
  g_baseline_hz = 0.0f;
}

static inline bool baseline_update(float hz){
  if (g_baseline_done) return false;

  if ((int32_t)(millis() - g_baseline_until_ms) < 0){
    g_baseline_acc += hz;
    g_baseline_n++;
    return false;
  }

  g_baseline_hz = (g_baseline_n ? (g_baseline_acc / (float)g_baseline_n) : 0.0f);
  g_baseline_done = true;
  printf("[BASELINE] %.2f Hz (n=%lu)\n", g_baseline_hz, (unsigned long)g_baseline_n);
  game_set_baseline(g_baseline_hz);
  return true;
}

/* small helpers */
static inline uint8_t clamp_u8(int v, int lo, int hi){
  if (v < lo) return (uint8_t)lo;
  if (v > hi) return (uint8_t)hi;
  return (uint8_t)v;
}

/* Rising zero-cross Hz estimate over ~window_ms using ADS samples */
static float estimate_hz_window_ms(uint32_t window_ms){
  const int16_t HYST = 8;
  uint32_t start = millis();
  int16_t prev = 0, s = 0;
  bool first_ok = false;

  if (ads_read_sample_ch1_timeout(2, &s) == 0){
    prev = s; first_ok = true;
  }

  uint32_t rises = 0;
  while ((millis() - start) < window_ms){
    if (ads_read_sample_ch1_timeout(2, &s) != 0) continue;
    if (first_ok){
      int16_t pz = (prev >  HYST) ? +1 : (prev < -HYST ? -1 : 0);
      int16_t cz = (s    >  HYST) ? +1 : (s    < -HYST ? -1 : 0);
      if (pz < 0 && cz >= 0) rises++;
    } else {
      first_ok = true;
    }
    prev = s;
  }
  float secs = (float)window_ms / 1000.0f;
  return (secs > 0.0f) ? (rises / secs) : 0.0f;
}

int main(void){
  // 80 MHz system clock (PLL, 16 MHz crystal)
  SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

  timer_init();

  // ADS bringup
  ads_init();

  // Enable global interrupts after peripherals are initialized
  IntMasterEnable();

  // Start OLED/game (non-blocking)
  game_init();

  baseline_begin(3000u);   // 3 s baseline window aligned with countdown

  uint32_t next_print = millis();
  uint32_t next_tick  = millis();

  while(1){
    // Update metrics about every ~100 ms for snappy UI
    float hz = estimate_hz_window_ms(100);

    // accumulate baseline during the first 3 s after game_init()
    (void)baseline_update(hz);

    // subtract baseline (floor at 0) before feeding UI
    extern float g_baseline_hz; // if not in header; otherwise remove this line
    float hz_adj = hz - g_baseline_hz; if (hz_adj < 0.0f) hz_adj = 0.0f;
    uint8_t pct = clamp_u8((int)((hz_adj * 100.0f / 250.0f) + 0.5f), 0, 100);
    game_set_metrics(hz_adj, pct);

    next_tick = millis() + 100u;

    // Console print ~4 Hz (150 ms window for a steadier number)
    if ((int32_t)(millis() - next_print) >= 0){
      float hz_now = estimate_hz_window_ms(150);
      extern bool g_baseline_done; // if not in header; otherwise remove this line
      if (g_baseline_done){ hz_now -= g_baseline_hz; if (hz_now < 0.0f) hz_now = 0.0f; }
      uint8_t pct_now = clamp_u8((int)((hz_now * 100.0f / 250.0f) + 0.5f), 0, 100);
      printf("Hz=%.1f  INT=%u%%\n", hz_now, (unsigned)pct_now);
      next_print = millis() + 250u;
    }

    // Advance OLED/game UI (non-blocking)
    game_tick();
  }
}
