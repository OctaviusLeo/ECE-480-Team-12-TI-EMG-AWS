/**
 * main.c
 * Complete EMG System with Test Suite + Full Signal Processing
 * 
 * Combines:
 * - Software test suite (filters, buffers, signal generators)
 * - Hardware ADC acquisition with complete EMG pipeline
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// TivaWare includes
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

// Custom modules
#include "ads131m04_driver.h"
#include "emg_processing.h"

// Old modules (for test suite)
#include "signal_processing.h"
#include "circular_buffer.h"
#include "test_signal_gen.h"

// CONFIGURATION FLAGS
#define RUN_HARDWARE_TEST   1  // 1 = Real ADC acquisition, 0 = Skip
#define ENABLE_TEST_SUITE   0  // 1 = Run software tests first, 0 = Skip
#define LED_FEEDBACK_ENABLE 1  // 1 = Use LEDs for activation, 0 = No LEDs


// GLOBAL STATE
// New EMG processors
EMGProcessor emg_ch1;
EMGProcessor emg_ch2;

// Old modules (for test suite)
MovingAverageFilter filter_ch1_old;
MovingAverageFilter filter_ch2_old;
CircularBuffer buffer_ch1_old;
CircularBuffer buffer_ch2_old;
SignalGenerator sig_gen;

// LED CONFIGURATION
void ConfigureLED(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)) {}
    
    // Red LED = PF1, Blue LED = PF2, Green LED = PF3
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    
    // All off initially
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0);
}

void LED_SetActivation(bool ch1_active, bool ch2_active) {
    uint8_t led_state = 0;
    
    if(ch1_active) led_state |= GPIO_PIN_1;  // Red for CH1
    if(ch2_active) led_state |= GPIO_PIN_3;  // Green for CH2
    
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, led_state);
}

// UART CONFIGURATION
void ConfigureUART(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    
    UARTStdioConfig(0, 115200, SysCtlClockGet());
}

/**
 * Test 1: Filter response to step input
 */
void Test_FilterStepResponse(void) {
    UARTprintf("\n=== TEST 1: Filter Step Response ===\n");
    
    Filter_Init(&filter_ch1_old);
    
    int32_t input = 5000000;  // 5M ADC units
    
    UARTprintf("Feeding constant 5M signal...\n");
    for(int i = 0; i < 200; i++) {
        int32_t output = Filter_Update(&filter_ch1_old, input);
        
        if(i % 20 == 0) {
            UARTprintf("Sample %d: Output = %d\n", i, output);
        }
    }
    
    UARTprintf("Filter should converge to 5M\n");
}

/**
 * Test 2: Circular buffer wraparound
 */
void Test_BufferWraparound(void) {
    UARTprintf("\n=== TEST 2: Circular Buffer Wraparound ===\n");
    
    Buffer_Init(&buffer_ch1_old);
    
    uint16_t writes = BUFFER_CAPACITY + 50;
    UARTprintf("Writing %d samples (capacity = %d)\n", writes, BUFFER_CAPACITY);
    
    for(uint16_t i = 0; i < writes; i++) {
        Buffer_Write(&buffer_ch1_old, i);
    }
    
    int32_t newest = Buffer_ReadRelative(&buffer_ch1_old, 0);
    int32_t oldest = Buffer_ReadRelative(&buffer_ch1_old, BUFFER_CAPACITY - 1);
    
    UARTprintf("Newest value: %d (expected %d)\n", newest, writes - 1);
    UARTprintf("Oldest value: %d (expected %d)\n", oldest, writes - BUFFER_CAPACITY);
    UARTprintf("Buffer is %s\n", Buffer_IsFull(&buffer_ch1_old) ? "FULL" : "not full");
}

/**
 * Test 3: Complete signal processing pipeline
 */
