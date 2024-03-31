#pragma once

#include <zephyr/kernel.h>
#include <zephyr/sys/sem.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/ring_buffer.h>
#include "core/Semaphore.h"
#include "ArmMatrixWrapper.h"
#include "Data.h"

class DataBufferManager {
// NOTE: needs to be fully static, only one instance should exist
private:
    static struct ring_buf buffer;
    static sample_t data_buffer[max_samples_ring_buffer];

    DataBufferManager();
    ~DataBufferManager() = default;
 
public:
    static Semaphore epoch_lock; // only used by the sigproc thread

    // ring functionality
    static bool WriteOneSample(const sample_t&);
    static bool ReadOneSample(sample_t&);
    static void ReadEpoch(ArmMatrixWrapper<num_samples_per_epoch, num_electrodes> &mat);
    static void DoneReadingEpoch();

    // helpers
    static bool IsEmpty();
    static size_t GetFreeSpace();
    static size_t GetUsedSpace();
    static void ResetBuffer();
};