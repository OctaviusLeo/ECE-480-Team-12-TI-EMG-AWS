#include "ssd1351.h"
#include <stdint.h>
#include <stdbool.h>
#include "board.h"
#include "timer.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"

#define CMD_SET_COLUMN       0x15
#define CMD_SET_ROW          0x75
#define CMD_WRITERAM         0x5C
#define CMD_DISPLAYOFF       0xAE
#define CMD_DISPLAYON        0xAF
#define CMD_SETREMAP         0xA0
#define CMD_STARTLINE        0xA1
#define CMD_DISPLAYOFFSET    0xA2
#define CMD_NORMALDISPLAY    0xA6
#define CMD_FUNCTIONSELECT   0xAB
#define CMD_SETVSL           0xB4
#define CMD_SETGPIO          0xB5
#define CMD_PRECHARGE        0xB1
#define CMD_CLOCKDIV         0xB3
#define CMD_MUXRATIO         0xCA
#define CMD_CONTRASTABC      0xC1
#define CMD_CONTRASTMASTER   0xC7
#define CMD_VCOMH            0xBE
#define CMD_COMMANDLOCK      0xFD
#define CMD_PRECHARGE2       0xB6

static inline void cs_low(void){  GPIOPinWrite(OLED_PORTA_BASE, OLED_PIN_CS, 0); }
static inline void cs_high(void){ GPIOPinWrite(OLED_PORTA_BASE, OLED_PIN_CS, OLED_PIN_CS); }
static inline void dc_cmd(void){  GPIOPinWrite(OLED_PORTB_BASE, OLED_PIN_DC, 0); }
static inline void dc_dat(void){  GPIOPinWrite(OLED_PORTB_BASE, OLED_PIN_DC, OLED_PIN_DC); }

// Busy-wait delay independent of SysTick
static void bw_delay_ms(uint32_t ms){
  while(ms--) SysCtlDelay(SysCtlClockGet()/3000u);
}

static void ssi_send8(uint8_t b){
  uint32_t dump;
  while(SSIBusy(OLED_SSI_BASE)){}
  SSIDataPut(OLED_SSI_BASE, b);
  while(SSIBusy(OLED_SSI_BASE)){}
  while(SSIDataGetNonBlocking(OLED_SSI_BASE, &dump)){} // flush
}

// Keep CS low across command+params
static void write_cmd(uint8_t c){
  cs_low(); dc_cmd(); ssi_send8(c); cs_high();
}
static void write_cmd1(uint8_t c, uint8_t d0){
  cs_low(); dc_cmd(); ssi_send8(c);
  dc_dat(); ssi_send8(d0);
  cs_high();
}
static void write_cmdN(uint8_t c, const uint8_t* p, int n){
  cs_low(); dc_cmd(); ssi_send8(c);
  dc_dat(); for(int i=0;i<n;i++) ssi_send8(p[i]);
  cs_high();
}

void ssd1351_set_window(uint8_t x, uint8_t y, uint8_t w, uint8_t h){
  uint8_t col[2]={ x, (uint8_t)(x+w-1) };
  uint8_t row[2]={ y, (uint8_t)(y+h-1) };
  write_cmdN(CMD_SET_COLUMN, col, 2);
  write_cmdN(CMD_SET_ROW,    row, 2);
  cs_low(); dc_cmd(); ssi_send8(CMD_WRITERAM); cs_high();
}

void ssd1351_push_pixels(const uint16_t *src, uint32_t count){
  cs_low(); dc_dat();
  for(uint32_t i=0;i<count;i++){
    uint16_t c = src ? src[i] : 0;
    ssi_send8(c >> 8); ssi_send8(c & 0xFF);
  }
  cs_high();
}