void Test_CompletePipeline(void) {
    UARTprintf("\n=== TEST 3: Complete Processing Pipeline ===\n");
    
    Filter_Init(&filter_ch1_old);
    Buffer_Init(&buffer_ch1_old);
    SigGen_Init(&sig_gen, SIGNAL_EMG_SIM, 4000000);
    
    UARTprintf("Simulating 2 seconds of EMG data (2000 samples)...\n");
    
    uint8_t decimation_counter = 0;
    
    for(uint16_t i = 0; i < 2000; i++) {
        int32_t raw_sample = SigGen_GetNext(&sig_gen);
        int32_t rectified = SignalProc_Rectify(raw_sample);
        int32_t filtered = Filter_Update(&filter_ch1_old, rectified);
        
        decimation_counter++;
        if(decimation_counter >= DECIMATION_FACTOR) {
            decimation_counter = 0;
            Buffer_Write(&buffer_ch1_old, filtered);
        }
        
        if(i % 500 == 0) {
            UARTprintf("Sample %d: Raw=%d, Rect=%d, Filt=%d\n", 
                      i, raw_sample, rectified, filtered);
        }
    }
    
    int32_t avg = Buffer_GetAverage(&buffer_ch1_old, 0);
    int32_t max = Buffer_GetMax(&buffer_ch1_old, 0);
    int32_t min = Buffer_GetMin(&buffer_ch1_old, 0);
    
    UARTprintf("\n--- Results ---\n");
    UARTprintf("Samples stored: %d\n", Buffer_GetCount(&buffer_ch1_old));
    UARTprintf("Average: %d ADC units\n", avg);
    UARTprintf("Max: %d\n", max);
    UARTprintf("Min: %d\n", min);
}

/**
 * Test 4: Different signal types
 */
void Test_SignalTypes(void) {
    UARTprintf("\n=== TEST 4: Signal Generator Types ===\n");
    
    SignalType types[] = {SIGNAL_SINE, SIGNAL_SQUARE, SIGNAL_EMG_SIM};
    const char* names[] = {"SINE", "SQUARE", "EMG_SIM"};
    
    for(int t = 0; t < 3; t++) {
        UARTprintf("\nTesting %s signal:\n", names[t]);
        
        SigGen_Init(&sig_gen, types[t], 3000000);
        Filter_Init(&filter_ch1_old);
        
        int32_t sum = 0;
        for(int i = 0; i < 100; i++) {
            int32_t raw = SigGen_GetNext(&sig_gen);
            int32_t rect = SignalProc_Rectify(raw);
            int32_t filt = Filter_Update(&filter_ch1_old, rect);
            sum += filt;
            
            if(i < 5) {
                UARTprintf("  Sample %d: Raw=%d, Filtered=%d\n", i, raw, filt);
            }
        }
        
        UARTprintf("  Average (filtered): %d\n", sum / 100);
    }
}

/**
 * Run all software tests
 */
void Run_TestSuite(void) {
    UARTprintf("\n");
    UARTprintf("╔═══════════════════════════════════════════╗\n");
    UARTprintf("║  SOFTWARE TEST SUITE                      ║\n");
    UARTprintf("╚═══════════════════════════════════════════╝\n");
    
    Test_FilterStepResponse();
    Test_BufferWraparound();
    Test_CompletePipeline();
    Test_SignalTypes();
    
    UARTprintf("\n");
    UARTprintf("╔═══════════════════════════════════════════╗\n");
    UARTprintf("║  ALL TESTS COMPLETE ✓                     ║\n");
    UARTprintf("╚═══════════════════════════════════════════╝\n");
}

