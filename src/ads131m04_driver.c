/**
 * ads131m04_driver.c
 * Implementation of ADS131M04 driver - FINAL CORRECTED VERSION
 */

#include "ads131m04_driver.h"

// TivaWare includes
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"

//============================================================================
// FSR LOOKUP TABLE (✅ ADDED)
//============================================================================

const float ADS_FSR_TABLE[8] = {
    1.2f,      // GAIN_1   = ±1.2V
    0.6f,      // GAIN_2   = ±600mV
    0.3f,      // GAIN_4   = ±300mV
    0.15f,     // GAIN_8   = ±150mV
    0.075f,    // GAIN_16  = ±75mV
    0.0375f,   // GAIN_32  = ±37.5mV
    0.01875f,  // GAIN_64  = ±18.75mV
    0.009375f  // GAIN_128 = ±9.375mV
};

//============================================================================
// TIMING MACROS
//============================================================================

#define DELAY_US(x) SysCtlDelay((SysCtlClockGet() / 3000000) * (x))
#define DELAY_MS(x) SysCtlDelay((SysCtlClockGet() / 3000) * (x))

// CS control
#define CS_LOW()    GPIOPinWrite(ADS_GPIO_PORT, ADS_CS_PIN, 0)
#define CS_HIGH()   GPIOPinWrite(ADS_GPIO_PORT, ADS_CS_PIN, ADS_CS_PIN)

//============================================================================
// INITIALIZATION
//============================================================================

/**
 * Initialize ADS131M04 interface
 * Configures SPI, GPIO, PWM, and ADC registers
 */
void ADS_Init(void) {
    uint32_t temp;
    
    // Enable peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    
    // Wait for peripherals
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_SSI0)) {}
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0)) {}
    
    // Configure SPI pins (PA2, PA4, PA5)
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);
    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5);
    
    // Configure CS pin (PA3) as GPIO output
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_3);
    CS_HIGH();
    
    // Configure DRDY pin (PB0) as input
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_0);
    
    // Configure RESET pin (PB1) as output
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_1);
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_PIN_1);  // Idle high
    
    // Configure SSI0: SPI Mode 1, 1 MHz, 16-bit data
    SSIConfigSetExpClk(SSI0_BASE,
                       SysCtlClockGet(),
                       SSI_FRF_MOTO_MODE_1,  // CPOL=0, CPHA=1
                       SSI_MODE_MASTER,
                       1000000,              // 1 MHz
                       16);                  // 16-bit
    
    SSIEnable(SSI0_BASE);
    
    // Flush RX FIFO
    while(SSIDataGetNonBlocking(SSI0_BASE, &temp)) {}
    
    // Generate 8 MHz clock on CLKIN (acceptable tolerance from 8.192 MHz)
    ADS_GenerateClockSignal();
    
    // Hardware reset
    ADS_HardwareReset();
    DELAY_MS(10);
    
    // Send NULL command to clear any pending data
    ADS_SendCommand(ADS_CMD_NULL);
    DELAY_MS(1);
    
    // Unlock registers (required before writing)
    ADS_SendCommand(ADS_CMD_UNLOCK);
    DELAY_MS(1);
    
    // ✅ CRITICAL FIX: Configure MODE register for continuous conversion
    // Bit 0: DRDY_SEL = 1 (DRDY pulses after most recent channel)
    // All other bits = 0 (defaults: 24-bit mode, CRC disabled, etc.)
    uint16_t mode_config = 0x0001;
    ADS_WriteRegister(ADS_REG_MODE, mode_config);
    DELAY_MS(1);
    
    // ✅ CRITICAL FIX: Configure CLOCK register
    // Use default settings for high-resolution mode
    ADS_WriteRegister(ADS_REG_CLOCK, 0x000E);
    DELAY_MS(1);
    
    // Verify communication - read device ID
    uint16_t device_id = ADS_ReadRegister(ADS_REG_ID);
    // Expected: 0x0004 for ADS131M04
    // Note: Could add error handling here if device_id != 0x0004
}

/**
 * Generate 8 MHz clock signal for CLKIN
 * Uses PWM on PB6 (M0PWM0)
 * 
 * NOTE: Generates 8 MHz instead of 8.192 MHz due to integer division
 *       This is within acceptable tolerance (±10%) for ADS131M04
 */
void ADS_GenerateClockSignal(void) {
    // Configure PB6 as PWM output
    GPIOPinConfigure(GPIO_PB6_M0PWM0);
    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);
    
    // Configure PWM generator
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0,
                    PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    
    // Calculate period for ~8 MHz from 80 MHz system clock
    // 80 MHz / 10 = 8 MHz (acceptable approximation of 8.192 MHz)
    uint32_t period = 10;
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, period);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, period / 2);  // 50% duty
    
    // Enable PWM output
    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, true);
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);
}

/**
 * Hardware reset via RESET pin
 */
void ADS_HardwareReset(void) {
    GPIOPinWrite(ADS_RESET_PORT, ADS_RESET_PIN, 0);  // Low
    DELAY_US(10);
    GPIOPinWrite(ADS_RESET_PORT, ADS_RESET_PIN, ADS_RESET_PIN);  // High
    DELAY_MS(5);
}

//============================================================================
// LOW-LEVEL SPI
//============================================================================