void ssd1351_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color){
  int x0 = x, y0 = y, x1 = x + w, y1 = y + h;
  if (x0 >= 128 || y0 >= 128 || x1 <= 0 || y1 <= 0) return;
  if (x0 < 0) x0 = 0;
  if (y0 < 0) y0 = 0;
  if (x1 > 128) x1 = 128;
  if (y1 > 128) y1 = 128;
  uint8_t cw = (uint8_t)(x1 - x0);
  uint8_t ch = (uint8_t)(y1 - y0);
  if (!cw || !ch) return;

  ssd1351_set_window((uint8_t)x0, (uint8_t)y0, cw, ch);
  cs_low(); dc_dat();
  for (uint32_t i=0;i<(uint32_t)cw*ch;i++){ ssi_send8(color>>8); ssi_send8(color&0xFF); }
  cs_high();
}

void ssd1351_fill(uint16_t color){
  ssd1351_draw_rect(0,0,128,128,color);
}

// small helper to stage SSI clock & mode
static void _ssi_set(uint32_t hz, uint32_t mode){
  SSIDisable(OLED_SSI_BASE);
  SSIConfigSetExpClk(OLED_SSI_BASE, SysCtlClockGet(),
                     mode, SSI_MODE_MASTER, hz, 8);
  SSIEnable(OLED_SSI_BASE);
}

void ssd1351_init(void){
  // Clocks and pins
  SysCtlPeripheralEnable(OLED_PERIPH_PORTA);
  SysCtlPeripheralEnable(OLED_PERIPH_PORTB);
  SysCtlPeripheralEnable(OLED_PERIPH_SSI);
  while(!(SysCtlPeripheralReady(OLED_PERIPH_PORTA) && SysCtlPeripheralReady(OLED_PERIPH_PORTB))) {}

  GPIOPinConfigure(GPIO_PA2_SSI0CLK);
  GPIOPinConfigure(GPIO_PA5_SSI0TX);
  GPIOPinTypeSSI(OLED_PORTA_BASE, OLED_PIN_CLK | OLED_PIN_TX);
  GPIOPinTypeGPIOOutput(OLED_PORTA_BASE, OLED_PIN_CS);

  GPIOPinTypeGPIOOutput(OLED_PORTB_BASE, OLED_PIN_DC | OLED_PIN_RST);

  // Idle levels
  cs_high(); dc_dat();

  // SPI: Motorola Mode 3. Start 4 MHz for init, raise later.
  _ssi_set(4000000u, SSI_FRF_MOTO_MODE_3);

  // Reset with generous delays
  GPIOPinWrite(OLED_PORTB_BASE, OLED_PIN_RST, OLED_PIN_RST); bw_delay_ms(10);
  GPIOPinWrite(OLED_PORTB_BASE, OLED_PIN_RST, 0);            bw_delay_ms(50);
  GPIOPinWrite(OLED_PORTB_BASE, OLED_PIN_RST, OLED_PIN_RST); bw_delay_ms(120);

  // Init sequence
  write_cmd1(CMD_COMMANDLOCK,   0x12);
  write_cmd1(CMD_COMMANDLOCK,   0xB1);
  write_cmd (CMD_DISPLAYOFF);
  write_cmd1(CMD_CLOCKDIV,      0xF1);
  write_cmd1(CMD_MUXRATIO,      127);
  write_cmd1(CMD_SETREMAP,      0x74);   
  write_cmd1(CMD_STARTLINE,     0x00);
  write_cmd1(CMD_DISPLAYOFFSET, 0x00);
  write_cmd1(CMD_SETGPIO,       0x00);
  write_cmd1(CMD_FUNCTIONSELECT,0x01);
  write_cmd1(CMD_PRECHARGE,     0x32);
  write_cmd1(CMD_VCOMH,         0x05);
  write_cmd (CMD_NORMALDISPLAY);
  { uint8_t abc[3]={0xC8,0x80,0xC8}; write_cmdN(CMD_CONTRASTABC, abc, 3); }
  write_cmd1(CMD_CONTRASTMASTER,0x0F);
  { uint8_t vsl[3]={0xA0,0xB5,0x55}; write_cmdN(CMD_SETVSL, vsl, 3); }
  write_cmd1(CMD_PRECHARGE2,    0x01);
  write_cmd (CMD_DISPLAYON);
  bw_delay_ms(120);

  // Raise to 8 MHz for drawing keep Mode 3
  _ssi_set(8000000u, SSI_FRF_MOTO_MODE_3);

  ssd1351_fill(0x0000);
}

