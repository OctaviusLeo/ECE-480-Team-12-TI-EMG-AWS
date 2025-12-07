/**
 * ads131m04_driver.h
 * Driver for ADS131M04 4-channel 24-bit ADC with programmable PGA
 * 
 * This header defines register addresses, command words, gain options,
 * and the public API used to configure and acquire data from the ADS131M04.
 */

#ifndef ADS131M04_DRIVER_H_
#define ADS131M04_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>

// HARDWARE CONFIGURATION
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

// REGISTER ADDRESSES
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

// COMMAND WORDS
#define ADS_CMD_NULL        0x0000
#define ADS_CMD_RESET       0x0011
#define ADS_CMD_STANDBY     0x0022
#define ADS_CMD_WAKEUP      0x0033
#define ADS_CMD_LOCK        0x0555
#define ADS_CMD_UNLOCK      0x0655

// Command construction macros
#define ADS_CMD_RREG(addr, num)  (0x2000 | ((addr & 0x1F) << 7) | (num & 0x7F))
#define ADS_CMD_WREG(addr, num)  (0x6000 | ((addr & 0x1F) << 7) | (num & 0x7F))

// PGA GAIN VALUES
/**
 * @brief Programmable gain amplifier settings and full-scale ranges.
 */
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

// FSR LOOKUP TABLE
/**
 * @brief Full-scale range lookup table indexed by ADS_PGA_Gain.
 *
 * Units are volts (V). Entry i corresponds to gain code i.
 */
extern const float ADS_FSR_TABLE[8];

// FUNCTION PROTOTYPES
// Initialization

/**
 * @brief Initialize ADS131M04 hardware and bring the device online.
 *
 * Sets up SPI, GPIO pins, and configures key device registers.
 */
void ADS_Init(void);

/**
 * @brief Configure a PWM output to drive ADS131M04 CLKIN.
 */
void ADS_GenerateClockSignal(void);

/**
 * @brief Assert and release the ADS131M04 reset pin.
 */
void ADS_HardwareReset(void);

// Register access

/**
 * @brief Read a 16-bit register from the ADS131M04.
 *
 * @param reg_addr Register address (0x00–0x1F).
 * @return 16-bit register value.
 */
uint16_t ADS_ReadRegister(uint8_t reg_addr);

/**
 * @brief Write a 16-bit value into an ADS131M04 register.
 *
 * @param reg_addr Register address (0x00–0x1F).
 * @param value    Value to write.
 */
void ADS_WriteRegister(uint8_t reg_addr, uint16_t value);

// PGA configuration

/**
 * @brief Set the gain for a specific ADC channel.
 *
 * @param channel ADC channel index [0..3].
 * @param gain    Desired gain setting.
 */
void ADS_SetChannelGain(uint8_t channel, ADS_PGA_Gain gain);

/**
 * @brief Read back the configured gain for a specific channel.
 *
 * @param channel ADC channel index [0..3].
 * @return Gain setting for that channel.
 */
ADS_PGA_Gain ADS_GetChannelGain(uint8_t channel);

/**
 * @brief Set the same gain value on all channels.
 *
 * @param gain Desired gain for CH1–CH4.
 */
void ADS_SetAllChannelsGain(ADS_PGA_Gain gain);

// Data acquisition

/**
 * @brief Check whether fresh data is available from the ADC.
 *
 * @return true if /DRDY indicates data ready, false otherwise.
 */
bool ADS_IsDataReady(void);

/**
 * @brief Read all four channels in one transfer.
 *
 * @param[out] ch1 Pointer to receive CH1 sample.
 * @param[out] ch2 Pointer to receive CH2 sample.
 * @param[out] ch3 Pointer to receive CH3 sample.
 * @param[out] ch4 Pointer to receive CH4 sample.
 */
void ADS_ReadAllChannels(int32_t *ch1, int32_t *ch2, int32_t *ch3, int32_t *ch4);

// Utility functions

/**
 * @brief Send a standalone command word to the ADS131M04.
 *
 * Used for RESET, STANDBY, WAKEUP, LOCK/UNLOCK, etc.
 */
void ADS_SendCommand(uint16_t command);

/**
 * @brief Transfer one 16-bit word over the SPI bus.
 *
 * @param tx_data Word to transmit.
 * @param[out] rx_data Pointer to receive the word returned by the ADC.
 */
void ADS_TransferWord(uint16_t tx_data, uint16_t *rx_data);

/**
 * @brief Convert a raw ADC code to voltage using the configured FSR.
 *
 * @param adc_value Raw signed 24-bit (or 32-bit-aligned) ADC code.
 * @param vref      Reference voltage in volts.
 *
 * @return Converted voltage in volts.
 */
float ADS_ToVoltage(int32_t adc_value, float vref);

#endif // ADS131M04_DRIVER_H_
