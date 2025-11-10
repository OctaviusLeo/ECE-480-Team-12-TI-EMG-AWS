#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"

#include "ads131m02.h"

#define ADS_PERIPH_SSI       SYSCTL_PERIPH_SSI2
#define ADS_PERIPH_GPIOB     SYSCTL_PERIPH_GPIOB
#define ADS_PERIPH_GPIOE     SYSCTL_PERIPH_GPIOE

#define ADS_GPIOB_BASE       GPIO_PORTB_BASE
#define ADS_GPIOE_BASE       GPIO_PORTE_BASE

#define ADS_PIN_CLK          GPIO_PIN_4    // PB4 SSI2CLK
#define ADS_PIN_FSS          GPIO_PIN_5    // PB5 (use as manual CS)
#define ADS_PIN_RX           GPIO_PIN_6    // PB6 SSI2RX (MISO)
#define ADS_PIN_TX           GPIO_PIN_7    // PB7 SSI2TX (MOSI)
#define ADS_PIN_DRDY         GPIO_PIN_3    // PE3 DRDY (input)
#define ADS_CS_LOW()         GPIOPinWrite(ADS_GPIOB_BASE, ADS_PIN_FSS, 0)
#define ADS_CS_HIGH()        GPIOPinWrite(ADS_GPIOB_BASE, ADS_PIN_FSS, ADS_PIN_FSS)
#define ADS_DRDY_IS_LOW()    ((GPIOPinRead(ADS_GPIOE_BASE, ADS_PIN_DRDY) & ADS_PIN_DRDY) == 0)

// -------- SPI mode/speed --------
// Many ADS13xx parts use CPOL=0, CPHA=1 (Motorola Mode 1). If your datasheet
// says otherwise, change SSI_FRF_MOTO_MODE_1 below.
#define ADS_SPI_MODE         SSI_FRF_MOTO_MODE_1
#define ADS_SPI_HZ           1000000U   // 1 MHz to start conservatively

static inline void ssi2_xfer8(uint8_t tx, uint8_t *rx){
  uint32_t r;
  SSIDataPut(SSI2_BASE, tx);
  while(SSIBusy(SSI2_BASE)){}
  SSIDataGet(SSI2_BASE, &r);
  if(rx) *rx = (uint8_t)r;
}

static void ads_send_cmd24(uint8_t b0, uint8_t b1, uint8_t b2){
  ADS_CS_LOW();
  ssi2_xfer8(b0, 0); ssi2_xfer8(b1, 0); ssi2_xfer8(b2, 0);
  ADS_CS_HIGH();
}

// Read one 24-bit word (big-endian) while clocking dummy bytes
static uint32_t ads_read_word24(void){
  uint8_t b0, b1, b2;
  ssi2_xfer8(0x00, &b0);
  ssi2_xfer8(0x00, &b1);
  ssi2_xfer8(0x00, &b2);
  return ((uint32_t)b0<<16) | ((uint32_t)b1<<8) | (uint32_t)b2;
}

void ads_init(void){
  // Clocks
  SysCtlPeripheralEnable(ADS_PERIPH_GPIOB);
  SysCtlPeripheralEnable(ADS_PERIPH_GPIOE);
  SysCtlPeripheralEnable(ADS_PERIPH_SSI);
  while(!SysCtlPeripheralReady(ADS_PERIPH_GPIOB)){}
  while(!SysCtlPeripheralReady(ADS_PERIPH_GPIOE)){}
  while(!SysCtlPeripheralReady(ADS_PERIPH_SSI)){}

  // PB4..PB7 pin mux
  GPIOPinConfigure(GPIO_PB4_SSI2CLK);
  GPIOPinConfigure(GPIO_PB6_SSI2RX);
  GPIOPinConfigure(GPIO_PB7_SSI2TX);
  GPIOPinTypeSSI(ADS_GPIOB_BASE, ADS_PIN_CLK | ADS_PIN_RX | ADS_PIN_TX);

  // PB5 as manual CS
  GPIOPinTypeGPIOOutput(ADS_GPIOB_BASE, ADS_PIN_FSS);
  ADS_CS_HIGH();

  // PE3 DRDY input
  GPIOPinTypeGPIOInput(ADS_GPIOE_BASE, ADS_PIN_DRDY);

  // SSI2 config
  SSIConfigSetExpClk(SSI2_BASE, SysCtlClockGet(), ADS_SPI_MODE, SSI_MODE_MASTER, ADS_SPI_HZ, 8);
  SSIEnable(SSI2_BASE);

  // ----- Minimal bring-up (generic; adjust to datasheet if needed) -----
  // Many ADS131 devices require a RESET, then UNLOCK, register writes (WREG),
  // then START. We leave exact opcodes to your datasheet; the skeleton below
  // is a placeholder so this file compiles and you can wire pins.

  // Example placeholders (NOP frame gap):
  ADS_CS_LOW();
  // clock out a few NOP bytes to wake SPI frame state
  for(int i=0;i<6;i++){ ssi2_xfer8(0x00, 0); }
  ADS_CS_HIGH();

  // TODO: Replace with real commands from ADS131M02 datasheet:
  // ads_send_cmd24(CMD_RESET_0, CMD_RESET_1, CMD_RESET_2);
  // ads_send_cmd24(CMD_UNLOCK_0, CMD_UNLOCK_1, CMD_UNLOCK_2);
  // Write registers: data rate, word length, enable channels...
  // ads_send_cmd24(CMD_START_0, CMD_START_1, CMD_START_2);
}

int16_t ads_read_sample_ch1_blocking(void){
  // Wait for DRDY falling edge (active low)
  while(!ADS_DRDY_IS_LOW()){}
  // One frame typically: STATUS + CH1 + CH2 (each 24-bit).
  ADS_CS_LOW();
  uint32_t status = ads_read_word24();
  uint32_t ch1    = ads_read_word24();
  uint32_t ch2    = ads_read_word24();
  ADS_CS_HIGH();

  // Sign-extend 24-bit to 32, then scale to 16-bit for our processing
  // (If your part is set to 32-bit words, adjust parsing.)
  int32_t s1 = (ch1 & 0x800000) ? (int32_t)(ch1 | 0xFF000000) : (int32_t)ch1;
  // Quick downscale: >> 8 (keep MSB significance)
  return (int16_t)(s1 >> 8);
}
