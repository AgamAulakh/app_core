#pragma once
// #include <stdlib.h>
#include <cstdint>

constexpr uint8_t num_bytes_per_sample = 3 * (8 + 1);
constexpr uint16_t default_num_samples = 250;
constexpr uint16_t rx_buf_len = num_bytes_per_sample * default_num_samples;

// UNUSED:
constexpr uint8_t max_electrodes = 6;
constexpr uint8_t max_sample_rate_hz = 250;
constexpr uint32_t max_samples = max_electrodes * max_sample_rate_hz * 30;

struct eeg_sample {
    uint8_t buf[3] = { 0 }; // for max 24 bit adc readings TODO: add units
    uint64_t timestamp_ms = 0;
};
