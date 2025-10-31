#ifndef CIRCULAR_BUFFER_H_
#define CIRCULAR_BUFFER_H_

#include <stdint.h>
#include <stdbool.h>

#define BUFFER_SECONDS          15      // Storage duration
#define STORAGE_RATE_HZ         100     // Decimated storage rate
#define BUFFER_CAPACITY         (BUFFER_SECONDS * STORAGE_RATE_HZ)  // 1500

/**
 * Circular Buffer for Time-Series Data
 * Fixed-size, auto-overwriting oldest data
 */
typedef struct 
{
    int32_t data[BUFFER_CAPACITY];  
    uint16_t write_index;          
    uint16_t count;                 
    uint32_t total_written;          
    uint32_t timestamp_ms;      
} CircularBuffer;

// Core operations
void Buffer_Init(CircularBuffer *buf);
void Buffer_Write(CircularBuffer *buf, int32_t sample);
int32_t Buffer_Read(const CircularBuffer *buf, uint16_t index);
int32_t Buffer_ReadRelative(const CircularBuffer *buf, uint16_t samples_ago);

// Query functions
bool Buffer_IsFull(const CircularBuffer *buf);
uint16_t Buffer_GetCount(const CircularBuffer *buf);
uint16_t Buffer_GetCapacity(void);

// Analysis functions
int32_t Buffer_GetAverage(const CircularBuffer *buf, uint16_t num_samples);
int32_t Buffer_GetMax(const CircularBuffer *buf, uint16_t num_samples);
int32_t Buffer_GetMin(const CircularBuffer *buf, uint16_t num_samples);

#endif 