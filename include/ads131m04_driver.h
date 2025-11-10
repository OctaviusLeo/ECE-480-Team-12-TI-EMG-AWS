/**
 * ads131m04_driver.h
 * Driver for ADS131M04 4-channel 24-bit ADC with programmable PGA
 * 
 * Final corrected version - October 2025
 */

#ifndef ADS131M04_DRIVER_H_
#define ADS131M04_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>

//============================================================================
// HARDWARE CONFIGURATION
//============================================================================

// SPI Module
#define ADS_SPI_BASE        SSI0_BASE

// GPIO Pins
#define ADS_GPIO_PORT       GPIO_PORTA_BASE
#define ADS_CS_PIN          GPIO_PIN_3

#define ADS_DRDY_PORT       GPIO_PORTB_BASE
#define ADS_DRDY_PIN        GPIO_PIN_0

#define ADS_RESET_PORT      GPIO_PORTB_BASE
#define ADS_RESET_PIN       GPIO_PIN_1

#define ADS_CLKIN_PORT      GPIO_PORTB_BASE
#define ADS_CLKIN_PIN       GPIO_PIN_6  // PWM output for clock

//============================================================================
// REGISTER ADDRESSES
//============================================================================

#define ADS_REG_ID          0x00
#define ADS_REG_STATUS      0x01
#define ADS_REG_MODE        0x02
#define ADS_REG_CLOCK       0x03
#define ADS_REG_GAIN1       0x0E
#define ADS_REG_GAIN2       0x0F
#define ADS_REG_GAIN3       0x10
#define ADS_REG_GAIN4       0x11
#define ADS_REG_CFG         0x06
#define ADS_REG_THRSHLD_MSB 0x09
#define ADS_REG_THRSHLD_LSB 0x0A

//============================================================================
// COMMAND WORDS
//============================================================================

#define ADS_CMD_NULL        0x0000
#define ADS_CMD_RESET       0x0011
#define ADS_CMD_STANDBY     0x0022
#define ADS_CMD_WAKEUP      0x0033
#define ADS_CMD_LOCK        0x0555
#define ADS_CMD_UNLOCK      0x0655

// Command construction macros
#define ADS_CMD_RREG(addr, num)  (0x2000 | ((addr & 0x1F) << 7) | (num & 0x7F))
#define ADS_CMD_WREG(addr, num)  (0x6000 | ((addr & 0x1F) << 7) | (num & 0x7F))

//============================================================================
// PGA GAIN VALUES
//============================================================================

typedef enum {
    ADS_GAIN_1   = 0x0000,  // Gain = 1,   FSR = ±1.2V
    ADS_GAIN_2   = 0x0001,  // Gain = 2,   FSR = ±600mV
    ADS_GAIN_4   = 0x0002,  // Gain = 4,   FSR = ±300mV
    ADS_GAIN_8   = 0x0003,  // Gain = 8,   FSR = ±150mV
    ADS_GAIN_16  = 0x0004,  // Gain = 16,  FSR = ±75mV
    ADS_GAIN_32  = 0x0005,  // Gain = 32,  FSR = ±37.5mV
    ADS_GAIN_64  = 0x0006,  // Gain = 64,  FSR = ±18.75mV
    ADS_GAIN_128 = 0x0007   // Gain = 128, FSR = ±9.375mV
} ADS_PGA_Gain;

//============================================================================
// FSR LOOKUP TABLE
//============================================================================

extern const float ADS_FSR_TABLE[8];

//============================================================================
// FUNCTION PROTOTYPES
//============================================================================

// Initialization
void ADS_Init(void);
void ADS_GenerateClockSignal(void);
void ADS_HardwareReset(void);

// Register access
uint16_t ADS_ReadRegister(uint8_t reg_addr);
void ADS_WriteRegister(uint8_t reg_addr, uint16_t value);

// PGA configuration
void ADS_SetChannelGain(uint8_t channel, ADS_PGA_Gain gain);
ADS_PGA_Gain ADS_GetChannelGain(uint8_t channel);
void ADS_SetAllChannelsGain(ADS_PGA_Gain gain);

// Data acquisition
bool ADS_IsDataReady(void);
void ADS_ReadAllChannels(int32_t *ch1, int32_t *ch2, int32_t *ch3, int32_t *ch4);

// Utility functions
void ADS_SendCommand(uint16_t command);
void ADS_TransferWord(uint16_t tx_data, uint16_t *rx_data);
float ADS_ToVoltage(int32_t adc_value, float vref);

#endif // ADS131M04_DRIVER_H_