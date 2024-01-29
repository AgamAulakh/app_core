#include "DataBufferManager.h"
// log level declaration
LOG_MODULE_REGISTER(data_buffer_manager, LOG_LEVEL_DBG);

struct dma_config DataBufferManager::dma_cfg = {
    .channel_direction = PERIPHERAL_TO_MEMORY,
    .complete_callback_en = 1,
    .source_burst_length = 1, // accept 1 sample at a time
    .dma_callback = DMATransfer,
};

// struct Semaphore DataBufferManager::eeg_buffer_semaphore;
// eeg_sample DataBufferManager::dma_buffer[max_samples];
// size_t DataBufferManager::buffer_index;

void DataBufferManager::Write() {

}

void DataBufferManager::Read() {

}

void DataBufferManager::DMASetup(device* spi_dev) {
    // set up basic source buffer for dma
    dma_block_cfg.dest_address = (uintptr_t)source_buffer;
    dma_block_cfg.source_address = (uintptr_t)&spi_dev->data;

    if (dma_config(dma_dev, AFE_DMA_CHANNEL, &dma_cfg)) {
        LOG_ERR("Failed to configure DMA");
        // TODO: error handling :D
        return;
    }

    if (dma_start(dma_dev, AFE_DMA_CHANNEL)) {
        LOG_ERR("Failed to start DMA transfer");
        // TODO: error handling
    }
}

void DataBufferManager::DMATransfer(const struct device *dev, void *user_data, uint32_t channel, int status) {
    if (status == 0) {
        LOG_DBG("DMA transfer complete success");
        for (size_t i = 0; i < AFE_DMA_BLOCK_SIZE; i++) {
            LOG_INF("Sample[%u]: %d", i, source_buffer[i]);
        }
        ResetBuffer();
        // Perform any post-transfer processing or signal completion
        
    } else {
        LOG_ERR("DMA transfer failed with status %d", status);
        // TODO: error handling
    }

    // in either case, start the DMA again
    if (dma_start(dma_dev, AFE_DMA_CHANNEL)) {
        LOG_ERR("Failed to start DMA transfer");
        // TODO: error handling
    }
}
