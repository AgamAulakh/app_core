#pragma once
// #include <stdlib.h>
#include <cstdint>

#define CH1_IDX 0
#define CH2_IDX 1
#define CH3_IDX 2
#define CH4_IDX 3
#define CH5_IDX 4
#define CH6_IDX 5
#define CH7_IDX 6
#define CH8_IDX 7

constexpr uint8_t num_electrodes = 8;
constexpr uint8_t max_epochs = 8;
constexpr uint8_t num_bytes_per_sample = 3 * (num_electrodes + 1);
constexpr uint16_t num_samples_per_test = 250;
constexpr uint16_t num_samples_per_epoch = 1024;
constexpr uint16_t default_rx_buf_len = num_bytes_per_sample * num_samples_per_test;

// NOTE: not used
struct eeg_sample {
    uint8_t buf[3] = { 0 }; // for max 24 bit adc readings TODO: add units
    uint64_t timestamp_ms = 0;
};
