#include <stdbool.h>
#include "timer.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"

static volatile uint32_t g_ms = 0;
static void SysTickThunk(void){ g_ms++; }

void timer_init(void){
  SysTickPeriodSet(SysCtlClockGet()/1000u);
  SysTickIntRegister(SysTickThunk);
  SysTickIntEnable();
  SysTickEnable();
}

uint32_t millis(void){ return g_ms; }

void delay_ms(uint32_t ms){
  uint32_t start = g_ms;
  // Fallback
  if (g_ms == start){
    while(ms--) SysCtlDelay(SysCtlClockGet()/3000u);
    return;
  }
  while((g_ms - start) < ms) { __asm(" wfi"); }
}
