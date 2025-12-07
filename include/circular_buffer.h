/**
 * @file circular_buffer.h
 * @brief Fixed-size circular buffer for EMG time-series data.
 *
 * Stores a sliding window of decimated samples (100 Hz, 15 seconds),
 * overwriting the oldest data when full and supporting simple queries
 * like average, min, and max over recent samples.
 */

#ifndef CIRCULAR_BUFFER_H_
#define CIRCULAR_BUFFER_H_

#include <stdint.h>
#include <stdbool.h>

/** Storage duration in seconds. */
#define BUFFER_SECONDS          15      // Storage duration
/** Decimated storage rate in Hz. */
#define STORAGE_RATE_HZ         100     // Decimated storage rate
/** Total buffer capacity in samples. */
#define BUFFER_CAPACITY         (BUFFER_SECONDS * STORAGE_RATE_HZ)  // 1500

/**
 * @brief Circular buffer for time-series data.
 *
 * Fixed-size, auto-overwriting oldest data when new samples arrive past
 * capacity.
 */
typedef struct 
{
    int32_t  data[BUFFER_CAPACITY];  ///< Sample storage.
    uint16_t write_index;            ///< Index where next sample will be written.
    uint16_t count;                  ///< Number of valid samples (<= BUFFER_CAPACITY).
    uint32_t total_written;          ///< Total samples ever written (monotonic).
    uint32_t timestamp_ms;           ///< Optional associated timestamp.
} CircularBuffer;

// Core operations
/**
 * @brief Initialize a buffer to the empty state.
 *
 * @param buf Pointer to buffer instance.
 */
void Buffer_Init(CircularBuffer *buf);

/**
 * @brief Append a sample, overwriting the oldest when full.
 *
 * @param buf    Pointer to buffer instance.
 * @param sample New sample value to store.
 */
void Buffer_Write(CircularBuffer *buf, int32_t sample);

/**
 * @brief Read a sample by absolute index [0..count-1], oldest-first.
 *
 * @param buf   Pointer to buffer instance.
 * @param index Index relative to oldest sample (0 = oldest).
 * @return Sample value at the requested index.
 */
int32_t Buffer_Read(const CircularBuffer *buf, uint16_t index);

/**
 * @brief Read a sample "samples_ago" from the newest entry.
 *
 * @param buf         Pointer to buffer instance.
 * @param samples_ago 0 = most recent sample, 1 = previous, etc.
 * @return Sample value at that relative position.
 */
int32_t Buffer_ReadRelative(const CircularBuffer *buf, uint16_t samples_ago);

// Query functions
/**
 * @brief Check whether the buffer is currently full.
 *
 * @param buf Pointer to buffer instance.
 * @return true if count == BUFFER_CAPACITY, false otherwise.
 */
bool Buffer_IsFull(const CircularBuffer *buf);

/**
 * @brief Get the number of valid samples in the buffer.
 *
 * @param buf Pointer to buffer instance.
 * @return Current sample count.
 */
uint16_t Buffer_GetCount(const CircularBuffer *buf);

/**
 * @brief Get the fixed capacity (max number of samples).
 *
 * @return BUFFER_CAPACITY.
 */
uint16_t Buffer_GetCapacity(void);

// Analysis functions
/**
 * @brief Compute the average of the last num_samples entries.
 *
 * @param buf         Pointer to buffer instance.
 * @param num_samples Number of most recent samples to include.
 * @return Arithmetic mean of the requested window.
 */
int32_t Buffer_GetAverage(const CircularBuffer *buf, uint16_t num_samples);

/**
 * @brief Get the maximum of the last num_samples entries.
 *
 * @param buf         Pointer to buffer instance.
 * @param num_samples Number of most recent samples to include.
 * @return Maximum sample value in the requested window.
 */
int32_t Buffer_GetMax(const CircularBuffer *buf, uint16_t num_samples);

/**
 * @brief Get the minimum of the last num_samples entries.
 *
 * @param buf         Pointer to buffer instance.
 * @param num_samples Number of most recent samples to include.
 * @return Minimum sample value in the requested window.
 */
int32_t Buffer_GetMin(const CircularBuffer *buf, uint16_t num_samples);

#endif /* CIRCULAR_BUFFER_H_ */
