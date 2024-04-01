#include <DataBufferManager.h>

// log level declaration
LOG_MODULE_REGISTER(data_buffer_manager, LOG_LEVEL_DBG);

ring_buf DataBufferManager::buffer;
sample_t DataBufferManager::data_buffer[max_samples_ring_buffer] = {0};
bool DataBufferManager::is_buffer_initialized = false;

void DataBufferManager::Initialize(){
    if (!is_buffer_initialized) {
        ring_buf_init(&buffer, total_size_ring_buffer_B, reinterpret_cast<uint8_t*>(data_buffer));
        LOG_DBG("DataBufferManager::%s -- Initialized Buffer at ms: %u",
            __FUNCTION__,
            k_uptime_get_32()
        );
        is_buffer_initialized = true;
    }
}

// ring functionality
bool DataBufferManager::WriteOneSample(const sample_t sample) {
    bool is_successful = ring_buf_put(
                            &buffer,
                            reinterpret_cast<const uint8_t*>(&sample),
                            sample_size_B
                        ) == sample_size_B;

    if(!is_successful) {
        LOG_INF("DataBufferManager::%s write status: %u, sample number: %u, used space B: %u",
            __FUNCTION__,
            is_successful,
            (GetUsedSpace() / sample_size_B),
            GetUsedSpace()
        );
    }

    return is_successful;
};

bool DataBufferManager::ReadOneSample(sample_t &sample) {
    bool is_successful = ring_buf_get(
                            &buffer,
                            reinterpret_cast<uint8_t*>(&sample),
                            sample_size_B
                        ) == sample_size_B;
    return is_successful;
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

// helpers
bool DataBufferManager::IsEmpty() {
    return ring_buf_is_empty(&buffer);
};
size_t DataBufferManager::GetFreeSpace() {
    return ring_buf_space_get(&buffer);
};
size_t DataBufferManager::GetUsedSpace() {
    return ring_buf_size_get(&buffer);
};
size_t DataBufferManager::GetNumSaplesWritten() {
    return (GetUsedSpace() / sample_size_B);
};
void DataBufferManager::ResetBuffer() {
    ring_buf_internal_reset(&buffer, 0);
    memset(data_buffer, 0, sizeof(data_buffer));
};