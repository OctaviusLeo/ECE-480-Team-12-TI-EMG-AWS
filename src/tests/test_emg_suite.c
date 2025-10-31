// //*****************************************************************************
// //
// // blinky.c - Simple example to blink the on-board LED.
// //
// // Copyright (c) 2012-2020 Texas Instruments Incorporated.  All rights reserved.
// // Software License Agreement
// // 
// // Texas Instruments (TI) is supplying this software for use solely and
// // exclusively on TI's microcontroller products. The software is owned by
// // TI and/or its suppliers, and is protected under applicable copyright
// // laws. You may not combine this software with "viral" open-source
// // software in order to form a larger program.
// // 
// // THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// // NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// // NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// // A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// // CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// // DAMAGES, FOR ANY REASON WHATSOEVER.
// // 
// // This is part of revision 2.2.0.295 of the EK-TM4C123GXL Firmware Package.
// //
// //*****************************************************************************

// #include <stdint.h>
// #include <stdbool.h>
// #include "inc/hw_memmap.h"
// #include "driverlib/debug.h"
// #include "driverlib/gpio.h"
// #include "driverlib/sysctl.h"

// //*****************************************************************************
// //
// //! \addtogroup example_list
// //! <h1>Blinky (blinky)</h1>
// //!
// //! A very simple example that blinks the on-board LED using direct register
// //! access.
// //
// //*****************************************************************************

// //*****************************************************************************
// //
// // The error routine that is called if the driver library encounters an error.
// //
// //*****************************************************************************
// #ifdef DEBUG
// void
// __error__(char *pcFilename, uint32_t ui32Line)
// {
//     while(1);
// }
// #endif

// //*****************************************************************************
// //
// // Blink the on-board LED.
// //
// //*****************************************************************************
// int
// main(void)
// {
//     volatile uint32_t ui32Loop;

//     //
//     // Enable the GPIO port that is used for the on-board LED.
//     //
//     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

//     //
//     // Check if the peripheral access is enabled.
//     //
//     while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
//     {
//     }

//     //
//     // Enable the GPIO pin for the LED (PF3).  Set the direction as output, and
//     // enable the GPIO pin for digital function.
//     //
//     GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);

//     //
//     // Loop forever.
//     //
//     while(1)
//     {
//         //
//         // Turn on the LED.
//         //
//         GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);

//         //
//         // Delay for a bit.
//         //
//         for(ui32Loop = 0; ui32Loop < 200000; ui32Loop++)
//         {
//         }

//         //
//         // Turn off the LED.
//         //
//         GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x0);

//         //
//         // Delay for a bit.
//         //
//         for(ui32Loop = 0; ui32Loop < 200000; ui32Loop++)
//         {
//         }
//     }
// }



// main.c
// Test harness for signal processing without hardware

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "signal_processing.h"
#include "circular_buffer.h"
#include "test_signal_gen.h"

// Simulated UART output (won't work without hardware, but compiles)
#define UARTprintf printf

//============================================================================
// GLOBAL STATE
//============================================================================

MovingAverageFilter filter_ch1;
CircularBuffer buffer_ch1;
SignalGenerator sig_gen;

//============================================================================
// TEST FUNCTIONS
//============================================================================

/**
 * Test 1: Filter response to step input
 */
