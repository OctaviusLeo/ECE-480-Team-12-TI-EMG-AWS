#ifndef PROJECT_H
#define PROJECT_H

// App tunables
#define SAMPLE_RATE_HZ   1000u
#define WINDOW_MS        250u
#define UI_RATE_HZ       60u
#define COUNTDOWN_S      3u
#define ROUND_S          12u
#define THRESH_K_SIG     3.0f
#define TIE_MARGIN_PCT   5.0f

// Colors (RGB565)
#define RGB565(r,g,b) ( \
  (uint16_t)((((r) & 0xF8) << 8) | \
             (((g) & 0xFC) << 3) | \
             (((b) & 0xF8) >> 3)) )

// Existing named colors
#define COL_BLACK  0x0000
#define COL_RED    0xF800
#define COL_DKRED  0x8000
#define COL_GRAY   0x7BEF
#define COL_WHITE  0xFFFF

// Extended palette 
#define COL_BLUE       0x001F
#define COL_GREEN      0x07E0
#define COL_CYAN       0x07FF
#define COL_MAGENTA    0xF81F
#define COL_YELLOW     0xFFE0
#define COL_ORANGE     0xFD20 
#define COL_PURPLE     0x8010
#define COL_VIOLET     0x901F
#define COL_PINK       0xF81F 
#define COL_BROWN      0xA145
#define COL_TAN        0xD5B1
#define COL_NAVY       0x000F
#define COL_TEAL       0x0410
#define COL_OLIVE      0x7BE0
#define COL_MAROON     0x7800
#define COL_SILVER     0xC618
#define COL_LTGRAY     0xD69A
#define COL_DKGRAY     0x528A
#define COL_GOLD       0xFEA0  
#define COL_TURQUOISE  0x1DB7 
#define COL_UT_TURQ    RGB565(0x1D,0xB7,0xB6) 
#define COL_UT_VIOLET  RGB565(0x7A,0x5C,0xFA) 

// DEMO MODE (no real baseline / sensors) 
#define DEMO_MODE 1  // CHANGE to 0 for REAL DATA

#if DEMO_MODE
  #define DEMO_COUNTDOWN_MS 1200u
  #define DEMO_READY_MS      500u
  #define DEMO_ROUND_MS     4000u
  #define DEMO_REPORT_MS    1500u
#endif

#endif
