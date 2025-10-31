#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"

#include "board.h"
#include "timer.h"
#include "ssd1351.h"
#include "gfx.h"
#include "emg_mock.h"
#include "process.h"
#include "project.h"
#include "game.h"

// LaunchPad red LED on PF1
#define LED_PERIPH   SYSCTL_PERIPH_GPIOF
#define LED_PORT     GPIO_PORTF_BASE
#define LED_PIN      GPIO_PIN_1
static void led_init(void){
  SysCtlPeripheralEnable(LED_PERIPH);
  while(!SysCtlPeripheralReady(LED_PERIPH)){}
  GPIOPinTypeGPIOOutput(LED_PORT, LED_PIN);
}
static inline void led_on(void){  GPIOPinWrite(LED_PORT, LED_PIN, LED_PIN); }
static inline void led_off(void){ GPIOPinWrite(LED_PORT, LED_PIN, 0); }
static void led_blink(int n, int ms){ for(int i=0;i<n;i++){ led_on(); delay_ms(ms); led_off(); delay_ms(ms); } }
static void u32_to_dec(uint32_t v, char* out, uint8_t cap);
static int  compute_fake_avg_ms_lite(uint32_t measure_ms);

// Text helpers (match gfx_text2 spacing: width = len*(5*scale+1)-1)
static uint8_t text_width_px(const char* s, uint8_t scale){
  if (scale == 0) scale = 1;
  uint32_t len = 0; for (const char* p=s; *p; ++p) len++;
  if (len == 0) return 0;
  uint32_t w = len * (5*scale + 1) - 1;        
  return (w > 255) ? 255 : (uint8_t)w;
}
static void draw_centered_hdr(const char* s, uint16_t color){
  uint8_t scale = (text_width_px(s, 2) <= 124) ? 2 : 1;
  uint8_t w = text_width_px(s, scale);
  uint8_t x = (w < 128) ? (uint8_t)((128 - w) / 2) : 0;
  // header band
  ssd1351_draw_rect(0, 0, 128, 18, COL_BLACK);
  gfx_text2(x, 2, s, color, scale);
}

// Bars
static void draw_bars_static(uint8_t hA, uint8_t hB){
  const uint8_t bxA=20, bxB=78, bw=30, base=120, maxh=100; // bxA and bxB mMUST be within OLED scope or it will not show up.
  if(hA>maxh) hA=maxh; if(hB>maxh) hB=maxh;

  // clear columns area
  ssd1351_draw_rect(bxA, 20, bw, maxh+2, COL_BLACK);
  //ssd1351_draw_rect(bxB, 20, bw, maxh+2, COL_BLACK);

  // small A/B labels at top of columns (persist each draw)
  gfx_text2(bxA + (bw/2) - 3, 20, "YOUR STRENGTH", COL_WHITE, 1);
  //gfx_text2(bxB + (bw/2) - 3, 20, "B", COL_WHITE, 1);

  // fills (bottom-up)
  ssd1351_draw_rect(bxA, base - hA, bw, hA, COL_RED);
  //ssd1351_draw_rect(bxB, base - hB, bw, hB, COL_RED);

  // midline tick for reference
  uint8_t mid = base - (uint8_t)(maxh/2);
  ssd1351_draw_rect(bxA, mid, bw, 1, COL_GRAY);
  //ssd1351_draw_rect(bxB, mid, bw, 1, COL_GRAY);
}

// HEADER TEST
static void _clear_header_band(void){
  for (uint8_t y = 0; y < 18; ){
    uint8_t h = (uint8_t)((18 - y) > 8 ? 8 : (18 - y));
    ssd1351_draw_rect(0, y, 128, h, COL_BLACK);
    y = (uint8_t)(y + h);
  }
}

/* compute average from fake EMG data over a timed window
*/
// returns an integer "envelope average" over measure_ms (1..4000ms)
// returns an integer "envelope average" over measure_ms (1..4000ms)
static int compute_fake_avg_ms_lite(uint32_t measure_ms){
  if (measure_ms < 50) measure_ms = 50;
  uint32_t t0 = millis(), last_hb = t0, rng = 1;
  // Fixed-point Q8.8 envelope with alpha ≈ 1/64 (cheap one-pole)
  uint32_t env_q8 = 0, sum = 0, cnt = 0;
  // pick a nominal amplitude (≈ "strong" flex)
  int32_t level = 900; // tweak 300..1500 to scale the average

  while ((millis() - t0) < measure_ms){
    // 1 kHz-ish pacing using millis (good enough for demo)
    static uint32_t last_ms = 0;
    uint32_t now = millis();
    if (now != last_ms){
      last_ms = now;
      // cheap pseudo-EMG sample: uniform noise [-level, +level]
      rng = 1664525u*rng + 1013904223u;
      int32_t s = (int32_t)((rng >> 16) & 1023) - 512;   // -512..+511
      s = (s * level) / 512;                             // scale
      int32_t a = (s < 0) ? -s : s;                      // |s|

      // envelope: env += (a - env)/64  in Q8.8
      int32_t a_q8 = a << 8;
      env_q8 += (uint32_t)((a_q8 - (int32_t)env_q8) >> 6);

      sum += (env_q8 >> 8);  // back to integer
      cnt++;
    }

    // tiny heartbeat, non-blocking
    if ((millis() - last_hb) > 120){
      led_on(); delay_ms(10); led_off();
      last_hb = millis();
    }
  }
  if (cnt == 0) return 0;
  return (int)((sum + (cnt>>1)) / cnt);
}