/**
 * Transfer one 16-bit word via SPI
 */
void ADS_TransferWord(uint16_t tx_data, uint16_t *rx_data) {
    uint32_t temp;
    
    SSIDataPut(SSI0_BASE, tx_data);
    while(SSIBusy(SSI0_BASE)) {}
    SSIDataGet(SSI0_BASE, &temp);
    
    if(rx_data != NULL) {
        *rx_data = (uint16_t)(temp & 0xFFFF);
    }
}

/**
 * Send command to ADS131M04
 */
void ADS_SendCommand(uint16_t command) {
    uint16_t response;
    
    CS_LOW();
    DELAY_US(1);
    
    ADS_TransferWord(command, &response);
    
    DELAY_US(1);
    CS_HIGH();
}

//============================================================================
// REGISTER ACCESS
//============================================================================

/**
 * Read register from ADS131M04
 */
uint16_t ADS_ReadRegister(uint8_t reg_addr) {
    uint16_t command, response, reg_value;
    
    command = ADS_CMD_RREG(reg_addr, 0);  // Read 1 register
    
    CS_LOW();
    DELAY_US(1);
    
    ADS_TransferWord(command, &response);
    ADS_TransferWord(ADS_CMD_NULL, &reg_value);
    
    DELAY_US(1);
    CS_HIGH();
    
    return reg_value;
}

/**
 * Write register to ADS131M04
 */
void ADS_WriteRegister(uint8_t reg_addr, uint16_t value) {
    uint16_t command, response;
    
    command = ADS_CMD_WREG(reg_addr, 0);  // Write 1 register
    
    CS_LOW();
    DELAY_US(1);
    
    ADS_TransferWord(command, &response);
    ADS_TransferWord(value, &response);
    
    DELAY_US(1);
    CS_HIGH();
    
    DELAY_US(10);  // Allow register write to complete
}

//============================================================================
// PGA CONFIGURATION
//============================================================================

/**
 * Set PGA gain for specific channel
 */
void ADS_SetChannelGain(uint8_t channel, ADS_PGA_Gain gain) {
    if(channel < 1 || channel > 4) return;
    
    uint8_t reg_addr = ADS_REG_GAIN1 + (channel - 1);
    ADS_WriteRegister(reg_addr, (uint16_t)gain);
}

/**
 * Get current PGA gain for channel
 */
ADS_PGA_Gain ADS_GetChannelGain(uint8_t channel) {
    if(channel < 1 || channel > 4) return ADS_GAIN_1;
    
    uint8_t reg_addr = ADS_REG_GAIN1 + (channel - 1);
    uint16_t reg_value = ADS_ReadRegister(reg_addr);
    
    return (ADS_PGA_Gain)(reg_value & 0x0007);
}

/**
 * Set all channels to same gain
 */
void ADS_SetAllChannelsGain(ADS_PGA_Gain gain) {
    for(uint8_t ch = 1; ch <= 4; ch++) {
        ADS_SetChannelGain(ch, gain);
    }
}

//============================================================================
// DATA ACQUISITION
//============================================================================

/**
 * Check if new data is ready
 */
bool ADS_IsDataReady(void) {
    return (GPIOPinRead(ADS_DRDY_PORT, ADS_DRDY_PIN) == 0);
}

/**
 * Read all four channels
 */
void ADS_ReadAllChannels(int32_t *ch1, int32_t *ch2, int32_t *ch3, int32_t *ch4) {
    uint16_t status_word;
    uint16_t word_high, word_low;
    int32_t channel_data[4];
    
    CS_LOW();
    DELAY_US(1);
    
    // Send NULL command and receive status
    ADS_TransferWord(ADS_CMD_NULL, &status_word);
    
    // Read each channel (24 bits = 1.5 words at 16-bit transfers)
    for(int ch = 0; ch < 4; ch++) {
        // Read high 16 bits
        ADS_TransferWord(0x0000, &word_high);
        
        // Read low 8 bits (in upper byte of next word)
        ADS_TransferWord(0x0000, &word_low);
        
        // Combine: bits[23:8] from word_high, bits[7:0] from upper byte of word_low
        int32_t raw = ((int32_t)word_high << 8) | ((word_low >> 8) & 0xFF);
        
        // Sign extend from 24-bit to 32-bit
        if(raw & 0x00800000) {
            channel_data[ch] = raw | 0xFF000000;
        } else {
            channel_data[ch] = raw & 0x00FFFFFF;
        }
    }
    
    DELAY_US(1);
    CS_HIGH();
    
    // Output
    if(ch1) *ch1 = channel_data[0];
    if(ch2) *ch2 = channel_data[1];
    if(ch3) *ch3 = channel_data[2];
    if(ch4) *ch4 = channel_data[3];
}

//============================================================================
// UTILITY
//============================================================================

/**
 * Convert ADC value to voltage
 * 
 * @param adc_value: Raw 24-bit ADC reading
 * @param vref: Reference voltage (use ADS_FSR_TABLE[gain] for correct value)
 * @return: Voltage in volts
 */
float ADS_ToVoltage(int32_t adc_value, float vref) {
    const float ADC_MAX = 8388607.0f;  // 2^23 - 1
    return ((float)adc_value / ADC_MAX) * vref;
}