void Test_FilterStepResponse(void) 
{
    UARTprintf("\n=== TEST 1: Filter Step Response ===\n");
    
    Filter_Init(&filter_ch1);
    
    // Feed constant signal
    int32_t input = 5000000;  // 5M ADC units
    
    UARTprintf("Feeding constant 5M signal...\n");
    for(int i = 0; i < 200; i++) 
    {
        int32_t output = Filter_Update(&filter_ch1, input);
        
        if(i % 20 == 0) 
        {  
            // Print every 20 samples
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
    
    Buffer_Init(&buffer_ch1);
    
    // Write more than capacity
    uint16_t writes = BUFFER_CAPACITY + 50;
    UARTprintf("Writing %d samples (capacity = %d)\n", writes, BUFFER_CAPACITY);
    
    for(uint16_t i = 0; i < writes; i++) 
    {
        Buffer_Write(&buffer_ch1, i);
    }
    
    // Check oldest and newest
    int32_t newest = Buffer_ReadRelative(&buffer_ch1, 0);
    int32_t oldest = Buffer_ReadRelative(&buffer_ch1, BUFFER_CAPACITY - 1);
    
    UARTprintf("Newest value: %d (expected %d)\n", newest, writes - 1);
    UARTprintf("Oldest value: %d (expected %d)\n", oldest, writes - BUFFER_CAPACITY);
    UARTprintf("Buffer is %s\n", Buffer_IsFull(&buffer_ch1) ? "FULL" : "not full");
}

/**
 * Test 3: Complete signal processing pipeline
 */
void Test_CompletePipeline(void) {
    UARTprintf("\n=== TEST 3: Complete Processing Pipeline ===\n");
    
    // Initialize all modules
    Filter_Init(&filter_ch1);
    Buffer_Init(&buffer_ch1);
    SigGen_Init(&sig_gen, SIGNAL_EMG_SIM, 4000000);
    
    UARTprintf("Simulating 2 seconds of EMG data (2000 samples)...\n");
    
    uint8_t decimation_counter = 0;
    
    // Simulate 2 seconds at 1kHz
    for(uint16_t i = 0; i < 2000; i++) 
    {
        // Get simulated ADC reading
        int32_t raw_sample = SigGen_GetNext(&sig_gen);
        
        // Rectify
        int32_t rectified = SignalProc_Rectify(raw_sample);
        
        // Filter
        int32_t filtered = Filter_Update(&filter_ch1, rectified);
        
        // Decimate and store
        decimation_counter++;
        if(decimation_counter >= DECIMATION_FACTOR) 
        {
            decimation_counter = 0;
            Buffer_Write(&buffer_ch1, filtered);
        }
        
        // Print periodically
        if(i % 500 == 0) 
        {
            UARTprintf("Sample %d: Raw=%d, Rect=%d, Filt=%d\n", 
                      i, raw_sample, rectified, filtered);
        }
    }
    
    // Calculate final statistics
    int32_t avg = Buffer_GetAverage(&buffer_ch1, 0);  // All samples
    int32_t max = Buffer_GetMax(&buffer_ch1, 0);
    int32_t min = Buffer_GetMin(&buffer_ch1, 0);
    
    UARTprintf("\n--- Results ---\n");
    UARTprintf("Samples stored: %d\n", Buffer_GetCount(&buffer_ch1));
    UARTprintf("Average: %d ADC units\n", avg);
    UARTprintf("Max: %d\n", max);
    UARTprintf("Min: %d\n", min);
    UARTprintf("Voltage: %.3f V\n", SignalProc_ADCToVoltage(avg, 2.5f));
}

/**
 * Test 4: Different signal types
 */
void Test_SignalTypes(void) 
{
    UARTprintf("\n=== TEST 4: Signal Generator Types ===\n");
    
    SignalType types[] = {SIGNAL_SINE, SIGNAL_SQUARE, SIGNAL_EMG_SIM};
    const char* names[] = {"SINE", "SQUARE", "EMG_SIM"};
    
    for(int t = 0; t < 3; t++) 
    {
        UARTprintf("\nTesting %s signal:\n", names[t]);
        
        SigGen_Init(&sig_gen, types[t], 3000000);
        Filter_Init(&filter_ch1);
        
        // Process 100 samples
        int32_t sum = 0;
        for(int i = 0; i < 100; i++) 
        {
            int32_t raw = SigGen_GetNext(&sig_gen);
            int32_t rect = SignalProc_Rectify(raw);
            int32_t filt = Filter_Update(&filter_ch1, rect);
            sum += filt;
            
            if(i < 5) 
            {  
                // Show first few
                UARTprintf("  Sample %d: Raw=%d, Filtered=%d\n", i, raw, filt);
            }
        }
        
        UARTprintf("  Average (filtered): %d\n", sum / 100);
    }
}

//============================================================================
// MAIN PROGRAM
//============================================================================

int main(void) 
{
    UARTprintf("\n");
    UARTprintf("=====================================\n");
    UARTprintf("EMG Signal Processing Test Suite\n");
    UARTprintf("Running without hardware\n");
    UARTprintf("=====================================\n");
    
    // Run all tests
    Test_FilterStepResponse();
    Test_BufferWraparound();
    Test_CompletePipeline();
    Test_SignalTypes();
    
    UARTprintf("\n=====================================\n");
    UARTprintf("All tests complete!\n");
    UARTprintf("=====================================\n");
    
    while(1) 
    {
        // Infinite loop 
    }
}