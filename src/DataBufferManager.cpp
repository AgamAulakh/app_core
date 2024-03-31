#include <DataBufferManager.h>

// log level declaration
LOG_MODULE_REGISTER(data_buffer_manager, LOG_LEVEL_DBG);

ring_buf DataBufferManager::buffer;
sample_t DataBufferManager::data_buffer[max_samples_ring_buffer] = {0};
Semaphore DataBufferManager::epoch_lock = Semaphore();

DataBufferManager::DataBufferManager(){
    ring_buf_init(&buffer, total_size_ring_buffer_B, reinterpret_cast<uint8_t*>(data_buffer));

    LOG_DBG("DataBufferManager::%s -- TOOK THE SEMAPHORE at ms: %u",
        __FUNCTION__,
        k_uptime_get_32()
    );

    epoch_lock.wait();
}

// ring functionality
bool DataBufferManager::WriteOneSample(const sample_t &sample) {
    bool is_successful = ring_buf_put(&buffer,
        const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(&sample)),
        sample_size_B
    ) == sample_size_B;

    LOG_INF("DataBufferManager::%s wrote new sample", __FUNCTION__);

    if ((GetUsedSpace() / sample_size_B) == num_samples_per_epoch) {
        epoch_lock.give();
    }
    return is_successful;
};

bool DataBufferManager::ReadOneSample(sample_t &sample) {
    return ring_buf_get(&buffer,
        const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(&sample)),
        sample_size_B
    ) == sample_size_B;
};

void DataBufferManager::ReadEpoch(ArmMatrixWrapper<num_samples_per_epoch, num_electrodes> &mat) {
    sample_t to_read = { 0 };
    for(uint32_t i = 0; i < num_samples_per_epoch; i++) {
        if (ReadOneSample(to_read)) {
            mat.set_at(to_read.ch1, i, CH1_IDX);
            mat.set_at(to_read.ch2, i, CH2_IDX);
            mat.set_at(to_read.ch3, i, CH3_IDX);
            mat.set_at(to_read.ch4, i, CH4_IDX);
            mat.set_at(to_read.ch5, i, CH5_IDX);
            mat.set_at(to_read.ch6, i, CH6_IDX);
            mat.set_at(to_read.ch7, i, CH7_IDX);
            mat.set_at(to_read.ch8, i, CH8_IDX);
        }
        else {
            LOG_ERR("DataBufferManager::%s COULD NOT READ REQUESTED SAMPLE", __FUNCTION__);
        }
    }
};

void DataBufferManager::DoneReadingEpoch() {
    epoch_lock.wait();
}

// helpers
bool DataBufferManager::IsEmpty() {
    return ring_buf_is_empty(&buffer);
};
size_t DataBufferManager::GetFreeSpace() {
    return ring_buf_space_get(&buffer);
};
size_t DataBufferManager::GetUsedSpace() {
    return total_size_ring_buffer_B - ring_buf_space_get(&buffer);
};
void DataBufferManager::ResetBuffer() {
    memset(data_buffer, 0, sizeof(data_buffer));
};