static void show_average_from_fake_lite(uint32_t measure_ms){
  gfx_header("Calculating...", 0x7BF0);          // unchanged
  int avg_val = compute_fake_avg_ms_lite(measure_ms);
  delay_ms(1001);                                 // unique delay

  ssd1351_fill(COL_BLACK);                        // you reported clears work

  gfx_header("AVERAGE:", 0x7BF2);                 // different color
  delay_ms(2011);                                 // unique delay

  static char num[12];
  u32_to_dec((uint32_t)avg_val, num, sizeof(num));
  gfx_header(num, 0xF806);                        // different color than label
  delay_ms(2013);                                 // unique delay
}

// Tiny decimal converter: no libc, minimal stack
static void u32_to_dec(uint32_t v, char* out, uint8_t cap){
  if (cap == 0) return;
  char tmp[10];           // enough for 32-bit
  uint8_t n = 0;
  do { tmp[n++] = (char)('0' + (v % 10)); v /= 10; } while (v && n < sizeof(tmp));
  // reverse into out
  uint8_t i = 0;
  if (n >= cap) n = (uint8_t)(cap - 1);
  while (n--) out[i++] = tmp[n];
  out[i] = '\0';
}

// Compute average (your existing routine), then show it without snprintf.
// Uses two gfx_header calls with distinct color+delay per your rule.
static void show_average_from_fake(uint32_t measure_ms){
  int avg_val = compute_fake_avg_ms_lite(measure_ms);
  if (avg_val < 0) avg_val = 0;

  // Label first
  gfx_header("AVERAGE:", 0x7BF2);      // new color not used elsewhere
  delay_ms(2011);                      // unique delay

  // Then the number
  static char num[12];                 // static → no stack pressure
  u32_to_dec((uint32_t)avg_val, num, sizeof(num));
  gfx_header(num, 0xF806);             // different color than label
  delay_ms(2013);                      // unique delay
}


// TEST : (RULE: FOR SOME REASON COLOR AND DELAY_MS CANNOT BE THE SAME FOR DIFFERENT GFX_HEADER, and mabe in same functions).
static void oled_smoke_test(void){

  // Just to shout out TEXAS INSTURMENTS
  gfx_header("TEXAS INSTRUMENTS",             0xF804);  delay_ms(4005);
  ssd1351_fill(COL_RED);

  // These are scripted and must have different colors and delay_ms (i dunno why)
  gfx_header("COUNTDOWN...",             0xFFFF);  delay_ms(3000);
  gfx_header("3",                     0xFFFD);  delay_ms(1002);
  gfx_header("2",                     0xFFFC);  delay_ms(1003);
  gfx_header("1",                     0x7BEE);  delay_ms(1004);
  gfx_header("FLEX!",                 0xF801);  delay_ms(6000);

  ssd1351_fill(COL_BLACK);
  // bars
  draw_bars_static(80, 40);

  gfx_header("Caluclating...",        0x7BF0);  delay_ms(5000);

  // clear screen 

  //show_average_from_fake_lite(3000);

  ssd1351_fill(COL_BLACK);

  delay_ms(1001);
  gfx_header("AVERAGE: 2.7", 0x7BF1);  delay_ms(2010);

  gfx_header("YOU'RE STRONGER\nTHAN", 0xF802);  delay_ms(3005);
  gfx_header("90%\nof People!",        0xF803); delay_ms(5005); // \n doesn't work

  // Send Off
  gfx_header("TEXAS INSTRUMENTS ",             0xF805);  

  // park with heartbeat so final frame is visible
  while(1){
    led_on();  delay_ms(20);
    led_off(); delay_ms(980);
  }
}

int main(void){
  // 80 MHz system clock (PLL, 16 MHz crystal)
  SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

  timer_init();
  IntMasterEnable();

  // Prove CPU is running
  led_init();
  led_blink(3, 100);      // 3 quick blinks at start

  // Display bring-up
  ssd1351_init();
  led_blink(2, 150);      // blink after OLED init returns

  // draw header + bars and stop here
  gfx_clear(COL_BLACK);
  oled_smoke_test();      

  // App path (later)
  emg_mock_init(SAMPLE_RATE_HZ);
  process_init(SAMPLE_RATE_HZ, WINDOW_MS);
  game_init();

  uint32_t t_ui = millis();
  while(1){
    led_on();  delay_ms(30);  led_off();

    game_step();
    if ((millis() - t_ui) >= (1000u/UI_RATE_HZ)){
      t_ui = millis();
      game_render();
    }
  }
}
