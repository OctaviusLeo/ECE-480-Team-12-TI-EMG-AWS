#ifndef EMG_MOCK_H
#define EMG_MOCK_H

#include <stdint.h>

typedef struct { int16_t a, b; } emg_sample_t;
void emg_mock_init(uint32_t fs_hz);
void emg_mock_set_level(float levelA, float levelB); 
int  emg_mock_read(emg_sample_t* out); 

#endif
