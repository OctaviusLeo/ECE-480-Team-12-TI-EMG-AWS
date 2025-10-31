#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include "project.h"
#include "timer.h"
#include "gfx.h"
#include "process.h"
#include "emg_mock.h"
#include "ssd1351.h"

typedef enum { S_IDLE, S_COUNT, S_READY, S_MEASURE, S_REPORT } state_t;
static state_t s=S_IDLE;
static uint32_t t_state_ms=0, t_round_ms=0;
static float mA=0, sA=0, mB=0, sB=0, thrA=0, thrB=0;
static float avgA=0, avgB=0;
static bool winnerA=false, tie=false;

#if DEMO_MODE
static uint32_t demo_cycle=0;  // 0: A wins, 1: B wins, 2: tie, then repeat
#endif

static void set_state(state_t ns){ s = ns; t_state_ms = millis(); }

void game_init(void){
  gfx_clear(COL_BLACK);
  // Labels
  gfx_text(10, 4,  "A", COL_RED);
  gfx_text(98, 4,  "B", COL_RED);
  set_state(S_COUNT);

#if DEMO_MODE
  // Quiet mock signal during "countdown"
  emg_mock_set_level(0.05f, 0.05f);
#else
  process_begin_baseline();
  emg_mock_set_level(0.05f, 0.05f);
#endif
}

void game_step(void){
  emg_sample_t f;
  if(!emg_mock_read(&f)) return;

  process_push(f.a, f.b);

  switch(s){
    case S_COUNT:{
#if DEMO_MODE
      // Fake a baseline and thresholds after a short countdown
      if(millis() - t_state_ms > DEMO_COUNTDOWN_MS){
        mA = 40.0f; sA = 10.0f;
        mB = 45.0f; sB = 12.0f;
        thrA = mA + THRESH_K_SIG*sA;   // ~70
        thrB = mB + THRESH_K_SIG*sB;   // ~81
        set_state(S_READY);
      }
#else
      // Real baseline accumulation
      process_push_baseline_sample();
      if(millis() - t_state_ms > COUNTDOWN_S*1000u){
        process_get_baseline(&mA,&sA,&mB,&sB);
        thrA = mA + THRESH_K_SIG*sA;
        thrB = mB + THRESH_K_SIG*sB;
        set_state(S_READY);
      }
#endif
    } break;

    case S_READY:{
#if DEMO_MODE
      // choosing scenario for this cycle
      bool demo_tie = (demo_cycle % 3) == 2;
      if (demo_tie){
        emg_mock_set_level(0.78f, 0.78f);     // tie
      } else if ((demo_cycle % 2) == 0){
        emg_mock_set_level(0.85f, 0.60f);     // A stronger
      } else {
        emg_mock_set_level(0.60f, 0.85f);     // B stronger
      }
      if(millis() - t_state_ms > DEMO_READY_MS){
        process_reset_round();
        t_round_ms = 0;
        set_state(S_MEASURE);
      }
#else
      // Wait for both sides to exceed thresholds
      if(process_envA()>thrA && process_envB()>thrB){
        process_reset_round();
        t_round_ms = 0;
        set_state(S_MEASURE);
      }
#endif
    } break;

    case S_MEASURE:{
      process_accumulate_round(process_envA(), process_envB());

#if DEMO_MODE
      t_round_ms += (1000u/SAMPLE_RATE_HZ);
      if(t_round_ms >= DEMO_ROUND_MS){
        process_get_round_avg(&avgA,&avgB);
        float maxv = (avgA>avgB? avgA:avgB);
        float diff = fabsf(avgA-avgB);
        bool force_tie = (demo_cycle % 3) == 2;

        if(force_tie){
          // Nudge into tie for display purposes
          avgB = avgA;
          tie = true; winnerA = false;
        } else {
          tie = (diff < (TIE_MARGIN_PCT/100.0f)*maxv);
          winnerA = (!tie && avgA>avgB);
        }
        set_state(S_REPORT);
      }
#else
      t_round_ms += (1000u/SAMPLE_RATE_HZ);
      if(t_round_ms >= ROUND_S*1000u){
        process_get_round_avg(&avgA,&avgB);
        float maxv = (avgA>avgB? avgA:avgB);
        float diff = fabsf(avgA-avgB);
        tie = (diff < (TIE_MARGIN_PCT/100.0f)*maxv);
        winnerA = (!tie && avgA>avgB);
        set_state(S_REPORT);
      }
#endif
    } break;

    case S_REPORT:{
#if DEMO_MODE
      if(millis() - t_state_ms > DEMO_REPORT_MS){
        demo_cycle++;
        set_state(S_COUNT);
        emg_mock_set_level(0.05f,0.05f);
      }
#else
      // Hold for 3 s then restart
      if(millis() - t_state_ms > 3000u){
        set_state(S_COUNT);
        process_begin_baseline();
        emg_mock_set_level(0.05f,0.05f);
      }
#endif
    } break;

    default: break;
  }
}

static void draw_bars(void){
  // Bar areas
  uint8_t bxA=20, bxB=78, bw=30, base=120, maxh=100;
  // Map env to height (auto-scale using thresholds as rough guide)
  float eA = process_envA(), eB = process_envB();
  float hA = (eA/(thrA>1?thrA*2:1000))*maxh; if(hA>maxh) hA=maxh; if(hA<0) hA=0;
  float hB = (eB/(thrB>1?thrB*2:1000))*maxh; if(hB>maxh) hB=maxh; if(hB<0) hB=0;

  // Clear bar columns
  ssd1351_draw_rect(bxA, 20, bw, maxh+2, COL_BLACK);
  ssd1351_draw_rect(bxB, 20, bw, maxh+2, COL_BLACK);

  // Draw fills (from bottom up)
  gfx_bar(bxA, base - (uint8_t)hA, bw, (uint8_t)hA, COL_RED);
  gfx_bar(bxB, base - (uint8_t)hB, bw, (uint8_t)hB, COL_RED);

  // Threshold ticks (midline vs auto-scale)
  uint8_t tA = base - (uint8_t)((thrA/(thrA*2))*maxh);
  uint8_t tB = base - (uint8_t)((thrB/(thrB*2))*maxh);
  ssd1351_draw_rect(bxA, tA, bw, 1, COL_GRAY);
  ssd1351_draw_rect(bxB, tB, bw, 1, COL_GRAY);
}

void game_render(void){
  // Status line
  ssd1351_draw_rect(0,0,128,16,COL_BLACK);
  if(s==S_COUNT){
    gfx_text(20,4,"COUNTDOWN 3..2..1", COL_GRAY);
  } else if(s==S_READY){
    gfx_text(28,4,"READY - FLEX!", COL_GRAY);
  } else if(s==S_MEASURE){
    gfx_text(28,4,"MEASURING...", COL_GRAY);
  } else if(s==S_REPORT){
    if(tie){
      gfx_text(44,4,"TIE", COL_RED);
    } else {
      gfx_text(22,4, winnerA?"WINNER: A":"WINNER: B", COL_RED);
    }
  }

  // Bars
  draw_bars();

  // Show round averages in REPORT
  if(s==S_REPORT){
    ssd1351_draw_rect(0, 122, 128, 6, COL_BLACK);
    char buf[20];
    snprintf(buf,sizeof(buf),"A:%d  B:%d",(int)avgA,(int)avgB);
    gfx_text(8, 116, buf, COL_GRAY);
  }
}