// CALIBRATION PROCEDURE
bool PerformCalibration(void) {
    int32_t ch1_raw, ch2_raw, ch3_raw, ch4_raw;
    uint16_t samples_collected = 0;
    uint16_t progress_updates = 0;
    
    UARTprintf("\n");
    UARTprintf("╔══════════════════════════════════════════╗\n");
    UARTprintf("║        CALIBRATION PROCEDURE             ║\n");
    UARTprintf("╚══════════════════════════════════════════╝\n\n");
    
    UARTprintf("Instructions:\n");
    UARTprintf("  1. Relax all muscles\n");
    UARTprintf("  2. Remain still for 3 seconds\n");
    UARTprintf("  3. Calibration will measure baseline noise\n\n");
    
    UARTprintf("Starting in: ");
    for(int i = 3; i > 0; i--) {
        UARTprintf("%d... ", i);
        SysCtlDelay(SysCtlClockGet() / 3);
    }
    UARTprintf("GO!\n\n");
    
    // Estimate DC offset from first 100 samples
    int64_t dc_sum_ch1 = 0, dc_sum_ch2 = 0;
    for(int i = 0; i < 100; i++) {
        while(!ADS_IsDataReady()) {}
        ADS_ReadAllChannels(&ch1_raw, &ch2_raw, &ch3_raw, &ch4_raw);
        dc_sum_ch1 += ch1_raw;
        dc_sum_ch2 += ch2_raw;
    }
    
    int32_t dc_offset_ch1 = dc_sum_ch1 / 100;
    int32_t dc_offset_ch2 = dc_sum_ch2 / 100;
    
    UARTprintf("DC Offsets measured:\n");
    UARTprintf("  CH1: %d ADC units (%.3f V)\n", 
               dc_offset_ch1, 
               ADS_ToVoltage(dc_offset_ch1, ADS_FSR_TABLE[ADS_GAIN_8]));
    UARTprintf("  CH2: %d ADC units (%.3f V)\n\n", 
               dc_offset_ch2,
               ADS_ToVoltage(dc_offset_ch2, ADS_FSR_TABLE[ADS_GAIN_8]));
    
    // Initialize processors
    EMG_Init(&emg_ch1, dc_offset_ch1);
    EMG_Init(&emg_ch2, dc_offset_ch2);
    
    EMG_StartCalibration(&emg_ch1);
    EMG_StartCalibration(&emg_ch2);
    
    UARTprintf("Collecting baseline (3000 samples):\n[");
    
    // Collect calibration data
    while(samples_collected < EMG_CALIBRATION_SAMPLES) {
        if(ADS_IsDataReady()) {
            ADS_ReadAllChannels(&ch1_raw, &ch2_raw, &ch3_raw, &ch4_raw);
            
            bool ch1_done = EMG_CalibrateStep(&emg_ch1, ch1_raw);
            bool ch2_done = EMG_CalibrateStep(&emg_ch2, ch2_raw);
            
            samples_collected++;
            
            if(samples_collected % (EMG_CALIBRATION_SAMPLES / 30) == 0) {
                UARTprintf("█");
                progress_updates++;
            }
            
            if(ch1_done && ch2_done) {
                break;
            }
        }
    }
    
    while(progress_updates < 30) {
        UARTprintf("█");
        progress_updates++;
    }
    UARTprintf("] 100%%\n\n");
    
    // Display calibration results
    CalibrationResult cal_ch1 = EMG_GetCalibrationResult(&emg_ch1);
    CalibrationResult cal_ch2 = EMG_GetCalibrationResult(&emg_ch2);
    
    UARTprintf("✓ Calibration complete!\n\n");
    
    UARTprintf("Channel 1:\n");
    UARTprintf("  Baseline Mean:   %.3f mV\n", cal_ch1.baseline_mean * 1000.0f);
    UARTprintf("  Baseline StdDev: %.3f mV\n", cal_ch1.baseline_stddev * 1000.0f);
    UARTprintf("  Threshold:       %.3f mV\n", emg_ch1.activation_threshold * 1000.0f);
    
    UARTprintf("\nChannel 2:\n");
    UARTprintf("  Baseline Mean:   %.3f mV\n", cal_ch2.baseline_mean * 1000.0f);
    UARTprintf("  Baseline StdDev: %.3f mV\n", cal_ch2.baseline_stddev * 1000.0f);
    UARTprintf("  Threshold:       %.3f mV\n\n", emg_ch2.activation_threshold * 1000.0f);
    
    return (cal_ch1.success && cal_ch2.success);
}

