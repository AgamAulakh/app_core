#ifndef DATA_H
#define DATA_H

// #include <stdlib.h>
#include <cstdint>

constexpr uint8_t max_electrodes = 6;
constexpr uint8_t max_sample_rate_hz = 250;
constexpr uint32_t max_samples = max_electrodes * max_sample_rate_hz * 30;

struct eeg_sample {
    uint8_t buf[3] = { 0 }; // for max 24 bit adc readings TODO: add units
    uint64_t timestamp_ms = 0;
};

#endif