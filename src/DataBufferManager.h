#pragma once

#include <zephyr/kernel.h>
#include <zephyr/sys/sem.h>
#include <zephyr/device.h>
#include <zephyr/drivers/dma.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>
#include "Data.h"
#include "core/Semaphore.h"

#define AFE_DMA_CHANNEL 0
#define AFE_DMA_BURST_NUM_SAMPLES 5
#define AFE_DMA_BLOCK_SIZE 256

namespace cool {
    void foo();
};

class DataBufferManager {
// NOTE: needs to be fully static, only one instance should exist
private:
    static struct device* dma_dev;
    static struct dma_config dma_cfg;
    static struct dma_block_config dma_block_cfg;
    // note: each afe sample is 15 bytes, burst size is 5, so max usage is 15 * 5 = 75
    static uint8_t source_buffer[AFE_DMA_BLOCK_SIZE];

    //// TODO: use this instead
    // static Semaphore eeg_buffer_semaphore;
    // static eeg_sample dma_buffer[max_samples];
    // static size_t buffer_index;

public:
    static void Read();
    static void Write();
    static void DMASetup(device* spi_dev);
    static void DMATransfer(const struct device *dev, void *user_data, uint32_t channel, int status);

    // inline function to be used in callback
    static void ResetBuffer() {
        memset(source_buffer, 0, sizeof(source_buffer));
    };
};