// HARDWARE EMG ACQUISITION
void Run_EMG_Acquisition(void) {
    int32_t ch1_raw, ch2_raw, ch3_raw, ch4_raw;
    uint32_t sample_count = 0;
    uint32_t last_display_time = 0;
    
    UARTprintf("\n");
    UARTprintf("╔══════════════════════════════════════════╗\n");
    UARTprintf("║     EMG ACQUISITION STARTED              ║\n");
    UARTprintf("╚══════════════════════════════════════════╝\n\n");
    
    UARTprintf("Real-time muscle activation detection enabled!\n");
#if LED_FEEDBACK_ENABLE
    UARTprintf("LED feedback: Red=CH1, Green=CH2\n");
#endif
    UARTprintf("Press any key to stop.\n\n");
    
    // Print table header
    UARTprintf("Time  | CH1 Raw | CH1 Env | CH1 | CH2 Raw | CH2 Env | CH2 | Status\n");
    UARTprintf("------+---------+---------+-----+---------+---------+-----+--------\n");
    
    while(1) {
        if(ADS_IsDataReady()) {
            // Read ADC
            ADS_ReadAllChannels(&ch1_raw, &ch2_raw, &ch3_raw, &ch4_raw);
            sample_count++;
            
            // Process through complete pipeline
            float ch1_envelope = EMG_ProcessSample(&emg_ch1, ch1_raw);
            float ch2_envelope = EMG_ProcessSample(&emg_ch2, ch2_raw);
            
            // Get activation states
            bool ch1_active = emg_ch1.is_active;
            bool ch2_active = emg_ch2.is_active;
            
#if LED_FEEDBACK_ENABLE
            LED_SetActivation(ch1_active, ch2_active);
#endif
            
            // Display update (every 100ms)
            if((sample_count - last_display_time) >= 100) {
                last_display_time = sample_count;
                
                float v1_raw = ADS_ToVoltage(ch1_raw, ADS_FSR_TABLE[ADS_GAIN_8]);
                float v2_raw = ADS_ToVoltage(ch2_raw, ADS_FSR_TABLE[ADS_GAIN_8]);
                
                const char* status;
                if(ch1_active && ch2_active) {
                    status = "BOTH";
                } else if(ch1_active) {
                    status = "CH1 ";
                } else if(ch2_active) {
                    status = "CH2 ";
                } else {
                    status = "----";
                }
                
                UARTprintf("%4.1fs | %7d | %7.2f | %s | %7d | %7.2f | %s | %s\n",
                          sample_count / 1000.0f,
                          ch1_raw, ch1_envelope * 1000.0f, ch1_active ? "✓" : " ",
                          ch2_raw, ch2_envelope * 1000.0f, ch2_active ? "✓" : " ",
                          status);
            }
            
            // Periodic threshold update
            if(sample_count % 5000 == 0) {
                EMG_UpdateThreshold(&emg_ch1);
                EMG_UpdateThreshold(&emg_ch2);
            }
        }
        
        // Check for stop command
        if(UARTCharsAvail(UART0_BASE)) {
            UARTgetc();
            break;
        }
    }
    
    // Final statistics
    UARTprintf("\n");
    UARTprintf("╔══════════════════════════════════════════╗\n");
    UARTprintf("║     ACQUISITION STOPPED                  ║\n");
    UARTprintf("╚══════════════════════════════════════════╝\n\n");
    
    UARTprintf("Summary:\n");
    UARTprintf("  Total samples: %lu\n", sample_count);
    UARTprintf("  Duration: %.1f seconds\n", sample_count / 1000.0f);
    UARTprintf("  CH1 max envelope: %.2f mV\n", emg_ch1.max_envelope * 1000.0f);
    UARTprintf("  CH2 max envelope: %.2f mV\n\n", emg_ch2.max_envelope * 1000.0f);
    
    LED_SetActivation(false, false);
}


