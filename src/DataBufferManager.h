#pragma once

#include <zephyr/kernel.h>
#include <zephyr/sys/sem.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/ring_buffer.h>
#include "drivers/ads1299.h"
#include "core/Semaphore.h"
#include "ArmMatrixWrapper.h"
#include "Data.h"

constexpr uint16_t max_samples_ring_buffer = 2048;
constexpr size_t sample_size_B = sizeof(sample_t);
constexpr size_t total_size_ring_buffer_B = max_samples_ring_buffer * sample_size_B;

class DataBufferManager {
// NOTE: needs to be fully static, only one instance should exist
private:
    static struct ring_buf buffer;
    static sample_t data_buffer[max_samples_ring_buffer];
    
    // can't read one sample at a time
    static bool ReadOneSample(sample_t&);
public:
    DataBufferManager();
    ~DataBufferManager() = default;

    static Semaphore buffer_lock;

    // ring functionality
    static bool WriteOneSample(const sample_t&);
    static void ReadEpoch(ArmMatrixWrapper<num_electrodes, num_samples_per_epoch> &mat);
    // helpers
    static bool IsEmpty();
    static size_t GetFreeSpace();
    static size_t GetUsedSpace();
    static void ResetBuffer();
};