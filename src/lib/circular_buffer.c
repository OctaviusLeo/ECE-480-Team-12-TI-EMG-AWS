// circular_buffer.c
#include "circular_buffer.h"

//============================================================================
// CORE OPERATIONS
//============================================================================

/**
 * Initialize circular buffer
 */
void Buffer_Init(CircularBuffer *buf) 
{
    // Zero memory initialization
    for(uint16_t i = 0; i < BUFFER_CAPACITY; i++) {
        buf->data[i] = 0;
    }
    
    // Zero variable initialization
    buf->write_index = 0;
    buf->count = 0;
    buf->total_written = 0;
    buf->timestamp_ms = 0;
}

/**
 * Writes sample to buffer
 * Automatically overwrites oldest when full
 * 
 * @param buf: Buffer pointer
 * @param sample: Data to store
 */
void Buffer_Write(CircularBuffer *buf, int32_t sample)
 {
    // Write at current position
    buf->data[buf->write_index] = sample;
    
    // Advance write pointer (modulo for wraparound)
    buf->write_index = (buf->write_index + 1) % BUFFER_CAPACITY;
    
    // Update count (saturates at capacity)
    if(buf->count < BUFFER_CAPACITY) 
    {
        buf->count++;
    }
    
    // Track total writes (for debugging/statistics)
    buf->total_written++;
}

/**
 * Read sample by absolute index
 * 
 * @param buf: Buffer pointer
 * @param index: Position (0 to capacity-1)
 * @return: Sample value
 * 
 * WARNING: Does not account for wraparound!
 * Use Buffer_ReadRelative() for time-based access
 */
int32_t Buffer_Read(const CircularBuffer *buf, uint16_t index) 
{
    // Out of bounds check
    if(index >= BUFFER_CAPACITY) 
    {
        return 0;  
    }
    return buf->data[index];
}

/**
 * Read sample relative to current position
 * 
 * @param buf: Buffer pointer
 * @param samples_ago: How many samples back (0 = most recent)
 * @return: Sample value
 * 
 * EXAMPLE: samples_ago=0 returns last written sample
 *          samples_ago=10 returns sample 10 writes ago
 */
int32_t Buffer_ReadRelative(const CircularBuffer *buf, uint16_t samples_ago) 
{
    if(samples_ago >= buf->count) 
    {
        return 0;  // Asking for data that doesn't exist yet
    }
    
    // Calculate read position with wraparound
    int32_t read_index = (int32_t)buf->write_index - samples_ago - 1;
    
    // Handle negative wraparound
    while(read_index < 0) 
    {
        read_index += BUFFER_CAPACITY;
    }
    
    return buf->data[read_index];
}

//============================================================================
// QUERY FUNCTIONS
//============================================================================

/**
 * Check if buffer is full
 */
bool Buffer_IsFull(const CircularBuffer *buf) 
{
    return (buf->count >= BUFFER_CAPACITY);
}

/**
 * Get number of valid samples in buffer
 */
uint16_t Buffer_GetCount(const CircularBuffer *buf) 
{
    return buf->count;
}

/**
 * Get buffer capacity (constant)
 */
uint16_t Buffer_GetCapacity(void) 
{
    return BUFFER_CAPACITY;
}

//============================================================================
// ANALYSIS FUNCTIONS
//============================================================================

/**
 * Calculate average over last N samples
 * 
 * @param buf: Buffer pointer
 * @param num_samples: How many samples to average (0 = all)
 * @return: Average value
 */
int32_t Buffer_GetAverage(const CircularBuffer *buf, uint16_t num_samples) 
{
    // Default to all samples if 0 specified
    if(num_samples == 0 || num_samples > buf->count) 
    {
        num_samples = buf->count;
    }
    
    if(num_samples == 0) return 0;
    
    // Sum last N samples
    int64_t sum = 0;
    for(uint16_t i = 0; i < num_samples; i++) 
    {
        sum += Buffer_ReadRelative(buf, i);
    }
    
    return (int32_t)(sum / num_samples);
}

/**
 * Find maximum value in last N samples
 */
int32_t Buffer_GetMax(const CircularBuffer *buf, uint16_t num_samples) 
{
    if(num_samples == 0 || num_samples > buf->count) 
    {
        num_samples = buf->count;
    }
    
    if(num_samples == 0) return 0;
    
    int32_t max = Buffer_ReadRelative(buf, 0);
    for(uint16_t i = 1; i < num_samples; i++) 
    {
        int32_t val = Buffer_ReadRelative(buf, i);
        if(val > max) max = val;
    }
    
    return max;
}

/**
 * Find minimum value in last N samples
 */
int32_t Buffer_GetMin(const CircularBuffer *buf, uint16_t num_samples) 
{
    if(num_samples == 0 || num_samples > buf->count) 
    {
        num_samples = buf->count;
    }
    
    if(num_samples == 0) return 0;
    
    int32_t min = Buffer_ReadRelative(buf, 0);
    for(uint16_t i = 1; i < num_samples; i++) 
    {
        int32_t val = Buffer_ReadRelative(buf, i);
        if(val < min) min = val;
    }
    
    return min;
}