// MAIN PROGRAM
int main(void) {
    
    // System clock to 80 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | 
                   SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    
    // Initialize peripherals
    ConfigureUART();
    ConfigureLED();
    
    // Startup banner
    UARTprintf("\n\n");
    UARTprintf("╔═══════════════════════════════════════════════════╗\n");
    UARTprintf("║                                                   ║\n");
    UARTprintf("║    ADVANCED EMG SIGNAL PROCESSING SYSTEM          ║\n");
    UARTprintf("║    TM4C123 + ADS131M04                            ║\n");
    UARTprintf("║                                                   ║\n");
    UARTprintf("║    Features:                                      ║\n");
    UARTprintf("║    • DC offset removal                            ║\n");
    UARTprintf("║    • High-pass filtering (cardiac removal)        ║\n");
    UARTprintf("║    • 60 Hz notch filter                           ║\n");
    UARTprintf("║    • Adaptive baseline subtraction                ║\n");
    UARTprintf("║    • Real-time activation detection               ║\n");
    UARTprintf("║                                                   ║\n");
    UARTprintf("╚═══════════════════════════════════════════════════╝\n\n");
    
    UARTprintf("System clock: %d MHz\n", SysCtlClockGet() / 1000000);
    UARTprintf("Build: %s %s\n\n", __DATE__, __TIME__);
    
    //------------------------------------------------------------------------
    // RUN SOFTWARE TEST SUITE (if enabled)
    //------------------------------------------------------------------------
    
#if ENABLE_TEST_SUITE
    UARTprintf(">> Running software test suite...\n");
    Run_TestSuite();
    
    UARTprintf("\n>> Press any key to continue to hardware test...\n");
    while(!UARTCharsAvail(UART0_BASE)) {
        SysCtlDelay(1000);
    }
    UARTgetc();
#endif
    
    // RUN HARDWARE ACQUISITION (if enabled)    
#if RUN_HARDWARE_TEST
    
    // Initialize ADC
    UARTprintf("Initializing ADS131M04 ADC...\n");
    ADS_Init();
    
    uint16_t device_id = ADS_ReadRegister(ADS_REG_ID);
    UARTprintf("Device ID: 0x%04X ", device_id);
    
    if(device_id != 0x0004) {
        UARTprintf("❌ ERROR!\n");
        UARTprintf("Check hardware connections.\n");
        while(1);
    }
    UARTprintf("✓\n");
    
    // Configure PGA
    ADS_SetChannelGain(1, ADS_GAIN_8);
    ADS_SetChannelGain(2, ADS_GAIN_8);
    UARTprintf("PGA configured: GAIN_8 (FSR = ±150 mV)\n");
    
    // Wait for ADC ready
    UARTprintf("Waiting for ADC ready...\n");
    uint32_t timeout = 0;
    while(!ADS_IsDataReady() && timeout < 1000000) {
        timeout++;
        SysCtlDelay(100);
    }
    
    if(timeout >= 1000000) {
        UARTprintf("❌ DRDY timeout!\n");
        while(1);
    }
    UARTprintf("✓ ADC ready\n\n");
    
    // Calibration
    if(!PerformCalibration()) {
        UARTprintf("❌ Calibration failed!\n");
        while(1);
    }
    
    UARTprintf("Press any key to start acquisition...\n");
    while(!UARTCharsAvail(UART0_BASE)) {
        SysCtlDelay(1000);
    }
    UARTgetc();
    
    // Main acquisition
    Run_EMG_Acquisition();
    
#else
    UARTprintf("\nHardware acquisition disabled.\n");
    UARTprintf("Set RUN_HARDWARE_TEST=1 to enable.\n");
#endif
    
    //------------------------------------------------------------------------
    // IDLE
    //------------------------------------------------------------------------
    
    UARTprintf("\nSystem halted. Reset to restart.\n");
    
    while(1) {
        // Idle
    }
}

// DEBUG ERROR HANDLER
#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line) {
    UARTprintf("Error in %s, line %d\n", pcFilename, ui32Line);
    while(1);
}
#endif