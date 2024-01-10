#ifndef DATA_BUFFER_MANAGER_H
#define DATA_BUFFER_MANAGER_H

#include <zephyr/kernel.h>
#include <zephyr/sys/sem.h>
#include "Data.h"
#include "core/Semaphore.h"

class DataBufferManager {
// NOTE: needs to be fully static, only one instance should exist
private:
    static Semaphore eeg_buffer_semaphore; // TODO: use this instead
    static eeg_sample dma_buffer[max_samples];
    static size_t buffer_index;

public:
    static void Read();
    static void Write();
    static void spi_dma_setup();
    static void spi_sensor_read_and_transfer();
};

#endif