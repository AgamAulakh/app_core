#pragma once
// #include <stdlib.h>
#include <cstdint>
#include "drivers/ads1299.h"

// GENERIC
#define CH1_IDX 0
#define CH2_IDX 1
#define CH3_IDX 2
#define CH4_IDX 3
#define CH5_IDX 4
#define CH6_IDX 5
#define CH7_IDX 6
#define CH8_IDX 7

#define DELTA_IDX 0
#define THETA_IDX 1
#define ALPHA_IDX 2
#define BETA_IDX  3

// SIGPROC
#define PROCESSED_SAMPLE_NUMBER 512
#define SAMPLE_FREQ 250 
#define CHANNELS_TEST 1 // Just for testing one channel 
#define BANDS 4

// DAQ
constexpr uint8_t num_electrodes = 8;
constexpr uint8_t max_epochs = 2;
constexpr uint8_t num_bytes_per_sample = 3 * (num_electrodes + 1);
constexpr uint16_t num_samples_per_test = 250;
constexpr uint16_t num_samples_per_epoch = 512;
constexpr uint16_t rx_buf_len = num_bytes_per_sample * num_samples_per_test;

// Data Buffer Manager
constexpr uint16_t max_samples_ring_buffer = 2048;
constexpr size_t sample_size_B = sizeof(sample_t);
constexpr size_t total_size_ring_buffer_B = max_samples_ring_buffer * sample_size_B;

typedef struct {
    float32_t delta;
    float32_t theta;
    float32_t alpha;
    float32_t beta;
} Electrode;

typedef struct {
    Electrode band_powers[num_electrodes];
    // uint64_t timestamp_ms;
} Result;